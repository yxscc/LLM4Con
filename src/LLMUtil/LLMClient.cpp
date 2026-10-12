#include "LLMUtil/LLMClient.h"
#include "Util/TargetPath.h"
#include <stdexcept>
namespace llm_client {
LLMClient::LLMClient(std::string baseUrl, std::string apiKey, std::string model, long timeout)
    : base_url_(std::move(baseUrl)), api_key_(std::move(apiKey)),
      default_model_(std::move(model)), timeout_seconds_(timeout) {
    if (api_key_.empty() || default_model_.empty() || timeout <= 0)
        throw std::invalid_argument("Model, API key and positive timeout are required");
}
nlohmann::json LLMClient::runAgent(const std::string& instructions, const std::string& input,
    const nlohmann::json& history, const std::vector<Tool>& tools,
    const std::string& toolChoice, int maxTurns, const SDKToolExecutor& execute) {
    nlohmann::json schemas = nlohmann::json::array();
    for (const auto& tool : tools) {
        auto j = tool.to_json();
        if (!j["parameters"].contains("properties")) j["parameters"]["properties"] = nlohmann::json::object();
        schemas.push_back(std::move(j));
    }
    auto result = runSDKWorker({{"action", "agent"}, {"base_url", base_url_},
        {"api_key", api_key_}, {"timeout", timeout_seconds_}, {"model", default_model_},
        {"instructions", instructions}, {"input", input}, {"history", history},
        {"tools", schemas}, {"tool_choice", toolChoice}, {"max_turns", maxTurns},
        {"source_root",TargetPath::getInstance()->getTargetAbsolutePath().string()},
        {"contract_path",(TargetPath::getInstance()->getOutputDir()/"module_contracts.json").string()}}, execute);
    {
        const auto& usage = result.at("usage");
        token_stats_.total_requests += usage.value("requests", size_t(0));
        token_stats_.total_prompt_tokens += usage.value("input_tokens", size_t(0));
        token_stats_.total_completion_tokens += usage.value("output_tokens", size_t(0));
    }
    return result;
}

}
