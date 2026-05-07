# Hypothesis DSL：基于 Happens-Before / 原子性的最小谓词集

> Lace 检测器下一阶段关键设计文档。
>
> **核心原则**：用最少谓词表达最多 bug；每个谓词都能落到一次 CCPG / CFG / 同步图查询。
> 框架特定的知识（kfree / refcount / RCU / completion / `*_FREED` flag …）**不进入** DSL，
> 它们以"边"的形式沉到 happens-before 同步图的构造里。

## 1. 理论起点：并发正确性的最小公理

不考虑死锁，并发缺陷的本质只有两类：

> **(I) Happens-Before 缺失**：有两个对同一内存位置的访问，至少一个是写，且二者之间不存在任何同步链——经典 data race / UAF / use-before-init / race-induced NULL deref 全部归此。
>
> **(II) 原子性破坏**：本应作为不可分块执行的多条语句，被另一线程的冲突操作可插入——TOCTOU / 非原子 RMW / read-modify-write bit ops 全部归此。

这两条用形式语言写出来就是：

```
race(a, b)        ≡  conflicts(a, b) ∧ ¬hb(a, b) ∧ ¬hb(b, a)

atomicity_break(s, e, w)
                  ≡  reachable(s, e)         (s..e 在同线程内是个块)
                   ∧ (conflicts(w, s) ∨ conflicts(w, e) ∨
                      ∃m∈path(s,e): conflicts(w, m))
                   ∧ ¬hb(w, s) ∧ ¬hb(e, w)   (w 可插入到块内部)
```

而所有"扩展性"工作（识别 `kfree` 是写 lifecycle、`refcount_dec_and_test` 释放与后续写之间没有 hb、`*_FREED` 的 flag set 后到 read 是有 hb 的、`local_bh_disable/enable` 之间形成与 softirq 上下文的互斥窗口…）都是同步图的边添加问题，**不是新谓词**。

## 2. DSL：5 个原语 + 3 个糖

### 2.1 原语（必须，全部由静态分析支持）

| 原语 | 形式化 | 实现来源 |
|---|---|---|
| `same_location(n1, n2)` | n1, n2 操作的内存对象有重叠 | Phasar 别名集合 ∩ + 字段级 `SharedFieldKey`（已有）|
| `op_kind(n) ∈ {READ, WRITE, RMW, CALL}` | n 的 IR 操作类型 | LLVM IR `LoadInst` / `StoreInst` / `AtomicRMWInst` / `CallInst` 直接读 |
| `in_thread(n, t)` | n 出现在线程 t 的可达集合中 | TCT + 调用图 reachability（已有）|
| `reachable(n1, n2)` | 同线程内 n1 → n2 在 CFG/CG 上可达 | CCPG 已有 |
| `hb(n1, n2)` | 同步图中存在 n1 happens-before n2 的有向链 | **本设计的核心引擎**，详见 §3 |

> 注意：**没有** `not_lock_protected`、`same_lock`、`alias`、`may_run_concurrently` 这些谓词。它们全部是 `hb` / `same_location` 的副产品。

### 2.2 糖（由原语推导，verifier 内置展开，prompt 里也保留以便 LLM 阅读）

| 糖 | 展开 |
|---|---|
| `conflicts(a, b)` | `same_location(a, b) ∧ (op_kind(a)=WRITE ∨ op_kind(b)=WRITE ∨ op_kind(a)=RMW ∨ op_kind(b)=RMW)` |
| `concurrent(a, b)` | `¬hb(a, b) ∧ ¬hb(b, a)` |
| `unsafe_atomic_block(start, end, witness)` | `reachable(start, end) ∧ conflicts(witness, start) ∧ ¬hb(witness, start) ∧ ¬hb(end, witness)` |

> "糖"不是新能力，只是 hypothesis JSON 里允许 LLM 直接写 `conflicts(a,b)` 而不必每次写 4 项展开式。verifier 把糖在解析阶段重写为原语再求解。

### 2.3 谓词总览

```
原语（5）：
  same_location(n1, n2)
  op_kind(n) = ?
  in_thread(n, t)
  reachable(n1, n2)
  hb(n1, n2)        ← 一切非 race 缺陷家族的实现差异都在这里

糖（3）：
  conflicts(a, b)
  concurrent(a, b)
  unsafe_atomic_block(start, end, witness)
```

LLM 视角共 **5+3=8** 个名字。verifier 真正需要实现求解的是 5 个原语。

## 3. Happens-Before 同步图

Lace 已有 LockSet 分析、CCPG、TCT。要支撑 `hb(·,·)`，需要把"同步原语 → HB 边"的映射做成一张可查询的有向图。这张图由以下边类型组成（**全部都是有限、可枚举的内核 API 集合**）：

### 3.1 程序内顺序边（同线程默认）
- 同一线程同一函数的 CFG 顺序：`a` 在 `b` 之前 ⇒ `hb(a, b)`
- 调用边 `caller → callee` 的进入；返回边 `callee → return_point`

### 3.2 锁/互斥边
- 一对锁 `release(L)@a` → `acquire(L)@b`（任意顺序）⇒ `hb(a, b)`
- 包括所有内核锁原语：`spin_lock/unlock`, `mutex_lock/unlock`, `*_bh`, `*_irq`, `*_irqsave`, `down/up`, `read/write_lock` …

### 3.3 RCU/SRCU 边
- `synchronize_rcu()` 之前所有 RCU read-side critical section 中的访问 ⇒ `hb(...)` ⇒ `synchronize_rcu` 之后的写
- `call_rcu(cb)` 注册后到 `cb` 执行之间所有 RCU readers ⇒ `hb` ⇒ `cb` 内的访问

### 3.4 原子操作 / 引用计数边
- `atomic_dec_and_test(c) == true` ⇒ HB 边到所有"该 c 之前的 inc 持有者的访问"（即引用计数模型）
- `refcount_dec_and_test`, `kref_put`, `put_*` 等同
- `atomic_store_release` ↔ `atomic_load_acquire` 配对

### 3.5 上下文边（覆盖 §F3）
- `local_bh_disable()` 起到 `local_bh_enable()` 止的代码段对所有 softirq-context 访问形成单向 HB；这意味着如果一段 process-context 代码处于 disable 区间内，它和并发 softirq 之间有 hb；不在区间内则 `¬hb`
- `local_irq_disable/enable`、`preempt_disable/enable` 同理，分别针对 hardirq / preempt

### 3.6 完成/等待边
- `complete(c)` → `wait_for_completion(c)` 后续访问 ⇒ `hb`
- `up(sem)` → `down(sem)` 后续 ⇒ `hb`
- `wake_up(wq)` 与对应 `wait_event` 之间形成 hb
- `flush_work(w)` / `cancel_work_sync(w)` ⇒ `hb` 到 `w` 内所有访问

### 3.7 生命周期/状态边（覆盖 §F5）
- 写"已释放/已禁用"flag（`*_FREED`、`is_dead`、`going_away`、`dead`、`shutdown` 等约定字段名 + 任意被 patch 显式新增的 flag）
- 检查相同 flag 为"alive"的 read ⇒ 与 flag set 之间形成 hb
- 工程上只需识别"读这个字段并据此分支跳过 deref"的 IR 模式即可
- 这是把 §"FREED flag 系列"沉为 hb 边的关键

### 3.8 边集的封闭性
上面 7 类边覆盖了 Linux 内核所有标准同步原语。新加一个 framework helper 只需把它的 entry/exit 标注为相应种类边的"端点"——**对 DSL 不增删任何谓词**。维护成本是一张表（≈ 80 项起步），与谓词集解耦。

## 4. 8 个并发缺陷家族 → DSL 配方

把数据集的 9 个 bug 家族（详见 README §M6）一一映射到这 5 个原语：

### F1 — plain data race needs READ_ONCE
```json
{
  "nodes": {"r": <load>, "w": <store>},
  "constraints": [
    {"predicate": "conflicts",   "args": {"a":"r", "b":"w"}},
    {"predicate": "concurrent",  "args": {"a":"r", "b":"w"}}
  ]
}
```
代表：`CVE-2024-40953`。fix 是 `READ_ONCE/WRITE_ONCE`，但 bug 形式仍是经典 race。

### F2 — missing lock = F1
锁的存在与否完全沉在 `hb` 里。判定式与 F1 完全相同。verifier 内部走同步图后发现两端没有锁配对边即返回 `¬hb`。代表：`CVE-2024-35999`、`CVE-2017-6346`。

### F3 — missing BH/IRQ/RCU context = F1
softirq 上下文被建模为虚拟"线程"；`local_bh_disable` 区间是与该虚拟线程的互斥边（§3.5）。`ila_output()` 没有 disable，于是 `hb(softirq_write, ila_read) = ⊥`，触发 race。判定式与 F1 完全相同。代表：`CVE-2024-41081`。

### F4 — use-before-fully-initialized = F1
init writes 在 thread A，use 在 thread B，二者跨 publish 点。如果 publish 点之前没有 release barrier、之后没有 acquire barrier，则 `hb(init_write, use) = ⊥`，又 `conflicts(init_write, use)` 成立（同 field、一写一读），构成 race。判定式与 F1 完全相同。代表：`CVE-2024-35977`、`CVE-2025-23151`。

### F5 — UAF / lifetime
```json
{
  "nodes": {"use": <deref>, "free": <kfree_call>},
  "constraints": [
    {"predicate": "conflicts",   "args": {"a":"use", "b":"free"}},
    {"predicate": "hb",          "args": {"a":"use", "b":"free"}, "expected": false}
  ]
}
```
说明：`free` 由同步图当作对 lifecycle state 的写处理（§3.7）；`conflicts` 因此成立。bug 条件 `¬hb(use, free)` 表示"use 不被强制先于 free 完成"。代表：`CVE-2024-43891`、`CVE-2024-26974`、`CVE-2025-22050`。

> 说明：相比传统 data race，UAF 是一种**有方向的** race——只要 use 不必在 free 之前就成立。所以这里的判定不是 `concurrent`（双向 ¬hb），而只是单向 `¬hb(use, free)`。verifier 接受 constraint 上带 `"expected": false` 的形式。

### F6 — TOCTOU = unsafe_atomic_block
```json
{
  "nodes": {"check": <load>, "use": <call|deref>, "witness": <store_other_thread>},
  "constraints": [
    {"predicate": "unsafe_atomic_block",
     "args": {"start":"check", "end":"use", "witness":"witness"}}
  ]
}
```
代表：`CVE-2025-38217`、`CVE-2025-38337`。

### F7 — 非原子 bitops / RMW = unsafe_atomic_block
RMW 三联（load → modify → store）作为块；任意并发对同位的写都是 witness：
```json
{
  "nodes": {"start": <rmw_load>, "end": <rmw_store>, "witness": <store_t2>},
  "constraints": [
    {"predicate": "unsafe_atomic_block",
     "args": {"start":"start", "end":"end", "witness":"witness"}}
  ]
}
```
若 IR 已是 `atomicrmw`，verifier 自动判定为 hb-self 闭合 → 不报告。代表：`CVE-2024-39508`、`CVE-2024-46704`。

### F8 — race-induced NULL deref = F5 的特化
```json
{
  "nodes": {"deref": <load_after_ptr>, "null_write": <store_NULL>},
  "constraints": [
    {"predicate": "conflicts",   "args": {"a":"deref", "b":"null_write"}},
    {"predicate": "hb",          "args": {"a":"deref", "b":"null_write"}, "expected": false}
  ]
}
```
代表：`CVE-2025-38337`、`CVE-2024-45000`。
对于纯 intra-thread 遗漏 NULL 检查（无并发因素，如 `CVE-2013-1792`），不在 DSL 范围内——LLM 不应把它当并发缺陷提交。

### F9 — 设计缺陷
不在 DSL 范围内（约 6%，`CVE-2024-27030`、`CVE-2024-42234` 等）。

### 配方汇总（8 家族总结成 3 种判定式）

| 判定模板 | 覆盖家族 |
|---|---|
| `conflicts(a,b) ∧ concurrent(a,b)` | F1, F2, F3, F4 |
| `conflicts(use,free) ∧ ¬hb(use,free)` | F5, F8 |
| `unsafe_atomic_block(start,end,w)` | F6, F7 |

LLM 选哪个 hypothesis 模板，本质上就是判 patch 修的是"普通 race / lifetime race / atomicity 破坏"中哪一种——这是非常少的认知负担。

## 5. 与现有 6 谓词的差异

| 现有 | 新设计中的归属 |
|---|---|
| `in_thread`           | 保留（原语）|
| `may_run_concurrently`| **删除**：由 `concurrent(a,b) ≡ ¬hb(a,b) ∧ ¬hb(b,a)` 推导 |
| `reachable`           | 保留（原语）|
| `not_lock_protected`  | **删除**：锁是 hb 边的一种 |
| `same_lock`           | **删除**：同上 |
| `alias`               | 由 `same_location` 取代（更精确的字段级别名）|

净变化：6 → 5 个原语 + 3 个糖，但表达力从覆盖 ≈ 25% 提升到 ≈ 94%。

## 6. 实现路线

### Phase A — 同步图引擎
1. `include/Util/HBGraph.h`：定义同步图节点（CCPG node）+ 7 类边（§3.1-§3.7）的标记。
2. `src/Util/HBGraph.cpp`：扫描 IR + CCPG，按以下规则插边：
   - 锁原语表（≈ 30 项，从 `include/linux/spinlock.h` 等抽取）
   - RCU / SRCU 表（≈ 12 项）
   - 引用计数表（`refcount_dec_and_test` 系 + `kref_put` 系，≈ 20 项）
   - 完成/等待表（completion / wait_event / wake_up / sem，≈ 10 项）
   - 上下文区间标注（`local_bh_disable/enable` 等成对，≈ 8 对）
   - 生命周期 flag 自动识别：扫描 patch 风格的 "if (!flag) skip; else use" 模式即可
3. 提供 BFS 接口 `bool hbReachable(const Node*, const Node*)`。

### Phase B — Verifier 重构
1. `include/Query/HypothesisVerifier.h`：把 `eval_*` 私有方法重写为 5 个原语 + 3 个糖。
2. `src/Query/HypothesisVerifier.cpp`：
   - `eval_hb` 直接走 `HBGraph::hbReachable`
   - `eval_conflicts`、`eval_concurrent`、`eval_unsafe_atomic_block` 是糖，按 §2.2 展开
3. 移除现有 `eval_not_lock_protected`、`eval_same_lock` 等（功能被 `hb` 接管）。

### Phase C — DetectorAgent prompt 与 schema 升级
1. `src/LLMUtil/DetectorAgent.cpp`：把 system prompt 改写为：
   - "你只用 5 个原语 + 3 个糖"
   - 给出 §4 的 8 个家族配方作为 few-shot
   - 强调 "你不需要知道 kernel API 名字；同步逻辑全部沉在 `hb` 里"
2. `propose_hypothesis` tool descriptor：args schema 限制 `predicate` 字段只能取 8 个名字。

### Phase D — 离线 sanity & 全量重测
1. 8 个 family 各选 1 个 CVE 手写 ground-truth hypothesis JSON，verifier 应全部 PASS 且对照 patch 命中。
2. 全 50 CVE 重跑 `batch_detect.sh` + `evaluate_recall.py`。
3. 期望：LLM-judge 总召回 26% → 70%+；TP_RELATED 中相当一部分应转为 TP_MATCH（因为新 DSL 让 LLM 真的能描述 lifetime / atomicity 类 bug）。

## 7. 取舍说明

### 7.1 为什么是 5 + 3，不是 18？
- LLM 在每个 hypothesis 里平均要选 4-6 个谓词；当谓词词汇 = 18 时组合空间过大，prompt 必须给大量示例 LLM 才能稳定输出，且容易"乱用谓词凑通过"
- 5 + 3 的词汇下，LLM 实际只在 §4 的 3 种判定模板里选一个，再填节点 ID，认知负担降到最低
- 表达力没有损失，只是把"框架特定知识"从谓词层下沉到同步图层

### 7.2 为什么不让 LLM 自己写谓词？
LLM 自由写谓词意味着 verifier 要 evalulate 任意自然语言断言——退化成"LLM 自我裁判"。受控词汇 + 自由组合是确保静态可验证的前提。

### 7.3 这是不是过拟合本数据集？
8 个家族对应 Linux 30 年并发缺陷的主流分类，KCSAN / lockdep / Coccinelle 文献均覆盖一致。同步图的 7 类边对应 Linux 同步原语的全集（无新增类型加入 mainline 至少 5 年）。新 CVE 落入既有家族不需要任何 DSL 改动；新增框架 helper 只增表项，是工程而非设计变化。

### 7.4 死锁怎么办？
本 DSL **明确不覆盖死锁**。死锁需要分析锁依赖图（lockdep 模型），是另一个独立课题。未来如要扩展，加 1 条原语 `lock_held_at(n, lock)` + 1 个糖 `lock_order_inversion(...)` 即可，与本 DSL 正交。

### 7.5 与 KCSAN/lockdep 的关系
- KCSAN 在运行期检测 race（即 §4 F1）。本 DSL 在静态期检测，覆盖更广（F4 / F5 / F6 / F7 / F8 是 KCSAN 不能或难以触发的）
- lockdep 在运行期检测锁顺序违反与上下文违反（即 §4 F3）。本 DSL 把它的同步规则吸收进 §3.5 的上下文边
- 二者的工程产物（`READ_ONCE` / `__must_hold` / `lockdep_assert_*` 注解）是 verifier 同步图的免费输入

## 8. 下一步

按 §6 顺序推进；每个 Phase 结束都跑一次召回评估，把数字写到 `kernel_experiment/README.md` 的"M7 谓词重构"段。如果 Phase A 的同步图建得好，Phase B-D 都是机械工作。
