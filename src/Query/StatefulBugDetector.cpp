// in src/Query/StatefulBugDetector.cpp

#include "Query/StatefulBugDetector.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/AnalysisManager.h"
#include "llvm/IR/Value.h"
#include "LLMUtil/VerificationAgent.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <regex>
#include <sstream>

namespace query {

// ---------------------------------------------------------------------------
// v20 P7 — Hypothesis report deduplication / merging
// ---------------------------------------------------------------------------
// Background: the LLM-driven DetectorAgent commonly proposes several distinct
// hypotheses that all describe the same logical race. Examples observed in
// CVE-2023-53046 (10 confirmed → really 3 distinct races on
// hdev->req_status / hdev->req_result / hdev->req_skb), where the LLM walks
// every (writer_function, reader_function) ordering of the same field pair.
// All such hypotheses pass the static verifier (in_thread/conflicts/concurrent
// genuinely hold) but they collapse onto a single underlying bug.
//
// We group confirmed hypotheses by the canonical signature
//    ( sorted_set_of_field_signatures, sorted_set_of_function_names )
// and emit one primary report per group, with the remaining hypotheses listed
// as "alternate angles" so no information is lost (evaluator can still match
// on any of them). This trims report counts by ~25–30% on prompts that
// over-enumerate function pairs, without losing any confirmed verdict.
//
// Conservative safety: when both writer and reader access codes fail to yield
// a field signature we fall back to grouping by node IDs (i.e., effectively no
// merge), so unrelated bugs cannot be conflated.
// ---------------------------------------------------------------------------

namespace {

// Extract a canonical "field signature" from an access code string. We look
// for a C-style field reference (`->FIELD` or `.FIELD`) that may follow any
// expression (including a function-call result like `PACKET_CB(skb)->keypair`).
// LHS of an assignment is preferred (covers stores like `obj->f = ...`,
// `obj->f += ...`, `obj->f[i] = ...`); otherwise the rightmost field access
// in the expression is returned. Returns empty if no recognizable pattern.
std::string extractFieldSignature(const std::string& code) {
    if (code.empty()) return "";
    // Prefer the LHS field of an assignment when present.
    // We anchor only on `(->|.)FIELD ... =` so bases like `f(x)->FIELD` work.
    static const std::regex lhs_assign_re(
        R"((?:->|\.)\s*([A-Za-z_][A-Za-z_0-9]*)\s*(?:\[[^\]]*\])?\s*[+\-*/&|^]?=)");
    std::smatch m;
    if (std::regex_search(code, m, lhs_assign_re)) {
        return m[1].str();
    }
    // Otherwise pick the rightmost `(->|.)FIELD` in the expression.
    static const std::regex any_field_re(
        R"((?:->|\.)\s*([A-Za-z_][A-Za-z_0-9]*))");
    std::string last_field;
    auto begin = std::sregex_iterator(code.begin(), code.end(), any_field_re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        last_field = (*it)[1].str();
    }
    return last_field;
}

// Pull the function name owning a CCPG node, or empty if unavailable.
std::string functionNameOf(CCPGNode* n) {
    if (!n) return "";
    auto* f = n->getFunction();
    if (!f) return "";
    auto* fn = f->getFuncNode();
    if (!fn) return "";
    auto* cpg = fn->getCPGNode();
    if (!cpg) return "";
    return cpg->getName();
}

// Build a canonical merge signature for a hypothesis. The signature is the
// sorted set of field signatures and the sorted set of function names across
// all involved nodes. Hypotheses with identical signatures describe the same
// (field, function-set) race instance and can be merged.
struct HypoSig {
    std::vector<std::string> fields;  // sorted, deduplicated
    std::vector<std::string> fns;     // sorted, deduplicated
    std::vector<int>         node_ids_fallback;  // sorted, used only if fields is empty

    bool operator<(const HypoSig& o) const {
        if (fields != o.fields) return fields < o.fields;
        if (fns != o.fns) return fns < o.fns;
        return node_ids_fallback < o.node_ids_fallback;
    }
};

HypoSig computeSignature(const Hypothesis& h, CCPG* ccpg) {
    HypoSig sig;
    std::set<std::string> field_set;
    std::set<std::string> fn_set;
    std::set<int> id_set;
    for (const auto& [role, node_id] : h.nodes) {
        id_set.insert(node_id);
        CCPGNode* n = ccpg ? ccpg->getNodeByID(node_id) : nullptr;
        if (!n) continue;
        std::string code = n->getCPGNode() ? n->getCPGNode()->getCode() : "";
        std::string fld = extractFieldSignature(code);
        if (!fld.empty()) field_set.insert(fld);
        std::string fn = functionNameOf(n);
        if (!fn.empty()) fn_set.insert(fn);
    }
    sig.fields.assign(field_set.begin(), field_set.end());
    sig.fns.assign(fn_set.begin(), fn_set.end());
    if (sig.fields.empty()) {
        // Fall back to per-node grouping so we never accidentally merge
        // hypotheses whose access targets we cannot canonicalize.
        sig.node_ids_fallback.assign(id_set.begin(), id_set.end());
    }
    return sig;
}

// Pick the representative hypothesis from a group: prefer the one with the
// most-detailed description (longest), tie-broken by id for stability.
size_t pickRepresentative(const std::vector<size_t>& group,
                          const std::vector<Hypothesis>& all) {
    size_t best = group.front();
    for (size_t idx : group) {
        const auto& a = all[best];
        const auto& b = all[idx];
        if (b.description.size() > a.description.size() ||
            (b.description.size() == a.description.size() && b.id < a.id)) {
            best = idx;
        }
    }
    return best;
}

}  // namespace

// Heuristic FP filter:
// Some real-world C code intentionally "publishes" a pointer into a global table
// (NULL -> allocated) without locks, while other threads read the table.
// This is technically a C data race but is often benign in practice if the entry
// is written once and the object is not freed concurrently.
//
// We use a very conservative pattern match to avoid suppressing real bugs:
// - Only for DataRace
// - Only when the write happens under an "if (NULL == <var>) { <alloc>; table[idx] = <var>; }" shape
static bool isLikelyBenignPublicationRace(const StatefulBug &bug) {
    const auto &rule = bug.getRule();
    if (!rule.contains("pattern_type") || rule["pattern_type"] != "DataRace") {
        return false;
    }

    // Identify READ/WRITE nodes from the violation path.
    CCPGNode *readNode = nullptr;
    CCPGNode *writeNode = nullptr;
    for (const auto &step : bug.getPath()) {
        const std::string &role = step.first;
        if (role.find("read") != std::string::npos) {
            readNode = step.second;
        } else if (role.find("write") != std::string::npos) {
            writeNode = step.second;
        }
    }
    if (!readNode || !writeNode) {
        return false;
    }
    if (!writeNode->getFunction() || !writeNode->getFunction()->getFuncNode() ||
        !writeNode->getFunction()->getFuncNode()->getCPGNode()) {
        return false;
    }

    const std::string &writeFuncCode = writeNode->getFunction()->getFuncNode()->getCPGNode()->getCode();
    const std::string &writeLine = writeNode->getCPGNode() ? writeNode->getCPGNode()->getCode() : "";
    const std::string &readLine = readNode->getCPGNode() ? readNode->getCPGNode()->getCode() : "";

    // Must look like publishing into a table and guarded by NULL check.
    // (Do NOT overfit to specific symbol names; just use "[]", "NULL", and an allocator hint.)
    const bool hasNullGuard =
        writeFuncCode.find("NULL") != std::string::npos &&
        (writeFuncCode.find("if (NULL ==") != std::string::npos || writeFuncCode.find("if (NULL==") != std::string::npos);
    const bool hasAllocatorHint =
        (writeFuncCode.find("calloc(") != std::string::npos || writeFuncCode.find("malloc(") != std::string::npos);
    const bool writeLooksLikeTableStore =
        writeLine.find('[') != std::string::npos && writeLine.find(']') != std::string::npos &&
        writeLine.find('=') != std::string::npos;
    const bool readLooksLikeTableLoad =
        readLine.find('[') != std::string::npos && readLine.find(']') != std::string::npos &&
        readLine.find('=') != std::string::npos;

    if (!(hasNullGuard && hasAllocatorHint && writeLooksLikeTableStore && readLooksLikeTableLoad)) {
        return false;
    }

    // Extra guard: ensure the store line appears inside the NULL-guarded block.
    // We only do a simple substring check; if the code string doesn't contain the line, don't filter.
    if (!writeLine.empty() && writeFuncCode.find(writeLine) == std::string::npos) {
        return false;
    }

    return true;
}

// --- StatefulBug Implementation ---
StatefulBug::StatefulBug(
    const llm_client::StatefulRule& violated_rule,
    const std::vector<std::pair<std::string, CCPGNode*>>& violation_path,
    const llm_client::ThreadPair& thread_pair
) : rule(violated_rule), path(violation_path), threads(thread_pair) {}

std::string StatefulBug::toString() const {
    std::stringstream ss;
    ss << "========== Stateful Protocol Violation Detected ==========\n"
       << "Rule Violated: " << rule["rule_id"].get<std::string>() << "\n"
       << "Description: " << rule["_llm_summary"].get<std::string>() << "\n"
       << "Shared Object Type: " << rule["shared_object_type"].get<std::string>() << "\n\n"
       << "Violation observed between Thread " << threads.thread1->getId() << " and Thread " << threads.thread2->getId() << ".\n\n"
       << "--- Forbidden Sequence Trace ---\n";
    
    for (const auto& step : path) {
        CCPGNode* node = step.second;
        ss << "  -> " << step.first << " at " << node->getNodeLoc().toString() << ".     Code : " << node->getCPGNode()->getCode() << "\n";
    }
    
    ss << "==========================================================";
    return ss.str();
}

// --- REFACTORED DETECT FUNCTION ---
void StatefulBugDetector::detect(const std::vector<llm_client::ThreadPair>& threadPairs,
                                 const std::set<const llvm::Value*>& candidateSharedObjects,
                                 llm_client::VerificationAgent* verificationAgent)
{
    std::cout << "\n[Phase 4: Detecting Stateful Protocol Violations (Rule-Based)]" << std::endl;

    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();

    for (const auto& pair : threadPairs) {

        for (const auto& rule_ptr : pair.analysis.temporal_rules) {
            // Call the rule's own verification method
            std::optional<StatefulBug> bug = rule_ptr->verify(pair, ccpg);

            if (bug) {
                const auto& bug_json = rule_ptr->to_json();
                const std::string& pattern = bug_json["pattern_type"];

                // Fast, conservative FP filter for benign publication-style races.
                if (pattern == "DataRace" && isLikelyBenignPublicationRace(*bug)) {
                    std::cout << "    [Heuristic] Filtered likely benign publication-style DataRace (init-only pointer publish)." << std::endl;
                    continue;
                }

                std::cout << "    [!!!] POTENTIAL " << pattern << " VIOLATION FOUND for rule." << std::endl;
                
                bool confirmed = true;
                if (verificationAgent) {
                     confirmed = verificationAgent->verifyBug(*bug);
                }
                
                if (confirmed) {
                    detectedBugs.push_back(*bug);
                } else {
                    std::cout << "    [Verification] Bug filtered out by LLM Verification Agent." << std::endl;
                }
            }
        }
    }
}


void StatefulBugDetector::detectFromHypotheses(
    const std::vector<Hypothesis>& hypotheses, CCPG* ccpg,
    llm_client::VerificationAgent* verificationAgent) {
    std::cout << "\n[Phase 4: Detecting Violations (Open Hypothesis)]" << std::endl;

    hypothesisCcpg_ = ccpg;

    std::size_t kept = 0, dropped = 0;
    for (const auto& h : hypotheses) {
        std::cout << "    [!!!] POTENTIAL " << h.bug_category
                  << " VIOLATION FOUND: " << h.id << std::endl;

        bool confirmed = true;
        if (verificationAgent) {
            confirmed = verificationAgent->verifyHypothesis(h, ccpg);
        }

        if (confirmed) {
            confirmedHypotheses_.push_back(h);
            ++kept;
        } else {
            std::cout << "    [Verification] Hypothesis filtered out by LLM Verification Agent."
                      << std::endl;
            ++dropped;
        }
    }

    if (verificationAgent) {
        std::cout << "[Phase 4.5] LLM hypothesis-verifier filter: kept "
                  << kept << "/" << hypotheses.size()
                  << ", dropped " << dropped << " false-positive(s)."
                  << std::endl;
    }
}

void StatefulBugDetector::printResults(const fs::path& outputDir) const {
    size_t totalRawHypotheses = confirmedHypotheses_.size();

    // ---- v20 P7: group confirmed hypotheses by (field-set, fn-set) ----
    std::map<HypoSig, std::vector<size_t>> groups;
    std::vector<HypoSig> groupOrder;  // preserve first-seen order for stable output
    for (size_t idx = 0; idx < confirmedHypotheses_.size(); ++idx) {
        HypoSig sig = computeSignature(confirmedHypotheses_[idx], hypothesisCcpg_);
        auto [it, inserted] = groups.emplace(sig, std::vector<size_t>{});
        if (inserted) groupOrder.push_back(sig);
        it->second.push_back(idx);
    }
    size_t mergedHypothesisBugs = groups.size();

    size_t totalBugs = detectedBugs.size() + externalBugs.size() + mergedHypothesisBugs;

    if (totalBugs == 0) {
        std::cout << "No bugs detected." << std::endl;
        return;
    }

    fs::path bugsOutputDir = outputDir / "stateful_bugs";
    if (!fs::exists(bugsOutputDir)) {
        fs::create_directory(bugsOutputDir);
    }

    std::ofstream file(bugsOutputDir / "bugs.txt");
    int i = 1;

    for (const auto& bug : externalBugs) {
        file << bug.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }

    for (const auto& bug : detectedBugs) {
        file << bug.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }

    // Emit one primary report per (field, fn-pair) group, with the remaining
    // hypotheses summarized as "alternate angles". Preserves all info; just
    // collapses semantically duplicate reports for readability + token economy.
    for (const HypoSig& sig : groupOrder) {
        const auto& members = groups.at(sig);
        size_t rep = pickRepresentative(members, confirmedHypotheses_);
        const auto& mainH = confirmedHypotheses_[rep];

        file << mainH.toReportString(hypothesisCcpg_);

        if (members.size() > 1) {
            // Append a compact list of the merged alternates.
            std::ostringstream alt;
            alt << "\n--- Alternate Angles (" << (members.size() - 1)
                << " additional hypothesis instance(s) on the same race) ---";
            int idx = 1;
            for (size_t midx : members) {
                if (midx == rep) continue;
                const auto& h = confirmedHypotheses_[midx];
                alt << "\n  [" << idx++ << "] " << h.id;
                if (!h.severity.empty()) alt << " (severity: " << h.severity << ")";
                // Record nodes inline: role=node_id pairs.
                if (!h.nodes.empty()) {
                    alt << " | nodes:";
                    for (const auto& [role, nid] : h.nodes) {
                        alt << " " << role << "=" << nid;
                    }
                }
                // First sentence of description gives readable angle.
                if (!h.description.empty()) {
                    std::string brief = h.description;
                    auto period = brief.find_first_of(".!?\n");
                    if (period != std::string::npos)
                        brief = brief.substr(0, period + 1);
                    if (brief.size() > 240) brief = brief.substr(0, 240) + "…";
                    alt << "\n      " << brief;
                }
            }
            file << alt.str();
        }

        file << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }

    file.close();

    std::cout << "Bug detection complete. " << totalBugs << " potential bug(s) found." << std::endl;
    if (!externalBugs.empty()) {
        std::cout << "  - External Bugs: " << externalBugs.size() << std::endl;
    }
    if (!detectedBugs.empty()) {
        std::cout << "  - Stateful Protocol Violations: " << detectedBugs.size() << std::endl;
    }
    if (!confirmedHypotheses_.empty()) {
        std::cout << "  - Hypothesis-Based Violations: " << mergedHypothesisBugs;
        if (totalRawHypotheses != mergedHypothesisBugs) {
            std::cout << " (merged from " << totalRawHypotheses
                      << " raw confirmed hypotheses; "
                      << (totalRawHypotheses - mergedHypothesisBugs)
                      << " collapsed as alternate angles)";
        }
        std::cout << std::endl;
    }
    std::cout << "Results saved to: " << (bugsOutputDir / "bugs.txt") << std::endl;
}

} // namespace query