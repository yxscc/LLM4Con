#include "LLMUtil/DetectorAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/LSAnalysis.h"
#include "CPG/Node.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <set>
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace llm_client {

DetectorAgent::DetectorAgent(std::shared_ptr<LLMClient> client, CCPG* ccpg)
    : Conversation(client, "", 100), ccpg_(ccpg)
{
    set_system_prompt(build_system_prompt());
}

std::string DetectorAgent::build_system_prompt() {
    return R"(You are an expert concurrency bug detector for C/C++ kernel modules. You receive a **variable-centric vulnerability surface** listing shared objects accessed by concurrent threads, with access types, lock protection, and risk flags.

**Your mission**: Investigate high-risk shared objects and propose **bug hypotheses**. Each hypothesis is verified by a static constraint engine that checks your conditions — you get instant feedback.

## How the Vulnerability Surface Works

Each shared object entry shows:
- All threads that access it, and how (Read / Write / Free)
- Whether each access is lock-protected, and by which lock
- Risk flags: [UAF_RISK] = free + use across threads; [UNPROTECTED_WRITE] = writes without locks; [INCONSISTENT_LOCK] = different locks across threads

## Verification Predicates

You can compose hypotheses from these 6 static analysis predicates:

| Predicate | Arguments | Meaning |
|-----------|-----------|---------|
| `in_thread` | `node` (role name or node_id), `thread` (thread_id) | Node belongs to the specified thread |
| `may_run_concurrently` | `thread1`, `thread2` (thread_ids) | The two threads can execute in parallel |
| `reachable` | `from` (role/id), `to` (role/id) | Intra-thread control flow path exists from→to |
| `not_lock_protected` | `node` (role/id) | The node is NOT inside any lock-protected region |
| `same_lock` | `node1` (role/id), `node2` (role/id) | Two lock-acquisition nodes refer to the same lock |
| `alias` | `node1` (role/id), `node2` (role/id) | Memory operations at these nodes access the same object |

In constraint args, you can use either:
- A **role name** (string) that references a key in your `nodes` map, e.g. `"check"`
- A **direct node_id** (integer), e.g. `549`

## Your Workflow

1. Call `get_vulnerability_surface` to see all shared objects and risk profiles.
2. Focus on highest risk_score objects (especially [UAF_RISK] and [UNPROTECTED_WRITE]).
3. Use `get_object_details` for full access details including node IDs.
4. Use `get_function_code` or `get_function_ops` to read actual source code.
5. Use `get_successors_chunked` to trace control flow and locate exact operation nodes.
6. Call `propose_hypothesis` with your hypothesis — you get instant constraint verification feedback.
7. If some constraints fail, adjust node IDs or constraints and retry.
8. Call `finish_detection` when done.

## Example Hypothesis (for reference only — you are NOT limited to these patterns)

**TOCTOU example**: Thread A checks `pool->free == NULL` (node 549), then uses `pool->free` (node 558). Concurrently, Thread B modifies `pool->free = cell` (node 596) without the pool lock.

```json
{
  "hypothesis_id": "pool_free_toctou",
  "description": "Check-then-use race on pool->free: Thread A checks NULL, Thread B mutates, Thread A uses stale result",
  "bug_category": "TOCTOU",
  "severity": "high",
  "nodes": {"check": 549, "modify": 596, "use": 558},
  "constraints": [
    {"predicate": "in_thread", "args": {"node": "check", "thread": 0}},
    {"predicate": "in_thread", "args": {"node": "use", "thread": 0}},
    {"predicate": "in_thread", "args": {"node": "modify", "thread": 1}},
    {"predicate": "may_run_concurrently", "args": {"thread1": 0, "thread2": 1}},
    {"predicate": "reachable", "args": {"from": "check", "to": "use"}},
    {"predicate": "not_lock_protected", "args": {"node": "modify"}}
  ]
}
```

**You can freely invent new bug categories** (e.g. "refcount_race", "signal_handler_race", "inconsistent_lock_protocol") and use any combination of predicates. The only requirement is that each constraint must use one of the 6 predicates above.

## Quality over quantity

You are evaluated on **signal**, not volume. A single well-verified bug
hypothesis is worth far more than ten plausible-looking variants of the
same issue. Concretely:

- **Stop at 5 confirmed hypotheses per run unless you have a genuinely
  new bug mechanism to propose.** Call `finish_detection` once the
  remaining surface offers only incremental variations of what you have
  already reported.
- Do NOT propose multiple hypotheses that differ only in the role names
  or that touch the same set of node IDs. The backend deduplicates by
  `(bug_category, sorted node-id set)` and will reject near-duplicates;
  if you see a `"dedupe"` field in the feedback, move on to a different
  shared object instead of re-proposing.
- `not_lock_protected` must mean "the LockSet at this node is empty"
  (i.e. no lock is held at all). If the only concern is that two
  threads hold *different* locks for the same object, use
  `same_lock` against a specific acquire node instead.
- Prefer UAF/free-after-race hypotheses backed by actual `[UAF_RISK]`
  flags and Write+Read cross-thread evidence over speculative
  `INCONSISTENT_LOCK` flags.

## Structural requirements for multi-site bugs

A `double_free`, `use_after_free`/`uaf`, `TOCTOU`, or `data_race`
hypothesis **must** reference at least TWO **distinct** CCPG node IDs
across its roles. Two threads passing through the *same* source line is
one event per thread, not two independent events on an object — so
`{"free_a": 6037, "free_b": 6037}` is not a double-free and will be
rejected with `"error": "structural_rejection"`. Pick two different
program points (e.g. one free in the error-cleanup path and another
free in the commit path, or a write site in Thread A and a separate
read site in Thread B).

## CRITICAL RULES

- You MUST call tools only. DO NOT output chat text.
- Always investigate [UAF_RISK] objects first.
- Use `get_function_ops` to find specific node IDs.
- The `propose_hypothesis` tool runs constraint verification internally and gives you instant pass/fail feedback per constraint.
- If verification fails, read the failure details and adjust — do NOT just give up.
- On `is_duplicate: true`, skip to a different object rather than retrying.
)";
}

std::vector<Tool> DetectorAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back({"get_vulnerability_surface",
        "Returns the full variable-centric vulnerability surface report.", {}});

    tools.push_back({"get_object_details",
        "Get full details for a shared object by 1-based index in the vulnerability surface.",
        {{"object_index", "number", "1-based index of the shared object.", true}}});

    tools.push_back({"get_function_code",
        "Get the source code of a function by name.",
        {{"name", "string", "The function name.", true}}});

    tools.push_back({"get_lock_protection",
        "Check if a specific CCPG node is protected by a lock.",
        {{"node_id", "number", "The CCPG node ID to check.", true}}});

    tools.push_back({"check_reachability",
        "Check if there is a control flow path from one node to another within the same thread.",
        {{"from_node_id", "number", "Source node ID.", true},
         {"to_node_id", "number", "Target node ID.", true}}});

    tools.push_back({"get_successors_chunked",
        "Get successor nodes in control flow via BFS from a starting node.",
        {{"node_id", "number", "The starting node ID.", true},
         {"chunk_size", "number", "Max nodes to return (default 15).", false}}});

    {
        std::vector<Parameter> hyp_params;
        hyp_params.emplace_back("hypothesis_id", "string", "A unique name for this hypothesis.", true);
        hyp_params.emplace_back("description", "string", "Natural language description of the bug.", true);
        hyp_params.emplace_back("bug_category", "string", "Free-form bug category (e.g. 'TOCTOU', 'refcount_race', 'data_race').", true);
        hyp_params.emplace_back("severity", "string", "high, medium, or low.", true);

        nlohmann::json nodes_schema;
        nodes_schema["type"] = "object";
        nodes_schema["description"] = "Map of role names to CCPG node IDs, e.g. {\"check\": 549, \"use\": 558}.";
        hyp_params.emplace_back("nodes", std::move(nodes_schema), true);

        nlohmann::json pred_prop;
        pred_prop["type"] = "string";
        pred_prop["description"] = "One of: in_thread, may_run_concurrently, reachable, not_lock_protected, same_lock, alias";

        nlohmann::json args_prop;
        args_prop["type"] = "object";
        args_prop["description"] = "Arguments for the predicate, e.g. {\"from\": \"check\", \"to\": \"use\"}";

        nlohmann::json item_schema;
        item_schema["type"] = "object";
        item_schema["properties"]["predicate"] = pred_prop;
        item_schema["properties"]["args"] = args_prop;
        item_schema["required"] = nlohmann::json::array({"predicate", "args"});

        nlohmann::json constraints_schema;
        constraints_schema["type"] = "array";
        constraints_schema["description"] = "Array of constraint objects for static verification.";
        constraints_schema["items"] = item_schema;
        hyp_params.emplace_back("constraints", std::move(constraints_schema), true);

        tools.push_back({"propose_hypothesis",
            "Propose a bug hypothesis with open-form constraints for immediate static verification. "
            "Returns per-constraint pass/fail feedback.",
            std::move(hyp_params)});
    }

    tools.push_back({"finish_detection",
        "Call when you have finished investigating all high-risk patterns.", {}});

    return tools;
}

std::string DetectorAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) return *shared_result;

    auto* ctx = static_cast<DetectorContext*>(this->get_context_for_tools());
    if (!ctx) return R"({"error": "Internal context error."})";

    if (tool_name == "get_vulnerability_surface") {
        return ctx->surface->toPromptString();
    }

    if (tool_name == "get_object_details") {
        int idx = arguments.at("object_index").get<int>();
        if (idx < 1 || idx > (int)ctx->surface->shared_objects.size()) {
            return R"({"error": "Invalid object_index. Must be 1-)" +
                   std::to_string(ctx->surface->shared_objects.size()) + R"(."})";
        }
        const auto& obj = ctx->surface->shared_objects[idx - 1];
        nlohmann::json result;
        result["name"] = obj.name;
        result["type"] = obj.type;
        result["risk_score"] = obj.risk_score;
        result["flags"] = nlohmann::json::object();
        result["flags"]["uaf_risk"] = obj.has_free_operation;
        result["flags"]["unprotected_write"] = obj.has_unprotected_write;
        result["flags"]["cross_thread_rw"] = obj.has_cross_thread_rw;
        result["flags"]["inconsistent_lock"] = obj.has_inconsistent_locking;

        result["accesses"] = nlohmann::json::array();
        for (const auto& a : obj.accesses) {
            result["accesses"].push_back({
                {"thread_id", a.thread_id},
                {"function", a.function_name},
                {"function_id", a.function_id},
                {"access_type", a.access_type},
                {"node_id", a.node_id},
                {"code", a.code_snippet},
                {"location", a.location},
                {"lock_protected", a.is_lock_protected},
                {"protecting_lock", a.protecting_lock}
            });
        }
        return result.dump();
    }

    if (tool_name == "get_function_code") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node*> nodes = ccpg_->getCPG()->findMethodsByName(name);
        if (nodes.empty()) {
            CPGNodeSet all_methods = ccpg_->getCPG()->getNodesByType("Method");
            for (Node* m : all_methods) {
                if (m->getName().find(name) != std::string::npos &&
                    m->getProperty("CODE") != "<empty>") {
                    nodes.insert(m);
                }
            }
        }
        nlohmann::json result = nlohmann::json::array();
        for (Node* node : nodes) {
            CCPGNode* ccpgNode = ccpg_->getCCPGNodeByCPGNode(node);
            ccpg::Function* func = ccpgNode ? ccpgNode->getFunction() : nullptr;
            if (func) {
                result.push_back({
                    {"function_id", func->getId()},
                    {"function_name", func->getFuncNode()->getCPGNode()->getName()},
                    {"function_body", func->getFuncNode()->getCPGNode()->getCode()}
                });
            }
        }
        if (result.empty()) return R"({"error": "Function not found: )" + name + R"("})";
        return result.dump();
    }

    if (tool_name == "get_lock_protection") {
        int node_id = arguments.at("node_id").get<int>();
        CCPGNode* node = ccpg_->getNodeByID(node_id);
        if (!node) return R"({"error": "Node not found."})";

        // Use the already-computed path-sensitive LockSets instead of the
        // "same file, line window" heuristic. For each reachable call
        // context, report the set of locks held. The response explicitly
        // flags unprotected contexts so the LLM can reason about data
        // races that only occur on some call paths.
        LSAnalysis* ls = LSAnalysis::getInstance();
        ccpg::Function* fn = node->getFunction();
        ccpg::ContextSet ctxs;
        if (fn) ctxs = fn->getContextSet();
        NodeLoc loc = node->getNodeLoc();

        nlohmann::json result;
        result["node_id"] = node_id;
        result["code"] = node->getCPGNode()->getCode();

        auto lockToCode = [](Lock* l) -> std::string {
            if (!l) return "?";
            CCPGNode* acq = l->getAcquire();
            return (acq && acq->getCPGNode()) ? acq->getCPGNode()->getCode() : "?";
        };

        nlohmann::json ctxArr = nlohmann::json::array();
        std::set<int> unionLockIds;
        std::vector<std::string> unionLockNames;
        bool anyUnprotected = false;

        auto addContextEntry = [&](const Context& c) {
            auto lockSet = ls->getLockSet(loc, c);
            nlohmann::json entry;
            entry["context"] = c.toString();
            nlohmann::json locks = nlohmann::json::array();
            for (Lock* l : lockSet) {
                if (!l) continue;
                std::string code = lockToCode(l);
                locks.push_back(code);
                if (unionLockIds.insert(l->getId()).second) {
                    unionLockNames.push_back(code);
                }
            }
            entry["locks_held"] = locks;
            entry["is_protected"] = !lockSet.empty();
            if (lockSet.empty()) anyUnprotected = true;
            ctxArr.push_back(std::move(entry));
        };

        if (ctxs.empty()) {
            addContextEntry(Context());
        } else {
            for (Context* ctx : ctxs) {
                addContextEntry(ctx ? *ctx : Context());
            }
        }

        result["contexts"] = ctxArr;
        result["all_locks_union"] = unionLockNames;
        // "Effectively unprotected" iff there's at least one call path with
        // an empty LockSet - matches the semantics used by the verifier.
        result["is_protected"] = !anyUnprotected && !unionLockNames.empty();
        result["has_unprotected_path"] = anyUnprotected;
        return result.dump();
    }

    if (tool_name == "check_reachability") {
        int from_id = arguments.at("from_node_id").get<int>();
        int to_id = arguments.at("to_node_id").get<int>();
        CCPGNode* from = ccpg_->getNodeByID(from_id);
        CCPGNode* to = ccpg_->getNodeByID(to_id);
        if (!from || !to) return R"({"error": "Node(s) not found."})";

        // BFS reachability
        std::queue<CCPGNode*> worklist;
        std::set<CCPGNode*> visited;
        worklist.push(from);
        visited.insert(from);
        bool found = false;
        while (!worklist.empty() && !found) {
            CCPGNode* current = worklist.front();
            worklist.pop();
            for (CCPGEdge* edge : current->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* next = edge->getDst();
                    if (next == to) { found = true; break; }
                    if (!visited.count(next)) {
                        visited.insert(next);
                        worklist.push(next);
                    }
                }
            }
        }
        return nlohmann::json{
            {"from_node_id", from_id},
            {"to_node_id", to_id},
            {"reachable", found}
        }.dump();
    }

    if (tool_name == "get_successors_chunked") {
        int start_node_id = arguments.at("node_id").get<int>();
        int chunk_size = arguments.value("chunk_size", 15);
        CCPGNode* start_node = ccpg_->getNodeByID(start_node_id);
        if (!start_node) return R"({"error": "Start node not found."})";

        nlohmann::json successors = nlohmann::json::array();
        std::queue<CCPGNode*> worklist;
        std::set<CCPGNode*> visited;
        worklist.push(start_node);
        visited.insert(start_node);

        while (!worklist.empty() && successors.size() < static_cast<size_t>(chunk_size)) {
            CCPGNode* current = worklist.front();
            worklist.pop();
            for (CCPGEdge* edge : current->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* successor = edge->getDst();
                    if (!visited.count(successor)) {
                        visited.insert(successor);
                        worklist.push(successor);
                        if (successors.size() < static_cast<size_t>(chunk_size)) {
                            successors.push_back({
                                {"node_id", successor->getId()},
                                {"code", successor->getCPGNode()->getCode()},
                                {"location", successor->getNodeLoc().toString()}
                            });
                        }
                    }
                }
            }
        }
        return successors.dump();
    }

    if (tool_name == "propose_hypothesis") {
        if (!ctx->verifier) return R"({"error": "Verifier not initialized."})";

        query::Hypothesis h;
        h.id = arguments.at("hypothesis_id").get<std::string>();
        h.description = arguments.at("description").get<std::string>();
        h.bug_category = arguments.at("bug_category").get<std::string>();
        h.severity = arguments.value("severity", "medium");

        if (!arguments.contains("nodes") || !arguments["nodes"].is_object()) {
            return R"({"error": "nodes must be an object mapping role names to node IDs."})";
        }
        for (auto& [role, val] : arguments["nodes"].items()) {
            if (!val.is_number_integer()) {
                return R"({"error": "Node ID for role ')" + role + R"(' must be an integer."})";
            }
            int nid = val.get<int>();
            if (!ccpg_->getNodeByID(nid)) {
                return R"({"error": "Node ID )" + std::to_string(nid) + R"( not found in CCPG."})";
            }
            h.nodes[role] = nid;
        }

        if (!arguments.contains("constraints") || !arguments["constraints"].is_array()) {
            return R"({"error": "constraints must be an array of {predicate, args} objects."})";
        }
        for (const auto& c : arguments["constraints"]) {
            query::VerificationConstraint vc;
            vc.predicate = c.value("predicate", "");
            vc.args = c.value("args", nlohmann::json::object());
            if (vc.predicate.empty()) {
                return R"({"error": "Each constraint must have a 'predicate' field."})";
            }
            h.constraints.push_back(std::move(vc));
        }

        // Structural sanity: a bug whose definition requires ≥2 distinct
        // program points (double-free, UAF, TOCTOU, data-race) cannot be
        // expressed by collapsing every role onto the *same* CCPG node.
        // Two threads passing through one source-code location is just
        // one event per thread, not two freestanding events on the same
        // object. Reject up-front so the LLM is forced to refine instead
        // of padding the confirmed-list with pseudo bugs.
        auto bc_lower = h.bug_category;
        std::transform(bc_lower.begin(), bc_lower.end(), bc_lower.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        const bool requires_two_sites =
            (bc_lower == "double_free" || bc_lower == "double-free" ||
             bc_lower == "uaf" || bc_lower == "use_after_free" ||
             bc_lower == "use-after-free" ||
             bc_lower == "toctou" || bc_lower == "time_of_check" ||
             bc_lower == "data_race" || bc_lower == "data-race");
        if (requires_two_sites && h.nodes.size() >= 2) {
            std::unordered_set<int> uniq_nodes;
            for (const auto& [role, nid] : h.nodes) uniq_nodes.insert(nid);
            if (uniq_nodes.size() < 2) {
                nlohmann::json err;
                err["error"] = "structural_rejection";
                err["reason"] =
                    "All roles in this '" + h.bug_category +
                    "' hypothesis map to the SAME CCPG node id. A " +
                    h.bug_category + " needs at least two DISTINCT program "
                    "points (e.g. two different free call sites, or a "
                    "write and a separate read). Two threads passing "
                    "through one source-code line is one event per "
                    "thread, not a bug. Refine the hypothesis with "
                    "distinct node ids for each role, or choose a "
                    "different bug_category.";
                return err.dump();
            }
        }

        auto result = ctx->verifier->verify(h);
        auto feedback = result.toFeedbackJson();

        if (result.all_satisfied) {
            // Phase 4 dedupe. Build a canonical fingerprint from bug
            // category + the sorted set of node ids touched by the
            // hypothesis. LLMs often re-propose structurally identical
            // hypotheses with only a renamed role ("check" -> "chk"),
            // which inflates the downstream count without new signal.
            std::vector<int> node_ids;
            node_ids.reserve(h.nodes.size());
            for (const auto& [role, nid] : h.nodes) node_ids.push_back(nid);
            std::sort(node_ids.begin(), node_ids.end());
            std::string fingerprint = h.bug_category + "|";
            for (std::size_t i = 0; i < node_ids.size(); ++i) {
                if (i > 0) fingerprint += ",";
                fingerprint += std::to_string(node_ids[i]);
            }
            if (!ctx->accepted_fingerprints.insert(fingerprint).second) {
                feedback["dedupe"] = "Duplicate of an earlier accepted hypothesis"
                    " with the same (bug_category, node-id set). Skipped from "
                    "confirmed list. Propose something substantively different "
                    "or call finish_detection.";
                feedback["is_duplicate"] = true;
            } else {
                ctx->confirmed_hypotheses.push_back(std::move(h));
            }
        }

        return feedback.dump();
    }

    if (tool_name == "finish_detection") {
        return "finish";
    }

    return R"({"error": "Unknown tool: )" + tool_name + R"("})";
}

DetectorAgent::DetectionResult DetectorAgent::runDetection(const query::VulnerabilitySurface& surface) {
    reset();

    query::HypothesisVerifier verifier(ccpg_, ThreadCreationTree::getInstance());

    DetectorContext ctx;
    ctx.surface = &surface;
    ctx.ccpg = ccpg_;
    ctx.tct = ThreadCreationTree::getInstance();
    ctx.verifier = &verifier;

    std::string prompt =
        "A pointer-analysis-based vulnerability surface has been computed for a concurrent C/C++ module. "
        "It is organized around **shared objects** — each entry shows one memory object and ALL threads "
        "that access it, with access types (Read/Write/Free), lock protection status, and risk flags.\n\n"
        "Start by calling `get_vulnerability_surface` to see the full report. "
        "Focus on objects with the highest risk_score (especially those marked [UAF_RISK]). "
        "Use `get_object_details` for full node IDs, then read code with `get_function_code` or "
        "`get_function_ops`, and propose hypotheses with `propose_hypothesis`.\n\n"
        "Begin now.";

    send_message(prompt, &ctx);

    DetectionResult result;
    result.confirmed = std::move(ctx.confirmed_hypotheses);
    return result;
}

std::string DetectorAgent::parseResult(const std::vector<ChatMessage>& history) {
    return "Detection complete.";
}

} // namespace llm_client
