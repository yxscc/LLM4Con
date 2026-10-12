"""Official OpenAI/Agents SDK runtime for the C++ static-analysis tool service.

One Runner invocation per request. Native SDK compaction manages model context;
the original history remains available for auditing. stdout is reserved for IPC.
"""
import asyncio
import json
import os
import re
import sys
from urllib.parse import urlsplit, urlunsplit, parse_qs


def base_url(endpoint):
    parts = urlsplit(endpoint)
    path = parts.path.rstrip("/")
    for suffix in ("/chat/completions", "/responses"):
        if path.endswith(suffix):
            path = path[:-len(suffix)]
            break
    # OpenAI joins resource paths to base_url.raw_path. Leaving a gateway's
    # authentication query here appends '/chat/completions' inside its value.
    # Supply queries separately through the SDK's default_query instead.
    return urlunsplit((parts.scheme, parts.netloc, path + "/", "", ""))


def endpoint_query(endpoint):
    return {key: values[0] if len(values) == 1 else values
            for key, values in parse_qs(urlsplit(endpoint).query, keep_blank_values=True).items()}


def endpoint_mode(endpoint):
    mode = os.getenv("LACE_ENDPOINT_MODE", "auto")
    if mode not in ("auto", "root", "exact"):
        raise ValueError("LACE_ENDPOINT_MODE must be auto, root or exact")
    if mode != "auto":
        return mode
    path = urlsplit(endpoint).path.rstrip("/")
    if not path or re.search(r"/v\d+$", path) or path.endswith(("/chat/completions", "/responses")):
        return "root"
    # v2 accepted a FULL arbitrary Chat Completions endpoint, including gateways
    # such as /.../crawl. Preserve that contract without implementing HTTP/SDK
    # serialization ourselves. Explicit root mode handles unusual API roots.
    return "exact"


def emit(value):
    print(json.dumps(value, ensure_ascii=False), flush=True)


def context_management(model_api, model):
    """Use the SDK capability; never emulate compaction by dropping old messages."""
    mode = os.getenv('LACE_CONTEXT_MANAGEMENT', 'auto')
    if mode not in ('auto', 'native', 'off'):
        raise ValueError('LACE_CONTEXT_MANAGEMENT must be auto, native or off')
    if mode == 'off' or (mode == 'auto' and model_api == 'chat_completions'):
        return None, {'requested': mode, 'enabled': False,
                      'reason': 'explicitly_disabled' if mode == 'off' else 'responses_api_required'}
    if model_api != 'responses':
        raise ValueError('SDK native compaction requires LACE_MODEL_API=responses and a Responses-compatible endpoint')
    from agents.sandbox.capabilities import Compaction, StaticCompactionPolicy
    threshold = os.getenv('LACE_COMPACT_THRESHOLD')
    if threshold is not None and int(threshold) <= 0:
        raise ValueError('LACE_COMPACT_THRESHOLD must be positive')
    capability = Compaction(policy=StaticCompactionPolicy(threshold=int(threshold))) if threshold else Compaction()
    settings = capability.sampling_params({'model': model})
    return capability, {'requested': mode, 'enabled': True, 'implementation': 'agents.sandbox.Compaction',
                        **settings}


async def run(request):
    if request['action'] == 'repository':
        from repository_context import handle
        return {'event': 'result', 'data': handle(request)}
    from openai import AsyncOpenAI, DefaultAsyncHttpxClient
    import httpx2

    async def use_exact_endpoint(http_request):
        target = httpx2.URL(request["base_url"])
        http_request.url = target.copy_with(query=http_request.url.query)

    hooks = {"request": [use_exact_endpoint]} if endpoint_mode(request["base_url"]) == "exact" else None
    http_client = DefaultAsyncHttpxClient(event_hooks=hooks)
    client = AsyncOpenAI(api_key=request["api_key"], base_url=base_url(request["base_url"]),
                         default_query=endpoint_query(request["base_url"]),
                         timeout=request.get("timeout", 180), max_retries=2, http_client=http_client)
    async with client:
        if request["action"] == "chat":
            response = await client.chat.completions.create(**request["body"])
            return {"event": "result", "response": response.model_dump(mode="json")}

        from agents import (Agent, Runner, RunConfig, ModelSettings, FunctionTool,
                            OpenAIChatCompletionsModel, OpenAIResponsesModel)
        from agents.agent import ToolsToFinalOutputResult
        from openai.types.shared import Reasoning

        tool_finished = False

        def make_tool(spec):
            async def invoke(context, arguments):
                nonlocal tool_finished
                emit({"event": "tool_call", "name": spec["name"], "arguments": json.loads(arguments)})
                line = sys.stdin.readline()
                if not line:
                    raise RuntimeError("Static tool service disconnected")
                output = json.loads(line)["output"]
                tool_finished = tool_finished or output == "finish"
                return output
            return FunctionTool(name=spec["name"], description=spec["description"],
                                params_json_schema=spec["parameters"], on_invoke_tool=invoke,
                                strict_json_schema=False)

        async def finished(context, results):
            return ToolsToFinalOutputResult(is_final_output=tool_finished,
                                            final_output="finish" if tool_finished else None)

        model_api = os.getenv('LACE_MODEL_API', 'chat_completions')
        if model_api not in ('chat_completions', 'responses'):
            raise ValueError('LACE_MODEL_API must be chat_completions or responses')
        compaction, context_config = context_management(model_api, request['model'])
        native_tools, source_session, source_config = [], None, None
        if request.get('source_root') or request.get('contract_path'):
            if not request.get('source_root') or not request.get('contract_path'):
                raise ValueError('SDK source tools require source_root and contract_path together')
            from source_workspace import source_tools
            native_tools, source_session, source_config = await source_tools(request)
        model_class = (OpenAIResponsesModel if model_api == "responses"
                       else OpenAIChatCompletionsModel)
        reasoning = os.getenv("LACE_REASONING_EFFORT")
        agent = Agent(name="LACE", instructions=request["instructions"],
                      model=model_class(model=request["model"], openai_client=client),
                      model_settings=ModelSettings(parallel_tool_calls=False,
                          reasoning=Reasoning(effort=reasoning) if reasoning else None,
                          store=False if model_api == 'responses' else None,
                          extra_args=compaction.sampling_params({'model': request['model']}) if compaction else None,
                          tool_choice=request.get("tool_choice", "auto") if request["tools"] else None),
                      tools=[make_tool(t) for t in request["tools"]]+native_tools, tool_use_behavior=finished,
                      # SDK defaults reset tool_choice after the first call.
                      # Structured workflows must retain it until their validated
                      # finish tool succeeds (or the explicit budget is hit).
                      reset_tool_choice=request.get("tool_choice", "auto") != "required")
        history = list(request.get("history", []))
        history.append({"role": "user", "content": request["input"]})
        # None means no turn cap; a user-selected cap raises MaxTurnsExceeded,
        # which the parent records as an incomplete run, never a clean result.
        turns = request.get("max_turns") or None
        # The Runner owns the loop. Its input hook delegates context reduction
        # to the SDK's opaque compaction item handling; accepted contracts remain
        # in the C++ ledger, never reconstructed from a compressed summary.
        def compact_input(data):
            from agents.run_config import ModelInputData
            return ModelInputData(input=compaction.process_context(data.model_data.input),
                                  instructions=data.model_data.instructions)
        try:
            result = await Runner.run(agent, history, max_turns=turns,
                                      run_config=RunConfig(tracing_disabled=True,
                                          call_model_input_filter=compact_input if compaction else None))
        except BaseException:
            if source_session is not None:
                try:
                    await source_session.aclose()
                except Exception as cleanup_error:
                    print('SDK source cleanup also failed: '+type(cleanup_error).__name__, file=sys.stderr)
            raise
        else:
            if source_session is not None:
                await source_session.aclose()
        usage = result.context_wrapper.usage
        return {"event": "result", "history": result.to_input_list(),
                "output": result.final_output, "tool_finished": tool_finished,
                "context_management": context_config,
                "source_tools": source_config,
                "usage": {"requests": usage.requests, "input_tokens": usage.input_tokens,
                          "output_tokens": usage.output_tokens}}


def main():
    request = {}
    try:
        request = json.loads(sys.stdin.readline())
        emit(asyncio.run(run(request)))
    except Exception as exc:
        message = str(exc)
        if request.get("api_key"):
            message = message.replace(request["api_key"], "<redacted>")
        for values in parse_qs(urlsplit(request.get("base_url", "")).query).values():
            for value in values:
                if value:
                    message = message.replace(value, "<redacted>")
        emit({"event": "error", "error_type": type(exc).__name__, "message": message})
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
