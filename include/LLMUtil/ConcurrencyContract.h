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

    // ===== ThreadContract requirement/guarantee content =====
    // Each clause is anchored to one shared resource/protocol object, but the
    // requirement side stays close to source code: it records the local statement
    // or region whose safety depends on the environment.
    //
    //   assume    ∈ {ORDER, CONFLICT_MEDIATED, REGION_ISOLATED,
    //                STABLE_DURING, PROGRESS_ENABLED}
    //              -- local execution obligations of this thread.
    //   guarantee ∈ {ORDER, EXCLUDE, LINEARIZE, WAIT}
    //              -- Level-0 synchronization effects contributed by code.
    //
    // High-level APIs (RCU, refcount, close-and-drain, validation/retry) are
    // represented in `detail` as macros that lower to the Level-0 atoms above.
    struct OrderReq {                 // In assume: local safety obligation.
        std::string relation;         // "ORDER" | "CONFLICT_MEDIATED" | ...
        std::string detail;           // e.g. "STABLE_DURING(use_region, live(obj))"
        std::string provenance;       // 出处行 / caller，支撑该要求
    };
    struct SyncProv {                 // In guarantee: synchronization effect.
        std::string relation;         // "ORDER" | "EXCLUDE" | "LINEARIZE" | "WAIT"
        std::string detail;           // e.g. "WAIT(close_and_drain_return, active_callbacks_empty)"
        std::string provenance;
    };
    struct OrderClause {
        std::string resource;             // 共享对象/字段（可含 surface object id）
        int objectId = -1;                // 主锚点 = objectIds 的首个（兼容旧字段；-1=未锚定）
        std::vector<int> objectIds;       // 本 clause 覆盖的全部 surface 对象下标（锁区合并：一条
                                          // EXCLUDE 可同时覆盖同锁下的多个字段，静态组合按它匹配）
        std::vector<std::string> sites;   // "func @ file:line" —— clause 涉及操作的 provenance
        std::vector<OrderReq> assume;     // Local requirements (LLM-inferred, anchored).
        std::vector<SyncProv> guarantee;  // Synchronization atoms/macros provided by code.
        // Contract COMPLETENESS (coverage invariant): when this thread has reviewed a
        // HIGH-RISK surface object (unprotected cross-thread write / free / list
        // mutation / self-race) and concluded it carries NO order obligation for this
        // thread — the racy value never flows into a branch/index/size/pointer/lifetime
        // decision AND there is no use-before-free / init-before-publish — it emits a
        // clause with noOrderNeeded=true + a justification INSTEAD of silently omitting
        // it. This turns "every dangerous object was considered" into a verifiable
        // property of the contract; Phase B treats such a clause as benign (no hazard
        // tier, never discharges), exactly as before.
        bool noOrderNeeded = false;
        std::string noOrderReason;
    };
    // True iff this clause carries actual order content (an assume requirement or a
    // guarantee). A bare noOrderNeeded clause is NOT order content.
    static bool clauseHasOrder(const OrderClause& cl) {
        return !cl.assume.empty() || !cl.guarantee.empty();
    }
    // True iff this clause "addresses" object oi for coverage purposes: it references
    // oi AND either states real order content or explicitly declares no obligation.
    bool addressesObject(int oi) const {
        for (const auto& cl : clauses) {
            bool refs = (cl.objectId == oi);
            if (!refs) for (int id : cl.objectIds) if (id == oi) { refs = true; break; }
            if (refs && (clauseHasOrder(cl) || cl.noOrderNeeded)) return true;
        }
        return false;
    }
    // 选择性产出：只为有真实 requirement/guarantee 内容的资源各一条（而非逐变量穷举）；
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

    // Requirement/guarantee 内容的构建方法
    void addClause(const OrderClause& clause) { this->clauses.push_back(clause); }
    void addOrdering(const std::string& o) { this->ordering.push_back(o); }
    bool hasOrderContent() const { return !clauses.empty() || !ordering.empty(); }
};

} // namespace LLM

#endif // CONCURRENCY_CONTRACT_H
