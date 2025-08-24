#pragma once

#include "LLMUtil/LLMClient.h"
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

class CCPG; // Forward declaration

namespace llm_client {

// A namespace for shared tools accessible by all agents
namespace SharedToolKit {

    // Returns a list of tool definitions that are common across agents.
    std::vector<Tool> get_shared_tools();

    // Handles the execution of a shared tool.
    // Returns the JSON result as a string if the tool is a known shared tool.
    // Returns std::nullopt if the tool_name is not handled by the shared toolkit.
    std::optional<std::string> handle_shared_tool(
        const std::string& tool_name, 
        const nlohmann::json& arguments, 
        CCPG* ccpg
    );

} // namespace SharedToolKit
} // namespace llm_client
