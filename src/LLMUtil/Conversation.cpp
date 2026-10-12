#include "LLMUtil/Conversation.h"
#include "Util/TargetPath.h"
#include <stdexcept>
namespace llm_client {
Conversation::Conversation(std::shared_ptr<LLMClient> client, std::string instructions)
    : client_(std::move(client)), instructions_(std::move(instructions)),
      trace_(TargetPath::getInstance()->getOutputDir() / "llm_simplified_trace.log", std::ios::app) {
    if (!client_ || !trace_) throw std::runtime_error("Cannot initialize module conversation/trace");
}
void Conversation::set_max_turns(int n) {
    if (n < 0) throw std::invalid_argument("max_turns must be nonnegative; 0 means unlimited");
    max_turns_ = n;
}
void Conversation::set_instructions(std::string instructions) {
    instructions_=std::move(instructions);
}
std::string Conversation::send_message(const std::string& input) {
    trace_ << "[sdk] User: " << input << std::endl;
    auto result = client_->runAgent(instructions_, input, history_, get_available_tools(),
        get_tool_choice(), max_turns_, [&](const std::string& name, const nlohmann::json& args) {
            trace_ << "[sdk] Tool: " << name << " " << args.dump() << std::endl;
            auto output = execute_tool(name, args);
            trace_ << "[sdk] Result: " << output << std::endl;
            return output;
        });
    history_ = result.at("history");
    trace_ << "[sdk] Completed: " << result.dump() << std::endl;
    return result.at("output").is_string() ? result.at("output").get<std::string>() : result.at("output").dump();
}
}
