# 并发契约词表 · 正式规范与数据集验证 (v1)

> 状态：设计规范（大改地基）。本文件**不重新发明**词表——它把仓库已有的封闭关系代数
> （见 `THREAD_CONTRACT_REDESIGN.md` §4.6 与 `kernel_experiment/CONTRACT_EXPECTATION_SCHEMA.md`）
> 提炼成正式 spec，并用全数据集（100 个 `ground_truth.json`）做**经验验证**，再从
> **合理性 / 完备性 / 可解释性** 三个角度补齐边界。
>
> 经验证据来自 `scripts/fix_mechanism_taxonomy.py` 与 `scripts/candidate_shape_diagnostic.py`，
> 产物分别为 `kernel_experiment/fix_mechanism_taxonomy.json` 与
> `kernel_experiment/full_staticcompose_p4x4_20260610_010518/candidate_shape_diagnostic.json`。

---

## 0. 为什么需要这份 spec（困境的一句话）

我们的核心差异化能力是 **contract**（逐线程 assume/guarantee + 唯一 mismatch rule），但实现里它被部署成
**risk 截断前端 + 启发式压制后端** 之间的挂件。诊断数据：

- **原始候选图已覆盖 95% 的 GT**（57/60 cleanly-identifiable），但端到端召回只有 ~12–14% → **~80 个点丢在下游**（risk 截断 + landing 判错），不是前端没看到。
- **成本单元用错了**：按 pair(773k)/shape(112k)/anchor(26k) 都比今天贵 1–2 个数量级；但按 **每线程一份 contract = 3,662 次**（≈今天实际 1,904 sessions / 1,000 calibrated 同量级）就负担得起。
- 真正的成本在 **step 3（最终 commit）**：只有 6% 冲突被 common-lock 在确定性阶段消解，其余 94% 全压给 LLM。

结论：**让 contract 当主干，成本按线程数计，把 discharge 搬回确定性 step 2，step 3 只批量验证残差。** 词表是这一切的地基——它必须 sound、complete、interpretable，且**覆盖我们的数据集**。

---

## 1. 核心原理（保持不变）

1. **契约 = 逐线程 assume/guarantee。** 对线程 `T` 触碰的每个共享资源 `R`：
   - `assume(T,R)`：T 为自身正确性**要求**的执行顺序/原子性（LLM 推断的意图，源码不显式写）。
   - `guarantee(T,R)`：T 用**同步**真正**建立**的顺序。
2. **缺陷 = 涌现式组合。** 单个契约自身永远合规；bug 只在**两个契约组合**后由**一条固定规则**涌现。
3. **缺陷类别是事后标签，不驱动检测。**

> 单一 mismatch rule（R0 = 一条 required order）：
> **存在 Ta 要求的 order `R0`；Tb 有事件可违反 `R0`；没有任何 guarantee 建立 `R0`；且违反交织静态可行（concurrent ∧ ¬HB ∧ ¬protected ∧ conflicts ∧ same_location）。**

---

## 2. 资源 / 锁 / 机制的归一化命名（确定性组合的前提）

step 2 能否**确定性 discharge**，取决于 `Ta.assume` 与 `Tb/any.guarantee` 能否按**字符串/结构相等**对上。因此契约里的 `R`、`L`、`m` 必须归一化：

- **资源 `R`**：`struct.<type>.<field>`（带 alias 归并到 canonical），全局走 `global:<symbol>`，堆对象走 `obj:<alloc-site>`。同一物理字段的不同 alias/容器视图必须折叠到同一 `R`（这是前端 hygiene 的职责，见 §7）。
- **锁 `L`**：`lock:<canonical-expr>`（如 `lock:sk->sk_lock`），把 `spin_lock_bh/irqsave/...` 归一为同一把锁的同一 token；上下文互斥（bh/irq/preempt/rcu-read-side）用保留 token `ctx:bh` / `ctx:irq` / `ctx:rcu_rs`。
- **机制 `m`**：取自 §3.3 的封闭枚举，不允许自由文本。

> 命名不归一 = step 2 对不上 = 残差膨胀 = step 3 爆。这是整个方案的**头号工程风险**（实测 common-lock 只 discharge 6%，主因就是锁名未归一 + RCU/refcount/publish 未识别，而非真的没保护）。

---

## 3. 封闭词表

### 3.1 Assume 谓词（线程要求的不变量）

| 谓词 | 含义 | 被违反 → 事后标签 |
|---|---|---|
| `prec(a, b)` | a 必须 happens-before b | `prec(use, free)`→UAF；`prec(init, expose)`→null/uninit/发布序 |
| `atomic(region)` | region 不被冲突外来事件交错；单事件退化为“无并发冲突写”= data-race 条件 | `atomic(单访问)`→data race；`atomic(区段)`→原子性违反；指针被清空→null-deref |
| `count_guarded(R, free)` | 释放必须在 refcount 归零后 | refcount/UAF/double-free |

**说明**：`prec(init, expose)` 是 `prec` 的一个实例（发布前必须初始化完成），用来安放 null_deref / uninit / publish-before-init —— 见 §5.2 完备性补充。

### 3.2 Guarantee 谓词（代码实际建立的同步）

| 谓词 | 含义 | 典型来源 |
|---|---|---|
| `serialize(L, region)` | 锁 / 上下文互斥使 region 串行 | spin/mutex/seqlock；`ctx:bh/irq/preempt`；`ctx:rcu_rs` |
| `order(a ≺ b via m)` | 机制 m 建立有向序 | 见 §3.3 |
| `counts(R)` | 引用计数纪律维持 `count_guarded` | refcount/kref/folio_ref/atomic_inc_not_zero |

### 3.3 机制 `m` 的封闭枚举 + 硬/软分级（合理性的核心）

| `m` | 例子 | 分级 | step 2 是否自动 discharge |
|---|---|---|---|
| `lock-rel/acq` | unlock→lock 的 release-acquire | **硬** | 是 |
| `rcu-grace` | synchronize_rcu / call_rcu / kfree_rcu | **硬** | 是 |
| `barrier` | smp_mb / smp_store_release / smp_load_acquire | **硬** | 是 |
| `join` | flush_work / cancel_work_sync / kthread_stop / wait_for_completion / del_timer_sync / synchronize_irq / napi_disable | **硬** | 是 |
| `refcount-RMW` | refcount_dec_and_test 等原子 RMW | **硬** | 是 |
| `published-flag` | 已发布的 `is_valid/dead/closing` 状态位建立的序 | **软** | 否（仅作 step 3 待验证 order） |
| `program-order+dep` | 同线程程序序 + 数据依赖（无屏障的 publish 重排） | **软** | 否（仅作 step 3 待验证 order） |

**合理性原则**：确定性 discharge **只信硬机制**（有硬件/编译器语义背书）。软机制（published-flag、program-order）只在“有屏障/依赖”时才真正建立顺序，单看静态会**误消解** → 一律不自动 discharge，下放 step 3 验证。

---

## 4. 唯一缺陷规则 + 确定性 discharge 算法

```
compose(contracts):
  for each resource R (canonicalized):
    A = { (T, clause) | clause ∈ assume(T,R) }          # 所有线程对 R 的要求
    G = { (T, clause) | clause ∈ guarantee(T,R) } ∪
        { 跨线程硬机制 order(... via m_hard) }            # 已建立的硬保证
    for each required R0 = (Ta, prec/atomic/count_guarded):
      if ∃ Tb≠Ta with event E that can violate R0:
        if ∃ hard guarantee g ∈ G that establishes R0:
            DISCHARGE(确定性，不进 step 3)                # 良性：保护到位
        elif 静态不可行(not concurrent ∨ HB ∨ protected ∨ ¬conflicts):
            DISCHARGE(确定性)                              # CCPG grounding 否决
        else:
            EMIT residual hypothesis(R, R0, E,
               已验证谓词集, 软保证候选)                   # 进 step 3 批量验证
```

- **CCPG 可判定部分（concurrent / conflicts / same_location / hb / not_protected）全部在 step 2 判完，不问 LLM。**
- 残差 = “有 required order、可被违反、无硬 guarantee 建立、且交织静态可行”。这才是 step 3 的输入。

---

## 5. 三个角度的评估（基于 100-case 实测）

### 5.1 合理性（soundness）

- **discharge 只信硬机制**（§3.3）：硬机制覆盖了数据集修复机制的 ~77%（annot+lock+refcount+rcu+atomic_op+join+barrier）。这部分可在 step 2 安全确定性消解。
- **软机制不参与 discharge**：published-flag(~11% 共现) + program-order/publish(~12–14%) 归入残差，交 step 3。既不漏（软的不敢放），又把确定性部分吃掉。
- **buggy 侧本就缺该 guarantee**：修复机制分布告诉我们“缺的是什么”；在 buggy 代码里该 guarantee 缺失 → step 2 自然 EMIT。discharge 的 soundness 风险只在**良性对**（保护其实存在，需被识别）——所以 §2 的归一化命名是 soundness 落地的前提。

### 5.2 完备性（completeness）—— 99/100 覆盖，逐 case 对齐

**Guarantee 侧（按每 case 主修复机制，n=100）：**

| 映射到的 Guarantee 谓词 | case 数 |
|---|---|
| `atomic(region)`（annot_atomic 37 + atomic_op 4） | 41 |
| `serialize(L,region)`（lock 19 + CVE-2024-35999） | 20 |
| `prec(...)`（reorder/publish 11 + CVE-2025-37772/47715 + SYZBOT-2e4de7fe846aba66） | 14 |
| `order(via m)`（rcu 5 + join 3 + barrier 3 + flag 3 + SYZBOT-63cbe31877bb80ef） | 15 |
| `counts(R)` / `count_guarded`（refcount） | 9 |
| **out-of-scope（liveness/deadlock）** | **1** |

**Assume 侧（按 bug class，n=100）：** data_race 62→`atomic`；UAF 16 + double_free 2→`prec(use,free)`/`count_guarded`；null_deref 10 + uninit 1→`prec(init,expose)`；oob 2→`atomic(loc)`（值流入 index）；deadlock 1→out-of-scope；other 6→多为 data_race/UAF 形。

**必须补的两处（否则漏 ~23%）：**
1. **`prec(init, expose)` / publish**：很多修复没加任何原语，只“先初始化再发布 / 把 free 移到最后一次 use 之后”。明确把它作为 `prec` 的实例。涉及案例（reorder/publish 主桶，已逐一核对）：
   `CVE-2013-1792`(null), `CVE-2016-7911`(UAF), `CVE-2017-15265`(UAF), `CVE-2024-43830`(UAF), `CVE-2024-50082`, `CVE-2024-58072`(UAF), `CVE-2025-37882`(UAF), `CVE-2025-38078`(UAF), `CVE-2025-38217`, `CVE-2025-38429`(uninit), `SYZBOT-5676077ba016d741`, 以及 UNMAPPED 里的 `CVE-2025-37772`(INIT_WORK 发布前初始化)、`SYZBOT-2e4de7fe846aba66`(napi 注册发布)、`CVE-2024-47715`。
2. **`order(via published-flag)` 作为 `m` 的取值（软）**：内核里 `is_valid/dead/closing/locked` 这类状态位极常见。涉及：`CVE-2024-53124`, `CVE-2024-56555`(oob), `CVE-2025-23151`, 以及 `SYZBOT-63cbe31877bb80ef`（新增 `bool locked/klocked/check_again` 状态机）。按 §3.3 它是**软**机制，仅作 step 3 待验证 order，不自动 discharge。

**唯一弃掉的 1 例**：`CVE-2024-53136`（deadlock / 锁序，属 liveness，不在 data-order mismatch rule 范围）。诚实标注 out-of-scope（对齐 schema 的 `scope_reason=liveness`）。

> 结论：**核心 5 谓词 + 把 `prec(init,expose)` 显式化 + 让 `m` 含 published-flag/program-order，覆盖 99/100。**

### 5.3 可解释性（interpretability）

- 每条报告 = 一句话：**“资源 R 上的 assume P 没有任何硬 guarantee 建立，被线程 T 的事件 E 违反”**；bug 类型是事后标签。封闭小集合 → 可解释、可审计。
- **互斥优先级（落地必须固定，否则解释/去重会乱）**：一个访问可能同时像多种 assume，按以下优先级**唯一**归类：
  `prec(use, free)` > `count_guarded` > `prec(init, expose)` > `atomic(region)`。
  （内存安全 > 生命周期 > 发布序 > 纯原子性；保证每个 GT 落到唯一谓词。）

---

## 6. 严重度：是验证的**输出**，不是排序的**输入**

实测证据：同一机制 `atomic(loc)`（annot 占 36%）**同时**覆盖良性 torn 计数器与危险 torn-index（oob，如 `last_boosted_vcpu`）。光看“atomic 被违反”分不出严重度——区别在**值流向哪个 sink**（index/pointer/free/identity），这是 dataflow/语义判断。

因此：
- **不做静态数值 severity 排序**（与“无信息排序≈随机截断”一致）。
- 严重度内生于两点：(a) 被违反 clause 的**类型**（`prec(use,free)`/`count_guarded` 按定义即内存安全）；(b) 对 `atomic`，严重度 = step 3 **值 sink 分析**的结果。
- **severity 是 step 3 的产物，绝不进 step 2 的截断决策。**

---

## 7. 三步管线与成本模型

| 步骤 | 内容 | 成本单元 | 量级（69 clean case 实测） |
|---|---|---|---|
| **Step 1** 契约生成（LLM） | 逐线程读码，产出归一化 assume/guarantee（封闭词表） | **#threads** | ~3,662（中位 32/case） |
| **Step 2** 组合 + discharge + grounding + 聚类（确定性，0 LLM） | §4 算法；硬保证消解；CCPG 否决；残差按 (R, clause, mut-op) 聚类 | — | 0 |
| **Step 3** 批量受限验证（LLM） | 仅验证残差；按资源聚类、固定 schema、控开销 | **残差类数**（待测，目标 ≪ 今天 1000） | 待测 |

- **预算按线程数，不按 risk。** 不再用任何 risk 分数截断资源——召回下界回到“原始候选图 ~95%”。
- **前端 resource hygiene 仍要做**，但定位是 (a) 修 alias/folding 把 95% 往上抬（如 `last_boosted_vcpu`）、(b) 降低组合期重复/邻近误报——**不是成本瓶颈**（线程数不受 13k 候选对象影响）。
- **shape/anchor 压缩**只用于 step 3 的残差去重/批量，**不用于 gating step 1 输入**。

---

## 8. 批量受限验证（step 3）schema

- **聚类单元**：`(resource_family, violated_clause)`。同一单元的所有 hypothesis 共享上下文（R 的访问 + 相关线程契约），**进同一个对话**，一次产出全组判定。
- **输入**：step 2 预 grounding 好的结构化假设（两个访问 file:line、required order、violating event、已验证谓词集、软保证候选），**不给整条 trace**。
- **输出（固定 schema，禁开放推理，封顶 token）**：

```json
{
  "resource": "struct.kvm.last_boosted_vcpu",
  "violated_clause": "atomic(region)",
  "verdicts": [
    {"hypothesis_id": "h1", "verdict": "real|benign",
     "value_sink": "index|pointer|free|identity|stat|none",
     "bug_label": "UAF|null_deref|data_race|oob|refcount|none",
     "why": "<=1 句", "evidence_lines": ["file.c:321"]}
  ]
}
```

- **控开销**：每对话封顶 N 条 hypothesis（超则分块）；上下文上限固定；验证用更便宜模型；dangerous clause（prec/count_guarded）优先跑。

---

## 9. Phase-A 契约 JSON schema（LLM 每线程产出）

```json
{
  "thread_id": 0,
  "entry": "wg_packet_decrypt_worker",
  "role": "decrypt/read path",
  "clauses": [
    {
      "resource": "struct.noise_replay_counter.counter",
      "sites": [{"function": "decrypt_packet", "file": "receive.c", "line": 252, "op_kind": "READ"}],
      "assume": ["atomic(counter)"],
      "guarantee": []
    }
  ]
}
```

- `assume` / `guarantee` 的值**只能**来自 §3 封闭词表；`resource`/`L`/`m` 走 §2 归一化命名。
- 空 `guarantee` = 该线程对该资源**未建立**任何同步（buggy 侧常见）。

---

## 10. Phase-B 组合伪代码（确定性）

见 §4。要点：硬保证 discharge + CCPG 否决在此完成；输出残差 + 聚类键；软保证只作 step 3 候选。

---

## 11. 待测指标 / open questions

1. **残差类数 / case**（决定 step 3 是否便宜）：关掉 budget、全覆盖跑 step 2，统计 EMIT 数与聚类后类数。目标：每 case ~10 dangerous 类 → 全覆盖 step 3 ≈ 69×10 ≈ 700 次 < 今天 1,000。
2. **硬保证 discharge 率**：归一化命名 + RCU/refcount/publish 识别后，能否从 6% 提到接近 77%。
3. **归一化命名的召回代价**：alias/folding 折叠后 `last_boosted_vcpu` 这类能否从 0 覆盖变为可覆盖。
4. **软机制误消解风险**：published-flag/program-order 全部下放 step 3 后，step 3 负载是否可控。

---

### 附：复现命令

```bash
python3 scripts/fix_mechanism_taxonomy.py          # 词表 vs 数据集覆盖
python3 scripts/candidate_shape_diagnostic.py      # 候选图 / 成本单元 / GT 覆盖
```
