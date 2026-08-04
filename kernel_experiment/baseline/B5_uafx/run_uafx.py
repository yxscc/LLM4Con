#!/usr/bin/env python3
"""B5 baseline: UAFX (NDSS'25 cross-entry static UAF detector, SUTURE/DR.CHECKER fork).

Per UAF-class CVE this:
  1. recompiles the patched files to *typed-pointer* LLVM-14 bitcode with clang-14
     (the dataset's own .ll are opaque-pointer LLVM-16, which UAFX cannot parse),
     reusing Lace's prepare_cve.sh checkout/expand/link logic (UAFX_OPT=-O1);
  2. derives entry functions with UAFX's native EntryPointIdentifier
     (driver ops-struct model: file_operations/proto_ops/...);
  3. runs the UAFX opt pass (-dr_checker, UAFDetector);
  4. extracts warnings and writes them as bug reports in the format the shared
     evaluate_recall.py judge consumes.

Native entries only (per request). Files with no driver entry point yield 0 -
that is the honest result for a driver-entry-model tool on heterogeneous code.
"""
import argparse, datetime, glob, json, os, re, shutil, subprocess, sys, time

LLM4CON  = os.environ.get(
    "LLM4CON_HOME",
    os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..")))
PLAY     = os.path.dirname(LLM4CON)
EXPBASE  = f"{LLM4CON}/kernel_experiment"
UAFX     = f"{PLAY}/external/baselines/uafx"
LLVM14   = f"{PLAY}/external/llvm14/bin"
KERNEL   = os.environ.get("LINUX_REPO", f"{PLAY}/linux.git")
WORK     = f"{PLAY}/external/baselines/uafx_work"
DUMP     = f"{EXPBASE}/baseline_dump/B5_uafx"
PREPARE  = f"{EXPBASE}/baseline/B5_uafx/prepare_cve_uafx.sh"

SO       = f"{UAFX}/llvm_analysis/MainAnalysisPasses/build_dir/SoundyAliasAnalysis/libSoundyAliasAnalysis.so"
ENTRYDIR = f"{UAFX}/llvm_analysis/AnalysisHelpers/EntryPointIdentifier"
EXT      = f"{UAFX}/ext_uaf_warns.py"

UAF_CWES = {"CWE-416", "CWE-415"}
OPT_TIMEOUT = int(os.environ.get("UAFX_OPT_TIMEOUT", "2400"))


def sh(cmd, **kw):
    return subprocess.run(cmd, shell=True, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def base_env():
    env = dict(os.environ)
    env["PATH"] = os.pathsep.join([LLVM14, env.get("PATH", "")])
    env["LD_LIBRARY_PATH"] = os.pathsep.join([
        f"{UAFX}/llvm_analysis/MainAnalysisPasses/z3/bin",
        f"{PLAY}/external/llvm14/lib", env.get("LD_LIBRARY_PATH", "")])
    env["LINUX_REPO"] = KERNEL
    env["EXPERIMENT_BASE"] = WORK
    env["CLANG"] = f"{LLVM14}/clang"
    return env


def uaf_cases():
    rows = []
    for gt in sorted(glob.glob(f"{EXPBASE}/*/ground_truth.json")):
        cve = os.path.basename(os.path.dirname(gt))
        try:
            j = json.load(open(gt))
        except Exception:
            continue
        if not (set(j.get("cwes") or []) & UAF_CWES):
            continue
        files = j.get("affected_files_from_patch") or j.get("files") or []
        cfiles = sorted({f for f in files if f.endswith(".c")})
        if not cfiles:
            continue
        rows.append({"cve": cve, "fix": j.get("fix_commit", ""), "cfiles": cfiles})
    return rows


def recompile(case, log):
    cdir = f"{WORK}/{case['cve']}"
    shutil.rmtree(cdir, ignore_errors=True)
    cmd = " ".join(["bash", PREPARE, case["cve"], case["fix"], *case["cfiles"]])
    r = sh(cmd, env=base_env(), cwd=LLM4CON)
    log.write(f"\n===== prepare {case['cve']} =====\n{r.stdout}\n")
    merged = f"{cdir}/merged.ll"
    if os.path.exists(merged):
        return merged
    lls = [p for p in glob.glob(f"{cdir}/*.ll")]
    return lls[0] if lls else None


def make_bc(ll, env):
    bc = ll[:-3] + ".bc" if ll.endswith(".ll") else ll + ".bc"
    r = sh(f"{LLVM14}/llvm-as {ll} -o {bc}", env=env)
    return (bc if os.path.exists(bc) else None), r.stdout


def gen_entries(bc, env):
    """Native UAFX entry identification -> <stem>.conf next to bc."""
    stem = os.path.basename(bc).split(".")[0]
    conf = os.path.join(os.path.dirname(bc), stem + ".conf")
    r = sh(f"python3 entry_point_identifier.py {bc}", env=env, cwd=ENTRYDIR)
    n = 0
    if os.path.exists(conf):
        with open(conf) as f:
            n = sum(1 for ln in f if ln.strip() and not ln.lstrip().startswith("#"))
    return conf, n, r.stdout


def run_uafx(bc, conf, env):
    log = bc + ".uafx.log"
    cmd = (f"timeout {OPT_TIMEOUT} {LLVM14}/opt -load {SO} -enable-new-pm=0 "
           f"-dr_checker -entryConfig={conf} {bc} -o /dev/null")
    r = sh(cmd, env=env)
    with open(log, "w") as f:
        f.write(r.stdout)
    done = "Bug Detection Phase finished" in r.stdout
    return log, done, r.returncode


def extract_warns(uafx_log, env):
    r = sh(f"python3 {EXT} {uafx_log} UAFDetector", env=env, cwd=UAFX)
    return r.stdout


_LOC = re.compile(r"\(\s*u?'([^']+)'\s*,\s*(\d+)\s*,\s*u?'([^']+)'\s*\)")


def parse_to_reports(uaf_text):
    """Each GROUP -> one bug report. Returns list of report strings."""
    reports = []
    groups = re.split(r"=+GROUP \d+=+", uaf_text)
    for g in groups[1:]:
        msum = g.split("#########################", 1)[0]
        locs = _LOC.findall(msum)
        flow = (re.search(r"Flow:\s*(\w+)", g) or [None, "?"])[1]
        if len(locs) >= 2:
            (f0, l0, fn0), (f1, l1, fn1) = locs[0], locs[1]
        elif len(locs) == 1:
            (f0, l0, fn0) = locs[0]; (f1, l1, fn1) = ("?", "?", "?")
        else:
            continue
        reports.append(
            "===== Hypothesis-Based Violation Detected =====\n"
            "Detector: UAFX (SUTURE cross-entry UAF / UAFDetector)\n"
            "Bug type: Use-After-Free (concurrency UAF)\n"
            f"Flow: {flow}  ({'cross-entry/concurrent' if flow.lower().startswith('con') else 'sequential'})\n"
            f"Free site  (LOC0): {f0}:{l0}  in function {fn0}\n"
            f"Use site   (LOC1): {f1}:{l1}  in function {fn1}\n")
    return reports


def write_dump(cve, reports, meta):
    ts = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    d = f"{DUMP}/{cve}_{ts}"
    os.makedirs(d, exist_ok=True)
    with open(f"{d}/confirmed_hypotheses.log", "w") as f:
        if reports:
            f.write("\n".join(reports) + "\n")
        else:
            f.write("# UAFX produced no UAF warnings for this target.\n")
    json.dump(meta, open(f"{d}/baseline_meta.json", "w"), indent=2)
    return d


def process(case):
    cve = case["cve"]
    env = base_env()
    os.makedirs(WORK, exist_ok=True)
    plog = open(f"{WORK}/{cve}.run.log", "w")
    t0 = time.time()
    meta = {"cve": cve, "tool": "UAFX", "fix": case["fix"], "cfiles": case["cfiles"],
            "opt_level": os.environ.get("UAFX_OPT", "-O1"), "status": "", "n_entries": 0,
            "n_warns": 0, "bitcode": None}

    ll = recompile(case, plog)
    if not ll:
        meta["status"] = "compile_failed"
        d = write_dump(cve, [], meta); plog.close()
        return cve, "COMPILE_FAIL", 0, 0, d

    bc, aslog = make_bc(ll, env); plog.write(f"[llvm-as]\n{aslog}\n")
    meta["bitcode"] = os.path.relpath(ll, WORK)
    if not bc:
        meta["status"] = "llvm_as_failed"
        d = write_dump(cve, [], meta); plog.close()
        return cve, "ASM_FAIL", 0, 0, d

    conf, n_entries, elog = gen_entries(bc, env)
    meta["n_entries"] = n_entries; plog.write(f"[entries={n_entries}]\n{elog}\n")

    uafx_log, done, rc = run_uafx(bc, conf, env)
    if not done:
        meta["status"] = f"uafx_incomplete_rc{rc}"
        d = write_dump(cve, [], meta); plog.close()
        return cve, f"UAFX_INCOMPLETE(rc={rc})", n_entries, 0, d

    uaf_text = extract_warns(uafx_log, env)
    reports = parse_to_reports(uaf_text)
    meta["n_warns"] = len(reports); meta["status"] = "ok"; meta["secs"] = round(time.time() - t0, 1)
    d = write_dump(cve, reports, meta); plog.close()
    return cve, "OK", n_entries, len(reports), d


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cve", nargs="*", help="restrict to these CVEs")
    ap.add_argument("--list", action="store_true", help="list UAF cases and exit")
    args = ap.parse_args()

    cases = uaf_cases()
    if args.cve:
        cases = [c for c in cases if c["cve"] in set(args.cve)]
    os.makedirs(WORK, exist_ok=True); os.makedirs(DUMP, exist_ok=True)

    if args.list:
        for c in cases:
            print(f"{c['cve']:18} fix={c['fix'][:12]:12} cfiles={len(c['cfiles'])}")
        print(f"total: {len(cases)}")
        return

    print(f"{'CVE':18} {'STATUS':22} {'entries':>7} {'warns':>5}")
    print("-" * 60)
    summ = []
    for c in cases:
        cve, status, ne, nw, d = process(c)
        print(f"{cve:18} {status:22} {ne:>7} {nw:>5}", flush=True)
        summ.append({"cve": cve, "status": status, "entries": ne, "warns": nw, "dump": d})
    json.dump(summ, open(f"{DUMP}/_run_summary.json", "w"), indent=2)
    nz = sum(1 for s in summ if s["warns"] > 0)
    print("-" * 60)
    print(f"done: {len(summ)} cases, {nz} with >=1 warning. summary -> {DUMP}/_run_summary.json")


if __name__ == "__main__":
    main()
