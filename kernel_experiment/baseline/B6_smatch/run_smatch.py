#!/usr/bin/env python3
"""B6 baseline: Smatch (Dan Carpenter's kernel static analyzer, actively maintained).

Per CVE (all 100):
  1. checkout the vulnerable kernel (fix~1) + modules_prepare (+ header shims),
     reusing Lace's prepare logic (prepare_smatch.sh, EXPAND_PATCH=0 so we only
     analyze the patch-touched .c files);
  2. run `smatch --project=kernel --succeed` single-TU on each patched .c with the
     kernel include set (robust to old-kernel/new-gcc breakage);
  3. collect warn/error lines, order concurrency-relevant ones first, and write
     them as bug reports for the shared evaluate_recall.py judge.

Single-TU mode (no cross-function sqlite DB) => Smatch's deepest cross-function
locking analysis is reduced; this is noted in the writeup.
"""
import argparse, datetime, glob, json, os, re, subprocess, sys, time

PLAY    = "/mlx_devbox/users/mayunlong.39/playground"
LLM4CON = f"{PLAY}/LLM4Con"
EXPBASE = f"{LLM4CON}/kernel_experiment"
SMATCH  = f"{PLAY}/external/baselines/smatch/smatch"
KERNEL  = os.environ.get("LINUX_REPO", f"{PLAY}/linux.git")
WORK    = f"{PLAY}/external/baselines/smatch_work"
DUMP    = f"{EXPBASE}/baseline_dump/B6_smatch"
PREPARE = f"{EXPBASE}/baseline/B6_smatch/prepare_smatch.sh"

CASE_TIMEOUT = int(os.environ.get("SMATCH_CASE_TIMEOUT", "900"))

# warnings whose text suggests a concurrency / memory-lifetime issue -> rank first
_CONC_KW = re.compile(
    r"lock|unlock|race|rcu|atomic|sleep|preempt|irq|spin|mutex|sema|"
    r"free|use-after|uaf|double|refcount|ref count|leak|barrier|concurr",
    re.IGNORECASE)
_WARN_LINE = re.compile(r"^(?P<path>\S+\.[ch]):(?P<line>\d+)\s+(?P<msg>.*\b(warn|error|warning):.*)$")


def sh(cmd, timeout=None, env=None, cwd=None):
    try:
        return subprocess.run(cmd, shell=True, text=True, timeout=timeout,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              env=env, cwd=cwd)
    except subprocess.TimeoutExpired as e:
        class R:  # mimic CompletedProcess
            returncode = 124
            stdout = (e.output or "") + "\n[TIMEOUT]\n"
        return R()


def base_env():
    env = dict(os.environ)
    env["LINUX_REPO"] = KERNEL
    env["EXPERIMENT_BASE"] = WORK
    env["EXPAND_PATCH"] = "0"          # only the patch-touched .c files
    env["SMATCH"] = SMATCH
    return env


def all_cases():
    rows = []
    for gt in sorted(glob.glob(f"{EXPBASE}/*/ground_truth.json")):
        cve = os.path.basename(os.path.dirname(gt))
        try:
            j = json.load(open(gt))
        except Exception:
            continue
        files = j.get("affected_files_from_patch") or j.get("files") or []
        cfiles = sorted({f for f in files if f.endswith(".c")})
        if not cfiles or not j.get("fix_commit"):
            continue
        rows.append({"cve": cve, "fix": j["fix_commit"], "cfiles": cfiles})
    return rows


def parse_warnings(cdir):
    findings = []
    seen = set()
    for txt in sorted(glob.glob(f"{cdir}/*.smatch.txt")):
        for ln in open(txt, errors="ignore"):
            ln = ln.rstrip()
            m = _WARN_LINE.match(ln)
            if not m:
                continue
            # normalise path to a basename:line key for dedupe
            key = (os.path.basename(m.group("path")), m.group("line"), m.group("msg"))
            if key in seen:
                continue
            seen.add(key)
            findings.append({
                "file": os.path.basename(m.group("path")),
                "line": m.group("line"),
                "text": ln,
                "conc": bool(_CONC_KW.search(m.group("msg"))),
            })
    # concurrency-relevant first, then by file/line
    findings.sort(key=lambda d: (not d["conc"], d["file"], int(d["line"])))
    return findings


def to_reports(findings):
    out = []
    for f in findings:
        out.append(
            "===== Hypothesis-Based Violation Detected =====\n"
            "Detector: Smatch (--project=kernel)\n"
            f"Location: {f['file']}:{f['line']}\n"
            f"Warning: {f['text']}\n")
    return out


def write_dump(cve, reports, meta):
    ts = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    d = f"{DUMP}/{cve}_{ts}"
    os.makedirs(d, exist_ok=True)
    with open(f"{d}/confirmed_hypotheses.log", "w") as fh:
        fh.write("\n".join(reports) + "\n" if reports
                 else "# Smatch produced no warn/error lines for this target.\n")
    json.dump(meta, open(f"{d}/baseline_meta.json", "w"), indent=2)
    return d


def process(case):
    cve = case["cve"]
    cdir = f"{WORK}/{cve}"
    sh(f"rm -rf {cdir}")
    cmd = " ".join(["bash", PREPARE, cve, case["fix"], *case["cfiles"]])
    t0 = time.time()
    r = sh(cmd, timeout=CASE_TIMEOUT, env=base_env(), cwd=LLM4CON)
    with open(f"{WORK}/{cve}.run.log", "w") as fh:
        fh.write(r.stdout or "")
    findings = parse_warnings(cdir)
    n_conc = sum(1 for x in findings if x["conc"])
    reports = to_reports(findings)
    meta = {"cve": cve, "tool": "smatch", "fix": case["fix"], "cfiles": case["cfiles"],
            "n_warns": len(findings), "n_conc": n_conc,
            "status": "timeout" if r.returncode == 124 else "ok",
            "secs": round(time.time() - t0, 1)}
    d = write_dump(cve, reports, meta)
    return cve, meta["status"], len(findings), n_conc, d


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cve", nargs="*")
    ap.add_argument("--list", action="store_true")
    args = ap.parse_args()
    cases = all_cases()
    if args.cve:
        cases = [c for c in cases if c["cve"] in set(args.cve)]
    os.makedirs(WORK, exist_ok=True); os.makedirs(DUMP, exist_ok=True)
    if args.list:
        for c in cases:
            print(f"{c['cve']:20} fix={c['fix'][:12]} cfiles={len(c['cfiles'])}")
        print(f"total: {len(cases)}")
        return
    print(f"{'CVE':20} {'STATUS':8} {'warns':>6} {'conc':>5}")
    print("-" * 48)
    summ = []
    for c in cases:
        cve, status, nw, nc, d = process(c)
        print(f"{cve:20} {status:8} {nw:>6} {nc:>5}", flush=True)
        summ.append({"cve": cve, "status": status, "warns": nw, "conc": nc})
    json.dump(summ, open(f"{DUMP}/_run_summary.json", "w"), indent=2)
    tot = sum(s["warns"] for s in summ); nz = sum(1 for s in summ if s["warns"] > 0)
    print("-" * 48)
    print(f"done: {len(summ)} cases, {nz} with >=1 warning, {tot} warnings total.")


if __name__ == "__main__":
    main()
