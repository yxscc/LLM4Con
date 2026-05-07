# M7 落地设计：HB-DSL 重构与 Surface 加权

> 这是 Lace 检测器下一阶段的**实施计划**。设计目标态见
> [`HYPOTHESIS_DSL_DESIGN.md`](./HYPOTHESIS_DSL_DESIGN.md)；本文档专注于
> **怎么把目标态在现有代码基上落地**，包含接口签名、文件清单、
> 渐进迁移路径与回退策略。
>
> 配套修订：[`README.md`](./README.md) §M6 的桶计数有 3 处小错位
> （`CVE-2025-38165` 应在 tp_related_only；`CVE-2025-38337` 应在 all_fp；
> mostly_fp 应为 1 个），不影响整体趋势，可在 M7 完成后随报告一并修订。

## 1. 现状盘点（以现有代码为准）

| 资产 | 文件 | 状态 | 利用方式 |
|---|---|---|---|
| `HB` 同步图骨架类 | `include/CCPG/HB.h` | **已定义但 .cpp 空缺** | 直接填实，重命名/扩展接口 |
| `CCPGEdge::EdgeType::HB` | `include/CCPG/CCPGEdge.h` | 已定义 | 边集层复用，HBGraph 内部走单独 adjacency 索引 |
| 唯一一处 HB 边 | `src/CCPG/CCPG.cpp:232` | fork→thread-entry | 保留，由 HBGraph 接管增补 |
| `LSAnalysis::nodeLockSets` | `src/CCPG/LSAnalysis.cpp:10-99` | 已实现 path-sensitive | 复用作为"§3.2 锁边"的事实来源 |
| `ThreadAPIUtil` API 表 | `src/CCPG/ThreadAPIUtil.cpp:1-300` | 80% 覆盖 §3.2/3.5/3.6 | 新增 refcount / RCU sync / completion 子集合 |
| `HypothesisVerifier` 6 谓词 | `src/Query/HypothesisVerifier.cpp` | 工作正常 | 保留 + 新增 5+3 谓词 |
| `DetectorAgent` system prompt | `src/LLMUtil/DetectorAgent.cpp:24-131` | 6 谓词 + few-shot | Phase C 重写 |
| `VulnerabilitySurfaceGenerator::computeRiskScores` | `src/Query/VulnerabilitySurfaceGenerator.cpp:273-347` | 权重过偏热点 | Phase D 加权重 + Top-N 自适应 |
| `Top-N=30` surface 截断 | `VulnerabilitySurfaceGenerator.h:65` | 写死常量 | Phase D 改成 token-budget 感知 |
| `5 hypothesis` 上限 | DetectorAgent prompt 软约束 | 受 prompt 影响 | Phase C 改为 surface-size 自适应 |

**结论**：HBGraph 不是从零写的新引擎。是把已有"锁集 + fork/join + completion + RCU
read-side"重新组合成 hb 关系图。新增的边类型只有 §3.4（refcount / kref 引用计数）
和 §3.7（lifecycle flag）这两块需要自己识别 IR 模式。

### 1.1 M6 根因 → M7 Phase 覆盖矩阵

> 这张表是 M7 设计的"覆盖性自检"。**Phase A/B/C 解决的是"判什么"，
> Phase D/E/F 解决的是"看得到什么"。**两者缺一不可，不能只做 DSL。

| 根因 (README §M6) | M6 受害 CVE | 责任 Phase | 在本文档的位置 |
|---|---|---|---|
| **A. CPG 入口实体解析失败**（kernel 路径不规整、ops_table 成员名带 IR suffix、static 函数被 LTO 改名） | CVE-2016-7911、CVE-2024-53136 (zero_reports = 2) | **Phase E** | §10 |
| **B. Verifier 谓词表达力不足**（缺 same_location/op_kind/hb/conflicts 等，无法表达 HB-violation 与 atomic-block-violation） | 25/37 MISS（按 README 桶 67.5%） | **Phase A + B + C** | §6/7/8 |
| **C. Surface 中标量字段被 alloc/free 热点淹没**（risk_score 偏 UAF；Top-N 截断写死） | CVE-2024-40953、CVE-2024-41005 等 5 个 | **Phase D** | §9 |
| **D. 跨文件 patch 单 .ll 视野不全**（prepare_cve.sh 不会自动展开 patch 涉及的全部 TU；llvm-link 只覆盖给定 SOURCE_FILES） | CVE-2024-43891 (5 文件)、CVE-2025-37920 (4 文件) 等 | **Phase F** | §11 |
| **E. Top-N=5 hypothesis 硬上限**（surface 大、对象多时系统性放大 B/C） | 全局 / 间接放大 B C | **Phase D（顺手）** | §9.4 |

**Phase A/B/C 是必做（解决 67.5% MISS），Phase D/E/F 是补漏**：
- Phase D：影响约 5 CVE，但实现成本最低（半天），首发；
- Phase E：影响 zero_reports 2 CVE，且**不修则 Phase A/B/C 对这 2 个 CVE 无效**（连入口都进不去 LSAnalysis 都跑不起来）；
- Phase F：影响约 6-8 CVE（README §M6 没单独统计，但 MISS 列表里跨 ≥3 文件的 CVE 共有 8 个），属于**编译期改动**，不动检测器代码。

## 2. 设计原则与回退策略

| 原则 | 描述 |
|---|---|
| **非侵入** | 现有 6 谓词与 HypothesisVerifier 保留至 M7 收尾。新 5+3 谓词与旧的并行存在，由 prompt 指引 LLM 优先使用新的 |
| **单测先行** | 每个 Phase 落地后，先用 §11 的 canary CVE 跑一遍，期望产出与 ground truth 对照不退化 |
| **可灰度** | DetectorAgent 提供 `--dsl-mode {legacy,hb,both}` flag。`hb` 模式只允许新谓词；`both` 默认值，旧 prompt 加 hb few-shot；`legacy` 完全回退 |
| **可观察** | HBGraph 提供 `dumpDot(outDir)` 把 hb 边打印为 .dot，方便人工核对 §3.x 的边是否插对 |
| **可回退** | 一切新增谓词的 eval 函数失败时返回 `unknown`（既不 satisfied 也不 failed），verifier 把它当 `satisfied=false`+detail 给 LLM 看，LLM 可自行调整 |

## 3. 整体流水线（红色为 M7 新增/改动）

```
INPUT: *.ll + src/
  │
  ├─ CPG / Phasar / Phase 0 ThreadAPI 发现        （不变）
  ├─ CCPG.build() 含 fork→entry HB 边              （不变）
  ├─ ThreadCreationTree.build()                    （不变）
  ├─ LSAnalysis.build() 算 nodeLockSets            （不变）
  │
+ ├─ ★HBGraph.build()  Phase A 新增              ★
+ │     · §3.1  CFG 内 ORDER 边批量复用为 hb
+ │     · §3.2  对每对 (release(L), acquire(L)) 加 hb
+ │     · §3.3  call_rcu/synchronize_rcu RCU 边
+ │     · §3.4  refcount_dec_and_test / kref_put 边
+ │     · §3.5  bh/irq/preempt 区间 → softirq virtual thread 互斥边
+ │     · §3.6  complete/wait_for_completion / flush_work 边
+ │     · §3.7  lifecycle flag 模式识别
+ │     · 暴露 hbReachable(a,b)/classifyEdge(a,b)
+ │
  ├─ Phase 1: VulnerabilitySurfaceGenerator
+ │     · ★Phase D 新增 risk-score 项：scalar_torn_access、
+ │       read_dominated_lone_writer、missing_atomic_annotation
+ │     · ★Top-N 改成 token-budget 自适应
  │
  ├─ Phase 2: DetectorAgent (LLM)
+ │     · ★Phase C 重写 system prompt：
+ │       新 5+3 谓词，§4 八个家族 few-shot，
+ │       quality-over-quantity → quality-over-budget
+ │     · ★propose_hypothesis schema：predicate enum 增加
+ │       same_location / op_kind / hb / conflicts /
+ │       concurrent / unsafe_atomic_block
  │       ↓ propose_hypothesis tool call
+ │     ★HypothesisVerifier:Phase B 重构
+ │     · 旧 eval_in_thread / eval_reachable 保留
+ │     · 新 eval_same_location 用 SharedFieldKey + AliasChecker
+ │     · 新 eval_op_kind 直接读 LLVM IR
+ │     · 新 eval_hb 走 HBGraph::hbReachable
+ │     · 糖 eval_conflicts/concurrent/unsafe_atomic_block
  │
  └─ Phase 4: 输出 confirmed_hypotheses.log（不变）
```

## 4. 核心数据结构

### 4.1 HBEdge 类型 (`include/CCPG/HBEdge.h` 新建)

```cpp
namespace ccpg {

// HB 边的来源类别。仅供调试 / DSL eval 时给 LLM 反馈用。
enum class HBEdgeKind {
    PROGRAM_ORDER,    // §3.1 同函数 CFG 顺序
    CALL_RETURN,      // §3.1 caller→callee 进入；callee→return 出口
    LOCK_RELEASE_ACQUIRE,   // §3.2 release(L) → acquire(L)
    FORK_TO_ENTRY,    // §3.1/3.6 fork → child thread entry
    JOIN_FROM_EXIT,   // §3.6 child exit → join 后续
    RCU_SYNC,         // §3.3 synchronize_rcu / call_rcu
    REFCOUNT_LAST_PUT,// §3.4 atomic_dec_and_test==true 上的最后 put
    REL_ACQ_PAIR,     // §3.4 store_release ↔ load_acquire
    BH_IRQ_INTERVAL,  // §3.5 local_bh_disable..enable 区间互斥
    COMPLETION,       // §3.6 complete → wait_for_completion
    LIFECYCLE_FLAG,   // §3.7 set FREED → check FREED
};

struct HBEdge {
    CCPGNode* src;
    CCPGNode* dst;
    HBEdgeKind kind;
    // 可选：哪个 lock / completion / flag 触发的（debug 用）
    std::string evidence;
};

} // namespace ccpg
```

### 4.2 HBGraph 接口 (`include/CCPG/HB.h` 重写)

把现有空壳改成下面这套 API。原文件里 `nodeHappensBefore` / `contextHappensBefore`
等命名保留作为 deprecated 别名，避免编译期破坏 in-flight 的引用。

```cpp
class HBGraph {
public:
    static HBGraph* getInstance();

    // 一次性扫 CCPG/IR 构建全部 hb 边。
    // 调用顺序：CCPG.build() → LSAnalysis.build() → HBGraph.build()
    // 因为它要复用 nodeLockSets。
    void build(CCPG* ccpg);

    // 核心查询：n1 是否 happens-before n2。
    // 实现：从 n1 起的双向 BFS，遇到 §3.x 的所有边类型；
    // 跨线程时只允许"出 src 线程的边"是 LOCK_RELEASE_ACQUIRE / FORK_TO_ENTRY /
    // JOIN_FROM_EXIT / RCU_SYNC / COMPLETION / LIFECYCLE_FLAG / BH_IRQ_INTERVAL。
    // depth 上限默认 16，可通过 env 调。
    bool hbReachable(CCPGNode* n1, CCPGNode* n2,
                     int max_depth = 16) const;

    // 调试：把所有 hb 边导出为 .dot
    void dumpDot(const std::filesystem::path& outDir) const;

    // 查询某条 hb 路径的边类型组成（给 LLM 反馈用）
    std::vector<HBEdgeKind> classifyPath(CCPGNode* n1, CCPGNode* n2) const;

private:
    // 边集（按 src 索引，BFS 用）
    std::unordered_map<CCPGNode*, std::vector<HBEdge>> outEdges_;
    // 反向索引（hbReachable 双向 BFS 减枝用）
    std::unordered_map<CCPGNode*, std::vector<HBEdge>> inEdges_;

    // ---- §3.x 各类边构造（私有，build 内调度）----
    void buildProgramOrderEdges(CCPG* ccpg);   // §3.1
    void buildLockEdges(CCPG* ccpg);           // §3.2 复用 LSAnalysis
    void buildRCUEdges(CCPG* ccpg);            // §3.3
    void buildRefcountEdges(CCPG* ccpg);       // §3.4
    void buildContextIntervalEdges(CCPG* ccpg);// §3.5
    void buildCompletionEdges(CCPG* ccpg);     // §3.6
    void buildLifecycleFlagEdges(CCPG* ccpg);  // §3.7

    void addEdge(CCPGNode* src, CCPGNode* dst,
                 HBEdgeKind kind, std::string evidence = "");
};
```

> **关键实现细节 — §3.2 锁边的 release→acquire 配对**：
> `LSAnalysis::nodeLockSets` 已经把每个 CCPGNode 关联到它持有的锁集。
> 配对规则：对每把 `Lock* L`，扫所有持有 L 的节点序列，找 `L->getRelease()`
> 节点 R；扫所有 acquire L 的节点 A（不一定同一函数）；如果 R 的线程 ≠ A 的线程
> 且 `mayThreadsRunConcurrently(R.thread, A.thread)`，则添加 `hb(R, A)` 边（kind=
> `LOCK_RELEASE_ACQUIRE`）。同线程内的 release→acquire 由 §3.1 的 CFG 顺序自动覆盖。

### 4.3 新 HypothesisVerifier 谓词 (`include/Query/HypothesisVerifier.h` 扩展)

```cpp
class HypothesisVerifier {
public:
    HypothesisVerifier(CCPG* ccpg, ThreadCreationTree* tct, HBGraph* hb);

    VerificationResult verify(const Hypothesis& h);

private:
    // ---- 旧 6 谓词（保留，标 deprecated 用于过渡）----
    bool eval_in_thread(int node_id, int thread_id, std::string& detail);
    bool eval_may_run_concurrently(int t1, int t2, std::string& detail);  // 实现改为糖
    bool eval_reachable(int from_id, int to_id, std::string& detail);
    bool eval_not_lock_protected(int node_id, std::string& detail);       // 实现改为糖
    bool eval_same_lock(int n1, int n2, std::string& detail);             // 实现改为糖
    bool eval_alias(int n1, int n2, std::string& detail);                 // 实现改为糖

    // ---- 新 5 个原语（§2.1）----
    bool eval_same_location(int n1, int n2, std::string& detail);
    enum class OpKind { READ, WRITE, RMW, CALL, OTHER };
    bool eval_op_kind(int node_id, OpKind expected, std::string& detail);
    // in_thread 已存在
    // reachable 已存在
    bool eval_hb(int n1, int n2, bool expected, std::string& detail);

    // ---- 3 个糖（§2.2）----
    bool eval_conflicts(int n1, int n2, std::string& detail);        // = same_location ∧ (write∨RMW)
    bool eval_concurrent(int n1, int n2, std::string& detail);       // = ¬hb(a,b) ∧ ¬hb(b,a)
    bool eval_unsafe_atomic_block(int start, int end, int witness,
                                  std::string& detail);

    HBGraph* hb_;  // 新增依赖
};
```

eval_hb 的 `expected` 参数对应 DSL 的 `"expected": false` 语法（§4 F5/F8 用）。

### 4.4 propose_hypothesis 工具 schema 改动 (`src/LLMUtil/DetectorAgent.cpp:174-198`)

`predicate` enum 由 6 项扩展为 14 项：

```
旧 6: in_thread, may_run_concurrently, reachable,
       not_lock_protected, same_lock, alias
新 +5+3: same_location, op_kind, hb,
         conflicts, concurrent, unsafe_atomic_block
```

LLM 仍可使用旧谓词（保持向后兼容），但 system prompt 会引导优先使用新的。

### 4.5 VulnerabilitySurface 评分 (`src/Query/VulnerabilitySurfaceGenerator.cpp:273-347`)

在 `computeRiskScores()` 内新增三类加分（针对根因 C）：

```cpp
// 新增信号 1: scalar 字段（< machine word）+ 跨线程读写 + 无 atomic/READ_ONCE 注解
//             (识别方式：obj.type 是 int/bool/u32/short 之一，且 has_cross_thread_rw)
if (isScalarType(obj.type) && obj.has_cross_thread_rw &&
    !hasAtomicAnnotation(obj)) {
    score += 35;   // 与 unprotected_write 同档
    obj.flags.push_back("SCALAR_TORN_ACCESS_RISK");
}

// 新增信号 2: 一线程多写、其他线程只读（典型 READ_ONCE 候选）
if (isReadDominatedLoneWriter(obj.accesses)) {
    score += 20;
    obj.flags.push_back("READ_DOMINATED");
}

// 新增信号 3: 该字段在 patch 风格的 path 中（heuristic：
//             被 if(flag) ... 模式分支保护）→ lifecycle 嫌疑
if (isLifecycleFlagShape(obj)) {
    score += 30;
    obj.flags.push_back("LIFECYCLE_FLAG_CANDIDATE");
}
```

### 4.6 Top-N 与 hypothesis 上限自适应 (Phase D)

| 旧 | 新 |
|---|---|
| `toPromptString(top_n=30)` 写死 | 按 token-budget：累加 obj 的 toPromptString 估算长度，到达 60% context window 即截断 |
| Prompt "stop at 5" 软约束 | 改为：surface size ≤ 5 时上限 8；6-15 时 12；>15 时 20，并在 prompt 中说明 |

## 5. 渐进迁移路径（6 个 Phase）

| Phase | 责任根因 | 工作量 | 验证 canary | 期望差值 |
|---|---|---|---|---|
| **D. Surface 加权 + Top-N 自适应** | C + E | 0.5 天 | `CVE-2024-40953` (last_boosted_vcpu)、`CVE-2024-41005` (poll_owner) | 两 CVE 至少各产 1 条 TP_RELATED 提到目标字段 |
| **E. CPG findMethod 兜底** | A | 0.5 天 | `CVE-2016-7911`、`CVE-2024-53136` | `getMain` 不再 null；ops_table 命中率 ≥ 70% |
| **F. patch-driven 文件展开** | D | 0.5 天 | `CVE-2024-43891`、`CVE-2025-37920` | merged.ll 含全部 patch 函数；hypothesis 跨 ≥ 2 文件 |
| **A. HBGraph build** | B（地基） | 2 天 | `CVE-2024-43891` 跑通 + dot 检查 ≥ 1 条 LIFECYCLE_FLAG 边 | 不动 LLM；只看 dot |
| **B. Verifier 扩谓词** | B（主体） | 1 天 | `CVE-2017-15265` 旧谓词通过；新谓词手写 4 条 hypothesis 全 pass | confirmed_hypotheses 数 ≥ 旧批 |
| **C. DetectorAgent prompt + schema** | B（收尾） | 0.5 天 | 重跑 8 个家族各 1 个 canary（§13） | LLM 输出新谓词比例 ≥ 60% |

完成 D→E→F→A→B→C 后再做一次全 50 CVE 跑批 + `evaluate_recall.py` 三类判定。
**Phase D/E/F 是"看得到"，Phase A/B/C 是"判得准"——前者保覆盖，后者保 recall**。

**期望数字**（与 M6 比较）：

| 指标 | M6 实测 | M7 目标 |
|---|---|---|
| HIT 数 | 13/50 (26%) | ≥ 25/50 (50%) |
| TP_MATCH | 24/264 (9%) | ≥ 50/300 (17%) |
| TP_RELATED + TP_MATCH (lenient prec) | 71.97% | ≥ 78% |
| FP rate | 28.03% | ≤ 22% |

## 6. Phase A 详细：HBGraph 构建

### 6.1 文件清单

```
新建：
  include/CCPG/HBEdge.h                ← 4.1 节
  include/CCPG/HBGraph.h               ← 4.2 节（替换 HB.h，旧 HB.h 内容用兼容别名导出）
  src/CCPG/HBGraph.cpp                 ← 全部 build* 函数实现

修改：
  src/llm_main.cpp                     ← LSAnalysis 后插一行 HBGraph::getInstance()->build(ccpg.get())
  src/CCPG/CMakeLists.txt              ← 加 HBGraph.cpp
  src/CCPG/CCPG.cpp:232                ← 把已有的 fork→entry HB 边迁移到 HBGraph，但保留 CCPGEdge 端
                                         （HBGraph 同时索引一份）
```

### 6.2 §3.x 边构造的最小实现顺序

按"直接利用现有数据"的难度从低到高：

1. **§3.1 PROGRAM_ORDER + CALL_RETURN**：扫所有 `CCPGEdge`，类型为 `ORDER` 或 `CALL` 的直接复制成 `HBEdge{kind=PROGRAM_ORDER}` 或 `CALL_RETURN`。（10 行）
2. **§3.6 FORK_TO_ENTRY + JOIN_FROM_EXIT**：CCPG 已有 `fork→entry` 的 HB 边；扫 thread 的 `joinNode`，加一条 `child_exit_node → joinNode->next` 的边。（30 行，要遍历 TCT）
3. **§3.2 LOCK_RELEASE_ACQUIRE**：扫 `LSAnalysis::getLocks()` 的每把 `Lock* L`：取 `L->getRelease()` 与 `L->getAcquire()`。这俩本来就是同一对。要做的是：找出**与这把锁别名（同一对象）的其它 acquire/release 节点**——实际上 LSAnalysis 已经在 build 时为每次 acquire 创建了独立的 `Lock` 对象，所以"同一物理锁"通过 `AliasChecker::isLockAlias` 关联。逻辑：对所有锁两两 isLockAlias 配对，每对 `(Lj.release, Lk.acquire)` 加边。（80 行）
4. **§3.5 BH_IRQ_INTERVAL**：因为 `local_bh_disable` 已是 ACQUIRE 类型，每个 disable→enable 区间在 LSAnalysis 中已是一把"锁"。建一个**虚拟 softirq 线程**，把这把"锁"持有期间的所有 CCPGNode → 任何 softirq 注册回调入口（已在 ThreadAPIUtil 标注为 `tasklet_schedule` 等的 fork target）之间加 hb 边。（120 行）
5. **§3.3 RCU_SYNC**：`synchronize_rcu` / `rcu_barrier` 已是 JOIN 类型。在 join 节点之前的所有 `rcu_read_unlock` 节点 → join 之后的所有节点加 hb 边。`call_rcu(cb)` 把 `cb` 的入口节点登记到一张"延迟回调表"，所有 `rcu_read_unlock` → cb 入口加 hb 边。（150 行）
6. **§3.6 COMPLETION**：`complete(c)` → `wait_for_completion(c)` 的下一节点加 hb 边，按 alias 配对 c。（60 行）
7. **§3.4 REFCOUNT_LAST_PUT**：识别 `atomic_dec_and_test`/`refcount_dec_and_test`/`kref_put` 调用。需要识别"==true 分支"——通过 LLVM IR 的 br + icmp 模式。这个稍复杂，**先做最弱形式**：只在所有 dec_and_test 节点之间加 hb 边（保守）。完整实现可推迟。（200 行，**Phase A 暂时跳过完整版**）
8. **§3.7 LIFECYCLE_FLAG**：识别 IR 模式 `if (obj->flag) skip_or_return;` 后接 deref。这是模式匹配，**Phase A 也先放只识别字段名含 `freed`/`dead`/`going_away`/`shutdown` 的版本**，完整版推迟。（150 行）

**Phase A MVP 的 hb 边覆盖**：1+2+3+4+5+6 = 约 450 行核心逻辑，覆盖 §F1-F4 + 部分 §F5。
**Phase A 完整版**追加 7+8 = 总约 800 行。

### 6.3 复用 LSAnalysis 的 §3.2 实现伪代码

```cpp
void HBGraph::buildLockEdges(CCPG* ccpg) {
    auto* ls = LSAnalysis::getInstance();
    auto* ac = AliasChecker::getInstance();

    // 把所有 Lock 按物理锁聚类
    std::vector<std::vector<Lock*>> classes;
    for (Lock* l : ls->getLocks()) {
        bool placed = false;
        for (auto& cls : classes) {
            if (cls.empty()) continue;
            if (ac->isLockAlias(cls.front()->getAcquire(), l->getAcquire())) {
                cls.push_back(l);
                placed = true;
                break;
            }
        }
        if (!placed) classes.push_back({l});
    }

    // 同一物理锁的所有 release-acquire 对，跨线程加 hb 边
    auto* tct = ThreadCreationTree::getInstance();
    auto findThread = [&](CCPGNode* n) -> Thread* {
        for (Thread* t : tct->getThreads())
            if (t->getNodes().count(n)) return t;
        return nullptr;
    };
    for (auto& cls : classes) {
        for (Lock* lr : cls) {
            CCPGNode* relN = lr->getRelease();
            if (!relN) continue;
            Thread* tr = findThread(relN);
            for (Lock* la : cls) {
                if (la == lr) continue;
                CCPGNode* acqN = la->getAcquire();
                if (!acqN) continue;
                Thread* ta = findThread(acqN);
                if (!tr || !ta || tr == ta) continue;
                if (!tct->mayThreadsRunConcurrently(tr, ta)) continue;
                addEdge(relN, acqN, HBEdgeKind::LOCK_RELEASE_ACQUIRE,
                        "lock " + std::to_string(lr->getId()));
            }
        }
    }
}
```

注意：**这条边的语义不是"锁对永远 hb"**。是"如果运行时 release 先发生，则 acquire 一定 hb 之后"。
HBGraph 的所有边都是"可能 hb"的关系，求解 `hb(a,b)` 时给的是"存在 a→b 的 hb 链"=>"*可能* 排序"。
这正好对应 DSL 设计文档 §3 的语义。

## 7. Phase B 详细：Verifier 扩谓词

### 7.1 eval_same_location 实现

```cpp
bool HypothesisVerifier::eval_same_location(int n1, int n2, std::string& detail) {
    CCPGNode* a = ccpg_->getNodeByID(n1);
    CCPGNode* b = ccpg_->getNodeByID(n2);
    if (!a || !b) { detail = "node not found"; return false; }

    auto* pa = dynamic_cast<PhasarPointerAnalysis*>(
        AnalysisManager::getInstance()->getPointerAnalyzer());
    auto* M = pa ? pa->getModule() : nullptr;
    if (!M) {
        // Fallback：转交给旧 alias 实现
        return eval_alias(n1, n2, detail);
    }

    // Step 1: 取两节点对应的 IR 内存操作
    auto* ac = AliasChecker::getInstance();
    auto accs1 = ac->getMemoryAccessesFromLocation(a->getNodeLoc(),
        a->getFunction()->getContextSet().empty() ? Context() :
        **a->getFunction()->getContextSet().begin());
    auto accs2 = ac->getMemoryAccessesFromLocation(b->getNodeLoc(),
        b->getFunction()->getContextSet().empty() ? Context() :
        **b->getFunction()->getContextSet().begin());

    // Step 2: 用 SharedFieldKey 比对（字段级，比 Phasar alias 严格）
    for (const auto& a1 : accs1) {
        auto k1 = SharedFieldKey::fromValue(a1.pointerOperand, *M);
        if (!k1) continue;
        for (const auto& a2 : accs2) {
            auto k2 = SharedFieldKey::fromValue(a2.pointerOperand, *M);
            if (!k2) continue;
            if (*k1 == *k2) {
                detail = "same field: " + k1->toString();
                return true;
            }
            // Step 3: 字段不等时降级到 Phasar pointer alias
            if (ac->isAlias(a1.pointerOperand, a2.pointerOperand)) {
                detail = "phasar-alias (field-level disagrees)";
                return true;
            }
        }
    }
    detail = "no shared location";
    return false;
}
```

### 7.2 eval_op_kind 实现

```cpp
bool HypothesisVerifier::eval_op_kind(int node_id, OpKind expected,
                                      std::string& detail) {
    CCPGNode* n = ccpg_->getNodeByID(node_id);
    if (!n) return false;
    Node* cpgN = n->getCPGNode();
    if (!cpgN) return false;

    // 直接读 LLVM IR；CCPGNode 不带 Instruction* 时回退到代码字符串模式匹配
    const llvm::Instruction* I = cpgN->getLLVMInstruction();  // 已存在的接口
    OpKind actual = OpKind::OTHER;
    if (I) {
        if (llvm::isa<llvm::LoadInst>(I))         actual = OpKind::READ;
        else if (llvm::isa<llvm::StoreInst>(I))   actual = OpKind::WRITE;
        else if (llvm::isa<llvm::AtomicRMWInst>(I) ||
                 llvm::isa<llvm::AtomicCmpXchgInst>(I))
            actual = OpKind::RMW;
        else if (llvm::isa<llvm::CallInst>(I))    actual = OpKind::CALL;
    }
    bool ok = (actual == expected);
    detail = "op_kind=" + opKindName(actual) +
             " expected=" + opKindName(expected);
    return ok;
}
```

### 7.3 eval_hb / 糖

```cpp
bool HypothesisVerifier::eval_hb(int n1, int n2, bool expected,
                                 std::string& detail) {
    CCPGNode* a = ccpg_->getNodeByID(n1);
    CCPGNode* b = ccpg_->getNodeByID(n2);
    if (!a || !b) return false;
    bool actual = hb_->hbReachable(a, b);
    bool ok = (actual == expected);
    detail = "hb(" + std::to_string(n1) + "," + std::to_string(n2) + ")="
           + (actual ? "true" : "false") + " expected="
           + (expected ? "true" : "false");
    return ok;
}

bool HypothesisVerifier::eval_concurrent(int n1, int n2, std::string& d) {
    std::string d1, d2;
    bool ab = !eval_hb(n1, n2, false, d1);  // ¬hb(a,b)
    bool ba = !eval_hb(n2, n1, false, d2);  // ¬hb(b,a)
    d = "concurrent: " + d1 + "; " + d2;
    return ab && ba;
}
```

> **`eval_may_run_concurrently` 改造**：旧实现走 TCT。新糖 `concurrent(a,b)` 通过
> `hb` 推导，更精确（同一对线程在某些路径上仍可能存在 hb 关系）。两者**并存**：
> 旧 LLM hypothesis 仍能跑，新 hypothesis 用 `concurrent` 拿到字段级精度。

### 7.4 旧谓词作为糖的兼容映射

| 旧 | 新糖展开 | 行为差异 |
|---|---|---|
| `not_lock_protected(n)` | `¬∃ lock acquire L: hb(L_acq, n) ∧ ¬hb(L_rel, n)` 即"运行到 n 时无锁未释放" | 一致（依赖 HBGraph LOCK 边正确） |
| `same_lock(n1, n2)` | `LSAnalysis.isProtectedBySameLock(n1, n2)` | 不变（直接转发到 LSAnalysis） |
| `alias(n1, n2)` | `same_location(n1, n2)` 的弱形式 | 一致 |
| `may_run_concurrently(t1, t2)` | TCT.mayThreadsRunConcurrently | 不变 |

旧 4 个谓词的 eval_* 函数体改为转发到新实现 + 标 `[deprecated]` 在 detail 里给 LLM 看。

## 8. Phase C 详细：DetectorAgent prompt 重写

### 8.1 新 system prompt 结构

```
你是 C/C++ 内核并发漏洞检测器。
工具：get_vulnerability_surface / get_function_code / get_successors_chunked / propose_hypothesis / finish_detection
DSL：5 个原语 + 3 个糖（不需要懂内核 API 名字；同步逻辑在 hb 引擎里）

谓词：
  same_location(a, b) — 字段级 alias
  op_kind(a) ∈ {READ, WRITE, RMW, CALL}
  in_thread(node, thread)
  reachable(from, to) — 同线程 CFG 可达
  hb(a, b)            — 同步图存在 a→b 路径
  conflicts(a, b)     ≡ same_location ∧ (write∨RMW 任一边)
  concurrent(a, b)    ≡ ¬hb(a,b) ∧ ¬hb(b,a)
  unsafe_atomic_block(start, end, witness)
                       ≡ reachable(start,end) ∧ conflicts(witness, start∨end∨middle)
                         ∧ ¬hb(witness,start) ∧ ¬hb(end,witness)

工作流：
1. get_vulnerability_surface → 找 high-risk objects
2. 判断 patch 修的是哪一类：
   · plain race / lock missing / context missing / publish race  → conflicts ∧ concurrent
   · UAF / lifetime / NULL deref                                  → conflicts ∧ ¬hb(use,free)（expected:false）
   · TOCTOU / non-atomic RMW / bit ops                            → unsafe_atomic_block
3. propose_hypothesis 提交，看 verifier 反馈调整

8 个家族 few-shot（粘贴 §4 的 JSON 示例即可）
```

### 8.2 关键变更点

- `system_prompt` 从 ~100 行改为 ~70 行（更清晰）
- `propose_hypothesis` schema 的 `predicate` enum 增加 8 项
- `bug_category` 仍 free-form，但 prompt 提示用 `data_race` / `lifetime_race` / `atomicity_break` 三大类（对应 §4 三种判定模板）
- 移除"stop at 5 confirmed hypotheses" → 改为 surface-size 自适应（§4.6）

## 9. Phase D 详细：Surface 加权与 Top-N

### 9.1 `computeRiskScores` 改动（约 60 行）

在现有循环里增加 §4.5 的三类信号检测。`isScalarType` 通过 LLVM IR 类型直接判断
（`IntegerType` 且 bitWidth ≤ 64）；`isReadDominatedLoneWriter` 通过统计同 obj 的
Read/Write 次数与发起线程数；`isLifecycleFlagShape` 通过字段名正则
（`(_freed|_dead|_dying|_shutdown|going_away|is_dead|state)`）+ obj 在被 if 分支
保护的位置出现次数。

### 9.2 `toPromptString` token-budget（约 80 行）

```cpp
std::string VulnerabilitySurface::toPromptString(std::size_t top_n) const {
    constexpr std::size_t TOKEN_BUDGET = 60000;  // ~60% of 100K context
    constexpr std::size_t CHARS_PER_TOKEN = 4;

    std::stringstream ss;
    // ... 头部不变 ...

    std::size_t emitted_chars = 0;
    std::size_t i = 0;
    for (; i < shared_objects.size(); ++i) {
        std::stringstream entry;
        renderObject(entry, shared_objects[i], i + 1);
        std::size_t cost = entry.str().size();
        if (emitted_chars + cost > TOKEN_BUDGET * CHARS_PER_TOKEN) break;
        if (top_n > 0 && i >= top_n) break;
        ss << entry.str();
        emitted_chars += cost;
    }
    if (i < shared_objects.size()) {
        ss << "\n... and " << (shared_objects.size() - i)
           << " more (truncated by token budget; use get_object_details to inspect).\n";
    }
    return ss.str();
}
```

## 10. Phase E 详细：CPG 入口实体容错（根因 A）

### 10.1 问题画像（来自日志验证）

- **CVE-2016-7911 (`blk_ioc_init` / `block/blk-ioc.c`)**：
  ```
  [DEBUG getMain] Looking for: funcName=blk_ioc_init, fileName=block/blk-ioc.c
  [DEBUG getMain] methods from full path: 0
  [DEBUG getMain] methods from filename only: 0
  Warning: No methods found in file block/blk-ioc.c or blk-ioc.c
  ```
  入口名拼写没问题，问题是 **CPG 该 method 节点的 `FILENAME` 字段
  与 IR debug-info 的相对路径对不上**（kernel build 出来的 CPG 路径是 `/home/.../linux.git/block/blk-ioc.c` 绝对/不同前缀，IR 是 `block/blk-ioc.c`）。
- **CVE-2024-53136 (`shmem_*` 42 个 ops_table 成员)**：
  M0 已加 ops_table 自动发现，能挖出 42 个候选名，但 `cpg->findMethod()` 对 41 个返回 null。
  原因在 IR 里这些 static 函数被 LLVM 改名（例：`shmem_fault.123`、`__llvm.shmem_fault`）或
  CPG 索引按 `name`，IR 端按 `mangledName`，两边不一致。

现状 `CPG::findMethod()`（`include/CPG/CPG.h:116-138`）只覆盖 syscall 前缀变体（`__x64_sys_*` 等 10 种），
对 static 函数 mangling 与文件名不匹配 0 兜底。

### 10.2 设计：三层兜底匹配

```
findMethod(name, hint_file=optional)
    ├─ Layer 1: exact name + (file 末段相等 OR 无 file 信息)        ← 现状
    ├─ Layer 2: syscall 名称 variant 兜底                          ← 现状
    │
+   ├─ Layer 3 (NEW): static / LLVM-mangled 兜底
+   │    candidates =  strip_suffix(".\d+")
+   │                | strip_suffix(".llvm.\w+")
+   │                | strip_prefix("__llvm.")
+   │                | strip_prefix("local_")        // some kernel CONFIGs
+   │
+   ├─ Layer 4 (NEW): 文件路径归一化匹配
+   │    a. 把 CPG node 的 FILENAME 与 hint_file 都做：
+   │       - lowercase
+   │       - 取末 N 段（N=1,2,3 滑窗）
+   │       - 去 ".part.N" suffix
+   │    b. 任一段命中即视为同文件
+   │
+   └─ Layer 5 (NEW): "全失败" 时返回结构化 nullptr 并打印 lookup table
+        - 输出 CCPG 中所有 method 的 (name, file) 列表 head 50
+        - 让人 / LLM 自己挑：findMethodSuggestions(name) 返回最相似 5 个
```

### 10.3 文件清单

```
修改：
  include/CPG/CPG.h
    - findMethod(std::string name)：加 Layer 3/4 兜底
    - 新增 std::vector<std::string> demangleVariants(name)
    - 新增 std::vector<Node*> findMethodSuggestions(name, k=5)
  src/CCPG/CCPG.cpp
    - getMain() 调用处：失败时打印 suggestions head 5（替换现 [DEBUG] 行）
    - ops_table 自动发现：每个候选名通过新 findMethod 走完 Layer 3
  src/CPG/CPG.cpp（如有）
    - 同步实现新 helper
新增：
  test/test_findmethod_fallback.cpp    ← 单测：mangling、part.N suffix、kernel path
```

### 10.4 实现要点

```cpp
static std::vector<std::string> demangleVariants(const std::string& name) {
    std::vector<std::string> out{ name };

    // 去 ".\d+"  / ".llvm.\w+"  / ".part.\d+"  /  "__llvm." 前缀
    static const std::regex kSuffix(R"((\.(?:llvm\.\w+|part\.\d+|\d+|isra\.\d+|cold|constprop\.\d+))+$)");
    static const std::regex kPrefix(R"(^(?:__llvm\.|local_))");

    std::string s = std::regex_replace(name, kSuffix, "");
    if (s != name) out.push_back(s);
    std::string p = std::regex_replace(s, kPrefix, "");
    if (p != s)   out.push_back(p);
    return out;
}

// Layer 4：文件路径归一化命中
static bool fileLikelyEqual(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) return true;        // 任一缺失视为通配
    auto norm = [](std::string x) {
        std::transform(x.begin(), x.end(), x.begin(), ::tolower);
        return x;
    };
    std::string A = norm(a), B = norm(b);
    if (A == B) return true;
    // 末 1/2/3 段匹配
    auto tail = [](const std::string& x, int n) {
        size_t pos = x.size();
        for (int i = 0; i < n && pos > 0; ++i) pos = x.rfind('/', pos - 1);
        return pos == std::string::npos ? x : x.substr(pos + 1);
    };
    for (int n = 1; n <= 3; ++n) {
        if (tail(A, n) == tail(B, n)) return true;
    }
    return false;
}
```

### 10.5 验证 canary

| CVE | M6 现象 | E 完成后预期 |
|---|---|---|
| CVE-2016-7911 | zero_reports，`getMain` 找不到 `blk_ioc_init` | `getMain` 通过 Layer 4 命中；hypothesis 数 ≥ 1 |
| CVE-2024-53136 | 42→1 入口塌缩 | ops_table 候选命中数 ≥ 30/42；至少进入 LSAnalysis |
| 13 baseline HIT | — | 不退化（Layer 1/2 fast path 不变，新加 Layer 仅在原 fallback 失败时才走） |

> Phase E 只做"找得到"，不保证 LLM 一定 propose 到对的字段。若 E 完成后这 2 个 CVE
> 仍 MISS，那是 Phase B/C/D 的责任，不再算 E 的失败。

## 11. Phase F 详细：跨文件 patch 切片（根因 D）

### 11.1 问题画像

`scripts/prepare_cve.sh` 当前只编译命令行指定的 SOURCE_FILES（每个 CVE 在 `cve_inputs.json` 里手填），
然后 `llvm-link` 合并。多文件 patch（如 CVE-2024-43891 涉及 `drivers/gpu/drm/xe/xe_*.{c,h}` 5 个文件）若漏了任一 .c：
- patch 修改的函数可能不在 .ll 中 → CCPG 找不到 vuln method
- 跨文件的同步顺序断裂（A 的 fork 在 .c1，B 的实际工作在 .c2，TCT 看不到 cross-TU 关系）

### 11.2 设计：patch-driven 文件展开

> 不改检测器，只改预处理脚本与一个新的辅助 Python 工具。

```
prepare_cve.sh   <CVE-ID>   <fix_commit>   [<extra_files>...]
                                ↓
+   1. 默认 SOURCE_FILES = `git show --name-only $fix_commit | grep "\.[ch]$"`
+   2. 若用户给了 extra_files，与上面 union
+   3. patch_expander.py 把 SOURCE_FILES 中所有 .h 反查同目录 .c
+      （只取在 patch hunk 中出现过的函数的 enclosing .c）
+   4. 仍然按现有流程 compile→llvm-link
+   5. 若 link 后 .ll 总行数 > 200k：分两批 link 并保留 merged.ll + fallback.ll
```

### 11.3 文件清单

```
修改：
  scripts/prepare_cve.sh
    - 第 7-10 行：从 fix_commit 自动抽 SOURCE_FILES（user 显式给的视为补充）
    - 在 step 2 (collect) 后调用 patch_expander.py
新增：
  scripts/patch_expander.py
    - input: kernel_dir, fix_commit, seed_files
    - logic:
        a) 跑 git show --stat $fix_commit  → 拿到 patch 涉及的全部 .c/.h
        b) 对每个 .h，grep 同目录其它 .c 是否 #include 它，且
           include 它的 .c 中有 patch hunk 提到的函数名 → 加入种子
        c) 输出最终 SOURCE_FILES 列表 + 一个 .json 描述展开理由
  kernel_experiment/<CVE>/expansion_report.json
    - 记录：原始 seed / 扩展后列表 / 每个文件的 join reason
```

### 11.4 关键脚本草稿（patch_expander.py 主流程）

```python
def expand(kernel_dir: Path, fix_commit: str, seeds: list[str]) -> ExpansionReport:
    patch_files = run(["git", "-C", str(kernel_dir),
                       "show", "--name-only", "--pretty=", fix_commit]).splitlines()
    patch_files = [f for f in patch_files if f.endswith((".c", ".h"))]
    final = set(seeds) | set(patch_files)

    # 抽 patch 中所有引用到的函数符号（粗：取 hunk 行头部 word）
    syms = extract_changed_symbols(kernel_dir, fix_commit)

    # 同目录 .c 反向找 #include + 函数引用
    for h in [f for f in final if f.endswith(".h")]:
        dir_ = (kernel_dir / h).parent
        for c in dir_.glob("*.c"):
            text = c.read_text(errors="ignore")
            if f'#include "{Path(h).name}"' in text and any(s in text for s in syms):
                final.add(str(c.relative_to(kernel_dir)))

    return ExpansionReport(seeds=seeds, expanded=sorted(final), reasons=...)
```

### 11.5 验证 canary

| CVE | patch 文件数 | 旧 SOURCE_FILES | F 完成后预期 |
|---|---|---|---|
| CVE-2024-43891 | 5 | 用户填 1-2 个 | expansion 后 ≥ 4；merged.ll 含全部 patch 函数 |
| CVE-2025-37920 | 4 | 用户填 2 个 | expansion 后 = 4；merged.ll 含全部 patch 函数 |
| CVE-2024-40953 | 1 | 1 | 不变（单文件 patch 走 fast path） |

> Phase F 不能保证多文件 link 后一定 HIT（仍受 B/C 限制），但**保证 vuln method 出现在
> CCPG 里**，把"看不到"的部分让位给"看得到但判不准"的可分析问题。

### 11.6 与现状的兼容性

- 现 `cve_inputs.json` 的 `source_files` 字段保留为 user override，与自动展开 union；
- 不影响已制备完成的 CVE 目录（除非用户主动重跑 prepare）；
- 失败时（`git show` 缺 commit、patch 含二进制等）回退到现有行为，不阻断流水线。

## 12. 风险与回退

| 风险 | 触发条件 | 缓解 |
|---|---|---|
| HBGraph 边过密导致 hbReachable 多数返回 true | §3.5 BH 区间边把 softirq virtual thread 与全部进程上下文连成一片 | depth=16 的 BFS 上限 + 深度衰减权重；canary CVE-2024-41005 验证 |
| HBGraph 边漏识别导致 ¬hb 假阳性激增 | refcount/lifecycle 模式识别保守 | Phase A 跳过 §3.4/§3.7，先观察 §3.1-§3.3+§3.5+§3.6 的覆盖；漏识别记入 detail，让 LLM 据此调整 |
| 新谓词的 `expected:false` 让 LLM 误用 | 训练数据无此模式 | system prompt 显式给 §4 F5/F8 的 JSON few-shot |
| 旧 prompt 已稳定的 13 HIT 退化 | Phase C prompt 重写后 LLM 行为变 | 全 50 CVE 重跑前先在这 13 个上做对照；任何退化要回滚 prompt |
| Surface 加权后热点 obj 沉底导致原 HIT 中的真 race 漏掉 | 新加分 ≥ 旧加分 | 新加分上限 35 < 旧 has_free_operation 50；不会反客为主 |

## 13. 测试矩阵：8 family × canary CVE + 根因 A/D 兜底

每个 Phase 完成后必跑这 8 个 family canary，加上 13 个原 HIT 做"不退化"基线，
再加 Phase E/F 各自的兜底 canary。

**Family canary（驱动 Phase A/B/C/D 的有效性）：**

| Family | Canary CVE | 检测器期望（Phase 完成后） |
|---|---|---|
| F1 plain race + READ_ONCE | CVE-2024-40953 | ≥1 hypothesis 提到 `last_boosted_vcpu` 字段（哪怕 TP_RELATED） |
| F2 missing lock | CVE-2017-6346 | ≥1 hypothesis 走 `conflicts ∧ concurrent` |
| F3 missing BH | CVE-2024-41081 | ≥1 hypothesis 用 `hb` 谓词 + softirq virtual thread 并 expected:false |
| F4 publish race | CVE-2024-35977 | 不退化（M5 已 HIT）|
| F5 UAF | CVE-2024-43891 | ≥1 hypothesis 用 `¬hb(use, free)` 模式 |
| F6 TOCTOU | CVE-2025-38217 | ≥1 hypothesis 用 `unsafe_atomic_block` |
| F7 non-atomic RMW | CVE-2024-46704 | ≥1 hypothesis 用 `unsafe_atomic_block` 描述 RMW 三联 |
| F8 race-induced NULL deref | CVE-2025-38337 | ≥1 hypothesis 用 `conflicts(deref, null_write) ∧ ¬hb` |

**根因 A/D 兜底 canary（驱动 Phase E/F 的有效性）：**

| 根因 | Canary CVE | 检测器期望 |
|---|---|---|
| A. 入口找不到 | CVE-2016-7911 | `getMain` 返回 non-null；至少 1 条 hypothesis（不要求 HIT） |
| A. 入口找不到 | CVE-2024-53136 | ops_table 候选命中率 ≥ 70%；进入 LSAnalysis 不崩 |
| D. 跨文件视野 | CVE-2024-43891 | merged.ll 包含全部 patch hunk 中函数；至少 1 条 hypothesis 跨 ≥ 2 文件 |
| D. 跨文件视野 | CVE-2025-37920 | 同上 |

> **注**：Phase E/F 的 canary 不要求 HIT。只要保证"入口可见"与"代码可见"，
> Phase A/B/C/D 才能在它们身上发挥。HIT 与否记入全 50 CVE 重跑指标。

## 14. 行动顺序（建议）

> 排序原则：**ROI 优先 → 风险递增**。每一步都能独立产出可观察的指标。
> 总工期 ≈ 6.5 天；任意单步若产出不及预期可暂停切换到回退路径。

1. ★ **Phase D 先做**（半天，根因 C+E 立竿见影）：`VulnerabilitySurfaceGenerator::computeRiskScores` 加 3 类信号；token-budget toPromptString；hypothesis Top-N 自适应。立刻在 CVE-2024-40953 / CVE-2024-41005 上验证。**改动最小，回退最容易**。
2. ★ **Phase E 紧跟**（半天，根因 A）：`CPG::findMethod` 加 Layer 3/4 兜底 + suggestions。在 CVE-2016-7911 / CVE-2024-53136 验证 `getMain` 不再返 null。**这步若不做，后续 Phase A/B/C 对这 2 个 CVE 永远无效**。
3. **Phase F**（半天，根因 D，并行可做）：`prepare_cve.sh` + `patch_expander.py`。重制 CVE-2024-43891 / CVE-2025-37920 的实验目录，验证 merged.ll 文件覆盖率。**这步是脚本改动，与 C++ 编译流水线解耦，可与 Phase A/B/C 并行**。
4. **Phase A MVP**（1.5 天，根因 B 起步）：HBGraph + §3.1/§3.2/§3.6 三类边。dumpDot 出来人工核对 5 个不同 CVE 的输出。
5. **Phase B**（1 天，根因 B 主体）：Verifier 加 `same_location/op_kind/hb/conflicts/concurrent/unsafe_atomic_block` 6 个新方法 + schema 扩展。**旧谓词暂不动**。
6. **Phase C**（半天，根因 B 收尾）：Prompt 切换到 hb-DSL；用 13 个原 HIT 做不退化对照。
7. **Phase A 补全**（1 天）：加 §3.3/§3.5/§3.7 三类边。
8. 全 50 CVE 重跑 + `evaluate_recall.py` → 写 README §M7。

> 路径 1+2 单独完成后，先看根因 A/C 的 7 个 MISS（2 zero_reports + 5 surface 淹没）
> 是否变 HIT；如果是，说明 surface/入口判断对，可以更有信心地推进 A-C；
> 如果不是，说明 LLM 即使看到了正确字段也没用旧谓词描述对，
> **这反而更证明 Phase B/C 必须做**。

> Phase F 是独立的编译期改动，**完成时机不影响检测器代码合并**。
> 建议在 Phase D/E 跑通后立即启动 F，让全 50 CVE 重跑（步骤 8）能享受到
> 跨文件展开的覆盖率提升。

---

## 附：M7 文件改动清单（实现期一目了然）

```
新建（Phase A/B：HB-DSL 主体）：
  include/CCPG/HBEdge.h                  ← §4.1
  include/CCPG/HBGraph.h                 ← §4.2 (重命名 HB.h 或并存)
  src/CCPG/HBGraph.cpp                   ← §6 全部 build* 实现

新建（Phase F：跨文件切片）：
  scripts/patch_expander.py              ← §11.4 patch-driven 文件展开

新建（Phase E 单测）：
  test/test_findmethod_fallback.cpp      ← §10.3 mangling/path 兜底

修改（Phase A/B/C/D 主体）：
  include/Query/HypothesisVerifier.h     ← §4.3 加 6 新 eval + ctor 加 HBGraph*
  src/Query/HypothesisVerifier.cpp       ← §7 新 eval 实现 + 旧谓词转糖
  src/LLMUtil/DetectorAgent.cpp          ← §8 prompt 重写 + propose_hypothesis schema
  src/Query/VulnerabilitySurfaceGenerator.cpp   ← §9 risk score 加权 + token budget
  include/Query/VulnerabilitySurfaceGenerator.h ← toPromptString 签名加 token_budget
  src/llm_main.cpp                       ← LSAnalysis.build() 后插 HBGraph.build()
  src/CCPG/CMakeLists.txt                ← 加 HBGraph.cpp

修改（Phase E：CPG 入口容错）：
  include/CPG/CPG.h                      ← §10.4 demangleVariants / fileLikelyEqual / Layer 3/4
  src/CPG/CPG.cpp                        ← 同步 helper 实现（如有）
  src/CCPG/CCPG.cpp                      ← getMain() 失败时打印 suggestions（替换现 [DEBUG] 行）

修改（Phase F：跨文件切片）：
  scripts/prepare_cve.sh                 ← §11.3 从 fix_commit 自动抽 SOURCE_FILES + 调 patch_expander
  scripts/batch_prepare.sh               ← 透传新参数（若有）

删除/废弃：
  include/CCPG/HB.h                      ← 内容迁移至 HBGraph.h，文件保留但只 include 新头作兼容

测试新增（M7 验收）：
  kernel_experiment/canary/M7_baseline.json   ← §13 12 个 canary（8 family + 4 兜底）的预期摘要
  scripts/run_canary.sh                       ← 跑全部 canary + diff 旧 confirmed_hypotheses

文档更新（M7 收尾）：
  kernel_experiment/README.md            ← 加 §M7 章节，含三维评估对比 + 根因覆盖回顾
  kernel_experiment/HYPOTHESIS_DSL_DESIGN.md  ← 状态从 "designed" 标为 "implemented in M7"
```
