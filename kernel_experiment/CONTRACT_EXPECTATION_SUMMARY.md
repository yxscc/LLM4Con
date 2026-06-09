# 契约预期标注：数据集级汇总（设计预期有效性）

> 来源：10 个子 agent 按 `CONTRACT_EXPECTATION_SCHEMA.md` 对 100 个 case 逐一预演，每个 case 目录下已写 `expected_contract.json`（近似 GT，中间过程）。
> 用途：① 开发期对照检测器输出 ② 检验 §4.6 这套 order/sync 契约设计在数据集上的**预期有效性**与**盲区**。
> 口径：本文件是统计 + 结论；逐例细节见各 `expected_contract.json`。所有判断是**子 agent 基于 GT 的预期/预测**，**非实测**（检测器还没跑）；**55/100 标了 `needs_human_review`，用作开发 oracle 前需人工过一遍**。

---

## 0. 两个问题分开问（最重要的区分）

这次标注只能回答**第一个**问题，不能回答第二个：

1. **表示力**：这个真实 bug 能不能写成"(线程A契约, 线程B契约) + 一条失配"的形式，并且读起来就是那个真错误？→ **能：88/100。**
2. **可检测性**：我们的 pipeline（静态找线程/对象 + LLM 推 assume/裁决 guarantee + grounding 核验）**能不能自动把这两个契约抽出来、并让失配真的触发**？→ **未实测**，本文件只给**预测**（high/medium/low），不是结论。

**别把这两件事混了**：high/medium/low 是对"第二个问题"的**预判**，不是跑出来的召回率。

---

## 1. 总览数字（100 例）

| 维度 | 分布 | 含义 |
|---|---|---|
| **范围 scope** | in_scope **88** / out_of_scope **12** | 能否用契约**表示**（第一个问题） |
| **预期召回 recall_plausibility** | high **7** · medium **44** · low **49** | 若按设计实现，**预测**能否检出（第二个问题，非实测） |
| **需人工复核** | **55** | 近似 GT 自身可信度，用作 oracle 前要复核 |

**high / medium / low 的判据（这是"预测"，不是跑出来的）：**
- **high**：bug 干净映射到契约，且设计机制大概率能成（静态能找到两线程/对象、order 要求好推、grounding 能确认）。例：`prec(use,free)` 的 UAF + grace-period 修复。
- **medium**：能映射，但成败**系于一个判断**（多半是"该不该拒绝一把真持有的锁"，或区段边界）。
- **low**：大概率漏，且有**具体原因**：① 良性 race，我们本就该不报；② 静态找不到第二线程/对象；③ 锁看着够其实不够；④ 对象身份难 ground。

**in_scope 类别分布（88，类别由 ρ 形态事后投影）：**

| 类别 | 数量 |
|---|---|
| data_race | 48 |
| UAF | 14 |
| atomicity_violation | 13 |
| null_deref | 6 |
| order_violation | 2 |
| double_free | 2 |
| refcount_uaf | 2 |
| publish_before_init | 1 |

**out_of_scope（12）按原因：** weak_memory 3（CVE-2024-26984, CVE-2025-38037, CVE-2025-38048）· liveness 2（CVE-2024-53136, CVE-2025-38104）· single_thread 1（CVE-2025-38165）· not_concurrency_order 1（CVE-2024-47715）· other 5（CVE-2024-46704 虚假/死值 race、CVE-2025-37882 陈旧硬件事件/DMA、CVE-2025-38429 对端是外部硬件、SYZBOT-4b16… GT 对象自相矛盾已排除、SYZBOT-63cbe3… 位域共享存储字 RMW）。

---

## 2. 结论一：contract 是不是一个可行的方案？两个 contract 能不能抽出真正的错误？

**作为"bug 的表示/模型"——能，且在难类上明确取胜（88/100 验证）。**
对全部 88 个 in_scope，子 agent 都能写出：线程A 的 `assume`（被要求的 order）、线程B 破坏它的事件、缺失的 guarantee、涌现的类别——读起来就是那个真错误。尤其**意图承载型缺陷**（UAF / atomicity / order / refcount / 发布序，共 ~38 例）几乎是教科书式：A 说"我要 use 前不被 free / check-use 要原子"，B 有 free/mutate，没有同步建立它 → bug 自己冒出来。这一类**语法 race 检测器看不到**（没有锁的影子可循），正是契约取胜处。封闭关系代数**零扩张**就吃下 88 例，说明表示力够、边界也清楚。

**作为"自动检测机制"——可行，但被两个已知工程风险卡住（都不是契约本身的缺陷）：**
- 静态要先找到两个线程并对齐共享对象（失手点 C）；
- LLM 要敢拒绝"持有但不够"的锁（失手点 B）。
预测说：这俩成立时，bug 就能冒出来（7 个 high + 大量 medium）；不成立时漏（low）。**失败几乎都在"基座+仲裁"，不在"契约这个想法"。**

**一句话**：契约方案在**表示层已被验证可行**（88/100，含语法检测拿不下的难类）；能否**自动召回**取决于静态发现与仲裁精度这两个**与契约概念无关、且设计早已点名的弱环**——外加一类(良性撕裂 race)我们**本就该有意不报**。

---

## 3. 结论二：预期召回为何偏冷静，且"冷静"该怎么读

high 7 / medium 44 / low 49——但 **low 是混合体，不能当"设计失败 49 例"读**：

### 失手点 A：良性 vs 有害的仲裁（最大单一信号）
~48/88 是 KCSAN 单字段 data race，其中很大一块是**故意无锁读、只用 READ_ONCE/WRITE_ONCE 注解修复**的良性撕裂；patch 在 `{serialize, order, counts}` 里**没有对应物**。这制造核心张力：仲裁器若按 §10"需原子原语 guard、拒绝良性 race"判 → 系统性漏这一类（对 KCSAN-GT 是 FN，但**漏得正确**）；若一律保留 → 退化成纯语法 race 检测器（§4.6.3/§10 极力避免）。**这其实强化了 §10 的拍板**，并指向：**论文主评估应以意图承载型缺陷为头牌，KCSAN 撕裂 race 明确按 benign 取舍。**

### 失手点 B：保护过度抑制（held-but-insufficient）
持了锁但不覆盖竞争访问（错锁 / 错锁模式 读vs写 / 中途放锁 / 近似充分的 quiesce）。命中取决于 LLM 敢拒绝真持有的锁——设计强项也是最弱 WF 环（§4.6.6 reject 候选锁 / WF3）。硬例：SYZBOT-4a03（写在共享读锁下）、CVE-2025-23151（同 `mhi_chan->lock`，缺的是 post-lock `ch_state` 复检）、CVE-2024-43891（两侧同 `event_mutex`）、SYZBOT-3cc3a1（不同锁）、CVE-2023-53046/CVE-2024-26974（`cancel_work_sync`/`wait_for_completion_timeout` 近似 quiesce）。

### 失手点 C：第二线程/对象发现（契约推理之前就失败）
**静态基座问题，非契约问题**：自竞争（同一入口两核并发）、跨 TU 对端（未编译的 `workqueue.c`）、对象身份跨哈希/idr/`container_of`、data-table 指针绑定的泛型写者（sysctl）。直接挂钩 `STATIC_CAPABILITY_REPAIR_LOG`。

### 加分处（验证核心故事）
高召回例全是 `prec(use,free)` UAF + grace-period/quiesce 修复（CVE-2015-7550、CVE-2016-7911、CVE-2025-38250、CVE-2024-56788）与真·keep/reject 锁仲裁（SYZBOT-01affb14、CVE-2024-35999、CVE-2025-38383、SYZBOT-1c486d0b、SYZBOT-44cf88）。

---

## 4. 对 §4.6 设计的具体反馈（待拍）

1. **`atomic(单访问)` 的良性 race 处置写死策略**：KCSAN-注解类（仅 READ_ONCE/WRITE_ONCE 修、无 order/exclusion）默认判 benign（不报），并在评估里把主指标放到意图承载型缺陷上；接受"对 KCSAN-GT 的 FN"是有意取舍。
2. **grounding oracle 必须锁模式敏感 + 区段敏感**（失手点 B 多例死在此）：把"读/写锁模式""锁覆盖区段是否含 use 点"列为 grounding 必查项（扩 §4.6.7 的 M，不扩代数）。
3. **`prec` 要能表达线程内顺序**：CVE-2024-43830 是纯线程内 free-vs-drain 顺序（buggy 与 fixed 同两句、只是次序不同），当前以 inter-thread HB 为主易漏；§4.6.5 的 `¬ordered` 要区分 intra/inter。
4. **把"patch 不增 `serialize/order/counts` ⇒ out_of_scope"写成 §4.6.0 的可操作判据**（已能自动识别）。
5. **§4.6.0 显式承认"亚字/位域粒度"出界**（与 weak_memory 并列；SYZBOT-63cbe3 是 `same_location` 的硬底）。

---

## 5. needs_human_review 优先复核清单（55 例中最该先看）
- **可能 GT 本身有问题**：SYZBOT-3872b8b1（两 GT 站点都持 `rnp->lock` 的写、不 race）、SYZBOT-4b16e156、CVE-2024-47715、CVE-2024-46704。
- **设计明确预期 MISS（校准期望，不当 bug 追）**：12 个 out_of_scope + CVE-2024-43830 + 失手点 B 硬例（4a03、23151、43891）。
- **类别与 GT `type` 不一致（ρ 更精确）**：CVE-2022-48830（data_race→atomicity_violation）、CVE-2013-1792（null_deref→publish_before_init）等。

---

## 6. 一句话结论
> 契约方案**在表示层已验证可行**（88/100 in-scope、代数零扩张，且在 UAF/atomicity/order 等语法检测拿不下的难类上明确取胜）——**两个契约确实能抽出真正的错误**。
> 能否**自动召回**未实测；预期召回的天花板**不在契约代数**，而在两处既有弱点：① 仲裁对"良性撕裂 / held-but-insufficient 锁"的判断（精度↔召回取舍）；② 静态基座的第二线程/对象发现 + grounding 的锁模式/区段精度。
> **下一步**：评估主战场放意图承载型缺陷；KCSAN 良性撕裂按 benign 取舍；grounding 补"锁模式 + 区段覆盖"；`prec` 补"线程内序"。
