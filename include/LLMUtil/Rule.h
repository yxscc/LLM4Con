#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "CCPG/CCPG.h"

namespace llm_client {


/**
 * @class Rule
 * @brief 并发规则的抽象基类.
 *
 * 定义了一个并发规则的通用接口，允许LLM根据规则类型提供特定的节点角色，
 * 并为每种规则提供定制的摘要生成和验证逻辑.
 */
class Rule {
public:
    virtual ~Rule() = default;

    void set_metadata(const std::string& id, const std::string& obj_type, const std::string& summary) {
        m_rule_id = id;
        m_shared_object_type = obj_type;
        m_summary = summary;
    }

    // 获取规则的模式类型，例如 "TOCTOU"
    virtual std::string get_pattern_type() const = 0;

    virtual std::string get_description() const = 0;

    // 获取此规则需要LLM提名的所有节点角色
    virtual std::vector<std::string> get_required_roles() const = 0;

    // 生成用于LLM确认的结构化摘要
    virtual std::string generate_confirmation_summary(CCPG* ccpg, const std::string& original_summary) const = 0;

    // 检查是否所有必需的角色都已经被提名了节点
    virtual bool is_ready() const = 0;
    
    // 存储或检索一个节点ID到特定的角色
    virtual void set_node_for_role(const std::string& role, int node_id) {
        nominated_nodes[role] = node_id;
    }

    virtual int get_node_for_role(const std::string& role) const {
        auto it = nominated_nodes.find(role);
        if (it != nominated_nodes.end()) {
            return it->second;
        }
        return -1; // 表示未找到
    }

    // 将规则转换为JSON对象以便存储
    virtual nlohmann::json to_json() const {
        nlohmann::json j;
        j["rule_id"] = m_rule_id;
        j["pattern_type"] = get_pattern_type();
        j["shared_object_type"] = m_shared_object_type;
        j["_llm_summary"] = m_summary; // 使用 _llm_summary 作为描述键
        j["nodes"] = nominated_nodes;
        return j;
    }

protected:
    std::map<std::string, int> nominated_nodes;
    std::string m_rule_id;
    std::string m_shared_object_type;
    std::string m_summary;
};


/**
 * @class TOCTOURule
 * @brief "检查后使用" (Time-of-Check to Time-of-Use) 规则的具体实现.
 */
class TOCTOURule : public Rule {
public:
    std::string get_pattern_type() const override { return "TOCTOU"; }

    std::vector<std::string> get_required_roles() const override {
        return {"state_check_operation", "state_modify_operation", "resource_use_operation"};
    }

    bool is_ready() const override {
        for (const auto& role : get_required_roles()) {
            if (nominated_nodes.find(role) == nominated_nodes.end()) {
                return false;
            }
        }
        return true;
    }

    std::string get_description() const override {
        return R"(Time-of-Check to Time-of-Use (TOCTOU): This pattern identifies a race condition where an operation in one thread (the 'victim') checks the state of a shared resource and then later uses it, but a concurrent operation in another thread (the 'modifier') invalidates the state between the victim's check and use.

Required roles:
- 'state_check_operation': In the victim thread, the operation that reads the state of the shared resource. This can be an explicit check (e.g., `if (ptr != NULL)`) or an implicit one (e.g., `local_ptr = shared_ptr;`). This marks the 'Time-of-Check'.
- 'state_modify_operation': In a concurrent modifier thread, the operation that alters the state of the shared resource, potentially invalidating the check (e.g., `shared_ptr = NULL;`).
- 'resource_use_operation': In the same victim thread, a subsequent operation that relies on the state read during the 'state_check_operation' (e.g., `local_ptr->do_something();`). This marks the 'Time-of-Use'.)";
    }

    std::string generate_confirmation_summary(CCPG* ccpg, const std::string& original_summary) const override {
        auto get_node_info = [&](const std::string& role) -> std::pair<std::string, std::string> {
            auto it = nominated_nodes.find(role);
            if (it != nominated_nodes.end()) {
                CCPGNode* node = ccpg->getNodeByID(it->second);
                if (node) {
                    return {node->getCPGNode()->getCode(), node->getNodeLoc().toString()};
                }
            }
            return {"[Node not nominated]", "[Location unknown]"};
        };

        auto check_info = get_node_info("state_check_operation");
        auto modify_info = get_node_info("state_modify_operation");
        auto use_info = get_node_info("resource_use_operation");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured TOCTOU Rule from Selected Nodes ---\n"
                           << "A check-then-use sequence was identified that could be vulnerable to a race condition.\n"
                           << "- The CHECK operation is: `" << check_info.first << "` (at " << check_info.second << ")\n"
                           << "- The concurrent MODIFY operation is: `" << modify_info.first << "` (at " << modify_info.second << ")\n"
                           << "- The subsequent USE operation is: `" << use_info.first << "` (at " << use_info.second << ")\n\n"
                           << "QUESTION: Does this structured description accurately and semantically match your original summary?";

        return structured_summary.str();
    }
};


/**
 * @class DataRaceRule
 * @brief "高影响力数据竞争" (High-Impact Data Race) 规则的具体实现.
 * 旨在识别那些极有可能导致严重后果（如崩溃、数据损坏）的非良性数据竞争。
 */
class DataRaceRule : public Rule {
public:
    std::string get_pattern_type() const override { return "DataRace"; }

    std::vector<std::string> get_required_roles() const override {
        // 保持角色简单，但描述会引导LLM进行更复杂的判断
        return {"write_operation", "read_operation"};
    }

    bool is_ready() const override {
        for (const auto& role : get_required_roles()) {
            if (nominated_nodes.find(role) == nominated_nodes.end()) {
                return false;
            }
        }
        return true;
    }

    std::string get_description() const override {
        return R"(**High-Impact Data Race**: This pattern identifies a race condition where concurrent, unsynchronized memory access is highly likely to cause critical bugs. Focus *only* on non-benign races. A race is considered high-impact if it meets the following criteria:

**Required roles**:
- 'write_operation': The operation in one thread that writes to the shared memory location. This is the potential source of corruption or invalidation.
- 'read_operation': The concurrent operation in another thread that reads from the same location. This is the operation that may observe the inconsistent state.)";
    }

    std::string generate_confirmation_summary(CCPG* ccpg, const std::string& original_summary) const override {
        auto get_node_info = [&](const std::string& role) -> std::pair<std::string, std::string> {
            auto it = nominated_nodes.find(role);
            if (it != nominated_nodes.end()) {
                CCPGNode* node = ccpg->getNodeByID(it->second);
                if (node) {
                    return {node->getCPGNode()->getCode(), node->getNodeLoc().toString()};
                }
            }
            return {"[Node not nominated]", "[Location unknown]"};
        };

        auto write_info = get_node_info("write_operation");
        auto read_info = get_node_info("read_operation");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured High-Impact Data Race Rule from Selected Nodes ---\n"
                           << "A potential high-impact data race was identified due to concurrent, unsynchronized access.\n"
                           << "- The WRITE operation (potential corruption source) is: `" << write_info.first << "` (at " << write_info.second << ")\n"
                           << "- The concurrent READ operation (observes inconsistent state) is: `" << read_info.first << "` (at " << read_info.second << ")\n\n"
                           << "QUESTION: Does this structured description, which implies a high potential for harm, accurately and semantically match your original summary?";

        return structured_summary.str();
    }
};

} // namespace llm_client

#endif // RULE_H