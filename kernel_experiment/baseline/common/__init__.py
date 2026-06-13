"""Shared utilities for Lace baseline experiments.

Sub-modules:
    llm_client  -- Thin wrapper around the ByteDance OpenAI-compatible
                   gateway. Mirrors the request shape used by Lace's
                   own LLMClient so model/endpoint/temperature
                   conventions stay identical.
    cve_loader  -- Reads kernel_experiment/CVE-*/ ground_truth.json,
                   vulnerable source, and patches. Same source-of-truth
                   as scripts/evaluate_recall.py.
    dump_writer -- Emits baseline findings in Lace's bugs.txt /
                   confirmed_hypotheses.log format so the existing
                   judge (scripts/evaluate_recall.py) can score every
                   baseline with zero modifications.
"""
