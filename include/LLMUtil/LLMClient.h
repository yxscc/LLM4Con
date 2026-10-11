#pragma once
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "LLMUtil/SDKBridge.h"
namespace llm_client {
struct Parameter {
    std::string name;
    std::string type;
    std::string description;
    bool required = false;
    std::optional<std::unique_ptr<Parameter>> items;
    std::optional<nlohmann::json> raw_schema;

    // Default constructor
    Parameter() = default;

    // Constructor for easy initialization of simple parameters
    Parameter(std::string n, std::string t, std::string d, bool req = false)
        : name(std::move(n)), type(std::move(t)), description(std::move(d)), required(req), items(std::nullopt) {}

    // Constructor for array parameters
    Parameter(std::string n, std::string t, std::string d, bool req, std::unique_ptr<Parameter> i)
        : name(std::move(n)), type(std::move(t)), description(std::move(d)), required(req) {
        if (i) {
            items = std::move(i);
        }
    }

    // Constructor for raw JSON schema override
    Parameter(std::string n, nlohmann::json schema, bool req = false)
        : name(std::move(n)), required(req), raw_schema(std::move(schema)) {}

    // Custom copy constructor and assignment to handle unique_ptr
    Parameter(const Parameter& other)
        : name(other.name),
          type(other.type),
          description(other.description),
          required(other.required),
          raw_schema(other.raw_schema) {
        if (other.items) {
            items = std::make_unique<Parameter>(**other.items);
        }
    }

    Parameter& operator=(const Parameter& other) {
        if (this == &other) {
            return *this;
        }
        name = other.name;
        type = other.type;
        description = other.description;
        required = other.required;
        raw_schema = other.raw_schema;
        if (other.items) {
            items = std::make_unique<Parameter>(**other.items);
        } else {
            items.reset();
        }
        return *this;
    }

    // Explicitly default the move operations
    Parameter(Parameter&& other) noexcept = default;
    Parameter& operator=(Parameter&& other) noexcept = default;
};

struct Tool {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;

    nlohmann::json to_json() const {
        nlohmann::json j;
        j["name"] = name;
        j["description"] = description;

        nlohmann::json params_obj;
        params_obj["type"] = "object";
        nlohmann::json properties_obj;

        std::vector<std::string> required_params;
        for (const auto& p : parameters) {
            nlohmann::json param_details;

            if (p.raw_schema.has_value()) {
                param_details = *p.raw_schema;
            } else {
                param_details["type"] = p.type;
                if (!p.description.empty()) {
                    param_details["description"] = p.description;
                }

                if (p.type == "array" && p.items) {
                    nlohmann::json items_json;
                    items_json["type"] = (*p.items)->type;
                    if (!(*p.items)->description.empty()) {
                        items_json["description"] = (*p.items)->description;
                    }
                    param_details["items"] = items_json;
                }
            }
            
            properties_obj[p.name] = param_details;

            if (p.required) {
                required_params.push_back(p.name);
            }
        }

        if (!properties_obj.empty()) {
            params_obj["properties"] = properties_obj;
        }
        
        if (!required_params.empty()) {
            params_obj["required"] = required_params;
        }

        j["parameters"] = params_obj;
        return j;
    }
};

// One explicit client owned by the module pipeline. The official SDK handles
// API transport, retries and tool rounds; C++ only supplies validated tools.
class LLMClient {
public:
    LLMClient(std::string baseUrl, std::string apiKey, std::string model, long timeout = 300);
    nlohmann::json runAgent(const std::string& instructions, const std::string& input,
        const nlohmann::json& history, const std::vector<Tool>& tools,
        const std::string& toolChoice, int maxTurns, const SDKToolExecutor& execute);
    struct TokenStats {
        size_t total_prompt_tokens = 0, total_completion_tokens = 0, total_requests = 0;
    };
    TokenStats get_token_stats() const { return token_stats_; }
private:
    std::string base_url_, api_key_, default_model_;
    long timeout_seconds_;
    TokenStats token_stats_;
};
}
