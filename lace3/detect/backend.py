"""Model backends for one detection task.

AgentsBackend runs an openai-agents session against the Chat Completions
gateway (Azure-style endpoint from lace3.config.llm_settings) with the fact
queries as tools. The session ends when `submit_verdicts` accepts a
well-formed submission; a malformed one is returned to the model as an error
so it can resubmit.

ScriptedBackend calls a Python function instead of a model. Offline tests
use it to drive the same packets, tools and verdict handling.
"""

import asyncio
import json
import time
from dataclasses import dataclass, field

from lace3.detect.prompt import SYSTEM, USER, USER_DIRECT, USER_EVIDENCE
from lace3.detect.verdicts import submission_problems


@dataclass
class Review:
    submission: dict | None
    status: str                       # submitted | max_turns | no_submission | error
    error: str = ""
    usage: dict = field(default_factory=dict)
    seconds: float = 0.0
    attempts: int = 1


def user_message(packet, clique, stage="evidence", focus=None, prior=""):
    focus = list(focus or clique.items)
    if stage == "direct":
        return USER_DIRECT.format(packet=packet.text, n_items=len(focus),
                                  item_ids=", ".join(focus))
    if prior:
        return USER_EVIDENCE.format(packet=packet.text, item_ids=", ".join(focus), prior=prior)
    return USER.format(packet=packet.text, n_items=len(focus), item_ids=", ".join(focus))


class ScriptedBackend:
    name = "scripted"

    def __init__(self, fn):
        self.fn = fn

    def review(self, packet, facts, clique, stage="evidence", focus=None, prior=""):
        t = time.time()
        facts.stage, facts.focus = stage, list(focus or clique.items)
        try:
            sub = self.fn(packet, facts, clique)
        except Exception as e:          # a broken script is a task error, not a crash
            return Review(None, "error", f"{type(e).__name__}: {e}", seconds=time.time() - t)
        if sub is None:
            return Review(None, "max_turns", "script gave no submission", seconds=time.time() - t)
        probs = submission_problems(sub)
        if probs:
            return Review(None, "error", "; ".join(probs), seconds=time.time() - t)
        return Review(sub, "submitted", seconds=time.time() - t)


class AgentsBackend:
    name = "agents"

    def __init__(self, settings, max_turns=12, retries=2, request_timeout=300):
        from agents import set_tracing_disabled
        from openai import AsyncAzureOpenAI
        set_tracing_disabled(True)
        # One loop for the backend's lifetime: the client's connection pool is
        # bound to the loop it first ran on.
        self.loop = asyncio.new_event_loop()
        self.settings = settings
        self.max_turns = max_turns
        self.retries = retries
        self.client = AsyncAzureOpenAI(api_key=settings.api_key,
                                       azure_endpoint=settings.azure_endpoint,
                                       api_version=settings.api_version,
                                       timeout=request_timeout, max_retries=2)

    def _agent(self, facts, holder, tools=True):
        from agents import (Agent, ModelSettings, OpenAIChatCompletionsModel,
                            ToolsToFinalOutputResult, function_tool)

        @function_tool
        def read_source(path: str, start_line: int, end_line: int) -> str:
            """Read lines of a source file (path as printed in the task, at most 120 lines)."""
            return facts.read_source(path, start_line, end_line)

        @function_tool
        def grep_source(pattern: str, path_filter: str) -> str:
            """Regex search over the source tree; path_filter is a substring or glob, or ""."""
            return facts.grep_source(pattern, path_filter)

        @function_tool
        def function_ops(function: str) -> str:
            """The indexed operations (accesses, locks, calls) of one function."""
            return facts.function_ops(function)

        @function_tool
        def callers(function: str) -> str:
            """Direct callers, function-pointer stores and ops-table slots of a function."""
            return facts.callers(function)

        @function_tool
        def accesses(key: str) -> str:
            """Every step on a key (e.g. "obj.state", or "*obj.buf" for the pointee) in all entries."""
            return facts.accesses(key)

        @function_tool
        def entry_info(entry: str) -> str:
            """Provenance, reentrancy and cut links of an activation (A<n> or function name)."""
            return facts.entry_info(entry)

        @function_tool
        def steps(activation: str, first: int, last: int) -> str:
            """Steps first..last of an activation's operation sequence."""
            return facts.steps(activation, first, last)

        @function_tool
        def item_contexts(item: str) -> str:
            """All recorded (activation, activation) contexts of an item."""
            return facts.item_contexts(item)

        @function_tool
        def expand(activation: str, reason: str) -> str:
            """Add another activation (A<n> or entry function) to this task and show its steps."""
            return facts.expand(activation, reason)

        @function_tool
        def submit_verdicts(submission_json: str) -> str:
            """Submit the final JSON object with `items` and `findings` (see instructions)."""
            try:
                sub = json.loads(submission_json)
            except json.JSONDecodeError as e:
                return f"error: not valid JSON ({e}); resubmit"
            probs = submission_problems(sub)
            if probs:
                return "error: " + "; ".join(probs) + "; resubmit"
            missing = [i for i in facts.focus
                       if i not in {str(e.get("id")) for e in sub["items"]}]
            holder["submission"] = sub
            return "accepted" + (f" (no verdict for {', '.join(missing)}: recorded incomplete)"
                                 if missing else "")

        def stop(ctx, results):
            for r in results:
                if r.tool.name == "submit_verdicts" and str(r.output).startswith("accepted"):
                    return ToolsToFinalOutputResult(is_final_output=True,
                                                    final_output=holder["submission"])
            return ToolsToFinalOutputResult(is_final_output=False)

        model = OpenAIChatCompletionsModel(model=self.settings.model, openai_client=self.client)
        # Every turn must call a tool, so the session can only end through
        # submit_verdicts (or max_turns); the SDK would otherwise reset
        # tool_choice after the first call and accept a prose answer.
        toolset = ([read_source, grep_source, function_ops, callers, accesses, entry_info,
                    steps, item_contexts, expand, submit_verdicts] if tools
                   else [submit_verdicts])
        return Agent(name="lace3-detect", instructions=SYSTEM, model=model, tools=toolset,
                     tool_use_behavior=stop, reset_tool_choice=False,
                     model_settings=ModelSettings(tool_choice="required"))

    def review(self, packet, facts, clique, stage="evidence", focus=None, prior=""):
        from agents import MaxTurnsExceeded, Runner
        facts.stage, facts.focus = stage, list(focus or clique.items)
        msg = user_message(packet, clique, stage, facts.focus, prior)
        turns = 3 if stage == "direct" else self.max_turns
        t, last = time.time(), ""
        for attempt in range(1, self.retries + 2):
            holder = {}
            try:
                res = self.loop.run_until_complete(
                    Runner.run(self._agent(facts, holder, tools=stage != "direct"), msg,
                               max_turns=turns))
                sub = holder.get("submission")
                if sub is None and isinstance(res.final_output, str):
                    sub = _json_object(res.final_output)
                    if sub is not None and submission_problems(sub):
                        sub = None
                if sub is None:
                    return Review(None, "no_submission",
                                  "session ended without submit_verdicts: "
                                  + str(res.final_output)[:500], usage=_usage(res),
                                  seconds=time.time() - t, attempts=attempt)
                return Review(sub, "submitted", usage=_usage(res),
                              seconds=time.time() - t, attempts=attempt)
            except MaxTurnsExceeded as e:
                sub = holder.get("submission")
                return Review(sub, "max_turns", f"max_turns {turns}: {e}",
                              seconds=time.time() - t, attempts=attempt)
            except Exception as e:
                last = f"{type(e).__name__}: {e}"
                if not _transient(e) or attempt > self.retries:
                    break
                time.sleep(5 * attempt)
        return Review(None, "error", last[:2000], seconds=time.time() - t, attempts=attempt)


def _usage(res):
    u = getattr(getattr(res, "context_wrapper", None), "usage", None)
    if u is None:
        return {}
    return {"requests": u.requests, "input_tokens": u.input_tokens,
            "output_tokens": u.output_tokens, "total_tokens": u.total_tokens}


def _json_object(text):
    i, j = text.find("{"), text.rfind("}")
    if i < 0 or j <= i:
        return None
    try:
        v = json.loads(text[i:j + 1])
    except json.JSONDecodeError:
        return None
    return v if isinstance(v, dict) else None


def _transient(e):
    name = type(e).__name__
    return name in ("APIConnectionError", "APITimeoutError", "RateLimitError",
                    "InternalServerError") or "timeout" in str(e).lower()
