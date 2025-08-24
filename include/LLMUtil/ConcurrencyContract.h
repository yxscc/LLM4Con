#ifndef CONCURRENCY_CONTRACT_H
#define CONCURRENCY_CONTRACT_H

#include <string>
#include <vector>
#include <set>
#include <optional>
#include "nlohmann/json.hpp"
#include "CCPG/CCPGNode.h" // 为了使用 NodeID 等类型

namespace LLM {

// 使用类型别名以增强代码可读性
using ThreadID = int;
using NodeID = int;

/**
 * @class ConcurrencyContract
 * @brief (V3) 支持渐进式构建的“并发契约”。
 *
 * 这个类通过一系列 "setter" 和 "adder" 方法，允许Agent在与LLM的多轮对话中逐步构建契约。
 */
class ConcurrencyContract {
public:
    // 1. 身份标识 (Identity)
    ThreadID threadId;
    NodeID entryPointFunctionId;

    // 2. 语义描述 (Semantics)
    std::string role;
    std::string summary;

    // 3. 核心共享状态 (Core Shared State)
    struct SharedVariable {
        std::string variableName;
        std::string variableType;
        std::string accessType; // "Read", "Write", "ReadWrite"
        std::vector<std::string> protectingPrimitives;
    };
    std::vector<SharedVariable> sharedVariables;

    // 4. 同步规约 (Synchronization Discipline)
    enum class PrimitiveType { MUTEX, SEMAPHORE, COND_VAR, CUSTOM_ATOMIC, UNKNOWN };
    
    struct SynchronizationPrimitive {
        std::string identifier;
        PrimitiveType type;
        std::string purpose;
        NodeID definitionNodeId;
    };

    struct SynchronizationDiscipline {
        std::string strategy;
        std::vector<SynchronizationPrimitive> primitives;
        std::vector<std::vector<std::string>> lockOrder;
    };
    SynchronizationDiscipline synchronization;

    // 5. 并行关系 (Parallelism)
    std::set<ThreadID> intendedParallelThreads;

public:
    // --- 构造与构建方法 ---

    // 构造函数现在只初始化必要的ID
    ConcurrencyContract(ThreadID tId, NodeID entryId) 
        : threadId(tId), entryPointFunctionId(entryId) {}
    
    // Setter 方法，用于逐步填充字段
    void setRole(const std::string& r) { this->role = r; }
    void setSummary(const std::string& s) { this->summary = s; }
    
    void addSharedVariable(const SharedVariable& var) {
        this->sharedVariables.push_back(var);
    }
    
    void setSynchronizationStrategy(const std::string& strat) {
        this->synchronization.strategy = strat;
    }

    void addSynchronizationPrimitive(const SynchronizationPrimitive& prim) {
        this->synchronization.primitives.push_back(prim);
    }

    void setLockOrder(const std::vector<std::vector<std::string>>& order) {
        this->synchronization.lockOrder = order;
    }

    void addIntendedParallelThread(ThreadID parallelThreadId) {
        this->intendedParallelThreads.insert(parallelThreadId);
    }
};

} // namespace LLM

#endif // CONCURRENCY_CONTRACT_H