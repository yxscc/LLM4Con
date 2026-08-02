# LLM4Con Full-72 Evaluation — Token & Time Cost Statistics

Run: `20260709_110902` | Model: `gpt-5.5-2026-04-24` (ByteDance gateway)

## 1. Overall Summary

| Metric | Value |
|---|---|
| Total cases | 72 |
| FOUND (>=1 report) | 40 |
| CLEAN/DONE | 32 |
| Total reports | 93 |
| Recall (TP cases) | 36/72 (50.0%) |
| Strict precision | ~61% (57/93) |

## 2. Time Cost

| Metric | Value |
|---|---|
| Total wall-clock time | 18343s (305.7 min) |
| Active cases (reqs > 0) | 59 |
| Avg time per active case | 310.5s |
| Median time per active case | 119s |
| Min time | 8s |
| Max time | 1236s |
| Avg time (FOUND cases) | 408.0s (n=40) |
| Avg time (CLEAN cases) | 105.3s (n=19) |

### Time Distribution

| Range | Count |
|---|---|
| 0s - 10s | 14 |
| 10s - 60s | 18 |
| 60s - 300s | 19 |
| 300s - 600s | 8 |
| 600s - 900s | 9 |
| 900s - 1500s | 4 |

## 3. Token Cost

| Metric | Value |
|---|---|
| Cases with token data | 72/72 |
| Total Prompt Tokens | 30,503,634 |
| Total Completion Tokens | 1,099,406 |
| Total Tokens | 31,603,040 |
| Total LLM API Requests | 1493 |
| Avg Prompt Tokens/case | 517,011 |
| Avg Completion Tokens/case | 18,634 |
| Avg Total Tokens/case | 535,645 |
| Median Total Tokens/case | 128,438 |

### Token Distribution

| Range | Count |
|---|---|
| 0K - 50K | 15 |
| 50K - 100K | 9 |
| 100K - 500K | 14 |
| 500K - 1000K | 11 |
| 1000K - 2000K | 6 |
| 2000K - 5000K | 4 |

## 4. LLM Request Statistics

| Metric | Value |
|---|---|
| Total requests | 1493 |
| Cases with requests | 59 |
| Avg requests/case | 25.3 |
| Median requests/case | 12 |
| Min requests | 1 |
| Max requests | 87 |

## 5. Cost by Result Category

| Category | N | Avg Time (s) | Median Time (s) | Avg Tokens | Median Tokens | Avg Requests |
|---|---:|---:|---:|---:|---:|---:|
| TP (命中GT) | 36 | 362 | 191 | 565,788 | 207,470 | 28.5 |
| FP-only (FOUND未命中) | 4 | 820 | 883 | 2,001,704 | 1,478,298 | 57.5 |
| CLEAN/DONE | 32 | 105 | 47 | 169,887 | 62,605 | 12.5 |

## 6. Estimated Monetary Cost

| Pricing Model | Total Cost | Per-case Cost |
|---|---:|---:|
| GPT-4o equivalent ($2.5/1M in, $10/1M out) | $87.25 | $1.21 |
| GPT-5.5 estimated (2x GPT-4o) | $169.01 | $2.35 |

Note: Actual cost depends on ByteDance gateway pricing; these are rough estimates based on public OpenAI pricing.

## 7. Shared Object Statistics

| Metric | Value |
|---|---|
| Total shared objects analyzed | 1303 |
| Cases with objects | 59 |
| Avg objects/case | 22.1 |
| Median objects/case | 10 |
| Max objects | 120 |
| Self-race cases | 17 |
| Total self-race objects | 203 |

## 8. Per-case Detail

| Case | Status | Recall | Threads | Objs | Bugs | Reqs | Time(s) | Prompt Tokens | Completion Tokens | Total Tokens |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CVE-2013-1792 | CLEAN/DONE | - | 1 | 10 | 0 | 7 | 61 | 67,808 | 3,432 | 71,240 |
| CVE-2015-7550 | CLEAN/DONE | - | 1 | 8 | 0 | 7 | 46 | 72,834 | 2,385 | 75,219 |
| CVE-2016-7911 | FOUND | HIT | 2 | 1 | 1 | 7 | 32 | 30,067 | 2,129 | 32,196 |
| CVE-2016-9806 | FOUND | HIT | 2 | 56 | 4 | 48 | 743 | 945,462 | 53,938 | 999,400 |
| CVE-2017-15265 | FOUND | HIT | 2 | 75 | 4 | 83 | 896 | 1,543,827 | 57,357 | 1,601,184 |
| CVE-2017-6346 | FOUND | HIT | 1 | 14 | 4 | 28 | 603 | 659,609 | 33,938 | 693,547 |
| CVE-2022-48830 | FOUND | HIT | 1 | 26 | 4 | 34 | 539 | 1,086,280 | 26,843 | 1,113,123 |
| CVE-2022-48931 | CLEAN/DONE | - | 1 | 6 | 0 | 8 | 54 | 88,166 | 2,671 | 90,837 |
| CVE-2022-49215 | FOUND | HIT | 2 | 3 | 2 | 12 | 107 | 91,761 | 6,502 | 98,263 |
| CVE-2022-49607 | FOUND | MISS | 5 | 42 | 2 | 29 | 462 | 430,226 | 27,357 | 457,583 |
| CVE-2023-53046 | CLEAN/DONE | - | 2 | 4 | 0 | 8 | 34 | 45,362 | 2,163 | 47,525 |
| CVE-2024-26974 | CLEAN/DONE | - | 2 | 4 | 0 | 10 | 36 | 58,633 | 2,489 | 61,122 |
| CVE-2024-26984 | FOUND | HIT | 1 | 1 | 1 | 5 | 44 | 33,942 | 1,894 | 35,836 |
| CVE-2024-27019 | FOUND | HIT | 2 | 2 | 1 | 7 | 42 | 32,113 | 2,212 | 34,325 |
| CVE-2024-27030 | FOUND | HIT | 1 | 1 | 1 | 5 | 35 | 27,092 | 1,343 | 28,435 |
| CVE-2024-27404 | FOUND | HIT | 2 | 28 | 6 | 32 | 614 | 640,247 | 37,704 | 677,951 |
| CVE-2024-35898 | FOUND | HIT | 2 | 4 | 1 | 9 | 192 | 71,216 | 9,575 | 80,791 |
| CVE-2024-35977 | CLEAN/DONE | - | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| CVE-2024-35986 | CLEAN/DONE | - | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| CVE-2024-35999 | CLEAN/DONE | - | 3 | 34 | 0 | 46 | 560 | 881,692 | 31,524 | 913,216 |
| CVE-2024-36938 | CLEAN/DONE | - | 2 | 3 | 0 | 8 | 47 | 42,301 | 3,512 | 45,813 |
| CVE-2024-39503 | FOUND | HIT | 5 | 32 | 3 | 48 | 512 | 664,691 | 26,729 | 691,420 |
| CVE-2024-39508 | FOUND | HIT | 2 | 39 | 1 | 49 | 498 | 898,485 | 33,554 | 932,039 |
| CVE-2024-40953 | FOUND | HIT | 1 | 1 | 1 | 3 | 18 | 13,014 | 931 | 13,945 |
| CVE-2024-41081 | FOUND | HIT | 1 | 4 | 2 | 15 | 190 | 127,081 | 10,085 | 137,166 |
| CVE-2024-42234 | CLEAN/DONE | - | 2 | 11 | 0 | 9 | 63 | 59,119 | 3,486 | 62,605 |
| CVE-2024-43830 | FOUND | HIT | 5 | 21 | 1 | 60 | 446 | 692,538 | 22,084 | 714,622 |
| CVE-2024-43891 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 6 | 0 | 0 | 0 |
| CVE-2024-45000 | CLEAN/DONE | - | 2 | 1 | 0 | 0 | 1 | 0 | 0 | 0 |
| CVE-2024-46704 | CLEAN/DONE | - | 2 | 39 | 0 | 28 | 233 | 387,342 | 13,553 | 400,895 |
| CVE-2024-47715 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| CVE-2024-50082 | CLEAN/DONE | - | 2 | 6 | 0 | 13 | 104 | 98,051 | 6,722 | 104,773 |
| CVE-2024-53124 | FOUND | MISS | 1 | 31 | 5 | 38 | 871 | 841,828 | 46,719 | 888,547 |
| CVE-2024-53136 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| CVE-2024-53160 | CLEAN/DONE | - | 2 | 26 | 0 | 52 | 475 | 975,330 | 27,468 | 1,002,798 |
| CVE-2024-53186 | FOUND | HIT | 2 | 7 | 1 | 15 | 162 | 188,239 | 8,125 | 196,364 |
| CVE-2024-56555 | FOUND | HIT | 2 | 95 | 6 | 86 | 1236 | 2,296,548 | 78,103 | 2,374,651 |
| CVE-2024-56788 | FOUND | HIT | 2 | 6 | 1 | 5 | 55 | 45,528 | 3,144 | 48,672 |
| CVE-2024-58072 | FOUND | HIT | 2 | 107 | 2 | 84 | 1016 | 2,500,077 | 74,602 | 2,574,679 |
| CVE-2025-21732 | FOUND | HIT | 2 | 9 | 2 | 8 | 83 | 102,919 | 6,285 | 109,204 |
| CVE-2025-22050 | FOUND | HIT | 2 | 3 | 1 | 9 | 47 | 75,946 | 2,437 | 78,383 |
| CVE-2025-23142 | FOUND | MISS | 2 | 120 | 1 | 76 | 1051 | 1,997,969 | 70,081 | 2,068,050 |
| CVE-2025-23151 | FOUND | HIT | 2 | 21 | 2 | 7 | 76 | 98,972 | 6,103 | 105,075 |
| CVE-2025-37772 | CLEAN/DONE | - | 2 | 10 | 0 | 1 | 8 | 804 | 165 | 969 |
| CVE-2025-37854 | CLEAN/DONE | - | 2 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| CVE-2025-37882 | FOUND | MISS | 2 | 70 | 4 | 87 | 895 | 4,525,586 | 67,050 | 4,592,636 |
| CVE-2025-37920 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| CVE-2025-38078 | FOUND | HIT | 2 | 21 | 3 | 43 | 534 | 995,294 | 31,391 | 1,026,685 |
| CVE-2025-38104 | FOUND | HIT | 2 | 50 | 1 | 10 | 119 | 156,969 | 7,564 | 164,533 |
| CVE-2025-38165 | FOUND | HIT | 2 | 32 | 3 | 35 | 268 | 541,628 | 15,204 | 556,832 |
| CVE-2025-38217 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| CVE-2025-38242 | CLEAN/DONE | - | 1 | 0 | 0 | 1 | 19 | 879 | 27 | 906 |
| CVE-2025-38250 | CLEAN/DONE | - | 2 | 4 | 0 | 7 | 21 | 38,553 | 1,125 | 39,678 |
| CVE-2025-38337 | FOUND | HIT | 1 | 2 | 1 | 8 | 89 | 82,261 | 4,076 | 86,337 |
| CVE-2025-38383 | CLEAN/DONE | - | 1 | 0 | 0 | 1 | 17 | 879 | 30 | 909 |
| CVE-2025-38429 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| SYZBOT-01affb1491750534 | FOUND | HIT | 1 | 15 | 1 | 38 | 630 | 655,197 | 30,919 | 686,116 |
| SYZBOT-1c486d0b62032c82 | CLEAN/DONE | - | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| SYZBOT-2d373c9936c00d7e | CLEAN/DONE | - | 2 | 6 | 0 | 15 | 114 | 122,282 | 6,156 | 128,438 |
| SYZBOT-2e4de7fe846aba66 | FOUND | HIT | 2 | 63 | 5 | 61 | 746 | 1,602,412 | 58,069 | 1,660,481 |
| SYZBOT-35301db28c64310d | FOUND | HIT | 3 | 23 | 3 | 47 | 863 | 929,376 | 43,137 | 972,513 |
| SYZBOT-3b6b32dc50537a49 | FOUND | HIT | 2 | 47 | 5 | 55 | 994 | 959,944 | 46,904 | 1,006,848 |
| SYZBOT-3cc3a12efa69aa6f | FOUND | HIT | 2 | 1 | 1 | 19 | 158 | 211,044 | 7,533 | 218,577 |
| SYZBOT-417aeb05fd190f3a | FOUND | HIT | 2 | 8 | 1 | 10 | 77 | 116,540 | 5,288 | 121,828 |
| SYZBOT-44cf88a58d91b12b | FOUND | HIT | 2 | 6 | 3 | 12 | 124 | 133,401 | 7,502 | 140,903 |
| SYZBOT-4a03518df1e31b53 | FOUND | HIT | 1 | 3 | 1 | 4 | 45 | 39,578 | 2,238 | 41,816 |
| SYZBOT-4a06d4373fd52f0b | CLEAN/DONE | - | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| SYZBOT-52e3dbded1f71729 | CLEAN/DONE | - | 2 | 1 | 0 | 0 | 9 | 0 | 0 | 0 |
| SYZBOT-5676077ba016d741 | FOUND | HIT | 2 | 17 | 1 | 24 | 209 | 303,024 | 10,614 | 313,638 |
| SYZBOT-5a486fef3de40e0d | CLEAN/DONE | - | 1 | 12 | 0 | 4 | 24 | 40,114 | 1,036 | 41,150 |
| SYZBOT-5cce5938c6c2c518 | CLEAN/DONE | - | 1 | 7 | 0 | 8 | 60 | 102,170 | 1,731 | 103,901 |
| SYZBOT-63cbe31877bb80ef | CLEAN/DONE | - | 2 | 4 | 0 | 5 | 25 | 33,384 | 2,468 | 35,852 |
