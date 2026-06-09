#include "LLMUtil/ObjectTriageAgent.h"

#include <iostream>
#include <sstream>

namespace llm_client {

ObjectTriageAgent::ObjectTriageAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, "", /*max_history=*/8) {
    set_system_prompt(build_system_prompt());
}

std::string ObjectTriageAgent::build_system_prompt() {
    return R"TRIAGE(You are triaging a list of shared objects BEFORE an expensive per-object
concurrency interleaving analysis. For each object you decide: is it worth a full
analysis session, or can it be skipped because it cannot carry a real concurrency bug?

SKIP an object only when it is clearly low-value:
  - a pure statistics / diagnostic counter whose value never flows into a pointer,
    index, size, branch, or lifetime decision (e.g. packet/byte counters, *_dropped,
    *_packets, *_bytes, net_device_stats.*) -- these are benign lost-update races;
  - an opaque, unnamed object (e.g. obj:val@0x...) with no field identity and no
    write conflict, that you cannot reason about.

ANALYZE (keep) everything that could carry a use-after-free, double-free,
uninitialized/publish-before-init, ordering, or CONSEQUENTIAL atomicity bug:
anything with a free or list/pointer mutation, a lifecycle/state/flag field that gates
access, a pointer or handle, or a racy value that is later used consequentially.

Be conservative: when unsure, KEEP. It is far worse to skip a real bug than to spend a
little extra analysis. Call submit_triage exactly once.)TRIAGE";
}

std::vector<Tool> ObjectTriageAgent::get_available_tools() const {
    std::vector<Parameter> params;
    nlohmann::json int_array = {{"type", "array"}, {"items", {{"type", "integer"}}}};

    params.emplace_back("analyze", int_array,
                        /*required=*/true);
    // `skip` is advisory (for the log); the keep set is derived from `analyze`.
    nlohmann::json skip_items = {
        {"type", "array"},
        {"items", {
            {"type", "object"},
            {"properties", {
                {"index", {{"type", "integer"}}},
                {"reason", {{"type", "string"}, {"description", "one or two words"}}}
            }},
            {"required", {"index"}}
        }}
    };
    params.emplace_back("skip", std::move(skip_items), /*required=*/false);

    return {{"submit_triage",
             "Submit the triage decision once: `analyze` = indices to analyze, "
             "`skip` = indices to drop (benign/low-value) with a short reason.",
             std::move(params)}};
}

std::string ObjectTriageAgent::execute_tool(const std::string& tool_name,
                                            const nlohmann::json& arguments) {
    if (tool_name != "submit_triage")
        return R"({"error":"unknown tool"})";

    auto* ctx = static_cast<TriageContext*>(this->get_context_for_tools());
    if (!ctx) return R"({"error":"no context"})";

    if (arguments.contains("analyze") && arguments["analyze"].is_array()) {
        for (const auto& v : arguments["analyze"]) {
            if (v.is_number_integer()) {
                int i = v.get<int>();
                if (i >= 0 && i < ctx->num_objects) ctx->analyze.insert(i);
            }
        }
    }
    ctx->got = true;
    return "finish";
}

std::set<int> ObjectTriageAgent::selectObjects(const query::VulnerabilitySurface& surface) {
    const int n = static_cast<int>(surface.shared_objects.size());
    std::set<int> keepAll;
    for (int i = 0; i < n; ++i) keepAll.insert(i);
    if (n == 0) return keepAll;

    // Build a compact, read-only object table.
    std::stringstream ps;
    ps << "Triage these " << n << " shared objects (one per line). "
          "Decide which deserve a full interleaving analysis.\n"
          "flags: W=unprotected_write F=free X=cross_thread_rw L=list_mutation "
          "T=torn_scalar S=self_race\n\n";
    for (int i = 0; i < n; ++i) {
        const auto& o = surface.shared_objects[i];
        ps << "[" << i << "] " << (o.name.empty() ? "<anon>" : o.name);
        if (!o.type.empty()) ps << " : " << o.type;
        ps << " | risk=" << o.risk_score
           << " threads=" << o.accessing_thread_ids.size()
           << " accesses=" << o.accesses.size() << " flags=";
        if (o.has_unprotected_write) ps << "W";
        if (o.has_free_operation)    ps << "F";
        if (o.has_cross_thread_rw)   ps << "X";
        if (o.has_list_mutation)     ps << "L";
        if (o.has_scalar_torn_access)ps << "T";
        if (o.is_self_race)          ps << "S";
        ps << "\n";
    }
    ps << "\nCall submit_triage once with `analyze` (indices to analyze) and `skip` "
          "(benign/low-value indices + short reason).";

    TriageContext ctx;
    ctx.num_objects = n;

    set_token_budget(24000);
    pin_next_user_message();
    try {
        send_message(ps.str(), &ctx);
    } catch (const std::exception& e) {
        std::cerr << "[ObjectTriageAgent] error: " << e.what()
                  << " -- keeping all objects." << std::endl;
        return keepAll;
    }

    // Fail-open: if the model gave nothing usable, analyze everything.
    if (!ctx.got || ctx.analyze.empty()) return keepAll;

    // Recall guardrail: force-keep lifecycle carriers regardless of the verdict.
    // free / list mutation / self-race are exactly where UAF, double-free and
    // structure-corruption bugs hide; a benign statistics counter has none.
    std::set<int> keep = ctx.analyze;
    for (int i = 0; i < n; ++i) {
        const auto& o = surface.shared_objects[i];
        if (o.has_free_operation || o.has_list_mutation || o.is_self_race)
            keep.insert(i);
    }
    return keep;
}

} // namespace llm_client
