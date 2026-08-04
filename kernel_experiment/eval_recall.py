#!/usr/bin/env python3
"""Score recall & precision of the static-composition detector against GT.

For each case we compare the detector's CONFIRMED bugs (LLM_dump/<CASE>_*/
stateful_bugs/bugs.txt) against the ground-truth racing object recorded in
expected_contract.json (shared_object) and ground_truth.json.

Recall  = the GT object/field is named by at least one confirmed bug's
          "Involved Nodes" (object-level match, NOT just "FOUND").
Precision proxy = (# bugs matching GT) / (# confirmed bugs). Non-GT bugs are
          counted as false positives w.r.t. THIS CVE (caveat: a few may be
          genuine unrelated races; flagged for manual spot-check).

Usage: python3 eval_recall.py [CASE ...]     (no args -> every case with a dump)
"""
import json, os, re, sys, glob

HERE = os.path.dirname(os.path.abspath(__file__))
DUMP = os.path.join(os.path.dirname(HERE), "LLM_dump")

# Generic tokens that must never be used as the discriminating GT field.
STOP = {
    "struct", "union", "list", "head", "node", "data", "ptr", "val", "obj",
    "field", "lock", "mutex", "spinlock", "rwlock", "entry", "state", "count",
    "counter", "buffer", "buf", "flags", "flag", "len", "id", "nr", "num",
    "the", "and", "for", "via", "int", "u32", "u64", "size", "base", "info",
    # common-but-non-discriminating identifiers
    "dev", "task", "work", "skb", "sock", "msg", "req", "dst", "src", "addr",
    "cmd", "page", "dma", "set", "map", "queue", "member", "nodes", "type",
    "types", "global", "reaches", "released", "owned", "membership", "field",
    "governing", "linkage", "device", "value", "index", "offset", "table",
    "backing", "cookie", "conn", "timer",
}

def gt_tokens(case):
    """Return (primary_field, all_tokens) extracted from GT annotations."""
    toks, primary = set(), ""
    ecp = os.path.join(HERE, case, "expected_contract.json")
    if os.path.exists(ecp):
        try:
            so = json.load(open(ecp)).get("shared_object", "")
        except Exception:
            so = ""
        # Prefer the most specific `->field` / `.field` mention.
        m = re.findall(r"[A-Za-z_]\w*\s*(?:->|\.)\s*([A-Za-z_]\w*)", so)
        if m:
            primary = m[-1]
        ids = re.findall(r"[A-Za-z_]\w{2,}", so)
        for t in ids:
            tl = t.lower()
            if tl not in STOP and not tl.startswith("struct"):
                toks.add(tl)
        if not primary:
            # fall back to the longest identifier
            cand = sorted((t for t in toks), key=len, reverse=True)
            primary = cand[0] if cand else ""
    return primary.lower(), toks

def parse_bugs(case):
    """Return list of dicts: {mech, severity, fields:set, verdict}."""
    dirs = sorted(glob.glob(os.path.join(DUMP, case + "_*")), key=os.path.getmtime,
                  reverse=True)
    if not dirs:
        return None, None
    # Pick the newest dump that actually ran the LLM detection phase (has a
    # bugs.txt). Static-only / early-exit runs create empty dump dirs that would
    # otherwise shadow the real detection result.
    d = None
    for cand in dirs:
        if os.path.exists(os.path.join(cand, "stateful_bugs", "bugs.txt")):
            d = cand
            break
    if d is None:
        return os.path.basename(dirs[0]), []
    bf = os.path.join(d, "stateful_bugs", "bugs.txt")
    txt = open(bf, errors="ignore").read()
    bugs = []
    for block in txt.split("Mechanism-Rule Violation Detected")[1:]:
        mech = re.search(r"Mechanism:\s*(.+)", block)
        sev = re.search(r"severity:\s*(\w+)", block)
        # collect field identifiers from Involved Nodes code snippets
        nodes = re.search(r"Involved Nodes ---(.+?)---", block, re.S)
        fields = set()
        if nodes:
            for line in nodes.group(1).splitlines():
                # code after the "): "
                code = line.split("):", 1)[1] if "):" in line else line
                # Drop the trailing "[<path>:<line>]" location marker; keeping
                # it would feed path components into the token match.
                code = re.split(r"\s*\[[^\]]*:\d+\]", code)[0]
                for t in re.findall(r"[A-Za-z_]\w{2,}", code):
                    tl = t.lower()
                    if tl not in STOP:
                        fields.add(tl)
        bugs.append({
            "mech": (mech.group(1).strip() if mech else "?")[:60],
            "sev": sev.group(1) if sev else "?",
            "fields": fields,
        })
    return os.path.basename(d), bugs

def main():
    tbl = json.load(open(os.path.join(HERE, "dataset_entrypoints.json")))
    cases = sys.argv[1:]
    if not cases:
        cases = sorted({os.path.basename(p).rsplit("_", 6)[0].split("_20")[0]
                        for p in glob.glob(os.path.join(DUMP, "CVE-*")) +
                                 glob.glob(os.path.join(DUMP, "SYZBOT-*"))})
    n_recall = n_bugs_total = n_fp_total = n_cases = n_clean = 0
    print(f"{'CASE':24s} {'GTfield':22s} {'#bug':4s} {'GT?':4s} matched / others")
    print("-" * 100)
    for c in sorted(cases):
        primary, toks = gt_tokens(c)
        dump, bugs = parse_bugs(c)
        if bugs is None:
            continue
        n_cases += 1
        match_toks = {t for t in toks if t not in STOP and len(t) >= 3}
        matched, others = [], []
        for b in bugs:
            bf = {f for f in b["fields"] if f not in STOP and len(f) >= 3}
            hit = (primary and primary not in STOP and primary in bf) or (match_toks & bf)
            (matched if hit else others).append(b)
        recalled = len(matched) > 0
        if recalled:
            n_recall += 1
        if not bugs:
            n_clean += 1
        n_bugs_total += len(bugs)
        n_fp_total += len(others)
        sr = "SR" if tbl.get(c, {}).get("self_race") else ""
        mlabel = matched[0]["mech"][:34] if matched else "-"
        olabel = "; ".join(b["mech"][:26] for b in others[:2])
        print(f"{c:24s} {primary[:22]:22s} {len(bugs):<4d} "
              f"{('YES' if recalled else ('---' if bugs else 'CLN')):4s} "
              f"{mlabel} | {olabel} {sr}")
    print("-" * 100)
    print(f"cases={n_cases}  recalled={n_recall} ({100*n_recall/max(1,n_cases):.0f}%)  "
          f"clean/miss={n_clean}  total_bugs={n_bugs_total}  "
          f"nonGT_bugs(FP)={n_fp_total}  "
          f"precision~={100*(n_bugs_total-n_fp_total)/max(1,n_bugs_total):.0f}%")

if __name__ == "__main__":
    main()
