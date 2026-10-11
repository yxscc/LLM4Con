"""Official OpenAI/Agents SDK runtime for the C++ static-analysis tool service.

One Runner invocation per request. No handwritten model/tool loop and no history
truncation. stdout is reserved for IPC; credentials are never persisted.
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

        model_class = (OpenAIResponsesModel if os.getenv("LACE_MODEL_API", "chat_completions") == "responses"
                       else OpenAIChatCompletionsModel)
        reasoning = os.getenv("LACE_REASONING_EFFORT")
        agent = Agent(name="LACE", instructions=request["instructions"],
                      model=model_class(model=request["model"], openai_client=client),
                      model_settings=ModelSettings(parallel_tool_calls=False,
                          reasoning=Reasoning(effort=reasoning) if reasoning else None,
                          tool_choice=request.get("tool_choice", "auto") if request["tools"] else None),
                      tools=[make_tool(t) for t in request["tools"]], tool_use_behavior=finished)
        history = list(request.get("history", []))
        history.append({"role": "user", "content": request["input"]})
        # None means no turn cap; a user-selected cap raises MaxTurnsExceeded,
        # which the parent records as an incomplete run, never a clean result.
        turns = request.get("max_turns") or None
        result = await Runner.run(agent, history, max_turns=turns,
                                  run_config=RunConfig(tracing_disabled=True))
        usage = result.context_wrapper.usage
        return {"event": "result", "history": result.to_input_list(),
                "output": result.final_output, "tool_finished": tool_finished,
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
