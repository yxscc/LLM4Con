## Summary

| Tool | Model | CVEs | FOUND% | recall@overall | recall@with-output | n_bugs | TP_match | TP_related | FP | precision_strict | precision_lenient | FP_rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B2_CCSR | gpt-5.5-2026-04-24 | 100 | 100.0% | 44.0% | 44.0% | 47 | 47 | 0 | 0 | 100.0% | 100.0% | 0.0% |
| B3_Mythos | gpt-5.5-2026-04-24 | 100 | 100.0% | 3.0% | 3.0% | 27 | 3 | 4 | 20 | 11.11% | 25.93% | 74.07% |


## Per-CVE recall (HIT means `evaluate_recall.py` judged the report matches the patch's root cause)

| CVE | B2_CCSR | B3_Mythos | any_hit |
| --- | --- | --- | --- |
| CVE-2013-1792 | MISS | MISS |  |
| CVE-2015-7550 | MISS | MISS |  |
| CVE-2016-7911 | HIT (1) | MISS | ✓ |
| CVE-2016-9806 | HIT (1) | MISS | ✓ |
| CVE-2017-15265 | HIT (1) | HIT (1) | ✓ |
| CVE-2017-6346 | HIT (1) | MISS | ✓ |
| CVE-2022-48830 | MISS | MISS |  |
| CVE-2022-48931 | MISS | MISS |  |
| CVE-2022-49215 | HIT (1) | MISS | ✓ |
| CVE-2022-49589 | MISS | MISS |  |
| CVE-2022-49607 | MISS | MISS |  |
| CVE-2022-49634 | HIT (1) | MISS | ✓ |
| CVE-2022-49641 | HIT (1) | MISS | ✓ |
| CVE-2023-53046 | HIT (1) | ERROR | ✓ |
| CVE-2024-26861 | MISS | MISS |  |
| CVE-2024-26862 | MISS | MISS |  |
| CVE-2024-26974 | HIT (1) | MISS | ✓ |
| CVE-2024-26984 | MISS | MISS |  |
| CVE-2024-27019 | HIT (1) | MISS | ✓ |
| CVE-2024-27030 | HIT (1) | MISS | ✓ |
| CVE-2024-27404 | HIT (1) | MISS | ✓ |
| CVE-2024-35898 | HIT (1) | MISS | ✓ |
| CVE-2024-35977 | MISS | MISS |  |
| CVE-2024-35986 | HIT (1) | MISS | ✓ |
| CVE-2024-35999 | HIT (1) | MISS | ✓ |
| CVE-2024-36938 | MISS | MISS |  |
| CVE-2024-38596 | MISS | MISS |  |
| CVE-2024-39503 | HIT (2) | HIT (1) | ✓ |
| CVE-2024-39508 | HIT (1) | MISS | ✓ |
| CVE-2024-40953 | HIT (1) | MISS | ✓ |
| CVE-2024-41005 | MISS | MISS |  |
| CVE-2024-41081 | HIT (1) | HIT (1) | ✓ |
| CVE-2024-42234 | HIT (1) | MISS | ✓ |
| CVE-2024-43830 | HIT (1) | MISS | ✓ |
| CVE-2024-43891 | HIT (1) | MISS | ✓ |
| CVE-2024-45000 | MISS | MISS |  |
| CVE-2024-46704 | MISS | MISS |  |
| CVE-2024-47715 | MISS | MISS |  |
| CVE-2024-50082 | HIT (1) | MISS | ✓ |
| CVE-2024-53124 | MISS | MISS |  |
| CVE-2024-53136 | MISS | MISS |  |
| CVE-2024-53160 | HIT (1) | MISS | ✓ |
| CVE-2024-53186 | HIT (1) | MISS | ✓ |
| CVE-2024-56555 | HIT (1) | MISS | ✓ |
| CVE-2024-56788 | MISS | MISS |  |
| CVE-2024-58072 | HIT (1) | MISS | ✓ |
| CVE-2025-21732 | MISS | MISS |  |
| CVE-2025-22050 | MISS | MISS |  |
| CVE-2025-23142 | HIT (3) | MISS | ✓ |
| CVE-2025-23151 | HIT (1) | MISS | ✓ |
| CVE-2025-37772 | MISS | MISS |  |
| CVE-2025-37854 | HIT (1) | MISS | ✓ |
| CVE-2025-37882 | HIT (1) | MISS | ✓ |
| CVE-2025-37920 | HIT (1) | MISS | ✓ |
| CVE-2025-38037 | MISS | MISS |  |
| CVE-2025-38048 | MISS | MISS |  |
| CVE-2025-38078 | HIT (1) | MISS | ✓ |
| CVE-2025-38104 | MISS | MISS |  |
| CVE-2025-38165 | MISS | MISS |  |
| CVE-2025-38217 | MISS | MISS |  |
| CVE-2025-38242 | HIT (1) | MISS | ✓ |
| CVE-2025-38250 | HIT (1) | MISS | ✓ |
| CVE-2025-38337 | MISS | MISS |  |
| CVE-2025-38383 | HIT (1) | MISS | ✓ |
| CVE-2025-38429 | HIT (1) | MISS | ✓ |
| SYZBOT-01affb1491750534 | MISS | MISS |  |
| SYZBOT-08f3e9d26e5541e1 | MISS | MISS |  |
| SYZBOT-123b88b9ddea8e98 | MISS | MISS |  |
| SYZBOT-1b830cb1f67689d4 | MISS | MISS |  |
| SYZBOT-1c486d0b62032c82 | HIT (1) | MISS | ✓ |
| SYZBOT-2d373c9936c00d7e | MISS | MISS |  |
| SYZBOT-2e4de7fe846aba66 | HIT (1) | MISS | ✓ |
| SYZBOT-35301db28c64310d | MISS | MISS |  |
| SYZBOT-3536db46dfa58c57 | HIT (1) | MISS | ✓ |
| SYZBOT-371a9ea56d82de71 | MISS | MISS |  |
| SYZBOT-373cf39d336f4370 | MISS | MISS |  |
| SYZBOT-3872b8b1d5ece2a8 | MISS | MISS |  |
| SYZBOT-392f4c8f5827466f | MISS | MISS |  |
| SYZBOT-3b6b32dc50537a49 | MISS | MISS |  |
| SYZBOT-3cc3a12efa69aa6f | MISS | MISS |  |
| SYZBOT-416320b6b3545cbf | MISS | MISS |  |
| SYZBOT-417aeb05fd190f3a | HIT (1) | MISS | ✓ |
| SYZBOT-44cf88a58d91b12b | MISS | MISS |  |
| SYZBOT-453b4249be2d6230 | MISS | MISS |  |
| SYZBOT-483d6c9b9231ea7e | MISS | MISS |  |
| SYZBOT-4a03518df1e31b53 | HIT (1) | MISS | ✓ |
| SYZBOT-4a06d4373fd52f0b | HIT (1) | MISS | ✓ |
| SYZBOT-4b16e156132582a9 | MISS | MISS |  |
| SYZBOT-4dfb96a94317a78f | MISS | MISS |  |
| SYZBOT-52cb782c704e243e | MISS | MISS |  |
| SYZBOT-52e3dbded1f71729 | MISS | MISS |  |
| SYZBOT-5366159cc4c1d817 | MISS | MISS |  |
| SYZBOT-565f500a8d3fb9b7 | MISS | MISS |  |
| SYZBOT-5676077ba016d741 | MISS | MISS |  |
| SYZBOT-5a486fef3de40e0d | MISS | MISS |  |
| SYZBOT-5cce5938c6c2c518 | MISS | MISS |  |
| SYZBOT-62955e4f963d38ab | MISS | MISS |  |
| SYZBOT-630679ddf6c0deba | MISS | MISS |  |
| SYZBOT-63cbe31877bb80ef | HIT (1) | MISS | ✓ |
| SYZBOT-6a2a295ae3340e8e | MISS | MISS |  |
