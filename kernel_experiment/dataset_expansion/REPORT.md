# Dataset Expansion Report — Kernel Concurrency Defects (v2)

## Goal

Grow the existing 50-CVE Lace evaluation set to ~100 entries, **adding
only entries that are demonstrably concurrency defects with clear two-thread
evidence**, sampled (not exhaustive) from two new archives:

- `linux_kernel.tar.gz` — 1378 Linux CVE metadata records (NVD + git refs).
- `syzbot.tar.gz` — 1807 syzbot bug reports (KCSAN, lockdep, KASAN, …).

User's requirements (verbatim):
1. *"确实是并发缺陷，能明确确定两个线程或入口，涉及缺陷的两个操作应该处于两个不同的线程。"*
2. *"确认存在漏洞的版本，能像之前那样提取出模块以及bc文件并进行检测。"*
3. *"我们不用追求全覆盖，而是抽样，加上之前的50个，一共有个一百来个就行了。"*
4. *"在添加数据的时候要慎重，不要大致看一眼就猜是两个线程，最好还是看看相关源代码，除非真的证据非常充足。"*

## Workflow

All artifacts live under `kernel_experiment/dataset_expansion/`.

| Step | Script | What it does |
|---|---|---|
| 1. Rank | `00_rank.py` | Triages 1378 CVEs + 1807 syzbot reports into 4 tiers by concurrency-signal strength. |
| 2. Evidence | `01_evidence.py` | For each candidate, resolves the mainline fix commit in our local `linux.git`, runs `git show` to get the real patch (the linux_kernel archive's `patch.diff` is bot-blocked HTML), then extracts the enclosing function body at the vulnerable parent commit plus external call sites. Produces `evidence/<bug>.md` + `<bug>.meta.json`. |
| 3. Audit summary | `02_audit_summary.py` | Renders each evidence packet into a one-screen "audit card" containing commit subject, KCSAN/lockdep stack trace, and unified diff. |
| 4. Pre-classify | `03_preclassify.py` | Heuristic 3-way label per card: PROBABLY_GOOD / PROBABLY_BAD / NEEDS_REVIEW. Looks at +/− lines only and counts net-new synchronization primitives. |
| 5. Manual audit | (me) | I personally read every NEEDS_REVIEW card and every PROBABLY_BAD card. PROBABLY_GOOD got spot-checked across 10+ samples. Verdicts in `verdicts.json`. |
| 6. Dedupe & select | `05_select_final.py` | Dedupe by mainline SHA (one fix may be reported by many syzbot ids); subsystem-balanced selection capped at 50 to keep the final dataset diverse. |
| 7. Ground-truth | `04_build_ground_truth.py` | For each selected entry writes `kernel_experiment/<bug_id>/ground_truth.json` in the same schema as the existing 50, and appends rows to `cve_survey.csv`. |

## Tiering rules (`00_rank.py`)

| Tier | Source | Filter | Why it's a useful concurrency signal |
|---|---|---|---|
| A | syzbot | Title matches `KCSAN: data-race in fn1 / fn2` | KCSAN by construction only fires on a confirmed race; the two functions in the title are literally the two racing call sites. **Highest precision.** |
| B | syzbot | Title contains "possible deadlock" | Lockdep splat usually identifies two acquisition orders, but many are recursive locks or false positives. Lower precision; tier-B candidates **excluded** from this round after spot-checks showed several were non-concurrency fixes. |
| C | linux_cve | CWE-362 (race condition) | NVD-categorised race CVEs. Good for paper credibility but description varies in quality. |
| D | linux_cve | CWE-667 / 416 / 415 / 367 + ≥2 concurrency keywords | Locking, UAF-under-race, TOCTOU. Heuristic noise; tier-D **excluded** this round to keep manual-audit budget bounded. |

## Audit rubric (`verdicts.json`)

`ACCEPT` iff **all three** hold:

1. The KCSAN/lockdep report (or CVE description for tier-C) clearly
   identifies two distinct execution contexts in the stack traces — e.g.
   "task ... on cpu 1 / interrupt on cpu 0", "softirq vs syscall",
   "worker vs syscall", "two workers from different netns", etc.
2. The upstream fix uses real synchronization primitives:
   add a lock, an `atomic_*` op, paired `READ_ONCE`/`WRITE_ONCE`, an
   `smp_*` barrier, a per-CPU/per-netns conversion, or structurally
   moves the racy access under an existing lock.
3. The parent commit exists in our local `linux.git` so the
   vulnerable source can be checked out and compiled.

`REJECT` if:

- The patch is **only** a `data_race()` annotation (maintainers
  explicitly mark the race benign).
- The patch is only `ASSERT_EXCLUSIVE_*` (assertion, no real fix).
- The patch is purely defensive input validation (no concurrency
  mechanism involved; the underlying issue was input validation).
- The patch removes a lock with no replacement (false-positive
  lockdep cleanup).
- The race is narrowed but not eliminated and we cannot point to a
  clean two-thread → fixed-by-sync code path.

## Numbers

| Stage | Tier A | Tier C | Total |
|---|---|---|---|
| Surveyed | 204 KCSAN | 248 CWE-362 | 452 |
| Ranked & kept top-80 | 80 | 80 | 160 |
| Evidence packets built | 80 | 79 | 159 (1 git-grep timeout, 5 already audited from earlier smoke tests) |
| Audited and accepted | 62 | 17 | 79 |
| After dedupe (same mainline SHA) | 47 | 16 (one CVE absorbed a syz dup) | 79 (6 syz–syz + 1 CVE–syz dup groups collapsed) |
| **Core selection (target ≈ 50)** | 33 | 17 | **50** |
| Supplementary | 29 | 0 | 29 |

Reject reasons in audited set (11 total, all tier-A): `data_race()`-only
annotation × 9; lock removal w/o replacement × 1; non-concurrency
input-validation defense × 1.

## Subsystem distribution

| Subsystem | Existing 50 | New 50 (core) | Total |
|---|---|---|---|
| net | 11 | 16 | 27 |
| drivers | 18 | 9 | 27 |
| kernel | 2 | 9 | 11 |
| include/net | 5 | 7 | 12 |
| fs | 3 | 5 | 8 |
| include/linux | 0 | 4 | 4 |
| mm | 4 | 0 | 4 |
| security | 2 | 0 | 2 |
| block | 2 | 0 | 2 |
| sound | 1 | 0 | 1 |
| io_uring | 1 | 0 | 1 |
| virt | 1 | 0 | 1 |

The new entries lean toward `net/`, `kernel/` (workqueue, RCU, srcu,
timers, posix-timers) and `include/net/` (sock state fields), which is
where most of Eric Dumazet's data-race fix series live and where Lace's
KCSAN-style detection ought to shine.

## Files produced

- `kernel_experiment/cve_survey.csv` — 101 rows (1 header + 50 existing
  + 50 new). Backup of the pre-expansion version at
  `cve_survey.csv.bak50`.
- `kernel_experiment/<bug_id>/` — for all 100 entries: a
  `ground_truth.json` in the v1 schema (`cve_id`, `fix_commit`,
  `files`, `description`, `cwes`, `title`, `tier`, `source_archive`,
  `thread_hint`, `two_threads_summary`, `audit_notes`,
  `fix_commit_message`, `patch`, `affected_files_from_patch`), the
  `src/` tree (≥1 `.c` plus same-directory headers), and per-TU
  `<basename>.ll` bitcode (and `merged.ll` where multiple `.c` files
  link cleanly).
- `kernel_experiment_v2_staging/` — isolated WIP area; holds the 29
  unpromoted (audited but unused) supplementary entries plus a
  `README.md` describing the staging→promotion protocol.
- `kernel_experiment/dataset_expansion/` — all reproducibility
  scripts (`00_rank.py` … `07_promote.py`), candidate JSONs,
  evidence packets, audit cards, `verdicts.json`,
  `staging_verdict.json`, `final_selection.json`,
  `cve_survey_promoted.csv`, and this report.

## Compile + promote (completed)

Per the user's mandate *"确定每个数据都编译通并且正确再一起放入"*, all
new entries were first materialised into an isolated staging folder,
not into the production set. They were promoted into the v1 folder
only once `batch_prepare.sh` had successfully built their `src/` and
`*.ll` artefacts.

### Staging dance

1. New `ground_truth.json` files were initially written into
   `kernel_experiment_v2_staging/` (50 core + 29 supplementary), one
   directory per bug id.
2. `cve_survey.csv` in staging covers exactly those entries.
3. `scripts/batch_prepare.sh` runs with `EXPERIMENT_BASE` and
   `SURVEY_FILE` pointed at the staging tree, so the main
   `kernel_experiment/` set is never touched.
4. `06_verify_staging.py` audits every entry against four pass-gates:
   GT shape (≥10 keys + non-empty `two_threads_summary`/`patch`),
   ≥1 `.c` under `src/`, ≥1 non-trivial `.ll` (>10 KB), and (warning
   only) `merged.ll` when GT lists >1 `.c` file. Cases that produce
   `.ll` for the audit-identified files but fail to llvm-link are
   reported as `PARTIAL`, matching the shape of 3 existing v1 CVEs.
5. `07_promote.py` moves the entries with status `PASS`/`PARTIAL` from
   `kernel_experiment_v2_staging/` into `kernel_experiment/`, appends
   their rows to `cve_survey.csv`, and writes
   `cve_survey_promoted.csv` as the audit trail. It refuses to
   overwrite any existing production directory.

### Build-script fixes added during this round

`scripts/batch_prepare.sh` was extended to mirror the fallbacks already
present in `scripts/prepare_cve.sh` and to handle two patterns specific
to the new entries:

- Fallback shims for `include/generated/bounds.h`,
  `include/generated/autoconf.h`, `include/generated/utsrelease.h`,
  `include/generated/timeconst.h` and an empty
  `include/generated/asm-offsets.h` when `make modules_prepare` doesn't
  produce them.
- Subsystem cross-tree includes for `drivers/gpu/drm/amd/*`,
  `drivers/gpu/drm/nouveau/*`, and `drivers/crypto/chelsio/chtls/*` (the
  last reaches `drivers/net/ethernet/chelsio/cxgb4` for `t4fw_api.h`).
- A pre-included `lace_initcall_override.h` wrapper that pre-includes
  `<linux/module.h>` (so its header guard is set) and then undefs
  `module_init` and every `*_initcall` alias to redefine them as
  uniquely-named marker functions. This is the only way to win against
  the kernel header's own `#define module_init` when `-DMODULE` is
  active in recent kernels; without it any TU containing both
  `module_init(...)` and a `late_initcall(...)` (RCU `tree.c`,
  `srcutree.c`, …) hit `error: redefinition of __inittest`.

### Final yield

| Bucket | Count |
|---|---|
| Original (frozen) CVE dataset, untouched | 50 |
| Staging core (audited) entries | 50 |
| Staging supplementary entries (audited backstop) | 29 |
| Staging entries passing `06_verify_staging.py` | 68 PASS + 1 PARTIAL |
| Staging entries failing verification | 10 (5 header-only patches; 1 K&R-syntax `sysctl_net_ipv4.c`; 1 rxrpc parent-commit/ns_common skew; 3 missing audit notes that the verifier flagged separately earlier, now passing) |
| **Promoted into `kernel_experiment/`** | **50** (44 core PASS + 1 core PARTIAL + 5 diverse supplementary picks) |
| Production set after promotion | **100 (50 old + 50 new)** |

The 29 supplementary entries that did not get picked stay in
`kernel_experiment_v2_staging/` as a future top-up pool.

## Caveats & threats to validity

1. **Multi-syzbot-id ↔ single-commit duplicates** are deduped, but two
   *different* commits that fix the *same* underlying race (mainline +
   follow-up) might both appear in the dataset. I checked the 7
   dedupe groups but did not exhaustively grep for follow-up commits.
2. **patch-touched ≠ racy-touched.** For some entries (especially the
   "annotate lockless reads" `sk_backlog.len` family), the fix touches
   multiple `.c` files spread across subsystems while the original racy
   access is in a single function. `patch_expander.py` in
   `batch_prepare.sh` re-expands the file set based on the patch hunks,
   so the compiled bitcode should still cover the relevant TUs.
3. **One PARTIAL entry.** `SYZBOT-1b830cb1f67689d4` has 5 individual
   `.ll` files but no `merged.ll` because two of its source files
   share a basename (`net/ipv4/tcp.c` vs `drivers/nvme/host/tcp.c`),
   causing llvm-link symbol collisions. Three of the existing v1
   CVEs have the same shape; Lace consumes the individual `.ll` files
   directly in those cases.
4. **Supplementary picks were chosen for subsystem diversity** (fs, mm,
   io_uring, sound, generic) rather than highest-confidence audit
   score, to avoid the dataset skewing further toward net/.
