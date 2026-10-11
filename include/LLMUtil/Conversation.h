#pragma once
#include "LLMUtil/LLMClient.h"
#include <fstream>
namespace llm_client {
// Full SDK history is continued between extraction and review. No pruning,
// synthetic summaries, message-count limits or alternate hand-written loop.
class Conversation {
public:
    Conversation(std::shared_ptr<LLMClient> client, std::string instructions);
    virtual ~Conversation() = default;
    std::string send_message(const std::string& input);
    void set_max_turns(int n);
protected:
    virtual std::vector<Tool> get_available_tools() const = 0;
    virtual std::string get_tool_choice() const { return "required"; }
    virtual std::string execute_tool(const std::string&, const nlohmann::json&) = 0;
private:
    std::shared_ptr<LLMClient> client_;
    std::string instructions_;
    nlohmann::json history_ = nlohmann::json::array();
    std::ofstream trace_;
    int max_turns_ = 0;
};
}
