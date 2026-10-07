# LACE v3：静态穷举候选 + agent 引证判定

> 状态：设计阶段，尚无代码落地。具体流程的细节待讨论后补充到本文件。
> 分支：`lace-v3`，从 `lace-v2`（3ff95ba）切出。`lace-v2` 保留论文路线（Phase A 契约 + L2 检查器）。
> 定位：面向挖掘真实内核并发缺陷的工具，不以论文创新点为目标。

---

## 0. 一句话目标

静态分析不精确，agent 的召回无法保证。v3 把两者的分工固定下来：
**检查哪些访问对由静态穷举决定，agent 只能删除候选，并且删除必须引用代码证据。**

---

## 1. 为什么要改：v2 里召回实际卡在 agent 上

v2（L2 路线）的流程：

| 步骤 | 谁做 | 做什么 | 产出 |
|---|---|---|---|
| 建图 | 静态 | Joern 从源码建 CPG；clang 编 IR；Phasar 建类型层次、调用图（CHA）、别名集 | 两套程序表示 |
| Phase 0 | agent | 找项目自定义的锁/线程 API 包装函数 | benchmark 上基本为 0 |
| 入口 | 人工 / 静态 | benchmark 用 `dataset_entrypoints.json`；hunt 用逃逸规则 | 线程根函数 |
| CCPG | 静态 | 上下文传播、锁集不动点、HB 图（fork/join、锁、RCU 宽限期） | 锁集、HB 边 |
| Phase 1 surface | 静态 | 按（结构体类型, 字段偏移）归并访问，≥2 线程且有写/释放即为共享对象 | 共享对象、冲突对 |
| Phase 2.5 分诊 | agent | 丢弃它认为不会出 bug 的对象 | 保留对象 |
| 预算裁剪 | 规则 | 对象 >80 时全局/线程集上限；每线程契约 ≤20 对象；每会话 ≤10 对象 | 更少对象 |
| Phase A 契约 | agent | 每线程写 requirement（MustPrecede / MustBeMediated / MustBeAtomic）和 guarantee（Order / Wait / Exclude / AtomicOp）；可 `declare_no_obligation` 跳过对象 | 每线程契约 |
| Phase B 检查 | 静态 | 每条 requirement 用 guarantee 顺序边、静态同步 HB、Exclude/AtomicOp 令牌、surface 锁字符串判定是否满足 | 未满足的 requirement = 候选 |
| Phase C 校准 | agent | 逐候选 keep/reject，只删不加 | 报告 |
| 去重 | 静态 | 合并同根因 | `bugs.txt` |

问题：

1. **候选只来自 Phase A 写下的 requirement**（`AgentManager.cpp`：
   "Candidates are NOT generated from bare surface conflicts here -- only from undischarged requirements"）。
   一个真实缺陷要被报出，必须依次通过：surface 有它 → 分诊没丢 → 预算没裁 → **agent 为它写了 requirement**
   → B 没 discharge → C 没删。L2 模式下覆盖补全轮被跳过（`!l2Mode_`），没有任何机制检查每个冲突对都被覆盖；
   `conflict_pairs` 只用于线程排序。实例：`CVE-2013-1792` surface 有 24 个对象，契约为空时报告为 0。
2. **B 的两类证据都不稳**：静态锁字符串跨上下文求并（`findProtectingLock`），共享辅助函数会虚假声称持锁；
   agent 的 Exclude 令牌由各线程独立写出，靠文本归一化跨线程匹配。
3. **自研 harness 性价比低**：`src/LLMUtil` 约 1.1 万行、约 25 个硬上限，出过空循环、会话被杀、fail-open 等问题；
   带 read/grep 的评审 subagent 做 Phase C 的工作反而更好。

---

## 2. v3 流程

| 步骤 | 谁做 | 做什么 | 产出 |
|---|---|---|---|
| S1 范围与编译 | 静态 | 模块文件 + 引用其导出符号 / ops 表的调用方文件（从 compile DB 找） | IR、源码树 |
| S2 入口 | 静态 | 逃逸规则产出入口，每个带来源：哪个 ops 结构体的哪个字段，或在哪一行注册到哪个 work/timer 对象；是否可重入 | 入口列表 + provenance |
| S3 候选穷举 | 静态 | 对每对可能并行的入口，枚举调用闭包里落在同一（类型, 偏移）、至少一处写/释放的访问对。**不经 agent，不做预算裁剪** | 全量候选 |
| S4 确定性排除 | 静态 | **只删能证明的**：读/读；同一入口同次执行内的程序序（入口非可重入）；逐上下文 must 锁集含同一把互斥锁；drain 事实（回调 `fn` 注册在 `w` 上，另一侧在 `flush/cancel_*_sync(w)` 之后）；`lockdep_assert_held`；重复文件。每次删除记录理由 | 剩余候选 + 删除记录 |
| S5 证据打包 | 静态 | 为每个剩余候选组装结构化证据（见 §3） | 证据包 |
| S6 分组 | 静态 | 按（对象, 入口对）分组，控制审查成本 | 审查单元 |
| S7 判定 | agent | SDK + 全树 read/grep + 少量事实查询工具，按固定清单判定；**拒绝必须引用 file:line，无引证的拒绝无效，候选保留** | 判定 + 引证 |
| S8 事实回写 | agent → 静态 | S7 确认的事实写入事实库，S4 对后续候选复用（例："`handle_tx_event` 的所有调用者都持 `xhci->lock`"） | 事实库 |
| S9 召回补充（可选） | agent | 对静态解析不了的函数指针、范围外文件，提出新的访问点或入口，回到 S3。**只增不减**，单独统计 | 新候选 |
| S10 输出 | 静态 | 已知缺陷过滤（对照 git 历史）、报告 | 报告 |

S7 的判定清单：

1. 两个入口能否真的同时运行（ops 槽语义、核心层是否已串行化）。
2. 两侧是否同一实例（追基址指针来源）。
3. 是否有静态漏掉的锁或顺序（范围外的调用者、包装函数、文档约定）。
4. 生命周期保护：发布前不可达、drain、引用计数。
5. 是否良性（统计计数器、`data_race()`、有意的无锁读）。

v2 的 guarantee 概念在 S8 保留：从"每线程预先声明、靠令牌文本匹配"改为"在具体对上按需发现、全局复用"。

---

## 3. 「召回」和「证据」的定义

**召回**：真实缺陷的访问对出现在 S3 的产出中。

- 可在 72 case 上直接测量，与 agent 无关（参考 `kernel_experiment/diagnose_surface.py`、`measure_surface.py`）。
- S4 的每次删除可以对 GT 回放，检查是否误删。
- agent 只影响精度；S9 的新增单独计数，不混入静态召回。

**证据**：每个候选附带以下字段：

- 两侧访问的 file:line 与代码；
- 从入口到访问的调用路径（可能多条）；
- 每条路径上的 must 锁集；
- 两侧入口的 provenance 与是否可重入；
- 已知 HB 事实（fork/join、drain、宽限期）；
- 访问注解（`READ_ONCE` / `WRITE_ONCE` / `atomic_*` / `data_race()`）；
- 基址指针来源（参数、全局、本函数新分配）；
- 静态无法判定的点（如"调用者在编译范围外"）。

---

## 4. 硬规则

1. 静态只在**能证明**时删除候选；may 信息只能作为证据，不能用于删除。
2. agent 不能删除没有 file:line 引证的候选。
3. agent 不能决定"检查什么"；只能通过 S9 增加候选。
4. 任何上限触发都必须在结果里标注"不完整"，不能静默丢弃。

---

## 5. 与 v2 的对照

| | v2 | v3 |
|---|---|---|
| 检查哪些对 | agent（Phase A requirement）+ 分诊 + 预算 | 静态穷举 |
| 静态事实用途 | 与 agent guarantee 一起在 B 判定，含 may 信息 | 只在能证明时删除，其余作为证据 |
| agent 删除条件 | 判 reject 即可 | 必须引用 file:line |
| agent 发现的保护 | 写在本线程契约，令牌文本跨线程匹配 | 进事实库，全局复用 |
| agent 工具 | 自研循环 + CPG 工具 | SDK + read/grep + 少量事实查询 |

---

## 6. 代价与风险

- **成本**：v2 的 Phase A 是 O(线程数)，本身是成本控制。v3 的审查成本是 O(候选组数)；
  surface 大的 case（如 `CVE-2025-23142` 有 121 个对象）组数会明显增加。
  缓解：S4 确定性排除、S6 分组、S8 事实复用、便宜模型初筛。实际成本需先在 72 case 上测量。
- **召回上界仍是静态 surface**：（类型, 偏移）归并偏向过近似，较少漏对；漏的主要来源是入口缺失、
  函数指针未解析、调用方不在编译范围内，由 S1 扩范围和 S9 补充。
- **S4 依赖的静态事实需先修**：`findProtectingLock` 跨上下文求并（需逐上下文 must 判定）；
  drain 在手工入口下从不触发（`ThreadCreationTree::handleJoins` 要求 FORK 节点）。

---

## 7. 可复用的现有部件

| 步骤 | 现有部件 |
|---|---|
| S1 | `hunt/prepare_module.sh` |
| S2 | 逃逸规则（`entry-escape-rule` 分支的工作）、`VulnerabilitySurfaceGenerator` 的 `entry_provenance` / `reentrant_entry` |
| S3 | `VulnerabilitySurfaceGenerator`（field 粒度 surface） |
| S4 | `LSAnalysis`（锁集不动点）、`ThreadAPIUtil` 的 drain 列表、`kernel_experiment/audit_common_lock.py`、`audit_dup_files.py`、`audit_pair_threads.py` |
| S7 | `collect_snapshot.py` → `build_judge_packets.py` → `aggregate_judgments.py` 的评审流程与 rubric 是 S7 的原型 |
| S10 | `hunt/known_defect_filter.py` |

---

## 8. 待讨论

- S2：入口对"可能并行"的判定口径（同一 ops 表内的槽之间、同一槽的自并发）。
- S3：候选粒度（访问对 vs 对象）与去重口径。
- S4：每条排除规则的精确前提，以及如何对 GT 回放验证。
- S5：证据格式，以及给 agent 的事实查询工具具体有哪些。
- S6：分组键与每组的代表对选择。
- S7：选用哪个 SDK；模型分级；判定输出 schema。
- S8：事实库的 schema、作用域（函数级 / 对象级）与失效条件。
- S9：是否在第一版启用。
- 评测：静态召回、删除误伤率、最终精度、单 case 成本四个指标的统计方式。
