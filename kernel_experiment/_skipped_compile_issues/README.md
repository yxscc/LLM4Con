# Skipped CVEs — compile issues not worth debugging

These CVEs were moved out of `kernel_experiment/` because their source TUs
fail to compile to LLVM bitcode in a way that is not easy to fix without
significant kernel-header surgery. They are intentionally excluded from
dataset-level metrics (recall, completeness, etc.).

| CVE            | Root cause                                                                 |
|----------------|----------------------------------------------------------------------------|
| CVE-2011-2183  | Very old kernel (pre-3.x) — missing `generated/bounds.h`, stale UAPI glue. |
| CVE-2024-53160 | Deep header dependency chain failing to preprocess; `tree_compile.log`.    |

If you want to re-enable them, move the folder back to `kernel_experiment/`
and re-run `scripts/prepare_cve.sh` with whatever compilation shims are
needed.
