#pragma once

// Manual thread-entry configuration.
//
// Rationale: automatic kernel entry-point discovery (EXPORT_SYMBOL / fops /
// notifier / syscall heuristics) is necessarily an over-approximation -- it
// promotes dozens of functions to "concurrent threads", which inflates the
// shared-object surface, blows the per-session budgets, and buries the real
// racing object. Concurrency analysis legitimately takes "which execution
// contexts run concurrently" as an analyst-provided input (this is the norm
// for race detectors). This config lets us optionally restrict the thread
// roots to an explicitly provided set.
//
// IMPORTANT scientific guardrail: we only configure thread *entry functions*
// (the concurrent contexts). We never configure the racing object/field --
// the detector must still discover the shared object and the harmful
// interleaving on its own.
//
// Sources (first non-empty wins):
//   LACE_ENTRYPOINTS_FILE : path to a text file, one root per line (commas
//                           also allowed; '#'-comments and blanks ignored)
//   LACE_ENTRYPOINTS      : comma-separated list of root function names
//
// When enabled, CCPG::build() bypasses all automatic entry discovery and uses
// only the configured roots; ThreadCreationTree honors them as mutually (and
// self-) concurrent kernel-entry threads.

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace manualentry {

// Normalize a kernel entry function name to a comparable "core" token:
// drop demangled signature/namespace, lowercase, strip syscall/ABI
// decorations and leading underscores. e.g.
//   "SyS_ioprio_get"        -> "ioprio_get"
//   "__x64_sys_keyctl"      -> "keyctl"
//   "exit_io_context"       -> "exit_io_context"
inline std::string normalizeCore(std::string n) {
    std::size_t paren = n.find('(');
    if (paren != std::string::npos) n = n.substr(0, paren);
    std::size_t ns = n.rfind("::");
    if (ns != std::string::npos) n = n.substr(ns + 2);
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const char* const kPrefixes[] = {
        "__x64_sys_", "__x32_sys_", "__ia32_sys_", "__arm64_sys_",
        "__arm_sys_", "__mips_sys_", "__riscv_sys_", "__s390x_sys_",
        "__s390_sys_", "__powerpc64_sys_", "__powerpc_sys_",
        "__se_compat_sys_", "__se_sys_", "__do_compat_sys_", "__do_sys_",
        "__sys_", "compat_sys_", "sys_", "syscall_"};

    bool changed = true;
    while (changed) {
        changed = false;
        for (const char* pfx : kPrefixes) {
            std::size_t l = std::strlen(pfx);
            if (n.size() > l && n.compare(0, l, pfx) == 0) {
                n = n.substr(l);
                changed = true;
                break;
            }
        }
    }
    std::size_t u = 0;
    while (u < n.size() && n[u] == '_') ++u;
    return n.substr(u);
}

struct Config {
    bool enabled = false;
    bool selfRace = false;           // analyst declared this case self-racing
    std::vector<std::string> roots;  // raw configured names (deduped, in order)
    std::set<std::string> cores;     // normalized cores for matching
};

namespace detail {

inline std::string trim(const std::string& s) {
    std::size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    std::size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

inline void splitInto(const std::string& raw, std::vector<std::string>& out) {
    // Split on BOTH ',' and '/'. Dataset entry configs sometimes group several
    // interchangeable role-entry functions on one line as "a / b / c" (e.g. the
    // set of sysfs show/store callbacks that all reach the same shared field).
    // LACE_ENTRYPOINTS joins roots with ',', so without also splitting '/' the
    // whole "a / b / c" string is treated as a single (unmatchable) function
    // name and every entry after the first is silently dropped -- which erased
    // the entire second concurrent thread role (e.g. the sysfs readers of
    // trigger_data in CVE-2024-43830) and left only one thread. '/' cannot
    // appear in a C identifier, so splitting on it is unambiguous.
    std::string norm = raw;
    for (char& ch : norm) if (ch == '/') ch = ',';
    std::stringstream ss(norm);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        std::string t = trim(tok);
        if (!t.empty() && t[0] != '#') out.push_back(t);
    }
}

inline Config load() {
    Config c;
    std::vector<std::string> toks;

    if (const char* f = std::getenv("LACE_ENTRYPOINTS_FILE")) {
        std::ifstream in(f);
        std::string line;
        while (std::getline(in, line)) {
            std::string t = trim(line);
            if (t.empty() || t[0] == '#') continue;
            splitInto(t, toks);
        }
    }
    if (toks.empty()) {
        if (const char* e = std::getenv("LACE_ENTRYPOINTS")) splitInto(e, toks);
    }

    if (!toks.empty()) {
        c.enabled = true;
        std::set<std::string> seen;
        for (const auto& t : toks) {
            if (seen.insert(t).second) c.roots.push_back(t);
            c.cores.insert(normalizeCore(t));
        }
    }
    if (const char* sr = std::getenv("LACE_SELF_RACE")) {
        std::string v = trim(sr);
        c.selfRace = (v == "1" || v == "true" || v == "TRUE" || v == "yes");
    }
    return c;
}

}  // namespace detail

inline const Config& get() {
    static const Config cfg = detail::load();
    return cfg;
}

inline bool enabled() { return get().enabled; }

// True when the analyst declared this manual-entry case as self-racing
// (dataset self_race=true). Gates self-race modeling that goes beyond the
// conservative default (run-once entries racing themselves, and a configured
// root racing itself while a child thread also touches the object).
inline bool selfRaceDeclared() { return get().enabled && get().selfRace; }

// True if a (possibly decorated) function name corresponds to one of the
// configured thread roots.
inline bool matches(const std::string& fnName) {
    const Config& c = get();
    if (!c.enabled || fnName.empty()) return false;
    return c.cores.count(normalizeCore(fnName)) > 0;
}

}  // namespace manualentry
