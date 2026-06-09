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

    // ===== §4.6 order/sync assume-guarantee 内容（thread-contract "老故事"核心）=====
    // 每个线程触碰的每项共享资源一条 clause。关系来自一个封闭、与子系统无关的代数 (WF4)：
    //   assume    ∈ {prec, atomic, count_guarded}      —— 该线程为自身正确性"要求"的 order
    //   guarantee ∈ {serialize, order, counts}         —— 该线程用同步"建立"的 order
    // 它们是通用 order/sync 关系，绝不是按 CVE 增删的缺陷签名。
    struct OrderReq {                 // 出现在 assume：本线程要求的一条 order
        std::string relation;         // "prec" | "atomic" | "count_guarded"
        std::string detail;           // 如 "prec(use, free)" / "atomic([check, use])"
        std::string provenance;       // 出处行 / caller，支撑该要求
    };
    struct SyncProv {                 // 出现在 guarantee：本线程用同步建立的一条 order
        std::string relation;         // "serialize" | "order" | "counts"
        std::string detail;           // 如 "serialize(key->sem, region)"
        std::string provenance;
    };
    struct OrderClause {
        std::string resource;             // 共享对象/字段（可含 surface object id）
        int objectId = -1;                // 主锚点 = objectIds 的首个（兼容旧字段；-1=未锚定）
        std::vector<int> objectIds;       // 本 clause 覆盖的全部 surface 对象下标（锁区合并：一条
                                          // serialize 可同时覆盖同锁下的多个字段，静态组合按它匹配）
        std::vector<std::string> sites;   // "func @ file:line" —— clause 涉及操作的 provenance
        std::vector<OrderReq> assume;     // 要求的 order（推断出的意图 —— 核心增量）
        std::vector<SyncProv> guarantee;  // 建立 order 的同步
    };
    // 选择性产出：只为有真实 order/sync 义务的资源各一条（而非逐变量穷举）；
    // 同锁覆盖的多字段合并为一条（objectIds 列多个）。未产出 clause 的对象由
    // Phase B 的 surface 冲突底线兜底（召回不丢）。
    std::vector<OrderClause> clauses;
    // 跨 clause 的线程级有向序（如 "writes_before_publish"）
    std::vector<std::string> ordering;

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

    // §4.6 order/sync 内容的构建方法
    void addClause(const OrderClause& clause) { this->clauses.push_back(clause); }
    void addOrdering(const std::string& o) { this->ordering.push_back(o); }
    bool hasOrderContent() const { return !clauses.empty() || !ordering.empty(); }
};

} // namespace LLM

#endif // CONCURRENCY_CONTRACT_H