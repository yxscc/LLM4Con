#!/usr/bin/env python3
"""Score the L2 run using ONLY each case's newest dump (the current run's).

eval_recall.py picks the newest dump that HAS stateful_bugs/bugs.txt, which
silently falls back to a STALE previous-run dump when the current run produced
no bugs.txt (0 hypotheses). That contaminates recall/precision. This scorer
takes the strictly-newest dump per case and treats a missing/empty bugs.txt as
a genuine 0-bug miss.

Usage: python3 score_l2_run.py [--after YYYY-MM-DD] [CASE ...]
"""
import os, re, sys, glob, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import eval_recall as ER  # reuse gt_tokens / STOP

DUMP = ER.DUMP

def parse_bugs_newest(case, after_ts):
    dirs = sorted(glob.glob(os.path.join(DUMP, case + "_*")), key=os.path.getmtime,
                  reverse=True)
    if not dirs:
        return None, None, None
    d = dirs[0]                      # strictly newest = current run
    mtime = os.path.getmtime(d)
    fresh = (after_ts is None) or (mtime >= after_ts)
    bf = os.path.join(d, "stateful_bugs", "bugs.txt")
    txt = open(bf, errors="ignore").read() if os.path.exists(bf) else ""
    bugs = []
    for block in txt.split("Mechanism-Rule Violation Detected")[1:]:
        mech = re.search(r"Mechanism:\s*(.+)", block)
        nodes = re.search(r"Involved Nodes ---(.+?)---", block, re.S)
        fields = set()
        if nodes:
            for line in nodes.group(1).splitlines():
                code = line.split("):", 1)[1] if "):" in line else line
                code = code.split("[mlx_devbox")[0]
                for t in re.findall(r"[A-Za-z_]\w{2,}", code):
                    tl = t.lower()
                    if tl not in ER.STOP:
                        fields.add(tl)
        bugs.append({"mech": (mech.group(1).strip() if mech else "?")[:60],
                     "fields": fields})
    stamp = os.path.basename(d).split(case + "_")[-1]
    return bugs, stamp, fresh

def main():
    args = sys.argv[1:]
    after_ts = None
    if args and args[0] == "--after":
        after_ts = time.mktime(time.strptime(args[1], "%Y-%m-%d"))
        args = args[2:]
    cases = args
    if not cases:
        cases = sorted({os.path.basename(p).rsplit("_", 6)[0].split("_20")[0]
                        for p in glob.glob(os.path.join(DUMP, "CVE-*")) +
                                 glob.glob(os.path.join(DUMP, "SYZBOT-*"))})
    n_recall = n_bugs = n_fp = n_cases = n_clean = n_stale = 0
    print(f"{'CASE':24s} {'GTfield':20s} {'#bug':4s} {'GT?':4s} {'stamp':21s} matched|others")
    print("-" * 110)
    for c in sorted(cases):
        primary, toks = ER.gt_tokens(c)
        bugs, stamp, fresh = parse_bugs_newest(c, after_ts)
        if bugs is None:
            print(f"{c:24s} {'-':20s} {'--':4s} {'NODUMP':4s}")
            continue
        n_cases += 1
        if not fresh:
            n_stale += 1
        match_toks = {t for t in toks if t not in ER.STOP and len(t) >= 3}
        matched, others = [], []
        for b in bugs:
            bf = {f for f in b["fields"] if f not in ER.STOP and len(f) >= 3}
            hit = (primary and primary not in ER.STOP and primary in bf) or (match_toks & bf)
            (matched if hit else others).append(b)
        recalled = len(matched) > 0
        n_recall += recalled
        if not bugs:
            n_clean += 1
        n_bugs += len(bugs)
        n_fp += len(others)
        flag = "" if fresh else "  <STALE!>"
        ml = matched[0]["mech"][:30] if matched else "-"
        ol = "; ".join(b["mech"][:22] for b in others[:2])
        print(f"{c:24s} {primary[:20]:20s} {len(bugs):<4d} "
              f"{('YES' if recalled else ('---' if bugs else 'CLN')):4s} {stamp:21s} {ml} | {ol}{flag}")
    print("-" * 110)
    print(f"cases={n_cases}  recalled={n_recall} ({100*n_recall/max(1,n_cases):.0f}%)  "
          f"clean/miss={n_clean}  total_bugs={n_bugs}  FP(nonGT)={n_fp}  "
          f"precision~={100*(n_bugs-n_fp)/max(1,n_bugs):.0f}%  stale_dumps={n_stale}")

if __name__ == "__main__":
    main()
