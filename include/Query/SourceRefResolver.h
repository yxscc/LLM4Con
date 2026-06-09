#pragma once

#include <string>

class CCPG;

namespace query {

// A source reference as cited by the LLM in the thread-contract (legacy)
// interleaving agent. Instead of forcing the model to plumb opaque CCPG node
// IDs, it cites a racing access by human-readable coordinates; we ground that
// back to a CCPG node here.
struct SourceRef {
    std::string file;     // base filename (optional; may be empty)
    int line = -1;        // 1-based source line (optional; -1 if unknown)
    std::string symbol;   // enclosing function name (STRONGLY recommended)
    std::string snippet;  // a substring of the racing line/expression (optional)
};

// Triple-locating resolver: enclosing-function symbol + line + code snippet,
// with nearest-neighbor line matching so it survives inlining / macro-expansion
// line drift. Returns the best-matching CCPG node id, or -1 if nothing
// sufficiently plausible is found.
//
// Soundness note: the result still flows into HypothesisVerifier's
// deterministic predicates, so a wrong/fuzzy match degrades to a failed
// verification (dropped hypothesis), never a fabricated bug.
int resolveSourceRef(CCPG* ccpg, const SourceRef& ref);

} // namespace query
