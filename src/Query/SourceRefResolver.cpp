#include "Query/SourceRefResolver.h"

#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CPG/CPG.h"
#include "CPG/Node.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

namespace query {

namespace {

// Collapse runs of whitespace to single spaces and trim, so snippet matching
// is robust to formatting differences between the LLM's citation and the
// node's stored code.
std::string normalizeWs(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool inSpace = false;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            inSpace = true;
            continue;
        }
        if (inSpace && !out.empty()) out.push_back(' ');
        inSpace = false;
        out.push_back(c);
    }
    return out;
}

bool snippetMatches(const std::string& code, const std::string& snippet) {
    if (snippet.empty()) return false;
    std::string nc = normalizeWs(code);
    std::string ns = normalizeWs(snippet);
    if (ns.empty()) return false;
    return nc.find(ns) != std::string::npos;
}

// Collect the CCPG functions whose name matches `symbol`. Uses the CPG method
// index (exact, then the CPG's own fuzzy fallback already lives in
// findMethodsByName callers; here we keep it exact to avoid over-broadening).
std::vector<ccpg::Function*> functionsByName(CCPG* ccpg, const std::string& symbol) {
    std::vector<ccpg::Function*> result;
    if (!ccpg || !ccpg->getCPG() || symbol.empty()) return result;
    std::unordered_set<Node*> methods = ccpg->getCPG()->findMethodsByName(symbol);
    std::unordered_set<ccpg::Function*> seen;
    for (Node* m : methods) {
        if (!m) continue;
        CCPGNode* cn = ccpg->getCCPGNodeByCPGNode(m);
        ccpg::Function* fn = cn ? cn->getFunction() : nullptr;
        if (fn && seen.insert(fn).second) result.push_back(fn);
    }
    return result;
}

} // namespace

int resolveSourceRef(CCPG* ccpg, const SourceRef& ref) {
    if (!ccpg) return -1;
    // Require at least one discriminator; a bare reference is unresolvable.
    if (ref.symbol.empty() && ref.line < 0 && ref.snippet.empty()) return -1;

    // Build the candidate node set. Prefer narrowing to the enclosing
    // function; only fall back to a full scan when the symbol is unknown.
    std::vector<CCPGNode*> candidates;
    std::vector<ccpg::Function*> fns = functionsByName(ccpg, ref.symbol);
    if (!fns.empty()) {
        for (ccpg::Function* fn : fns) {
            for (CCPGNode* n : fn->getNodes()) candidates.push_back(n);
        }
    } else {
        // No symbol match: fall back to all nodes (bounded by line/snippet
        // scoring below). This path is rare and runs at most once per cited
        // reference, so the linear scan is acceptable.
        for (CCPGNode* n : ccpg->getNodes()) candidates.push_back(n);
    }

    const int kExactLineBonus = 100;
    const int kNearLineCap = 40;     // decays by 1 per line of distance
    const int kSnippetBonus = 60;
    const int kFileBonus = 20;

    int bestId = -1;
    int bestScore = 0;
    int bestLineDist = std::numeric_limits<int>::max();

    for (CCPGNode* n : candidates) {
        if (!n || !n->getCPGNode()) continue;
        const std::string& code = n->getCPGNode()->getCode();
        if (code.empty() || code == "<empty>") continue;

        const NodeLoc& loc = n->getNodeLoc();
        int nodeLine = loc.getLineNumber();
        int lineDist = std::numeric_limits<int>::max();

        int score = 0;
        if (ref.line >= 0 && nodeLine > 0) {
            lineDist = std::abs(nodeLine - ref.line);
            if (lineDist == 0) {
                score += kExactLineBonus;
            } else {
                score += std::max(0, kNearLineCap - lineDist);
            }
        }
        if (snippetMatches(code, ref.snippet)) score += kSnippetBonus;
        if (!ref.file.empty() &&
            NodeLoc::fileNamesMatch(loc.getBaseFileName(), ref.file)) {
            score += kFileBonus;
        }

        if (score <= 0) continue;

        // Pick the highest score; break ties by nearest line, then smallest
        // id for determinism.
        if (score > bestScore ||
            (score == bestScore && lineDist < bestLineDist) ||
            (score == bestScore && lineDist == bestLineDist &&
             (bestId < 0 || n->getId() < bestId))) {
            bestScore = score;
            bestLineDist = lineDist;
            bestId = n->getId();
        }
    }

    return bestId;
}

} // namespace query
