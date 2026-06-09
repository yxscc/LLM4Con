#pragma once

#include "LLMUtil/Conversation.h"
#include "Query/VulnerabilitySurfaceGenerator.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace llm_client {

// One-shot, read-tool-free triage over the static shared-object list, run BEFORE
// the expensive per-object interleaving sessions (thread-contract / "old story"
// entry only). It lets the model drop objects that cannot carry a real
// concurrency bug -- chiefly pure benign statistics/diagnostic counters (KCSAN
// lost-update races whose value never flows into a pointer/index/size/lifetime
// decision) and opaque unnamed objects -- so we neither analyze nor report them.
//
// Fail-open by construction: any parse/LLM failure keeps ALL objects, and objects
// carrying lifecycle signals (free / list mutation / self-race) are force-kept
// regardless of the model's verdict, so triage can only ever drop the low-value
// long tail, never sacrifice recall.
class ObjectTriageAgent : public Conversation {
public:
    explicit ObjectTriageAgent(std::shared_ptr<LLMClient> client);

    // Returns the set of indices into surface.shared_objects worth analyzing.
    std::set<int> selectObjects(const query::VulnerabilitySurface& surface);

private:
    std::vector<Tool> get_available_tools() const override;
    std::string get_tool_choice() const override { return "required"; }
    std::string execute_tool(const std::string& tool_name,
                             const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>&) override { return ""; }
    static std::string build_system_prompt();
};

struct TriageContext {
    int num_objects = 0;
    std::set<int> analyze;   // indices the model chose to keep
    bool got = false;
};

} // namespace llm_client
