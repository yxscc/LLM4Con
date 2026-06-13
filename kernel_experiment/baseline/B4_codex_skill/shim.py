#!/usr/bin/env python3
"""OpenAI Responses  <->  Chat-Completions  <->  ByteDance gateway shim.

Codex CLI (>=0.137) speaks ONLY the OpenAI *Responses* API and always
streams. Our internal gateway is a single fixed endpoint that speaks
*Chat Completions* (non-streaming):

    https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=<AK>

This shim lets Codex run on that gateway (so the B4 baseline shares the
exact GPT-5.5 backbone as B1/B2/B3). It:

  POST /v1/responses
    * Translates the Responses request -> Chat Completions request
      (instructions -> system msg; input items -> messages; function-call /
       function-call-output history -> assistant tool_calls / tool msgs,
       grouping consecutive calls; Responses function tools -> chat tools;
       non-function tools [custom/web_search/tool_search] dropped).
    * Calls the gateway (non-streaming, sanitised to llm_client's shape).
    * Re-emits the single chat completion as a Responses SSE event stream
      (response.created -> output_item.added/done [+ text/args deltas] ->
       response.completed), which Codex consumes.

  POST /v1/chat/completions  passthrough (kept for direct testing).
  GET  /v1/models            one-entry list for client preflight.

Env:
  GW_URL / LLM_BASE_URL   gateway URL incl. ?ak=
  GW_KEY / LLM_API_KEY     real AK for Bearer header
  SHIM_PORT (8799), SHIM_LOG (trace file), LLM_MODEL, SHIM_TIMEOUT
"""
from __future__ import annotations

import json
import os
import sys
import time
import urllib.request
import urllib.error
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

GW_URL = os.environ.get("GW_URL") or os.environ.get("LLM_BASE_URL") or ""
GW_KEY = (
    os.environ.get("GW_KEY")
    or os.environ.get("LLM_API_KEY")
    or os.environ.get("API_KEY")
    or ""
)
PORT = int(os.environ.get("SHIM_PORT", "8799"))
LOG = os.environ.get("SHIM_LOG", "")
MODEL = os.environ.get("LLM_MODEL", "gpt-5.5-2026-04-24")
TIMEOUT = int(os.environ.get("SHIM_TIMEOUT", "600"))


def _log(*a):
    if not LOG:
        return
    try:
        with open(LOG, "a") as f:
            f.write(" ".join(str(x) for x in a) + "\n")
    except Exception:
        pass


# --------------------------------------------------------------------------
# content / payload helpers
# --------------------------------------------------------------------------
def _flatten_content(content):
    if content is None or isinstance(content, str):
        return content
    if isinstance(content, list):
        parts = []
        for p in content:
            if isinstance(p, str):
                parts.append(p)
            elif isinstance(p, dict):
                parts.append(
                    p.get("text")
                    or p.get("input_text")
                    or p.get("output_text")
                    or ""
                )
        return "".join(parts)
    return str(content)


def _gpt5(model: str) -> bool:
    m = (model or "").lower()
    return m.startswith("gpt-5") or m.startswith("gpt5")


def _sanitize_chat(body: dict) -> dict:
    """Reduce an OpenAI chat body to the gateway's known-good fields."""
    out: dict = {"model": body.get("model") or MODEL}
    msgs = []
    for m in body.get("messages", []):
        if not isinstance(m, dict):
            continue
        role = m.get("role", "user")
        if role == "developer":
            role = "system"
        nm = {"role": role}
        c = _flatten_content(m.get("content"))
        if m.get("tool_calls"):
            nm["tool_calls"] = m["tool_calls"]
        if m.get("tool_call_id"):
            nm["tool_call_id"] = m["tool_call_id"]
        if m.get("name"):
            nm["name"] = m["name"]
        nm["content"] = c if c is not None else ""
        msgs.append(nm)
    out["messages"] = msgs
    mt = body.get("max_tokens") or body.get("max_completion_tokens") or 8000
    try:
        out["max_tokens"] = int(mt)
    except Exception:
        out["max_tokens"] = 8000
    if not _gpt5(out["model"]) and body.get("temperature") is not None:
        out["temperature"] = body["temperature"]
    for k in ("tools", "tool_choice", "response_format"):
        if body.get(k) is not None:
            out[k] = body[k]
    return out


def _call_gateway(payload: dict):
    body = json.dumps(payload).encode("utf-8")
    headers = {"Content-Type": "application/json",
               "Authorization": f"Bearer {GW_KEY}"}
    req = urllib.request.Request(GW_URL, data=body, headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
        return json.loads(resp.read().decode("utf-8", "replace"))


# --------------------------------------------------------------------------
# Responses request  ->  Chat Completions request
# --------------------------------------------------------------------------
def _responses_to_chat(req: dict) -> dict:
    messages = []
    instr = req.get("instructions")
    if instr:
        messages.append({"role": "system", "content": instr})

    pending_calls = []  # consecutive function_call items -> one assistant msg

    def flush_calls():
        if pending_calls:
            messages.append({
                "role": "assistant",
                "content": "",
                "tool_calls": [dict(tc) for tc in pending_calls],
            })
            pending_calls.clear()

    for item in req.get("input", []):
        if not isinstance(item, dict):
            continue
        it = item.get("type")
        if it == "function_call":
            pending_calls.append({
                "id": item.get("call_id") or item.get("id"),
                "type": "function",
                "function": {
                    "name": item.get("name", ""),
                    "arguments": item.get("arguments", "") or "",
                },
            })
        elif it == "function_call_output":
            flush_calls()
            out = item.get("output")
            if isinstance(out, (dict, list)):
                out = json.dumps(out)
            messages.append({
                "role": "tool",
                "tool_call_id": item.get("call_id") or item.get("id"),
                "content": _flatten_content(out) if out is not None else "",
            })
        elif it == "message":
            flush_calls()
            role = item.get("role", "user")
            if role == "developer":
                role = "system"
            messages.append({
                "role": role,
                "content": _flatten_content(item.get("content")) or "",
            })
        elif it == "reasoning":
            # No chat-completions analogue; drop (history continuity only).
            continue
        else:
            # Unknown item types (e.g. custom tool output) -> best-effort text.
            flush_calls()
            txt = _flatten_content(item.get("content"))
            if txt:
                messages.append({"role": "assistant", "content": txt})
    flush_calls()

    # Tools: keep only standard function tools; convert to chat nesting.
    chat_tools = []
    for t in req.get("tools", []) or []:
        if isinstance(t, dict) and t.get("type") == "function" and t.get("name"):
            chat_tools.append({
                "type": "function",
                "function": {
                    "name": t["name"],
                    "description": t.get("description", ""),
                    "parameters": t.get("parameters", {"type": "object", "properties": {}}),
                },
            })

    chat: dict = {"model": req.get("model") or MODEL, "messages": messages}
    if chat_tools:
        chat["tools"] = chat_tools
        tc = req.get("tool_choice")
        chat["tool_choice"] = tc if tc in ("auto", "none", "required") else "auto"
    chat["max_tokens"] = 16000
    return _sanitize_chat(chat)


# --------------------------------------------------------------------------
# Chat completion  ->  Responses SSE events
# --------------------------------------------------------------------------
def _sse(event_type: str, data: dict, seq: list) -> str:
    data = dict(data)
    data["type"] = event_type
    data["sequence_number"] = seq[0]
    seq[0] += 1
    return f"event: {event_type}\ndata: {json.dumps(data)}\n\n"


def _completion_to_responses_sse(completion: dict) -> bytes:
    choice = (completion.get("choices") or [{}])[0]
    msg = choice.get("message") or {}
    rid = completion.get("id", "resp_shim")
    created = completion.get("created", int(time.time()))
    model = completion.get("model", MODEL)

    items = []
    text = msg.get("content") or ""
    if text:
        items.append({
            "type": "message",
            "id": "msg_0",
            "status": "completed",
            "role": "assistant",
            "content": [{"type": "output_text", "text": text, "annotations": []}],
        })
    for i, tc in enumerate(msg.get("tool_calls") or []):
        fn = tc.get("function", {})
        items.append({
            "type": "function_call",
            "id": f"fc_{i}",
            "call_id": tc.get("id", f"call_{i}"),
            "name": fn.get("name", ""),
            "arguments": fn.get("arguments", "") or "",
            "status": "completed",
        })

    usage_in = completion.get("usage") or {}
    usage = {
        "input_tokens": usage_in.get("prompt_tokens", 0),
        "output_tokens": usage_in.get("completion_tokens", 0),
        "total_tokens": usage_in.get("total_tokens", 0),
    }

    def resp_obj(status, output):
        return {
            "id": rid, "object": "response", "created_at": created,
            "status": status, "model": model, "output": output,
            "usage": usage if status == "completed" else None,
        }

    seq = [0]
    buf = [_sse("response.created", {"response": resp_obj("in_progress", [])}, seq)]
    for idx, item in enumerate(items):
        buf.append(_sse("response.output_item.added",
                        {"output_index": idx, "item": {**item, "status": "in_progress"}}, seq))
        if item["type"] == "message":
            t = item["content"][0]["text"]
            buf.append(_sse("response.content_part.added",
                            {"item_id": item["id"], "output_index": idx, "content_index": 0,
                             "part": {"type": "output_text", "text": "", "annotations": []}}, seq))
            buf.append(_sse("response.output_text.delta",
                            {"item_id": item["id"], "output_index": idx, "content_index": 0,
                             "delta": t}, seq))
            buf.append(_sse("response.output_text.done",
                            {"item_id": item["id"], "output_index": idx, "content_index": 0,
                             "text": t}, seq))
            buf.append(_sse("response.content_part.done",
                            {"item_id": item["id"], "output_index": idx, "content_index": 0,
                             "part": {"type": "output_text", "text": t, "annotations": []}}, seq))
        elif item["type"] == "function_call":
            buf.append(_sse("response.function_call_arguments.delta",
                            {"item_id": item["id"], "output_index": idx,
                             "delta": item["arguments"]}, seq))
            buf.append(_sse("response.function_call_arguments.done",
                            {"item_id": item["id"], "output_index": idx,
                             "arguments": item["arguments"]}, seq))
        buf.append(_sse("response.output_item.done",
                        {"output_index": idx, "item": item}, seq))
    buf.append(_sse("response.completed", {"response": resp_obj("completed", items)}, seq))
    return "".join(buf).encode("utf-8")


# --------------------------------------------------------------------------
# HTTP server
# --------------------------------------------------------------------------
class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *a):
        pass

    def _send_json(self, obj, code=200):
        data = json.dumps(obj).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _send_sse(self, data: bytes):
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path.rstrip("/").endswith("/models"):
            self._send_json({"object": "list",
                             "data": [{"id": MODEL, "object": "model", "owned_by": "gateway"}]})
            return
        self._send_json({"error": {"message": f"no route {self.path}"}}, 404)

    def _read_body(self):
        n = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(n) if n else b"{}"
        return json.loads(raw.decode("utf-8", "replace"))

    def do_POST(self):
        is_responses = self.path.endswith("/responses")
        is_chat = self.path.endswith("/chat/completions")
        if not (is_responses or is_chat):
            self._send_json({"error": {"message": f"no route {self.path}"}}, 404)
            return
        try:
            body = self._read_body()
        except Exception as e:
            self._send_json({"error": {"message": f"bad request: {e}"}}, 400)
            return

        if is_responses:
            chat_payload = _responses_to_chat(body)
            want_stream = True  # Codex always streams
        else:
            want_stream = bool(body.pop("stream", False))
            chat_payload = _sanitize_chat(body)

        _log("REQ", time.strftime("%H:%M:%S"),
             "api=%s" % ("responses" if is_responses else "chat"),
             "msgs=%d" % len(chat_payload.get("messages", [])),
             "tools=%d" % len(chat_payload.get("tools", []) or []))

        try:
            completion = _call_gateway(chat_payload)
        except urllib.error.HTTPError as e:
            try:
                eb = e.read().decode("utf-8", "replace")
            except Exception:
                eb = ""
            _log("GW_HTTP_ERR", e.code, eb[:600])
            self._send_json({"error": {"message": f"gateway {e.code}: {eb[:400]}"}}, 502)
            return
        except Exception as e:
            _log("GW_ERR", repr(e))
            self._send_json({"error": {"message": f"gateway error: {e}"}}, 502)
            return

        if "choices" not in completion:
            _log("GW_BADSHAPE", json.dumps(completion)[:600])
            self._send_json({"error": {"message": "gateway returned no choices"}}, 502)
            return

        ch = (completion.get("choices") or [{}])[0]
        _log("RESP", "finish=%s" % ch.get("finish_reason"),
             "tools=%s" % bool((ch.get("message") or {}).get("tool_calls")))

        if is_responses:
            self._send_sse(_completion_to_responses_sse(completion))
        elif want_stream:
            # minimal chat SSE (kept for direct chat testing)
            msg = ch.get("message", {})
            delta = {"role": "assistant"}
            if msg.get("content") is not None:
                delta["content"] = msg["content"]
            if msg.get("tool_calls"):
                delta["tool_calls"] = [{**t, "index": i} for i, t in enumerate(msg["tool_calls"])]
            c1 = {"id": completion.get("id"), "object": "chat.completion.chunk",
                  "choices": [{"index": 0, "delta": delta, "finish_reason": None}]}
            c2 = {"id": completion.get("id"), "object": "chat.completion.chunk",
                  "choices": [{"index": 0, "delta": {}, "finish_reason": ch.get("finish_reason", "stop")}]}
            self._send_sse((f"data: {json.dumps(c1)}\n\n"
                            f"data: {json.dumps(c2)}\n\ndata: [DONE]\n\n").encode())
        else:
            self._send_json(completion)


def main():
    if not GW_URL or not GW_KEY:
        print("[shim] FATAL: GW_URL / GW_KEY not set (source setup_env.sh)", file=sys.stderr)
        sys.exit(2)
    srv = ThreadingHTTPServer(("127.0.0.1", PORT), Handler)
    print(f"[shim] listening on http://127.0.0.1:{PORT}/v1 (responses+chat) "
          f"-> {GW_URL.split('?')[0]}?ak=...", flush=True)
    srv.serve_forever()


if __name__ == "__main__":
    main()
