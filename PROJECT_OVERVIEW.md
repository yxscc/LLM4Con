# LLM4Con — 项目总览（双故事架构）

> 本文档描述 `LLM4Con` 仓库的整体设计。仓库在**同一套静态基座**之上维护**两个独立的检测方法（"两个故事"）**，分别对应两篇拟投稿的论文。两者共享静态分析与验证设施，但**核心 idea 严格分离（red line）**，互不污染。
>
> - **新故事（Lace）**：领域知识库 + 场景化微调 pattern。入口 `--agent-mode`（默认）。
> - **老故事（Thread-Contract）**：逐线程并发契约 + 显式交织推理 + 涌现式单失配规则。入口 `--legacy-workflow --abl-contract on`。
>
> 相关文档：`README.md`（构建/上手）、`THREAD_CONTRACT_REDESIGN.md`（老故事契约重设计过程）、`FACTS_EXTRACTOR.md`（事实抽取）。

---

## 0. 目录

1. 设计哲学与 red line
2. 共享静态基座（CCPG / 线程入口 / 漏洞面 / 验证器）
3. 新故事：Lace（机制知识库 + 场景微调 pattern）
4. 老故事：Thread-Contract（并发契约 + 交织推理）
   - 4.1 契约的形式化定义
   - 4.2 单失配 / 涌现式组合规则
   - 4.3 当前流水线（折叠式）
   - 4.4 静态组合流水线（Phase A/B/C，实验中）
5. 良性 race（tearing / READ_ONCE / WRITE_ONCE）的处理
6. 数据集
7. 运行方式（CLI / 环境变量 / 脚本）
8. 评估与消融
9. 代码地图

---

## 1. 设计哲学与 red line

并发缺陷检测的根本困难在于：缺陷不在单条语句里，而在**线程交织产生的、违反某种隐含顺序约束**的执行中。纯静态分析能识别锁、特定 API 等**显式同步**，但无法识别由 flag、状态机、引用计数等**语义保护**所建立的顺序——这正是 LLM 的用武之地。

仓库用两种思路解决这一问题，并坚持以下 **red line** 防止两篇文章互相稀释创新点：

| | 新故事 Lace | 老故事 Thread-Contract |
|---|---|---|
| 出发点 | 传统静态分析依赖**期望泛化的 pattern**，但 pattern 泛化性不足 | 缺陷的本质是**顺序（order）失配**，需逐线程刻画顺序义务再做交织 |
| Key idea | 以并发缺陷**机制库（KB）**为先验，让 LLM 在**具体场景**中**微调 pattern** | 为每个线程生成**并发契约**（顺序/同步的 assume-guarantee），再**组合**契约推出涌现缺陷 |
| 核心词汇 | mechanism / pattern / rule instantiation | contract / assume / guarantee / 单失配 |
| 入口 | `--agent-mode`（默认 true） | `--legacy-workflow --abl-contract on` |
| 关键代码 | `MechanismKnowledgeBase`, `DetectorAgent`, `Rule` | `ContractGeneratorAgent`, `InterleavingAnalysisAgent`, `AgentManager::runAnalysisContractMode` |

**严格隔离的内容**：老故事的交织 agent **不** import 机制库 / `retrieve_mechanism_priors`（新故事签名）；也**不**用固定的 Rule 6-模板 / `start_rule` 填槽。老故事的缺陷类别是从交织推理中**涌现**的标签，而非模板选择。

---

## 2. 共享静态基座

两套方法跑在同一条静态管线上（纯静态、无 LLM）：

### 2.1 CCPG（Concurrency-aware Code Property Graph）
- 由 LLVM bitcode（`*.ll` / `merged.ll`）+ 源码构建，融合控制流 / 数据流 / 调用图。
- 关键源码：`src/CCPG/CCPG.cpp`, `src/CPG/`, `src/PhasarUtil/PhasarPointerAnalysis.cpp`。

### 2.2 线程入口识别（ThreadCreationTree）
- 从**线程创建机制**（而非字符串匹配）出发识别并发执行流：`kthread_run`、workqueue、timer、softirq/tasklet、IRQ handler、RCU callback、以及用户态可触达的 syscall / fops 入口等。
- **Kernel Module Mode**：当整库 bitcode 找不到 `main` 时，用结构信号自动发现入口。结构信号（`StructuralSignal`）：
  - `SIG_SECTION_INIT`（`.init.text` / `.exit.text`）
  - `SIG_EXPORT_SYMBOL`（`EXPORT_SYMBOL[_GPL]`）
  - `SIG_SYSCALL`（`SYSCALL_DEFINE*` 元数据）
  - `SIG_OPS_MEMBER`（结构体全局中的函数指针，即注册回调）
  - `SIG_INDIRECT_FORK`（取地址 / 传入 fork 类 API）
- **入口分层（tiering）**：strong 信号无条件保留；纯 export 的 weak 入口会被**可达性剪枝**（若被另一入口在调用图里到达，则它是被调用者而非独立并发根）。目的：既覆盖真实并发根，又避免"入口爆炸"（入口越多，后续分析负担越大）。
- 已知尺度问题：整库 SYZBOT case 上仍有**面膨胀**（syscall 的 ABI 变体重复 `__x64_/__ia32_/__se_/__do_/__sys_`、间接派发的叶子函数被当成独立根等），是当前优化方向之一。

### 2.3 漏洞面（VulnerabilitySurface）
- 文件：`include/Query/VulnerabilitySurfaceGenerator.h`, `src/Query/VulnerabilitySurfaceGenerator.cpp`。
- 把"哪些共享对象被哪些线程怎样访问"沉淀成结构化的面：

```
SharedObject { name, type, accesses[], 风险标志..., accessing_thread_ids }
ThreadAccess { thread_id, function_name, containing_function, access_type(Read/Write/Free),
               node_id, code_snippet, location, is_lock_protected, protecting_lock }
```

- 派生**风险标志**（用于分诊/聚类，不直接当缺陷）：`has_unprotected_write`、`has_free_operation`、`has_cross_thread_rw`、`has_inconsistent_locking`、`has_scalar_torn_access`（READ_ONCE/WRITE_ONCE 与裸访问混用）、`has_read_dominated_lone_writer`、`has_missing_atomic_annotation`、`has_list_mutation`、`is_self_race`（可重入入口与自身并发副本竞争）。
- 漏洞面是**两套方法共同的召回底线**：它枚举了所有跨线程冲突访问对（≥1 写）。LLM 在其上做精度/语义判断。

### 2.4 假设验证器（HypothesisVerifier）
- 文件：`include/Query/HypothesisVerifier.h`, `src/Query/HypothesisVerifier.cpp`。
- LLM 提出的每条 `Hypothesis`（`role -> CCPG node_id` + 约束）都要被**静态接地（grounding）**后才记账。谓词词汇（M7 5+3）：

| 谓词 | 含义 |
|---|---|
| `concurrent {a,b}` | a、b 可并行（may-happen-in-parallel） |
| `conflicts {a,b}` | a、b 访问同一位置且 ≥1 写 |
| `same_location {a,b}` | a、b 访问同一内存位置 |
| `op_kind {node,kind}` | kind ∈ READ\|WRITE\|RMW\|CALL |
| `not_lock_protected {node}` | 不在保护锁区内 |
| `same_lock {n1,n2}` | 同锁持有（用于**反驳**） |
| `reachable {from,to}` | 控制流可达 |
| `hb {a,b,expected}` | a happens-before b（expected=false 断言**无** HB） |
| `in_thread {node,thread}` | node 在某线程内执行 |

- 典型数据竞争：`concurrent{w,r} + conflicts{w,r} + not_lock_protected{w}`；典型 UAF：`concurrent{free,use} + same_location{free,use} + hb{free,use,expected:false}`。

---

## 3. 新故事：Lace（机制知识库 + 场景微调 pattern）

**动机**：传统静态/规则检测把"并发缺陷模式"写死成固定 pattern，期望其泛化到所有场景，但真实代码里同一机制的表现千差万别，pattern 经常**匹配不上**或**误匹配**。

**Key idea**：不直接拿死 pattern 扫代码，而是
1. 维护一个**机制知识库**（传统并发缺陷的机制先验）；
2. 让 LLM 看过具体 surface 后，通过 API / 代码项 / 风险标志等信号**召回**相关机制 pattern；
3. 在**具体场景**中把 pattern **微调/实例化**成可被静态验证器接地的规则。

**实现**：
- `MechanismKnowledgeBase`（`include/LLMUtil/MechanismKnowledgeBase.h`）：`Pattern { id, mechanism, description, template_hint, bug_category_hint, object_name_regex, code_terms_any, function_terms_any, risk_flags_required/any, positive_examples, negative_hints, ... }`；知识落盘在 `kernel_experiment/knowledge_base/mechanism_patterns.json`。
- `DetectorAgent`（`include/LLMUtil/DetectorAgent.h`）：mechanism-first rule-instantiation，单 LLM 会话，先提交结构化的上下文分析（`context_analyses`），再实例化"意图级"规则，内部经 `HypothesisVerifier` 接地；用 fingerprint 去重。
- 入口：`AgentManager::runAnalysisAgentMode()`（`--agent-mode`，默认）。

---

## 4. 老故事：Thread-Contract（并发契约 + 交织推理）

**动机**：静态分析能找到线程、访问、锁和可能的危险操作，但很难恢复框架 API、状态变量、refcount/RCU、callback drain 等同步语义。Lace 让 LLM 从静态证据中恢复这些语义，再把它们写成可组合的 ThreadContract。

**Key idea**：为每个线程生成**并发契约**：`assume` 记录局部语句/区域对环境线程的安全需求，`guarantee` 记录同步对执行历史产生的效果。多线程组合时，checker 判断这些 guarantee 是否足以 discharge requirement；若仍允许违反性交错，则形成候选缺陷。

### 4.1 契约的形式化定义

数据结构：`include/LLMUtil/ConcurrencyContract.h`。线程触碰的共享资源或协议对象以 clause 记录：

```cpp
struct OrderClause {
    std::string resource;        // 共享对象/字段（展示用）
    int         objectId;        // surface 共享对象下标（静态组合按它做跨线程匹配；-1=未锚定）
    std::vector<std::string> sites;     // "func @ file:line" provenance
    std::vector<OrderReq>    assume;    // 局部 statement/region 的安全需求
    std::vector<SyncProv>    guarantee; // 代码提供的同步效果
};
std::vector<OrderClause> clauses;       // 每项共享资源一条
std::vector<std::string> ordering;      // 跨 clause 的线程级有向序（如 "writes_before_publish"）
```

关系来自一个**封闭、与子系统无关的代数**，但不再按 bug symptom 设计。

**assume（局部安全需求）**
| 关系 | 含义 |
|---|---|
| `ORDER(A, B)` | A 必须发生在 B 之前 |
| `CONFLICT_MEDIATED(A, B)` | A/B 可能冲突，需要 order、non-overlap 或 linearized protocol 调停 |
| `REGION_ISOLATED(region, hazards)` | region 执行时不应被指定 hazard 插入 |
| `STABLE_DURING(region, P)` | region 执行期间资源或谓词 P 必须保持有效 |
| `PROGRESS_ENABLED(wait, enabler)` | 可选扩展：wait 需要匹配的 enabler |

**guarantee（Level-0 同步效果）**
| 关系 | 含义 |
|---|---|
| `ORDER(a, b)` | a happens-before b |
| `EXCLUDE(token, region, mode)` | 同 token 且冲突 mode 的 region 不能重叠 |
| `LINEARIZE(object, operation)` | operation 在线性化点生效 |
| `WAIT(wait_event, condition)` | wait_event 不能在 condition/enabler 满足前通过 |

RCU、refcount、callback close-and-drain、handoff、validation/retry 等高级机制都作为这些 Level-0 atoms 上的宏表示。

**诚实原则**：guarantee 只列代码**真正提供且确实覆盖该资源/协议**的同步效果。守护别的字段、wait 别的 domain、或纯 state check 都不能直接 discharge 当前 requirement。

> 注：还保留了一组 legacy 描述字段（`role`, `summary`, `sharedVariables`, `synchronization`, `intendedParallelThreads`），仅在无 order clause 时作展示回退。承重的是上面的 assume/guarantee。

### 4.2 Requirement Discharge / 涌现式组合规则

对某条 requirement `R`，候选缺陷产生于：

```
(1) 某线程的 concrete statement/region 有 R；
(2) 另一线程或同线程并发实例有 event 可违反 R；
(3) 所有相关 guarantee 组合后仍无法 discharge R；
(4) static surface 仍允许该违反性交错：concurrent ∧ ¬HB/¬NO_OVERLAP ∧ conflicts。
```

缺陷类别只是后验标签：例如 `STABLE_DURING(use_region, live(R))` 被 retire 违反可解释为 lifetime/UAF，`REGION_ISOLATED(check_use_region, invalidators)` 被冲突写插入可解释为 atomicity/check-use。

### 4.3 当前流水线（折叠式，默认）

`AgentManager::runAnalysisContractMode(useContracts=true)`：

1. **Phase 1 — 漏洞面**（纯静态）。
2. **Phase 2.5 — 对象分诊（Object Triage）**：一次廉价 LLM 调用，剔除不可能承载真实并发缺陷的对象（纯统计计数器、不透明匿名对象等），生命周期载体（free/list/self-race）强制保留。fail-open。可用 `LACE_DISABLE_OBJECT_TRIAGE=1` 关闭。
3. **冲突簇聚类**：把被**同一组线程**访问的对象聚成一个会话（session 数随"不同线程集"而非"对象数"增长；不丢对象，召回安全）。超大簇按上限切块。
4. **Phase 3 — 逐簇交织分析**（`InterleavingAnalysisAgent::analyzeCluster`）：每个簇一个 LLM 会话，**内联派生**该簇对象上的逐线程 requirement/guarantee，按 discharge 逻辑判定，对每个真实违例调用 `propose_race_hypothesis`（经验证器接地后才记账）。源代码预加载进 prompt 以减少读工具调用；并带"lockset focus"把注意力引到无锁/跨锁的竞争前沿。

> "折叠"指：把独立的契约生成阶段并入 Phase 3——契约仍在理论上存在（会话内逐线程派生），但不再单独成阶段，省去重复的子句产出开销。

### 4.4 静态组合流水线（Phase A/B/C，实验中，`LACE_STATIC_COMPOSE=1`）

为了让**契约真正承重**、并把昂贵的"逐簇交织推理"拆成"廉价静态 + 小范围校准"，新增一条可门控的三段式流水线（默认关闭，folded 路径不变）：

- **Phase A — 逐线程契约**（agent，每线程一次）：用 surface 上该线程触碰的对象、当前线程访问、同一对象的环境线程访问摘要、以及预加载源码 **seed** `ContractGeneratorAgent`，产出 requirement/guarantee；每条 clause 通过 `object_id` **锚定到 surface 对象下标**，供跨线程确定性匹配。每个线程的源码**只读一次**（在 fan-in case 上摊薄重复读）。
- **Phase B — 确定性 requirement-discharge 组合**（无 LLM）：在 surface 的冲突访问对上机械检查 requirement 是否被 guarantee 消解——
  - **召回底线**来自 surface 冲突（与今天聚类同源，不更差）；
  - **消解（discharge）**：lock/EXCLUDE 走 surface common-lock + contract guarantee 的保守 AND；RCU/refcount/barrier/join/drain 等 hard non-lock guarantee 默认作为强证据交给 Phase C 复核，只有在 `LACE_TRUST_HARD_NONLOCK_DISCHARGE=1` 的消融/提速配置下才直接消解；
  - **分档**：`STABLE_DURING`+free/retire → high(lifetime)；`REGION_ISOLATED`+conflicting write/free → high(atomicity)；`CONFLICT_MEDIATED` 或其它 requirement 冲突 → medium；无任何 assume 的裸冲突 → low；其中**良性 torn-scalar**（scalar-torn 且无 free/list/self-race）默认**抑制**（`LACE_COMPOSE_KEEP_LOW=1` 可保留）。
- **Phase C — agent 校准**（只看存活候选）：把 Phase A 的契约 + Phase B 的候选清单（含理由/分档）喂给交织 agent，逐条**确认 / 否决**，并允许**补充**组合漏掉的危害，最终经验证器接地产出 `Hypothesis`。无存活候选的簇**直接跳过**。

成本/精度权衡：在**高 fan-in**（一个线程出现在很多簇）上，Phase A 的"每线程读一次"摊薄 + Phase B 的消解/抑制能显著减少昂贵会话；在低 fan-in 的中小 case 上，Phase A 的逐线程开销可能让总时长持平或略增。该路径正在 case 上实测调参。

---

## 5. 良性 race（tearing / READ_ONCE / WRITE_ONCE）

数据集里（尤其 SYZBOT）存在**良性撕裂竞争（tearing race）**：对读多写少的配置标量做无注解的并发读写，内核用 `READ_ONCE()/WRITE_ONCE()/data_race()` 注解"承认但容忍"——其修复**只是加注解**，并非真正的逻辑缺陷。surface 上它们表现为 `has_scalar_torn_access` / `has_missing_atomic_annotation` 等。

处理策略（与用户达成一致）：
- 这类对象**良性不报**——对象分诊会过滤，静态组合 Phase B 默认抑制 low-tier 的 torn-scalar，交织 agent 的 prompt 也明确"纯统计/诊断计数器的丢更新是良性，跳过"。
- 纯 annotation 且不影响内存安全、对象身份或生命周期的样本已从 active benchmark 中排除：详见 `kernel_experiment/dataset_benign_exclusions.json`。
- 这些 benign case 的原始目录已移动到 `kernel_experiment/backup_benign_excluded_20260611/`，可按需恢复。

---

## 6. 数据集

位置：`kernel_experiment/`。原始数据集共 **100** 个 case（**65 个 `CVE-*` + 35 个 `SYZBOT-*`**）。

当前 active benchmark 已排除良性开发者注解竞争：
- `dataset_benign_exclusions.json` 中 28 个 benign case 已移动到 `backup_benign_excluded_20260611/`。
- 顶层 active case 目录剩 **72** 个。
- runner 还会根据 `dataset_exclusion_candidates.json` 排除 3 个仍在顶层、但 `dual_thread_model=false` 的 case。
- 因此当前有效评估集为 **69** 个 case。详见 `kernel_experiment/dataset_active_manifest.json`。

每个 case 目录的典型结构：

```
kernel_experiment/<CVE-xxxx-yyyy | SYZBOT-xxxxxxxx>/
├── src/                    # 该缺陷涉及的内核源码子集（覆盖线程入口与 call graph）
├── <tu>.ll                 # 各翻译单元 bitcode
├── merged.ll               # 整库/合并 bitcode（whole-module 分析常用）
├── flow_annotation.json    # 人工/模型标注的近似 ground truth
└── entry_points.txt        # 可选：显式入口配置（--entry-config）
```

- `flow_annotation.json`：含缺陷摘要（`summary`）、涉及的两个（或多个）线程、核心指针/变量、`coverage.selected_bitcode`（推荐使用的 bitcode）等，用于**校验静态能力**（至少识别出两个线程及关键变量）和**近似召回判定**。
- 数据集做过修订：早期部分 case 的 `src` 不全（覆盖不到线程入口或 call graph）或 `merged.ll` 不匹配，现已大致修正；使用时需注意 **CPG 与最新 src/bitcode 的一致性**。
- 其它目录：`kernel_experiment/knowledge_base/`（新故事机制库）、`kernel_experiment_v2_staging/`（暂存）、`kernel_experiment/_skipped_compile_issues/`（编译问题暂排除）。

---

## 7. 运行方式

### 7.1 构建
```bash
# 全量重建（会清空 build 目录）
bash build.sh            # Release，产物在 Release-build/
# 增量构建（推荐日常）
CC=clang CXX=clang++ cmake --build Release-build -j 8
```
主要产物：`Release-build/llm_detector`（检测器）、`Release-build/llm_comparison`。

### 7.2 命令行
```bash
llm_detector --input-bc <bitcode.ll> --input-src src [--entry-config entry_points.txt] \
  <模式开关> \
  --llm-provider openai --llm-url <URL> --llm-key <KEY> --llm-model <MODEL>
```

| 开关 | 含义 |
|---|---|
| `--agent-mode`（默认 true） | **新故事**：mechanism-rule DetectorAgent |
| `--legacy-workflow` | **老故事**：thread-contract 交织入口（覆盖 `--agent-mode`） |
| `--abl-contract on\|off` | 老故事消融：`on` 注入逐线程契约；`off` 让交织 agent 仅凭源码推理 |
| `--only-thread-entry` | 只建 CPG/CCPG 并解析线程入口后退出（省 token，做静态扫描） |
| `--input-bc / --input-src / --entry-config` | bitcode / 源码目录 / 入口配置 |

### 7.3 环境变量

| 变量 | 作用 |
|---|---|
| `LACE_STATIC_COMPOSE=1` | 老故事启用 **Phase A/B/C 静态组合**流水线（默认关闭=折叠式） |
| `LACE_COMPOSE_KEEP_LOW=1` | 静态组合保留 low-tier（良性 torn-scalar）候选（recall 消融用） |
| `LACE_EARLY_EXIT_AFTER_SURFACE=1` | Phase 1 出 surface 后即退出（静态扫描/统计入口与对象规模） |
| `LACE_DISABLE_OBJECT_TRIAGE=1` | 关闭对象分诊（消融） |
| `LACE_TRACE_STDERR=1` | 实时打印 LLM 交互 trace（盯进度用） |
| `LACE_EXP_REACH_PRUNE_STRONG=1` | 实验：把可达性剪枝扩展到 strong 入口（默认仅诊断，见 §2.2 面膨胀） |

### 7.4 便捷脚本（测试用，位于 `/tmp`）
- `run_one_test.sh <CASE> [suffix]`：单 case 折叠式老故事运行（带 trace）。
- `run_compose.sh <CASE> [suffix]`：单 case **静态组合**运行（`LACE_STATIC_COMPOSE=1`；`KEEP_LOW=1` 透传 `LACE_COMPOSE_KEEP_LOW`）。
- `static_one.sh <CASE>`：仅静态（`LACE_EARLY_EXIT_AFTER_SURFACE=1`），用于全量静态扫描、统计线程/对象规模。

---

## 8. 评估与消融

- **召回判定**：建议交给 LLM 比对 `flow_annotation.json` 的 GT 线程/变量与产出 `Hypothesis`（确定性字符串匹配不可靠：匹配上未必对应，匹配不上也可能对应）。
- **精度**：确认的 `Hypothesis` 计 TP/FP；重复 FP 视作 FP。LLM 判定"真实 race"的可标注为"llm 判断真实"另列。
- **主要消融轴**：
  - 老故事 `--abl-contract off`：去掉契约 framing（衡量契约贡献）。
  - `LACE_DISABLE_OBJECT_TRIAGE=1`：去掉对象分诊。
  - `LACE_STATIC_COMPOSE` on/off：折叠式 vs 静态组合（衡量"契约承重 + 确定性组合"的精度/成本收益）。
  - `LACE_COMPOSE_KEEP_LOW`：良性抑制对召回/精度的影响。
- 产出落盘：输出目录下的 `vulnerability_surface.json`、`confirmed_hypotheses.log` 等。

---

## 9. 代码地图（主要文件）

| 子系统 | 路径 |
|---|---|
| 入口 / CLI | `src/llm_main.cpp` |
| 编排（两故事调度） | `src/LLMUtil/AgentManager.cpp`, `include/LLMUtil/AgentManager.h` |
| 新故事：机制库 / 检测 agent | `src/LLMUtil/MechanismKnowledgeBase.cpp`, `src/LLMUtil/DetectorAgent.cpp`, `src/LLMUtil/Rule.cpp` |
| 老故事：契约生成 | `src/LLMUtil/ContractGeneratorAgent.cpp`, `include/LLMUtil/ConcurrencyContract.h` |
| 老故事：交织 / 校准 agent | `src/LLMUtil/InterleavingAnalysisAgent.cpp` |
| 静态：CCPG / 指针分析 | `src/CCPG/`, `src/CPG/`, `src/PhasarUtil/PhasarPointerAnalysis.cpp` |
| 静态：线程入口 | `src/CCPG/ThreadCreationTree.cpp` |
| 静态：漏洞面 | `src/Query/VulnerabilitySurfaceGenerator.cpp`, `include/Query/VulnerabilitySurfaceGenerator.h` |
| 验证器 | `src/Query/HypothesisVerifier.cpp`, `include/Query/HypothesisVerifier.h` |
| 共享工具集（LLM 读/导航工具） | `src/LLMUtil/SharedToolKit.cpp` |
| 数据集 | `kernel_experiment/`（CVE-* / SYZBOT-* / knowledge_base/） |

---

*本文档随实现演进；老故事的静态组合流水线（§4.4）与静态面膨胀治理（§2.2）为当前活跃的优化方向。*
