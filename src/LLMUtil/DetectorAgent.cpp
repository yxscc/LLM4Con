#include "LLMUtil/DetectorAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/LSAnalysis.h"
#include "CCPG/HBGraph.h"
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
- Risk flags: [UAF_RISK] = free + use across threads; [UNPROTECTED_WRITE] = writes without locks; [INCONSISTENT_LOCK] = different locks across threads; [SCALAR_TORN_ACCESS] = scalar field with mixed READ_ONCE/plain access; [READ_DOMINATED_LONE_WRITER] = many readers + one writer (typical READ_ONCE candidate); [MISSING_ATOMIC_ANNOTATION] = atomic-like field but write path uses plain stores

## Verification Predicates (M7 happens-before DSL)

The verifier accepts a focused **5+3 predicate vocabulary** that maps every common concurrency bug to one of three judgement templates. You can also still use the legacy 6 predicates listed at the bottom for backward compatibility, but prefer the new ones.

### Primitives (5)

| Predicate | Arguments | Meaning |
|-----------|-----------|---------|
| `same_location` | `a`, `b` (role/id) | The two nodes operate on the same memory cell (field-level alias). |
| `op_kind` | `node`, `kind` ∈ {`READ`,`WRITE`,`RMW`,`CALL`} | The node's IR operation matches the requested kind. |
| `in_thread` | `node`, `thread` (thread_id) | The node executes inside the given thread. |
| `reachable` | `from`, `to` | Intra-thread CFG path from→to (cross-function BFS, depth ≤ 8). |
| `hb` | `a`, `b`, optional `expected` (default `true`) | The synchronization graph contains a happens-before chain a→b. Set `"expected": false` to assert the *absence* of an hb chain (used by UAF / NULL-deref templates). |

### Sugars (3) — verifier expands them internally

| Sugar | Definition |
|---|---|
| `conflicts(a, b)` | `same_location(a, b) ∧ (op_kind(a)∈{WRITE,RMW} ∨ op_kind(b)∈{WRITE,RMW})` |
| `concurrent(a, b)` | `¬hb(a, b) ∧ ¬hb(b, a)` |
| `unsafe_atomic_block(start, end, witness)` | `reachable(start, end) ∧ conflicts(witness, start∨end) ∧ ¬hb(witness, start) ∧ ¬hb(end, witness)` |

In constraint args, you can use either a **role name** (string) keyed in your `nodes` map, e.g. `"check"`, or a **direct node_id** (integer), e.g. `549`.

## Three Bug-Family Templates (toolbox, not a checklist)

The verifier accepts *any* well-formed combination of the predicates above. The three templates below are common, well-tested *shapes* that cover most concurrency bugs in real kernel patches — use them when they fit, mix them when needed, or invent your own constraint set when none of them captures what you see.

> **`bug_category` is FREE-FORM**. The downstream LLM-judge evaluates whether your hypothesis matches the patch's *root cause* (i.e. the same field, same threads, same flow), not whether you used a particular bug_category string or template shape. So if a use-after-free is more naturally expressed as `data_race` on the freed pointer, that is fine — the judge will still credit it as a hit. **Do not** distort your description just to fit a label.

### Template 1 — Concurrent conflict (covers most plain data races, missing lock, missing BH-disable, publish-race)
```json
{"predicate": "conflicts",  "args": {"a": "writer", "b": "reader"}},
{"predicate": "concurrent", "args": {"a": "writer", "b": "reader"}}
```

### Template 2 — Directional hb-violation (UAF / lifetime / NULL-deref where one side is the "release" event)
```json
{"predicate": "conflicts", "args": {"a": "use",  "b": "free"}},
{"predicate": "hb",        "args": {"a": "use",  "b": "free", "expected": false}}
```
The `expected: false` is the bug condition: "use is NOT forced to happen-before free". Useful when you can clearly identify the freeing / NULLing / disabling event and want to assert that some thread can still observe the prior state.

### Template 3 — Unsafe atomic block (TOCTOU / non-atomic RMW / non-atomic bit-ops)
```json
{"predicate": "unsafe_atomic_block",
 "args": {"start": "check", "end": "use", "witness": "modify"}}
```
For non-atomic RMW: pick `start` = the load, `end` = the store, `witness` = the conflicting store in another thread.

> When in doubt, Template 1 is the safest fallback — `conflicts ∧ concurrent` correctly characterises *every* concurrency bug at its core (it just doesn't carry the directional information that Templates 2 and 3 add).

## Few-shot Hypotheses (one example per template; the order is illustrative, not prescriptive)

### F1 — plain data race (CVE-2024-40953-like) — Template 1
```json
{
  "hypothesis_id": "boost_field_torn_access",
  "description": "Thread T0 writes kvm->last_boosted_vcpu without atomic; Thread T1 reads it without atomic.",
  "bug_category": "data_race",
  "severity": "high",
  "nodes": {"writer": 412, "reader": 718},
  "constraints": [
    {"predicate": "in_thread",  "args": {"node": "writer", "thread": 0}},
    {"predicate": "in_thread",  "args": {"node": "reader", "thread": 1}},
    {"predicate": "conflicts",  "args": {"a": "writer", "b": "reader"}},
    {"predicate": "concurrent", "args": {"a": "writer", "b": "reader"}}
  ]
}
```

### F5 — use-after-free (CVE-2024-43891-like) — Template 2
```json
{
  "hypothesis_id": "port_use_after_free",
  "description": "T1 dereferences port->addr while T0 has freed port via kfree() with no synchronizing lock between the two.",
  "bug_category": "use_after_free",
  "severity": "high",
  "nodes": {"use": 21, "free": 121},
  "constraints": [
    {"predicate": "in_thread",  "args": {"node": "use",  "thread": 1}},
    {"predicate": "in_thread",  "args": {"node": "free", "thread": 0}},
    {"predicate": "op_kind",    "args": {"node": "free", "kind": "CALL"}},
    {"predicate": "conflicts",  "args": {"a": "use", "b": "free"}},
    {"predicate": "hb",         "args": {"a": "use", "b": "free", "expected": false}}
  ]
}
```

### F6/F7 — TOCTOU / non-atomic RMW (CVE-2025-38217-like) — Template 3
```json
{
  "hypothesis_id": "fb_rmw_lost_update",
  "description": "T0 loads counter at L1 then stores L1+1 at L2. T1 stores its own counter+1 between L1 and L2; T0's update is lost.",
  "bug_category": "atomicity_break",
  "severity": "medium",
  "nodes": {"start": 305, "end": 309, "witness": 612},
  "constraints": [
    {"predicate": "in_thread",          "args": {"node": "start",   "thread": 0}},
    {"predicate": "in_thread",          "args": {"node": "end",     "thread": 0}},
    {"predicate": "in_thread",          "args": {"node": "witness", "thread": 1}},
    {"predicate": "unsafe_atomic_block","args": {"start": "start", "end": "end", "witness": "witness"}}
  ]
}
```

## Workflow

1. Call `get_vulnerability_surface` to see all shared objects and risk profiles.
2. Focus on highest risk_score objects (especially `[UAF_RISK]`, `[UNPROTECTED_WRITE]`, `[SCALAR_TORN_ACCESS]`, `[LIFECYCLE_FLAG_CANDIDATE]`).
3. Use `get_object_details` for full access details including node IDs.
4. Use `get_function_code` or `get_function_ops` to read actual source code.
5. Use `get_successors_chunked` to trace control flow and locate exact operation nodes.
6. Decide which of the 3 templates matches the patch behaviour.
7. Call `propose_hypothesis` — you get instant pass/fail per constraint.
8. If a constraint fails, read the detail and adjust node IDs or template (do NOT just retry the same thing). Common pitfalls:
   - `same_location FAILED` → check that you picked the IR access on the *same field*, not a sibling field.
   - `hb=true expected=false` → use is actually ordered after free; pick another use site.
   - `concurrent: hb(a,b)=T` → there *is* a synchronization chain between a and b; pick uses across truly independent threads.
9. Call `finish_detection` when the surface offers no genuinely new mechanism.

## Quality over quantity

You are evaluated on **signal**, not volume. A single well-targeted hypothesis that names the *patch's actual fix* is worth more than ten plausible variants on the same shared object.

- The backend deduplicates by `(bug_category, sorted node-id set)`. On `is_duplicate: true`, jump to a *different* shared object — do not re-propose.
- The hypothesis budget is **adaptive to surface size**: small surface (≤5 objects) → ≤8 hypotheses; medium (6-15) → ≤12; large (>15) → ≤20. Stop earlier if remaining surface only offers incremental variants.
- Multi-site bugs (`double_free`, `use_after_free`, `TOCTOU`, `data_race`, `atomicity_break`) **must** reference at least TWO distinct CCPG node IDs across roles. `{"free_a": 6037, "free_b": 6037}` is one event, not two — it will be rejected with `"error": "structural_rejection"`.

## Legacy predicates (still accepted, prefer the new ones above)

`may_run_concurrently(thread1, thread2)`, `not_lock_protected(node)`, `same_lock(node1, node2)`, `alias(node1, node2)`. These remain valid for backward compatibility but produce coarser results than `concurrent` / `hb` / `same_location` from the new vocabulary.

## CRITICAL RULES

- You MUST call tools only. DO NOT output chat text.
- Always investigate `[UAF_RISK]` and `[SCALAR_TORN_ACCESS]` objects first.
- Use `get_function_ops` to find specific node IDs.
- The `propose_hypothesis` tool runs constraint verification internally and gives you instant pass/fail feedback per constraint.
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
        pred_prop["description"] =
            "Verification predicate. Prefer the M7 happens-before DSL: "
            "primitives = same_location, op_kind, in_thread, reachable, hb; "
            "sugars = conflicts, concurrent, unsafe_atomic_block. "
            "Legacy (still accepted, coarser): may_run_concurrently, "
            "not_lock_protected, same_lock, alias.";

        nlohmann::json args_prop;
        args_prop["type"] = "object";
        args_prop["description"] =
            "Arguments for the predicate. Prefer {a, b} for binary predicates "
            "(same_location/conflicts/concurrent/hb), {node, kind} for op_kind, "
            "{node, thread} for in_thread, {from, to} for reachable, and "
            "{start, end, witness} for unsafe_atomic_block. The hb predicate "
            "additionally accepts \"expected\": true|false (default true) — "
            "set to false to assert the *absence* of a happens-before chain.";

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

    query::HypothesisVerifier verifier(ccpg_, ThreadCreationTree::getInstance(),
                                       HBGraph::getInstance());

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
