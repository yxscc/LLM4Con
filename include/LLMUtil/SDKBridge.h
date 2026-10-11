#pragma once
#include <functional>
#include <string>
#include "nlohmann/json.hpp"

namespace llm_client {
using SDKToolExecutor = std::function<std::string(const std::string&, const nlohmann::json&)>;
// Local, bidirectional JSON-lines IPC. Credentials travel through the pipe,
// never through argv, a temporary file, or the trace. The SDK owns the agent loop.
nlohmann::json runSDKWorker(const nlohmann::json& request,
                            const SDKToolExecutor& execute = {});
}
