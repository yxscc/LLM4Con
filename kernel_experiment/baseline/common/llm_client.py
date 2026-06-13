"""Thin OpenAI-compatible HTTP client targeting the ByteDance internal
GPT gateway (search.bytedance.net).

Design notes
------------
* The endpoint used by Lace (`scripts/batch_detect.sh`) is
  `https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=$AK`
  and the same URL is reused here. The gateway accepts both AK in the
  query string AND a Bearer header; we send both to match Lace.
* `gpt-5.x` reasoning models reject `temperature` != 1, so the field
  is omitted for them (same logic as scripts/evaluate_recall.py:286).
* Retries are explicit (no SDK) so the request shape is auditable and
  no extra deps are required (only stdlib).
"""
from __future__ import annotations

import json
import os
import re
import time
import urllib.error
import urllib.request
from dataclasses import dataclass
from typing import Any, Dict, List, Optional


_DEFAULT_BASE_URL = os.environ.get(
    "LLM_BASE_URL",
    "https://search.bytedance.net/gpt/openapi/online/v2/crawl"
    "?ak=${LLM_API_KEY}",
)


def _resolve_endpoint(base_url: str) -> str:
    """Replicate scripts/evaluate_recall.py:294-299 logic so endpoint
    handling is identical to Lace's judge."""
    if "?" in base_url or base_url.rstrip("/").endswith(
            ("/chat/completions", "/crawl", "/completions")):
        return base_url
    return f"{base_url.rstrip('/')}/chat/completions"


@dataclass
class ChatResult:
    text: str
    raw: Dict[str, Any]
    model: str
    elapsed_s: float
    error: Optional[str] = None
    tool_calls: Optional[List[Dict[str, Any]]] = None


class LLMClient:
    def __init__(
        self,
        api_key: Optional[str] = None,
        model: Optional[str] = None,
        base_url: Optional[str] = None,
        max_tokens: int = 8000,
        timeout_s: int = 300,
        retries: int = 2,
    ) -> None:
        self.api_key = (
            api_key
            or os.environ.get("LLM_API_KEY")
            or os.environ.get("API_KEY")
            or ""
        )
        if not self.api_key:
            raise RuntimeError(
                "LLM_API_KEY / API_KEY not set; cannot construct LLMClient."
            )
        self.model = model or os.environ.get(
            "LLM_MODEL", "gpt-5.5-2026-04-24"
        )
        self.base_url = (base_url or _DEFAULT_BASE_URL).replace(
            "${LLM_API_KEY}", self.api_key
        )
        self.endpoint = _resolve_endpoint(self.base_url)
        self.max_tokens = max_tokens
        self.timeout_s = timeout_s
        self.retries = retries

    def chat(
        self,
        messages: List[Dict[str, Any]],
        *,
        max_tokens: Optional[int] = None,
        temperature: Optional[float] = None,
        tools: Optional[List[Dict[str, Any]]] = None,
        tool_choice: Any = None,
        response_format: Optional[Dict[str, Any]] = None,
    ) -> ChatResult:
        payload: Dict[str, Any] = {
            "model": self.model,
            "messages": messages,
            "max_tokens": max_tokens or self.max_tokens,
        }
        # GPT-5* reasoning models reject any temperature != 1; skip it.
        if not re.match(r"^gpt-?5", self.model, re.IGNORECASE):
            payload["temperature"] = 0.1 if temperature is None else temperature
        if tools:
            payload["tools"] = tools
            if tool_choice is not None:
                payload["tool_choice"] = tool_choice
        if response_format:
            payload["response_format"] = response_format

        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {self.api_key}",
        }
        body = json.dumps(payload).encode("utf-8")
        start = time.time()
        last_err: Optional[str] = None
        for attempt in range(self.retries + 1):
            try:
                req = urllib.request.Request(
                    self.endpoint, data=body, headers=headers, method="POST"
                )
                with urllib.request.urlopen(req, timeout=self.timeout_s) as resp:
                    raw = json.loads(resp.read().decode("utf-8", "replace"))
                msg = raw["choices"][0]["message"]
                text = (msg.get("content") or "").strip()
                tool_calls = msg.get("tool_calls")
                return ChatResult(
                    text=text,
                    raw=raw,
                    model=self.model,
                    elapsed_s=time.time() - start,
                    tool_calls=tool_calls,
                )
            except urllib.error.HTTPError as e:
                try:
                    err_body = e.read().decode("utf-8", "replace")
                except Exception:
                    err_body = ""
                last_err = f"HTTP {e.code}: {err_body[:300]}"
            except Exception as e:
                last_err = f"{type(e).__name__}: {e}"
            time.sleep(1.5 * (attempt + 1))
        return ChatResult(
            text="",
            raw={},
            model=self.model,
            elapsed_s=time.time() - start,
            error=f"LLM call failed after {self.retries + 1} attempts: {last_err}",
        )

    def preflight(self) -> Optional[str]:
        """Smoke-test the endpoint; returns None on success, error str
        otherwise. Mirrors batch_detect.sh:43-67."""
        r = self.chat(
            [{"role": "user", "content": "ping"}],
            max_tokens=4,
        )
        if r.error:
            return r.error
        return None


def parse_json_block(text: str) -> Optional[Dict[str, Any]]:
    """Best-effort extract a JSON object from a possibly-fenced LLM
    response. Mirrors the strict-mode logic from evaluate_recall.py."""
    if not text:
        return None
    if text.startswith("```"):
        text = re.sub(r"^```[a-zA-Z]*\n?", "", text)
        text = re.sub(r"\n?```$", "", text)
        text = text.strip()
    # Take the outermost JSON object.
    start = text.find("{")
    if start < 0:
        return None
    end = text.rfind("}")
    if end <= start:
        return None
    blob = text[start : end + 1]
    try:
        return json.loads(blob)
    except json.JSONDecodeError:
        # Try fenced code blocks anywhere in the text.
        for m in re.finditer(r"```(?:json)?\s*\n(.*?)\n```", text, flags=re.S):
            inner = m.group(1).strip()
            if inner.startswith("{"):
                try:
                    return json.loads(inner)
                except json.JSONDecodeError:
                    continue
    return None
