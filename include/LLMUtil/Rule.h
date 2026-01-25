// in include/LLMUtil/Rule.h

#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "CCPG/CCPG.h"

namespace query {
    class StatefulBug;
}

namespace llm_client {
    struct ThreadPair;
}

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

    virtual std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const = 0;

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

    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

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

    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

    std::string get_description() const override {
        return R"(**Inconsistent Locking / Data Race**: This pattern identifies unsynchronized memory access or inconsistent locking strategies that could cause data corruption, crashes, or logical errors.

**High-Impact Criteria (You MUST report these):**
1.  **Inconsistent Locking**: If Thread A accesses a shared variable (e.g., `conns`) with Lock X, but Thread B accesses the same variable WITHOUT Lock X (or with a different lock), this is a CRITICAL BUG. You MUST report this even if you don't see a specific crash scenario.
2.  **Missing Protection**: If a complex shared structure (like a list, hash table, or state machine) is written by one thread and read by another without ANY apparent synchronization, this is a CRITICAL BUG.

**Required roles**:
- 'write_operation': The operation in one thread that writes to the shared memory location (or the operation that uses a lock).
- 'read_operation': The concurrent operation in another thread that reads from the same location (or the operation that uses NO lock or a different lock).)";
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

/**
 * @class UseAfterFreeRule
 * @brief "释放后使用" (Use-After-Free) 规则的具体实现.
 */
class UseAfterFreeRule : public Rule {
public:
    std::string get_pattern_type() const override { return "USE_AFTER_FREE"; }

    std::vector<std::string> get_required_roles() const override {
        return {"free_operation", "use_operation"};
    }

    bool is_ready() const override {
        return nominated_nodes.count("free_operation") && nominated_nodes.count("use_operation");
    }

    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

    std::string get_description() const override {
        return R"(**Use-After-Free**: This pattern identifies a vulnerability where a program continues to use a pointer after it has been freed.

Required roles:
- 'free_operation': The operation that frees a memory location.
- 'use_operation': The concurrent or subsequent operation that uses the same memory location after it has been freed.)";
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

        auto free_info = get_node_info("free_operation");
        auto use_info = get_node_info("use_operation");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured Use-After-Free Rule from Selected Nodes ---\n"
                           << "A potential Use-After-Free vulnerability was identified.\n"
                           << "- The FREE operation is: `" << free_info.first << "` (at " << free_info.second << ")\n"
                           << "- The subsequent USE operation is: `" << use_info.first << "` (at " << use_info.second << ")\n\n"
                           << "QUESTION: Does this structured description accurately match your original summary?";

        return structured_summary.str();
    }
};

/**
 * @class DoubleFreeRule
 * @brief "重复释放" (Double-Free) 规则的具体实现.
 */
class DoubleFreeRule : public Rule {
public:
    std::string get_pattern_type() const override { return "DOUBLE_FREE"; }

    std::vector<std::string> get_required_roles() const override {
        return {"first_free_operation", "second_free_operation"};
    }

    bool is_ready() const override {
        return nominated_nodes.count("first_free_operation") && nominated_nodes.count("second_free_operation");
    }

    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

    std::string get_description() const override {
        return R"(**Double-Free**: This pattern identifies a vulnerability where a program frees the same memory location twice.

Required roles:
- 'first_free_operation': The first operation that frees a memory location.
- 'second_free_operation': The second operation that frees the same memory location.)";
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

        auto first_free_info = get_node_info("first_free_operation");
        auto second_free_info = get_node_info("second_free_operation");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured Double-Free Rule from Selected Nodes ---\n"
                           << "A potential Double-Free vulnerability was identified.\n"
                           << "- The FIRST FREE operation is: `" << first_free_info.first << "` (at " << first_free_info.second << ")\n"
                           << "- The SECOND FREE operation is: `" << second_free_info.first << "` (at " << second_free_info.second << ")\n\n"
                           << "QUESTION: Does this structured description accurately match your original summary?";

        return structured_summary.str();
    }
};

/**
 * @class NullPointerDereferenceRule
 * @brief "空指针解引用" (Null-Pointer-Dereference) 规则的具体实现.
 */
class NullPointerDereferenceRule : public Rule {
public:
    std::string get_pattern_type() const override { return "NULL_POINTER_DEREFERENCE"; }

    std::vector<std::string> get_required_roles() const override {
        return {"null_assignment_operation", "dereference_operation"};
    }

    bool is_ready() const override {
        return nominated_nodes.count("null_assignment_operation") && nominated_nodes.count("dereference_operation");
    }


    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

    std::string get_description() const override {
        return R"(**Null-Pointer-Dereference**: This pattern identifies a vulnerability where a program dereferences a pointer that is, or could be, null.

Required roles:
- 'null_assignment_operation': The operation that sets a pointer to NULL, or a function call that could return NULL.
- 'dereference_operation': The subsequent operation that dereferences this pointer without a proper null check.)";
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

        auto null_assignment_info = get_node_info("null_assignment_operation");
        auto dereference_info = get_node_info("dereference_operation");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured Null-Pointer-Dereference Rule from Selected Nodes ---\n"
                           << "A potential Null-Pointer-Dereference vulnerability was identified.\n"
                           << "- The NULL ASSIGNMENT operation is: `" << null_assignment_info.first << "` (at " << null_assignment_info.second << ")\n"
                           << "- The DEREFERENCE operation is: `" << dereference_info.first << "` (at " << dereference_info.second << ")\n\n"
                           << "QUESTION: Does this structured description accurately match your original summary?";

        return structured_summary.str();
    }
};

/**
 * @class DeadlockRule
 * @brief "死锁" (Deadlock) 规则的具体实现.
 */
class DeadlockRule : public Rule {
public:
    std::string get_pattern_type() const override { return "DEADLOCK"; }

    std::vector<std::string> get_required_roles() const override {
        return {"thread1_first_lock", "thread1_second_lock", "thread2_first_lock", "thread2_second_lock"};
    }

    bool is_ready() const override {
        return nominated_nodes.count("thread1_first_lock") && nominated_nodes.count("thread1_second_lock") &&
               nominated_nodes.count("thread2_first_lock") && nominated_nodes.count("thread2_second_lock");
    }

    std::string get_description() const override {
        return R"(**Deadlock**: This pattern identifies a potential deadlock situation where two or more threads are blocked forever, waiting for each other. This typically occurs when Thread 1 locks resource A and then tries to lock resource B, while Thread 2 has locked resource B and tries to lock resource A.

Required roles:
- 'thread1_first_lock': The first lock acquisition by the first thread.
- 'thread1_second_lock': The second lock acquisition by the first thread, which could block.
- 'thread2_first_lock': The first lock acquisition by the second thread.
- 'thread2_second_lock': The second lock acquisition by the second thread, which could block.)";
    }

    std::optional<query::StatefulBug> verify(const ThreadPair& pair, CCPG* ccpg) const override;

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

        auto t1_lock1_info = get_node_info("thread1_first_lock");
        auto t1_lock2_info = get_node_info("thread1_second_lock");
        auto t2_lock1_info = get_node_info("thread2_first_lock");
        auto t2_lock2_info = get_node_info("thread2_second_lock");

        std::stringstream structured_summary;
        structured_summary << "--- Your Original Summary ---\n"
                           << "\"" << original_summary << "\"\n\n"
                           << "--- Structured Deadlock Rule from Selected Nodes ---\n"
                           << "A potential Deadlock vulnerability was identified.\n"
                           << "Thread 1 acquires lock 1: `" << t1_lock1_info.first << "` (at " << t1_lock1_info.second << ")\n"
                           << "Then Thread 1 tries to acquire lock 2: `" << t1_lock2_info.first << "` (at " << t1_lock2_info.second << ")\n"
                           << "Thread 2 acquires lock 2: `" << t2_lock1_info.first << "` (at " << t2_lock1_info.second << ")\n"
                           << "Then Thread 2 tries to acquire lock 1: `" << t2_lock2_info.first << "` (at " << t2_lock2_info.second << ")\n\n"
                           << "QUESTION: Does this structured description accurately match your original summary?";

        return structured_summary.str();
    }
};
} // namespace llm_client

#endif // RULE_H