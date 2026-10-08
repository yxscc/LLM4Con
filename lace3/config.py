"""Settings read from the environment.

The LLM gateway values reuse the names setup_env.sh already exports for the v2
detector (LLM_API_KEY, LLM_BASE_URL, LLM_MODEL), so both generations run from
the same shell. Everything v3-specific is prefixed LACE3_.
"""

import os
import shutil
from dataclasses import dataclass
from urllib.parse import urlsplit, urlunsplit


def _clang16():
    for cand in ("clang-16", "/usr/lib/llvm-16/bin/clang"):
        if shutil.which(cand):
            return cand
    return "clang-16"


@dataclass(frozen=True)
class LLMSettings:
    api_key: str
    azure_endpoint: str
    api_version: str
    model: str


def llm_settings():
    key = os.environ.get("LACE3_API_KEY") or os.environ.get("LLM_API_KEY")
    if not key:
        raise RuntimeError("no gateway key: set LLM_API_KEY (source setup_env.sh)")
    endpoint = os.environ.get("LACE3_AZURE_ENDPOINT")
    if not endpoint:
        base = os.environ.get("LLM_BASE_URL")
        if not base:
            raise RuntimeError("no gateway endpoint: set LLM_BASE_URL or LACE3_AZURE_ENDPOINT")
        # The v2 client posts to the full URL with ?ak=<key>; the Azure client
        # wants the bare endpoint and adds the key and api-version itself.
        p = urlsplit(base)
        endpoint = urlunsplit((p.scheme, p.netloc, p.path, "", ""))
    return LLMSettings(
        api_key=key,
        azure_endpoint=endpoint,
        api_version=os.environ.get("LACE3_AZURE_API_VERSION", "2024-03-01-preview"),
        model=os.environ.get("LACE3_MODEL") or os.environ.get("LLM_MODEL", "gpt-5.5-2026-04-24"),
    )


def clang():
    # The kernel IR consumers in this repo were validated on clang-16 output;
    # clang-19 miscompiles older trees (see setup_env.example.sh).
    return os.environ.get("LACE3_CLANG", _clang16())


def linux_repo():
    return os.environ.get("LINUX_REPO")
