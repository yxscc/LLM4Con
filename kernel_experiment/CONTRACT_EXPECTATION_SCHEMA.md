# 契约预期标注规范（中间过程近似 GT）

> 目的：在**老故事（thread-contract 入口）**开发之前，为数据集每个例子**人工/子 agent 预演**一遍：
> 给定我们的契约设计（`LLM4Con/THREAD_CONTRACT_REDESIGN.md` §4.6），**这个 bug 的两个线程的契约应该长什么样、缺陷应如何从组合中涌现**。
> 产物是开发期的**近似 GT**（中间过程），用来 ① 对照检测器的契约/假设输出 ② 提前检验"这套设计在数据集上预期能不能召回"。
>
> **这不是要你重新发现 bug**——bug 的 GT 已经在 `flow_annotation.json` / `ground_truth.json` 里。
> 你的任务是把已知的 GT **翻译成我们的契约形式（order/sync 关系代数）**，并**诚实评估它能否被我们的设计召回**。

---

## 0. 先读设计（必须）

动手前**通读** `LLM4Con/THREAD_CONTRACT_REDESIGN.md` 的 **§4.6 全节**（4.6.0~4.6.11）。核心就三件事：

1. **契约 = 逐线程的 order/sync assume-guarantee。** 对线程 T 触碰的每个共享资源 R：
   - `assume(T,R)`：T 为自身正确性**要求**的执行顺序/原子性（LLM 推断的意图，源码里不显式写）。
   - `guarantee(T,R)`：T 用**同步**真正**建立**的顺序。
2. **缺陷 = 涌现式组合**：单个线程的契约**自身永远是合规的**；bug 只在**两个契约组合**后，由**一条固定规则**涌现：
   > 存在 Ta 要求的一条 order `ρ`；Tb 有事件违反 `ρ`；没有任何同步 guarantee 建立 `ρ`；且违反交织可行（并发∧无 HB∧无保护∧冲突）。
3. **封闭关系代数（只能用这些词，禁止自创 pattern/缺陷目录）**：
   - assume 侧：`prec(a,b)`（a 必须 happens-before b）、`atomic(region)`（region 不被冲突外来事件交错；单事件退化为"无并发冲突写"=data race 条件）、`count_guarded(R,free)`（释放须在 refcount 归零后）。
   - guarantee 侧：`serialize(L, region)`（锁/上下文互斥使 region 串行）、`order(a ≺ b via m)`（发布/quiesce/join/barrier 建立有向序）、`counts(R)`（引用计数维持 `count_guarded`）。
   - **缺陷类别是事后标签，不驱动检测**：`prec(use,free)`→UAF；`prec(init,read)`→未初始化读/发布序；`atomic(单访问)`→data race；`atomic(区段)`→原子性违反；`count_guarded`→refcount UAF；指针被置空类→null-deref（atomic 区段被清空事件破坏）。

**红线**：只用上面 6 个关系。**不要**写"check-use-release pattern""state-guard pattern"这类话——那是新故事的签名。这里只讲"某线程要求某 order，某线程的事件破坏它，没有同步建立它"。

---

## 1. 每个例子的输入

在每个 case 目录（`SYZBOT-xxx/` 或 `CVE-xxxx-xxxxx/`）下读：
- `flow_annotation.json`（**主**）：`true_interleaving.thread_a/thread_b`（role/entry/call_chain/bug_site）、`ground_truth_access`（type/object/access_a/access_b/**required_ordering**）、有的还有 `vulnerability_surface`（access_sites/lifetime/aliases）。
- `ground_truth.json`：CVE 描述 + **patch diff**（patch 怎么修＝缺的 guarantee 是什么，极有用）。
- `src/**/*.c`：需要看具体几行时读（确认 op_kind、是否真有锁/同步、region 边界）。**只在需要确认时读，不要通读整库。**

---

## 2. 输出：每个 case 目录写一个 `expected_contract.json`

严格用下面的 schema（字段不可增减；值按本例填）：

```json
{
  "bug_id": "<同 flow_annotation.bug_id>",
  "shared_object": "<gt 的共享对象 R，如 user_key_payload>",

  "scope": "in_scope | out_of_scope",
  "scope_reason": "<in_scope 留空字符串；out_of_scope 必填：weak_memory | liveness | not_concurrency_order | single_thread | other:...>",

  "thread_a": {
    "role": "<read path / revoke path / ...>",
    "entry": "<entry function>",
    "clause": {
      "resource": "<R>",
      "sites": [
        {"function": "...", "file": "...", "line": 0, "op_kind": "READ|WRITE|CALL|free|publish|check"}
      ],
      "assume": ["prec(use, free(R)) | atomic([a,b]) | count_guarded(R,free)"],
      "guarantee_protection": ["serialize(L, region) | order(a≺b via m) | counts(R) | (空数组=无)"]
    }
  },
  "thread_b": { "role": "...", "entry": "...", "clause": { "...同上..." } },

  "emergent_bug": {
    "violated_requirement": "ρ = <从哪个线程的哪条 assume>",
    "violating_event": "e@<site> = <另一线程破坏 ρ 的事件，如 free(R)/clear/mutate/publish>",
    "missing_guarantee": "<为什么没有同步建立 ρ：哪边没持锁/没 quiesce/没发布序>",
    "feasibility": {"concurrent": true, "ordered": false, "protected": false, "conflicts": true},
    "category_label": "UAF | double_free | null_deref | data_race | atomicity_violation | uninit_read | order_violation | publish_before_init | refcount_uaf",
    "category_rationale": "<ρ 形态 → 该标签的一句话>"
  },

  "fix_maps_to": "<patch 增加了哪条 guarantee 使 ρ 被 establish；如 'down_read(key->sem) 前移 → serialize(key->sem, [validate,use])'>",

  "recall_plausibility": "high | medium | low",
  "recall_risk_notes": "<我们的设计在这个例子上可能在哪一步失手：静态找不到第二个线程/对象？assume 这条 order LLM 推不出来？region 边界难定？guarantee 裁决易错？>",

  "confidence": "high | medium | low",
  "needs_human_review": false
}
```

### 填写要点
- **assume 来自 `required_ordering` + bug 语义**，不是源码字面。把 gt 的 `required_ordering` 翻译成 `prec/atomic/count_guarded` 之一（或组合）。
- **guarantee_protection 填该线程在 vuln 版本里真正建立的同步**（常常一边有、一边没有，或都没有）。**patch 加的同步不要填进 vuln 版契约**——它属于 `fix_maps_to`。
- **`emergent_bug.violated_requirement` 必须能在某个线程的 `assume` 里找到**；`violating_event` 必须是另一个线程 `sites` 里的某个危险 op。两者+缺失 guarantee+可行性，就是组合规则的实例化。
- **类别是事后贴的**：先确定 ρ 形态，再按 §0 的映射贴 `category_label`。不要反过来"先认定是 UAF 再编 ρ"。
- **op_kind** 用：`READ/WRITE/CALL`（结构层）或语义标 `free/publish/check/mutate`（诱导危险事件）。

---

## 3. 范围判定（§4.6.0，必须诚实）

- **in_scope**：safety + 顺序一致（SC/DRF-SC）下的 race / 原子性违反 / order violation / 发布序(publish-before-init) / UAF / double-free / 未初始化读 / null-deref-race / refcount-UAF。
- **out_of_scope（必须如实标，不要硬塞进契约）**：
  - `weak_memory`：缺内存屏障 / release-acquire / 双重检查锁乱序 / out-of-thin-air —— 程序序看着有序、只有内存模型能看出违反。
  - `liveness`：死锁 / 活锁 / 饥饿 / lost-wakeup 挂死 —— 是"所有执行/公平性"性质，不是某条执行违反某个序。
  - `not_concurrency_order`：根本不是并发顺序问题（如纯逻辑错误被误标）。
  - `single_thread`：GT 实际是单线程缺陷。
- **out_of_scope 的例子仍要填 `thread_a/thread_b/shared_object` 等已知字段**（方便统计），但 `emergent_bug` 里写明"本设计预期不覆盖"，`recall_plausibility=low`，`scope_reason` 必填。

---

## 4. 完整样例（CVE-2015-7550，按本规范填好，照此对齐）

输入要点（来自其 `flow_annotation.json` / patch）：
- 对象 R = `user_key_payload`（`key->payload.data[0]`）。
- thread_a：read path `SyS_keyctl → keyctl_read_key → user_read`；vuln 版**先 `key_validate(key)` 再 `down_read(&key->sem)`**，`user_read():193` 解引用 `upayload->datalen`。
- thread_b：revoke path `SyS_keyctl → keyctl_revoke_key → user_revoke`；`user_revoke():152` 在写侧 `rcu_assign_keypointer(key, NULL)` 清空 payload。
- `required_ordering`：key 的有效性检查必须在持 `key->sem` 期间完成，使 revoke 不能在 read 回调跑之前清空 payload。
- patch：把 `down_read(&key->sem)` **前移到 `key_validate` 之前**。

对应 `expected_contract.json`：

```json
{
  "bug_id": "CVE-2015-7550",
  "shared_object": "user_key_payload (key->payload.data[0])",

  "scope": "in_scope",
  "scope_reason": "",

  "thread_a": {
    "role": "read path",
    "entry": "SyS_keyctl",
    "clause": {
      "resource": "user_key_payload",
      "sites": [
        {"function": "keyctl_read_key", "file": "security/keys/keyctl.c", "line": 751, "op_kind": "check"},
        {"function": "user_read", "file": "security/keys/user_defined.c", "line": 193, "op_kind": "READ"}
      ],
      "assume": ["atomic([check(payload non-revoked), use(payload)])"],
      "guarantee_protection": []
    }
  },
  "thread_b": {
    "role": "revoke path",
    "entry": "SyS_keyctl",
    "clause": {
      "resource": "user_key_payload",
      "sites": [
        {"function": "user_revoke", "file": "security/keys/user_defined.c", "line": 152, "op_kind": "free"}
      ],
      "assume": [],
      "guarantee_protection": ["serialize(key->sem, write-side clear)"]
    }
  },

  "emergent_bug": {
    "violated_requirement": "ρ = atomic([check, use]) from thread_a (validate→deref 之间不得被清空交错)",
    "violating_event": "e@user_revoke:152 = clear/free(payload) via rcu_assign_keypointer(key, NULL)",
    "missing_guarantee": "reader 在 vuln 版先 validate 后才 down_read(&key->sem)，[check,use] 区段没被 key->sem 串行；writer 单边持 sem 不构成对 reader 该区段的互斥 → 无 g establishes ρ",
    "feasibility": {"concurrent": true, "ordered": false, "protected": false, "conflicts": true},
    "category_label": "null_deref",
    "category_rationale": "ρ 是 atomic 区段被一个把指针清空的事件交错破坏 → TOCTOU 后解引用 NULL（原子性违反具体表现为 null-deref）"
  },

  "fix_maps_to": "down_read(&key->sem) 前移到 key_validate 之前 → 为 reader 的 [check,use] 区段增加 serialize(key->sem, region) → establishes ρ → 不再触发",

  "recall_plausibility": "high",
  "recall_risk_notes": "风险在 region 边界：LLM 需把 assume 定到 [validate, use] 整段而非单点；若只标 use 单点会退化成 use-after-clear 的 prec(use,clear)，仍可召回但 region 语义更准。两个线程同入口(SyS_keyctl 不同 cmd)，静态需把 read/revoke 两条 dispatch 都识别成并发线程。",
  "confidence": "high",
  "needs_human_review": false
}
```

> 注意这个例子**只讲"reader 要求 [check,use] 原子、revoke 的清空破坏了它、没有同步建立它"**，全程没出现任何 "pattern"。这就是老故事该有的叙述方式。

---

## 5. 红线 & 诚实要求（最重要）

1. **只用 §0 的 6 个关系**，不自创 pattern / 缺陷签名。
2. **类别事后贴**，不要先选类别再编 ρ。
3. **范围诚实**：弱内存/liveness/单线程的，如实标 `out_of_scope`，不要硬掰成 `prec/atomic`。**标出 out_of_scope 是有价值的信号（帮我们确认 scope 声明），不是失败。**
4. **召回可行性诚实**：如果你认为我们的设计在这个例子上很可能失手（静态找不到第二线程、order 要求 LLM 难推断、region 难定界、对象在 gt 里就很模糊），如实写 `recall_plausibility=low` + `recall_risk_notes`，并把 `needs_human_review` 设 true。
5. **拿不准时 `confidence=low` + `needs_human_review=true`**，不要编一个看起来工整但站不住的契约。
6. 不要改任何已有文件，只**新增** `expected_contract.json`。
```
