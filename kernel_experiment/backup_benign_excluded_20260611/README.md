# Benign Excluded Case Backup

This directory contains cases moved out of `kernel_experiment/` because they are classified as benign developer-annotated races in `../dataset_benign_exclusions.json`.

Moving them out makes glob-based experiment runners over `CVE-*` / `SYZBOT-*` operate on the active benchmark by default. Restore by moving a case directory back to `kernel_experiment/`.

Moved now: 28; active top-level cases: 72; effective runner cases after dual-thread exclusions: 69.
