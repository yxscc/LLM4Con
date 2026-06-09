# Thread-Contract 入口（老故事）重构设计蓝图

> 状态：设计阶段（动手前蓝图）。本文件只描述目标架构与改造方案，不含已落地代码。
> 适用范围：`llm_detector` 的 **legacy / thread-contract 入口**（`--legacy-workflow`）。
> 不影响：`--agent-mode`（新故事 / surface + 机制 KB）。

---

## 0. 一句话目标

把老故事（**逐线程契约 + 显式交织推理**）的 agent 从"古早的 CCPG 节点级工具 + 僵硬 rule 模板 + 粗暴 FIFO 上下文"，
重构成"**source-first 读码 + 自由交织推理 + file:line→节点 grounding**"，
并把 LLM 开销从"**对线程数 T 敏感（O(T) 契约 + O(P) 线程对）**"降到"**对共享对象数敏感（≈O(对象数)）**"，
**全程不引入机制知识库 / pattern 特化**（那是新故事的签名贡献，必须排除在本入口之外）。

---

## 1. 两个故事的红线（必须严格保持）

| | 老故事（本文件） | 新故事（`--agent-mode`） |
|---|---|---|
| 知识单元 | **逐线程 ConcurrencyContract** | **机制知识库（传统并发缺陷 pattern）** |
| 推理方式 | **显式交织推理**，bug 类型从推理涌现 | **取 KB pattern → 落到场景特化/微调** |
| 解决痛点 | 让模型"感知线程"、理解线程交互 | 传统静态 pattern 不能泛化 → LLM 适配到场景 |
| 标志组件 | `ContractGeneratorAgent` + 交织 agent | `MechanismKnowledgeBase` + `retrieve_mechanism_priors` |

**红线规则：**
1. **机制 KB 绝不进老入口。** `MechanismKnowledgeBase` / `retrieve_mechanism_priors` / pattern 特化只属于新入口。老入口的 bug 假设**只能来自契约 + 交织推理**。
2. **老入口移除僵硬的 6-pattern rule 模板**（`start_rule` 选 TOCTOU/DataRace/UAF/…）。pattern 作为固定分类会让老故事看起来像"先选 pattern 再套"——这正是新故事批判的对象，也会让两篇撞车。pattern 退化为**内部验证词汇**，不在 LLM 工作流里 up-front 出现。
3. **可共享、不承载故事卖点的只有"底座 plumbing"**：静态基座、source-first 读码工具、上下文管理、`file:line→CCPG 节点` resolver + 确定性验证器。

---

## 2. 现状（已在当前仓库）

- 老逻辑已存在并以 flag 形式接好：`src/llm_main.cpp` 用 `--agent-mode`(默认) / `--legacy-workflow` 切换。
- `AgentManager::runAnalysisLegacy()`（`src/LLMUtil/AgentManager.cpp`）= standalone `LLM4Con-thread-contract` 仓库的 `runAnalysis()`，**逻辑一致**。
- agent 源码与 standalone 近乎一致（`ContractGeneratorAgent`/`Rule.*`/`ConcurrencyContract.h`/`ThreadPair.h` 0 差异；`ParallelAnalysisAgent` / `FindingThreadEntryAgent` 当前仓库还更新一点）。
- **两个入口共享同一套（已优化的）静态基座**：CPG + Phasar(`PhasarPointerAnalysis`) + CCPG/`ThreadCreationTree` + `HBGraph`。
- 结论：**不需要从 standalone 回灌代码**；要做的是"在新底座上现代化老 agent"。

---

## 3. 成本 / 复杂度分析（为什么老方法对线程数敏感）

记号：`T`=线程根数；`C`=不同入口行为类数(≤T)；`P_conf`=过 MHP（生命周期 + `doEntriesHaveSharedData`）后的"真正共享冲突对"数；`O`=共享对象数；`k`=平均每个对象被几个线程触碰。

| 阶段 | 代码事实 | LLM 开销 |
|---|---|---|
| Phase 1 契约 | `runAnalysisLegacy` 对**所有**线程跑 `generateContractForThread`，每个 ≤**25 轮**多轮工具对话 | **O(T)**，无去重、无剪枝 |
| Phase 2 交织 | 遍历 C(T,2)；`mayHappenInParallel` 静态剪枝到 `P_conf`；每个存活对 `analyze_parallelism` ≤**45 轮** | **O(P_conf)**，热点对象处退化为二次 |
| Phase 4 验证 | `rule->verify()` 确定性（可选过 `VerificationAgent`） | ≈0 |

净 LLM 开销 ≈ **O(T) + O(P_conf)**。两个痛点：

1. **契约阶段是纯 O(T) 且无剪枝/去重**：循环在剪枝之前，连"跟谁都不冲突"的线程都先花一个 25 轮 session。
2. **交织阶段在"热点共享对象"上退化为二次**：MHP 的共享剪枝只判"是否共享任一冲突字段"，一组都碰同一热点全局的入口两两都通过 → C(k,2) 个 45 轮 session。

> 关键事实：`ThreadCreationTree::mayHappenInParallel`（内核入口分支，parent==null）**已**做两道静态剪枝——① 生命周期不兼容；② `PhasarPointerAnalysis::doEntriesHaveSharedData`。所以 `P_mhp` 已经≈`P_conf`，进一步优化要落在**契约阶段**和**session 粒度**上。

### 我们最近静态优化的副作用
为提召回做的"多 root"（导出符号、ops 表、跨 TU 孤儿、self-race）把 `T` 推高（rcu=76，部分 merge 模块 200+）。
- 对新方法：几乎无感（聚合成对象，DetectorAgent 基本一个 session）。
- 对老方法：线性更多契约 + 二次更多潜在冲突对。**同一静态基座，对两种方法代价敏感度截然不同**——这正是"老版对线程数更敏感"的根因。

---

## 4. agent 现代化（三个病灶 → 三个改造）

### 病灶 1：工具集是 CCPG 控制流节点粒度
`ParallelAnalysisAgent` 现状：`get_function_entry_node` → `get_successors_chunked`（每次 10 个 ORDER 边节点）→ `nominate_node_for_role`（LLM 必须吐**精确 node_id**）。
- 慢（图导航吃光 45 轮预算）、反直觉（模型读连续源码远强于读割裂 node 碎片）、把"静态不全"泄漏进 agent（CCPG 缺节点 → 模型指不到竞争行）、ID 管道脆。

**改造 1：source-first、语义化、低基数工具**
- 主视角读码：复用新 side 的 `get_function_code(name)`（按名返回源码，见 `DetectorAgent`），新增 `read_thread_entry(thread_id)`（入口源码 + 与共享状态相关的 callee 闭包，连续代码 + file:line）。
- `get_callees/get_callers` 保留，返回**名字 + 源码片段**，不返回 node_id。
- 复用语义查询：`get_lock_protection(obj)`、`check_reachability(from,to)`（`DetectorAgent` 已有），回答模型真正的问题，免手动走 CFG。
- 竞争点定位：LLM **引用 `file:line` / 代码片段**，而非 `nominate_node_for_role(node_id)`。
- `get_successors_chunked / get_function_entry_node` 降级为"函数体被截断时"的 fallback，移出主路径。

### 病灶 2：僵硬的 rule-role 模板（违反故事红线）
**改造 2：去模板，自由交织推理 + 内部验证词汇**
- 删除 LLM 面向的 `start_rule(选 pattern) / nominate_node_for_role / propose / confirm` 这套。
- 改为：模型用自然语言描述**有害交织**（线程 A 在 X 行做什么、线程 B 在 Y 行做什么、为什么交织出 bug），并引用冲突源码行。
- bug 类型作为模型输出的一个 `bug_category` 字段（涌现，不是 up-front 选模板）。
- pattern/规则只在**验证器内部**作为检查词汇存在（见第 5 节 grounding）。

### 病灶 3：粗暴 FIFO 上下文截断
现状 `Conversation::prune_history` 按**消息条数**删**最老**消息 → 先删掉最值钱的任务设定/契约/源码，留下近期 node dump；且单条巨型工具输出能撑爆窗口。

**改造 3：token 预算 + 钉住 + 压缩**
- 预算按 **token**（每条消息估算），非消息条数。
- **钉住核心**：system prompt + 涉及线程的契约 + 任务陈述，永不裁剪；只裁中间 tool 调用/观察 chatter。
- **压缩代替删除**：超预算时把旧 tool 观察摘要成"已知事实"便签（如"func X 第12行无锁读 g_foo；func Y 第40行持锁 L 写 g_foo"）替换原始多轮。
- **瘦身工具输出**：只返回必要内容（带行号源码，而非全节点数组），大函数体分页 + 截断。
- 实现层面：可在 `Conversation` 基类引入可选的 token 预算 + pin 列表 + 压缩回调，老/新 agent 都能受益（属于公共底座，不污染故事）。

---

## 4.6 契约定义：order/sync 的 assume-guarantee 演算（动手基准，已拍板）

> 老故事的理论根：**并发缺陷 = 一条"被要求的执行顺序/原子性约束"在某个可行交织下未被同步建立。** 契约 = 每个线程对共享状态的 **order 要求(assume/rely)** 与它用同步**提供的 order(guarantee)**；缺陷 = 某条被要求的 order 没有任何线程的同步去建立，且存在可行的违反交织。
> 关键定位：**order 是统一的中间表示 / 检查演算，不号称"并发的本质"。** 语义层（内存/值）仍承重——它**产生** order 要求（"用已释放内存是 UB" ⇒ 要求 `prec(use, free)`），order 演算只负责**统一表达并 grounding** 这些要求。

### 4.6.0 范围与模型假设（先划清楚，避免被反例打沉）
- **只做 safety + 顺序一致（SC / 假设 DRF-SC）那一段**：UAF/double-free、未初始化读、null-deref race、data race、**原子性违反（atomicity violation）**、order violation、发布序（publish-before-init）。
- **明确 out of scope（由浅入深，后续增量）**：
  - **弱内存可见性 / 缺屏障**（missing `smp_wmb`/release-acquire、双重检查锁乱序、out-of-thin-air）：需要 `po/sw/rf/mo`+fence 的内存模型，代价极大，且内核逻辑并发 CVE 基准里极少。源码程序序里 `data ≺ flag` 看着有序，Lamport HB 看不到违反——这类不强求。
  - **liveness**：死锁、活锁、饥饿、lost-wakeup 挂死——是"所有执行 / 公平性 / 无限轨迹"的性质，不是某条执行上违反某个序；死锁另有成熟的环检测专门方法。
- 即便如此，覆盖面（race + 原子性 + order violation + 发布序 + UAF/未初始化）已远超"只做简单 data race / 死锁 / UAF"的传统检测器。

### 4.6.1 核心模型（order-centric assume-guarantee）
- 契约**逐线程**，沿用 assume-guarantee 的**格式**（rely-guarantee, Jones 1983）——但只借**分解形式**，不继承其 soundness/completeness（我们的契约是 LLM **推断、未验证**，且做的是**反驳**而非证明）。
- 对线程 T 触碰的每一项共享资源 R：
  - **assume(T,R)**（rely）：T 为自身正确性**要求**的 order（如"我用 R 之前 R 不得被释放"）——这是**推断出来的意图**，surface 里没有。
  - **guarantee(T,R)**：T 用同步**建立**的 order（持锁串行、发布前已初始化、释放前已 quiesce、refcount 归零才释放…）。
- **两层结构**：语义/值层**产生** order 要求（为什么这条 order 必需 + 违反后叫什么）；order 演算层**表达并检查**。对**原子性 / order violation / 发布序**，order 层**严格更强**（内存谓词根本说不出来）；对内存安全，是**等价改写**。

### 4.6.2 同步 = 两条正交轴（不说"恰好两种"）
order 由同步建立，沿两条**正交**轴刻画（现代内存模型把"原子性"和"可见性"分开，可单独出现）：
- **互斥 / 原子性（exclusion）**：让冲突区段不交错（谁先都行，就是不能插进来）。原语：锁、原子 RMW、`preempt_disable`/`local_irq_save`（按上下文互斥）。
- **排序 / 可见性（ordering）**：让 A **必然先于** B（有向）。原语：发布（`rcu_assign`）、quiesce 等待（`synchronize_rcu`/`flush_work`/…）、join/barrier、refcount 归零才释放。
- 另有三种**复合结构**单独标注、不硬塞进两轴：**乐观校验-重试**（seqlock/TM）、**纪元/grace-period**（RCU 读侧）、**计数**（refcount）。

### 4.6.3 关系代数（封闭演算，守红线）
契约里能出现的全部"话" = 一个**小的、与子系统无关的封闭关系代数**，不是 CVE 目录：
- **order 要求（出现在 assume）**：
  - `prec(a, b)`：事件 a 必须 happens-before b（有向）。
  - `atomic(region)`：region（≥1 个事件）不得被冲突外来事件交错；单事件退化为"无并发冲突写"（即 data race 的条件）。
  - `count_guarded(R, free)`：R 的释放须在引用计数归零之后（N 元计数不变式）。
- **同步提供（出现在 guarantee）**：
  - `serialize(L, region)`：互斥（锁/上下文）使 region 串行。
  - `order(a ≺ b via m)`：协作同步 m（发布/quiesce/join/barrier）建立有向序。
  - `counts(R)`：引用计数纪律维持 `count_guarded`。
> **红线**：`prec/atomic/count` 是**通用关系**，不是 bug 签名；**绝不按 CVE 增删**（那会退化成新故事的机制 KB）。新机制只允许扩"同步原语如何 ground"（即 §4.6.7 的 M），**不扩这个代数、也不扩缺陷类别**。`atomic` 是 region-interval 推理、`count_guarded` 是计数不变式——它们是代数里**另外的构件**，不是被成对 `prec` 规则"吸收"的。

### 4.6.4 数据结构（语义层，最终 C++ 头动手时定稿）
```
ThreadContract {
  thread_id; entry_function; role; summary;   // role/summary 仅信息性
  clauses:  [Clause];        // 每项共享资源一条
  ordering: [Predicate];     // 跨 clause 的线程级有向序（如 writes_before_publish）
}
Clause {
  resource;                  // 共享对象/字段（+ surface object id）   ← 静态先验(候选)
  sites: [SourceRef|node_id];// 实现该 clause 的源码站点 + op_kind     ← 静态先验
  assume:   [OrderReq];      // T 要求的 order：prec / atomic / count_guarded   ← 全部 LLM 推断（核心增量）
  guarantee: {
    protection: [SyncProv];  // serialize / order / counts             ← 静态给候选, LLM 裁决 keep/reject/add
    // 危险动作（free/mutate/publish）由 sites 的 op 诱导，作为失配里"违反 order 的一方"
  }
  // assume 与语义 guarantee/裁决结论均带 provenance（出处行 / caller）
}
```

### 4.6.5 唯一的失配 schema（report-when，非 ⟺；守住红线）
一条 schema，跑在 §4.6.3 的封闭关系代数上：
```
Report(R, Ta, Tb, sa, sb)  ⇐
  ∃ ρ ∈ assume(Ta, R)                          // Ta 要求的一条 order(prec/atomic/count_guarded)
  ∧ ∃ 事件 e@sb ∈ Tb 使 e 违反 ρ                // Tb 的冲突事件(free/mutate/publish/…)
  ∧ ¬∃ g ∈ ⋃guarantee · establishes(g, ρ)       // 没有任何同步建立 ρ
  ∧ feasible_violation:
        concurrent(sa,sb) ∧ ¬ordered(sa,sb) ∧ ¬protected(sa,sb) ∧ conflicts(sa,sb,R)
```
- **用 `⇐`（report-when），不写 `⟺`**：`ρ` 与语义 guarantee 是 LLM **推断、未验证**，两个方向都不成立（幻觉→FP，漏推→FN）。
- `establishes / ordered / protected / conflicts / concurrent` 是 **arbiter 提供的 oracle 关系**（精确 MHP / 可行性不可判定）；**soundness 相对 oracle 成立**，精度/召回随 oracle 精度走（故为**经验性质**，见 §4.6.6、§4.6.8）。
- **类别是事后涌现的标签，不驱动检测**：`ρ=prec(use,free)`→UAF；`ρ=prec(init,read)`→未初始化读/发布序；`ρ=atomic(单访问)`→data race；`ρ=atomic(区段)`→原子性违反；`ρ=count_guarded`→refcount UAF。规则从不"去找某类 bug"。
> 红线：没有 KB、没有缺陷签名；只有"被要求的 order 没被同步建立"这一条通用逻辑。这是"单一规则跑在封闭关系代数上"，不是 2/4 条目的缺陷目录。

### 4.6.6 三角色分工（静态噪声先验 / LLM 语义仲裁 / 静态结构 grounding）
- **① 静态先验（LLM 之前）—— 只给候选，不给结论**：surface 给每个对象的访问站点、op、**候选锁/已知同步 API**。明确标"候选，可能不全、也可能根本不护这个对象"。
- **② LLM 语义仲裁（契约的真正增量）**：读真实源码 + caller，产出 `assume`（order 要求）、语义 `guarantee`，并对候选保护**逐条 keep / reject / add**（reject=这把锁护的是别的字段；add=静态看不到的 flag/状态守卫），每条带 provenance。
- **③ 静态结构 grounding（LLM 之后）—— sound 的结构过滤器**：把 schema 编译成确定性谓词在 CCPG 上核验，抓 LLM 的**结构性错误**（对象认错、其实不并发、被 HB 排开）；并**确认**被 keep 的锁是否真在两站点持有。**不替模型重判"护不护"**。
> **精度与召回都是经验性质**（LLM 两个方向都参与裁决）：靠"flag/语义守卫静态拿不到"的论证 + 静态把候选锁喂给模型（治"看漏"）+ 实测来辩护；**不宣称"精度构造即成立"**。残余风险：误否真锁 / 误加不成立的守卫都伤召回，漏看语义守卫伤精度——评估里**专测"保护裁决准确率"**。

### 4.6.7 grounding：schema → HypothesisVerifier 谓词（结构层确定性核验）
| schema 项 | 产出的 Hypothesis 约束 |
|---|---|
| concurrent(sa,sb) | `concurrent{a,b}` |
| ¬ordered（无 HB） | `hb{violator→use, expected:false}`（intra-thread 用 `reachable`） |
| ¬protected | `not_lock_protected{a}`（必要时 `same_lock{a,b}` 取假）— 仅核验 LLM **keep** 的锁 |
| same resource R / conflicts | `same_location{a,b}` / `conflicts{a,b}` |
| op：free=CALL, use=READ | `op_kind{free,CALL}`、`op_kind{use,READ}` |
| 线程绑定 | `in_thread{site,tid}` |
> 即：order/sync 演算是**模型产出的语义层**，schema **编译成现有验证器约束**做确定性 grounding。**只有结构层（concurrent/ordered/protected/conflicts via M）与模型无关、是 sound 的结构过滤器；触发器（`ρ` 被违反、裁决保护）是 LLM 主张的**。不宣称"soundness 不依赖模型"。

### 4.6.8 两个 soundness 引理（都挂前提，诚实切分）
- **模式可表达性（schema soundness）**：线程并发相关行为 = 其共享事件投影 + 同步；契约记录之 ⇒ 是 sound 抽象（rely-guarantee 可组合性，限 Φ 可表达范围）。**前提**：SC / 假设 DRF-SC；共享事件集按路径/值**过近似**（控制流依赖私有值时取上界）；别名分析 sound；**保留发布前的"私有"初始化写**（它们正是 `writes_before_publish` 的关键，不能当私有步骤丢掉）。
- **单线程零误报（sequential soundness）**：单线程时程序序**全序化**其事件 ⇒ 所有 `prec` 满足、无外来事件违反 `atomic` ⇒ schema 不触发。**前提**：单执行体；**无信号/中断/DMA/重入**（否则同上下文也有"外来事件"）；**线程集抽对**——实践第一大误报源是"多抽了一个并不存在的并发线程"，接静态线程识别能力（见 §STATIC_CAPABILITY 修复工作）。
- **诚实切分**：以上两条是 **schema 层可证**；**抽出来那份契约是否忠实**（LLM 有没有漏记共享事件 / 同步）是**经验问题**，不混进 soundness 声明。

### 4.6.9 实例

**(a) flag 语义守卫（motivation 头牌）—— 同一套推理同时讲召回与精度**
```
reader：
  clause R = obj->data:
    sites: [ if (atomic_read(&obj->active)) ; use(obj->data) ]
    assume: [ prec(use, free(R)) ]          // 用 R 之前 R 不得被释放（active 守它的生命周期）
    guarantee.protection: []                 // 无锁；active 是否“充分守卫”由 LLM 判
closer：
  clause R = obj->data:
    sites: [ obj->active = 0 ; free(obj->data)（经 ->deactivate 回调）]
    op 诱导危险事件: free(R)
    guarantee.protection: []                 // 未对读者做 quiesce/refcount/锁
```
- **召回向**：LLM 判 `active` 守卫**不充分**（check→use 之间能被清零+释放，TOCTOU）→ 无 guarantee `establishes(prec(use,free))` → grounding 确认 `concurrent`/同对象/无 HB → **报 UAF**。静态对 flag 语义是盲的，这条召回不可替代。
- **精度向（镜像）**：若 closer 改为 `synchronize_rcu()` 等读者退场，LLM 判守卫**充分** → **add** 一条 `order(free ≺ … via quiesce)` guarantee → `establishes` 成立 → **否决**；而语法 race 检测器因"看不到锁"会误报。
> 红线：此处只能讲成"reader 的 order 要求 vs closer 的同步是否建立它"，**不能讲成"flag 是个 guard pattern、按场景特化"**（那是新故事的签名）。

**(b) CVE-2015-7550（读/撤销 → UAF）映射到 `prec`**
reader 要求 `prec(use, free(user_key_payload))`；updater 的 `kfree_rcu` 是 `free` 事件；reader 非 `rcu_reader` 且不持 `key->sem` ⇒ 无同步 `establishes` 该 `prec` ⇒ `concurrent`/无 HB ⇒ **UAF**。补丁 = 给 reader 加 `serialize(key->sem)` 这条 guarantee。"reader 要求用前不被释放"在 access 列表/锁信息里都没有——正是契约该装、surface 装不了的。

### 4.6.10 对 ablation 的含义（让 off 真的丢东西）
- `--abl-contract on`：交织 agent 拿到 assume(order 要求)/guarantee(同步) → schema 能判依赖意图的缺陷（如 reader 的 `prec(use,free)`），并用 guarantee（裁决后的保护）**否决 benign**。
- `--abl-contract off`：只给 access 列表 → 丢意图层 → 预期**召回下降**（意图/order 型缺陷）或**精度下降**（无法否决 benign）。
> 这才让"逐线程契约"的贡献在数据上立得住，而非 on/off 无差别。

### 4.6.11 良构义务（WF —— "契约若能这样构建"的前提）
- **(WF1)** 每个 `sites` 站点都是真实 CCPG 节点（provenance 可 ground）。
- **(WF2)** 候选保护来自静态 `is_lock_protected/protecting_lock`，LLM 只能在候选上 keep/reject 或 add 有 provenance 的语义守卫，**不许凭空造锁**。
- **(WF3)** 每条意图文字（`assume`、语义 `guarantee`、`ordering`、以及"reject 某候选锁"的裁决）都**附 provenance**（出处行/caller）。"reject 静态锁"是最弱的一环（无静态可复核）→ 评估专测。
- **(WF4)** LLM 只能产出 §4.6.3 关系代数内的关系，不得自由散文。

---

## 5. 目标架构：老入口新 pipeline

```
[静态基座: CPG + Phasar + CCPG/ThreadCreationTree + HBGraph]   (与新入口共用)
        │
        ▼
[静态冲突关系 + 共享对象]  ← 复用 VulnerabilitySurfaceGenerator 的"每线程共享访问/锁"结果
        │                   (仅作为剪枝/预填的 oracle，不引入 KB/pattern)
        ▼
Phase 1  逐线程契约 (ContractGeneratorAgent, 现代化)
   • 惰性: 只为"参与 ≥1 个 P_conf 对"的线程生成契约
   • 去重: 按入口函数缓存 (相同入口的多线程只生成一次)
   • 静态预填: shared vars / 锁 由 surface 预填, LLM 只补 role/intent (1~2 轮)
        │
        ▼
Phase 2  按【共享对象】交织推理 (取代 per-pair 的 ParallelAnalysisAgent)
   • 粒度: 一个 session 看"碰对象 O 的那 k 个线程"的契约 + 按需读码
   • 工作流:
       1) 静态辅助过滤: 用内核 HB 经验 + check_reachability/get_lock_protection
          确认哪些访问真冲突、能交织 (不走 CFG)
       2) 自由假设: 自然语言描述有害交织 + 引用冲突源码行 (file:line)
       3) grounding: file:line → CCPG node_id, 组装 Hypothesis
        │
        ▼
Phase 3  grounding + 验证 (复用新 side 底座)
   • query::Hypothesis { id, description, bug_category, severity,
                         nodes(role->node_id), constraints }
   • query::HypothesisVerifier::verify(h)  (确定性: reachable/same_location/
       op_kind/hb/conflicts/concurrent/not_lock_protected/same_lock …)
   • (可选) llm_client::VerificationAgent 二次 FP 过滤
        │
        ▼
StatefulBugDetector::detectFromHypotheses(hypotheses, ccpg, verifier)
   → 与 --agent-mode 共用同一输出链路
```

### 复杂度对照

| | 现状 | 重构后 |
|---|---|---|
| 契约 session | O(T)，每个 25 轮 | O(C_conf)，每个 1~2 轮 |
| 交织 session | O(P_conf)，每个 45 轮，热点二次 | O(O)，每对象一个 |
| 对 T 敏感度 | 线性~二次 | 基本脱敏（∝ 共享对象数） |

---

## 6. 复用 / 独有 / 排除 三分清单

**复用新 side 已有底座（公共 plumbing，不承载故事卖点）：**
- 静态基座：`PhasarPointerAnalysis`、`CCPG`、`ThreadCreationTree`（含 `mayHappenInParallel` 剪枝）、`HBGraph`。
- 读码 / 语义工具：`get_function_code(name)`、`get_lock_protection`、`check_reachability`（`DetectorAgent`）。
- 共享对象 / 每线程访问：`query::VulnerabilitySurfaceGenerator`（只取其静态产物做剪枝/预填）。
- grounding / 验证：`query::Hypothesis`、`query::HypothesisVerifier`（`include/Query/HypothesisVerifier.h`）、`StatefulBugDetector::detectFromHypotheses`、`AgentManager::getConfirmedHypotheses()`、可选 `VerificationAgent`。
- 上下文管理：在 `Conversation` 基类增强的 token 预算 / pin / 压缩。

**老方法独有（故事卖点，需改造/保留）：**
- `ConcurrencyContract` + `ContractGeneratorAgent`（加惰性 / 去重 / 静态预填）。
- 交织推理 agent（**重写**：per-object、去 rule 模板、source-first、自由假设）。
- `runAnalysisLegacy()` 主控（改成调用新交织 agent，产出 `Hypothesis` 而非 `ThreadPair`+`Rule`）。

**严格排除（新故事签名，绝不进老入口）：**
- `MechanismKnowledgeBase` / `retrieve_mechanism_priors` / 任何 KB pattern 特化。
- `start_rule`/`nominate_node_for_role`/`Rule` 6-pattern 模板（作为 LLM 工作流）。

---

## 6.5 Ablation 设计（取代 old-vs-new 对比）

**原则：ablation 不做"旧 agent vs 新 agent"的整体对比，而是测每个组件的增益。** 因此不保留旧 `--legacy-workflow` 路径；新老入口的对照价值在于"故事不同"，不在于"实现新旧"。老入口要内建若干**组件开关**，以便定量回答"这个组件值不值得"。

可消融组件（按重要性）：
1. **contract on/off（头号 ablation，验证老故事论点）**：
   - on：交织 agent 先看每线程 `ConcurrencyContract` 再推理；
   - off：交织 agent 直接读源码做交织推理，不给契约。
   - 目的：**证明"逐线程契约"对召回/精度有正贡献**——这是老故事的核心卖点。
2. **static-prefill on/off**：契约由 LLM 探索生成 vs 由 surface 静态预填后 LLM 补全。测"静态预填省了多少 token、是否损质"。
3. **granularity：per-object vs per-pair**：测 session 粒度对开销/召回的影响（per-pair 仅作 ablation，不作主路径）。
4. **context-compaction on/off**：token 预算+压缩 vs 朴素截断。测上下文机制对长会话质量的影响。

实现要求：上述开关用 config/flag 暴露（如 `--abl-contract=on|off`），默认值=主路径配置；**旧 Rule 模板路径不作为 ablation 项**。

---

## 7. 新增 / 改动接口草案

> 仅草案，动手时再定稿。

- `read_thread_entry(thread_id) -> {entry_name, source_with_line_numbers, relevant_callees:[{name, snippet}]}`
- `resolveSourceRef(file, line, symbol, snippet) -> node_id`（**三重定位**：行号 + 符号名 + 代码片段，做最近邻匹配以抗内联/宏展开错位；**LLM 不可见 node_id**）。
- `ContractGeneratorAgent`：新增 `generateContractsLazyDedup(conflictingThreads, surface)`；契约结构按 surface 预填 shared vars / 锁。
- 新交织 agent（暂名 `InterleavingAnalysisAgent`）：`std::vector<Hypothesis> analyzeObject(const SharedObject&, const std::map<Thread*, ConcurrencyContract>&)`。
- `Conversation`：可选 `setTokenBudget(n)` / `pinMessage(idx)` / 压缩回调（默认关闭，不改新 agent 行为）。

---

## 8. 实施阶段（动手时的顺序，每步可独立验证）

> P0~P1 是公共底座，不改变老入口对外行为，风险低，可先做、先回归。

- **P0 上下文底座**：`Conversation` 加 token 预算 + pin + 压缩（默认关闭）。单测：长对话不丢 pin 项。
- **P1 工具底座**：把 `read_thread_entry` + `resolveSourceRef` 接好；老 agent 仍走旧流程，仅多出可用工具。
- **P2 契约现代化**：惰性 + 去重 + 静态预填。验证：契约 session 数 ≈ C_conf；产出契约字段不劣化。
- **P3 交织 agent 重写**：per-object + 去模板 + 自由假设 + grounding，产出 `Hypothesis`。
- **P4 主控切换**：`runAnalysisLegacy()` 改为 P2+P3 链路 → `detectFromHypotheses`；老 `ParallelAnalysisAgent`/`Rule` 6-pattern 模板路径从主路径**移除**（不作为 ablation 保留）。同时接好 §6.5 的组件开关（`--abl-contract` 等）。
- **P5 冒烟 + 对比**：挑 1~2 个 CVE（含一个高线程数模块，如 rcu）跑 `--legacy-workflow`，确认 ① 召回不退 ② session 数/时长显著下降 ③ 不沾 KB。

> 注意：动手前先对 `--legacy-workflow` 在当前静态基座做一次**基线冒烟**（它久未运行，可能 bit-rot），以便重构有 before/after 对照。

---

## 9. 召回安全性论证（每个优化为何不丢 ground truth）
- **惰性契约**：跟谁都不共享冲突位置的线程在数据竞争语义上不可能竞争 → 其契约对 bug 检测无用。剪枝来源与新 side 同源（`doEntriesHaveSharedData`）。
- **按入口去重**：相同入口函数的线程 footprint 相同（内核可重入入口尤其成立）→ 契约可复用。
- **per-object 交织**：模型在一个 session 看到碰 O 的全部 k 个线程上下文，能覆盖任意线程对；信息量不减反增。风险仅在 k 很大时的 token 预算 → 用第 4 节的压缩 + 分页 chunk 兜底。
- **去 rule 模板**：grounding 仍由 `HypothesisVerifier` 确定性把关，soundness 不依赖 LLM 选对模板。

---

## 10. 决策记录 & 待定问题

**已拍板：**
- [x] **resolveSourceRef 三重定位**：行号 + 符号名 + 代码片段 + 最近邻匹配，抗内联/宏展开错位。（见 §7）
- [x] **不保留旧 `--legacy-workflow` 作为 ablation**：ablation 改为测组件效果（contract 等），旧 Rule 模板路径从主路径移除。（见 §6.5、§8-P4）
- [x] **契约 = order/sync 的 assume-guarantee 演算**：并发缺陷 = 被要求的 order(prec/atomic/count_guarded) 未被同步建立；order 是统一 IR、非"并发本质"，语义层产生要求、order 层表达检查。失配为 `⇒`(report-when) 非 `⟺`。旧描述性契约（role/summary/vars/locks）作废。（见 §4.6.1、§4.6.5）
- [x] **范围 = safety + SC/DRF-SC**：覆盖 race/原子性违反/order violation/发布序/UAF/未初始化；**弱内存可见性 + liveness（死锁/活锁/饥饿）明确 out of scope**（由浅入深，后续增量；死锁另有专门方法）。（见 §4.6.0）
- [x] **三角色分工 + 精度/召回为经验性质**：静态=噪声先验（候选锁/API，不全也不准）；LLM=双向语义仲裁（推断 order 要求 + keep/reject/add 保护）；静态 grounding=sound 结构过滤器（不替模型判"护不护"）。**不宣称"精度构造即成立 / soundness 不依赖模型"**。（见 §4.6.6、§4.6.7）
- [x] **关系代数冻结、不按 CVE 增删**（原待定问题 2 已拍）：契约词汇 = 封闭关系代数 `{prec, atomic, count_guarded / serialize, order, counts}`；新机制只扩"同步如何 ground"(M)，**绝不扩代数或缺陷类别**（否则退化成新故事机制 KB）。`assume` 默认补全：无保护读者的"隐式独占"统一表达为 `atomic(单访问)`，且需 `atomic(R)` 这类原子原语作为 guard，避免退化成纯语法 race 检测器。（见 §4.6.3）

**待定（动手过程中再拍）：**
1. per-object session 的 `k` 上限与 chunk 策略（热点全局可能 k 很大）。
2. 基准核查：50+ CVE 里弱内存/liveness 类占比（用数据撑住 §4.6.0 的 scope 声明，确认不丢召回）。
3. `atomic(region)` 的 region 边界选取与 `count_guarded` 的计数事实如何 ground（N 元/区段构件的落地）。
