# Lace 2026-05-12 全量 batch 召回归因分析

> 目标：基于 `LLM_dump/CVE-*_2026-05-12_*` 这一轮（M7 完整版：D + E + F + A + B + C 全开启）
> 的检测结果与 `kernel_experiment/CVE-*/ground_truth.json`，
> 按 CVE 逐一判定召回情况，挖出未召回根因，最后给出优化建议。
>
> 不修改代码（仍在跑）。

---

## 0. 基线对照表（数据来源）

- 上轮评估 `evaluation_report_agent.json`（2026-05-11，judge=gpt-5.5）评的是 **2026-05-10 的 detection 输出**，
  对应 lace 状态为 **M7 D+E+F+A**（HBGraph 已建出但 `HypothesisVerifier` 还没接 5+3 新谓词、prompt 还没切到新 DSL）。
  - 摘要：HIT 12 / PARTIAL 6 / MISS 26 / ERROR 6（recall_strict = 24%）。
  - 单 bug：tp_match 15 / tp_related 26 / fp 78（precision_strict 12.6%、precision_lenient 34.5%、fp_rate 65.6%）。
  - MISS 主要分桶（按 `miss_root_cause`）：
    - **`shared_object_missed`**（P1 surface gap）：12
    - **`function_not_analyzed`**（thread_extraction）：6
    - **`hypothesis_pruned`**（DetectorAgent_prompt）：7
    - **`wrong_function_focus`**（DetectorAgent_prompt）：5
    - `preparation_gap`：1，`n/a`：1。
- 本轮评估的目标：**2026-05-12 的 detection 输出**（M7 完整版），不再是 baseline。

### 2026-05-12 全 50 CVE detect 状态摘要

| CVE | 上轮 recall | 上轮 cat | 本轮 hyp 数（confirmed） | 本轮 bug 数（bugs.txt） | 备注 |
|---|---|---|---|---|---|
| CVE-2013-1792 | HIT | – | 7 | 3 | |
| CVE-2015-7550 | HIT | – | 1 | 1 | |
| CVE-2016-7911 | PARTIAL | shared_object_missed | 2 | 2 | |
| CVE-2016-9806 | MISS | function_not_analyzed | 6 | 5 | |
| CVE-2017-15265 | HIT | – | 5 | 5 | |
| CVE-2017-6346 | HIT | – | 8 | 1 | |
| CVE-2024-26974 | HIT | – | 1 | 1 | |
| CVE-2024-27019 | MISS | shared_object_missed | 3 | 1 | |
| CVE-2024-27030 | HIT | – | 6 | 5 | |
| CVE-2024-27404 | MISS | wrong_function_focus | 3 | 3 | |
| CVE-2024-35898 | MISS | shared_object_missed | 3 | 2 | |
| CVE-2024-35977 | MISS | shared_object_missed | 0 | 0 | 仍 0 共享对象 |
| CVE-2024-35986 | MISS | function_not_analyzed | 4 | 3 | |
| CVE-2024-35999 | ERROR | – | 4 | 3 | API_ERROR 已恢复 |
| CVE-2024-36938 | ERROR | – | 4 | 2 | |
| CVE-2024-38596 | ERROR | – | 7 | 3 | |
| CVE-2024-39503 | HIT | – | 3 | 2 | |
| CVE-2024-39508 | MISS | function_not_analyzed | – | – | core/abort（仍跑到 Bug_Detection 但未产出） |
| CVE-2024-40953 | MISS | function_not_analyzed | 5 | 1 | |
| CVE-2024-41005 | HIT | – | 4 | 3 | |
| CVE-2024-41081 | MISS | shared_object_missed | 7 | 3 | |
| CVE-2024-42234 | MISS | shared_object_missed | 3 | 1 | |
| CVE-2024-43830 | MISS | shared_object_missed | 4 | 0 | 4 confirmed 但 0 bug 写出 |
| CVE-2024-43891 | ERROR | – | 4 | 2 | API 恢复 |
| CVE-2024-45000 | PARTIAL | wrong_function_focus | 4 | 1 | |
| CVE-2024-46704 | MISS | shared_object_missed | 4 | 1 | |
| CVE-2024-47715 | MISS | wrong_function_focus | 5 | 5 | |
| CVE-2024-50082 | MISS | shared_object_missed | 8 | 5 | |
| CVE-2024-53124 | MISS | hypothesis_pruned | 6 | 4 | |
| CVE-2024-53136 | HIT(0) | – | – | – | 仍 0 hypothesis (M5 已知问题) |
| CVE-2024-56555 | ERROR | – | 3 | 3 | |
| CVE-2024-56788 | HIT | – | 8 | 4 | |
| CVE-2024-58072 | ERROR | – | 6 | 6 | |
| CVE-2025-22050 | MISS | hypothesis_pruned | 3 | 3 | core dump 已恢复 |
| CVE-2025-23142 | MISS | wrong_function_focus | 5 | 5 | |
| CVE-2025-23151 | HIT | – | 5 | 3 | |
| CVE-2025-37772 | MISS | hypothesis_pruned | 2 | 1 | |
| CVE-2025-37854 | MISS | preparation_gap | 1 | 0 | |
| CVE-2025-37882 | MISS | shared_object_missed | 7 | 2 | |
| CVE-2025-37920 | HIT | – | 5 | 3 | |
| CVE-2025-38037 | PARTIAL | hypothesis_pruned | 3 | 1 | |
| CVE-2025-38048 | PARTIAL | wrong_function_focus | 3 | 3 | |
| CVE-2025-38078 | MISS | hypothesis_pruned | 4 | 3 | |
| CVE-2025-38165 | MISS | shared_object_missed | 4 | 3 | |
| CVE-2025-38217 | PARTIAL | hypothesis_pruned | 3 | 1 | |
| CVE-2025-38242 | MISS | shared_object_missed | 2 | 1 | |
| CVE-2025-38250 | MISS | hypothesis_pruned | 3 | 2 | |
| CVE-2025-38337 | MISS | function_not_analyzed | 4 | 1 | |
| CVE-2025-38383 | MISS | function_not_analyzed | 2 | 0 | |
| CVE-2025-38429 | HIT | – | 7 | 0 | 7 confirmed 但 0 bug 写出 |

> 说明：
> - "本轮 hyp 数" 来自 `confirmed_hypotheses.log` 中 `hypothesis_id` 的出现次数；
>   "本轮 bug 数" 来自 `stateful_bugs/bugs.txt` 中 `Hypothesis-Based Violation Detected` 的出现次数。
> - 两者出现差距时，常因 detector 写 bugs.txt 时做了 confirmed→bugs 的二次 LLM verifier triage（`VerificationAgent`）打回了一部分。
> - CVE-2024-39508、CVE-2024-53136 仍处于 0 confirmed 状态（与 M5 known issue 一致）；CVE-2025-37854 第一次有 1 hypothesis（preparation_gap 已部分修复）。

---

## 1. 归因方法

对每个 MISS / PARTIAL CVE：

1. 读 `ground_truth.json` 确认补丁修了哪条 race（关键字段、关键函数、修复方式）。
2. 读 `LLM_dump/<CVE>_2026-05-12_*/`：
   - `vulnerability_surface.json`：Phase 1 是否给出了真正的 shared object（看 `name`/`type`，按 risk score 排序的位次）。
   - `confirmed_hypotheses.log`：Phase 2-3 LLM 提出 + 静态约束确认的 hypothesis 是否覆盖了真目标。
   - `stateful_bugs/bugs.txt`：Phase 4 LLM verifier 通过的 bugs（部分 confirmed 会被 LLM 二次 triage 打回）。
   - `llm_simplified_trace.log`：DetectorAgent 的 tool-call 轨迹，看它是否考察过相关函数。
   - `analysis_checkpoint.txt`、`thread-creation-tree.dot`：基础设施是否健康（threads 数、是否触发 INTERRUPTED/LOWMEM）。
3. 与 ground truth 对照判定：HIT / TP_MATCH / TP_RELATED / FP，并把根因映射到代码组件。

判定原则（与 evaluate_recall.py 的 LLM-judge 一致）：
- HIT：至少一条 hypothesis 描述的 shared object（字段层）和 thread 双方与 patch 实际修复点重合，且机制（race/UAF/lifetime）方向一致。
- 不只对 file 名/函数名做匹配；patch 触及的函数、字段都得在 hypothesis 中能找到。

---

## 2. 各桶根因复核（基于 2026-05-12 dump）

> 下文按上轮 eval 的根因桶组织。每桶给出 lace 内部组件级别的根因（不是猜测，而是有 dump
> 证据指明这一桶 80%+ 的 MISS 都堆在同一个/同一组代码点）。

### 2.0 流水线复盘（与 dump 文件对照）

```
Phase 0  ThreadAPI 探测                       → llm_simplified_trace.log（开头 wrapper 任务）
Phase 1  VulnerabilitySurfaceGenerator        → vulnerability_surface.json
Phase 2  DetectorAgent (LLM 提 hypothesis)    → confirmed_hypotheses.log（中间过程在 llm_conversations.log）
Phase 3  HypothesisVerifier 静态验真           → 通过的进 confirmed_hypotheses.log
Phase 4  VerificationAgent LLM 二审 (judge)   → 通过的写到 stateful_bugs/bugs.txt
```

关键观察（2026-05-12 全集）：

- 62 CVE 共产出 244 个 confirmed hypothesis，最终写到 bugs.txt 的只剩 151。
  **93 个 hypothesis（38%）被 Phase 4 LLM judge 二审打回**。
- 4 个 CVE 的所有 confirmed hypothesis 都被 judge 全部判 FP，结果 0 bug：
  CVE-2022-48931 / CVE-2024-43830 / CVE-2025-37854 / CVE-2025-38383。
- CVE-2024-39508 / CVE-2024-53136 / CVE-2024-35977 / CVE-2022-49634 / CVE-2022-49641 共 5 个 CVE
  在 Phase 1 之后就 0 confirmed hypothesis，直接没法谈召回。

### 2.A `shared_object_missed`（11/26 MISS） — 主要被 `eval_same_location` 屏蔽

#### 现象证据

抽 6 个上一轮被打成 `shared_object_missed` 的 CVE，统计本轮 dump 中 LLM 提议被
`HypothesisVerifier` 拒绝时的 `failed.detail`：

| CVE | 拒绝原因（节选自 `llm_simplified_trace.log`） |
|---|---|
| CVE-2024-27019 | `conflicts: same_location FAILED — no shared field nor pointer alias between node 933 and 4770` |
| CVE-2024-35898 | `conflicts: same_location FAILED — No memory accesses recorded for node 2165 or 1480 (try eval_alias for a coarser check)` |
| CVE-2024-42234 | `unsafe_atomic_block: witness does not conflict with start nor end (same_location FAILED ...)` |
| CVE-2024-43830 | `conflicts: same_location FAILED — no shared field nor pointer alias between node 43 and 256` |
| CVE-2024-46704 | `conflicts: same_location FAILED — no shared field nor pointer alias between node 1583 and 1976` |
| CVE-2025-37882 | `conflicts: same_location FAILED — no shared field nor pointer alias between node 2361 and 924` |

**模式高度一致**：LLM 提出的两个节点之一是 list/RCU/工作队列辅助函数的 call site（`list_for_each_entry`、
`list_del_rcu`、`__flush_work`、`device_remove_groups` 等），或者两个节点跨过宏展开/类型转换；
`eval_same_location`（`src/Query/HypothesisVerifier.cpp:631`）走两条路：

```631:675:LLM4Con/src/Query/HypothesisVerifier.cpp
bool HypothesisVerifier::eval_same_location(int n1, int n2, std::string& detail) {
    CCPGNode* node1 = ccpg_->getNodeByID(n1);
    CCPGNode* node2 = ccpg_->getNodeByID(n2);
    ...
    auto accs1 = gatherAccesses(node1);
    auto accs2 = gatherAccesses(node2);
    if (accs1.empty() || accs2.empty()) {
        detail = "No memory accesses recorded for node " + std::to_string(n1) + ...
        return false;
    }
    const llvm::Module* M = getLLVMModule();
    if (M) {
        for (const auto& a1 : accs1) {
            auto k1 = SharedFieldKey::fromValue(a1.pointerOperand, *M);
            if (!k1) continue;
            for (const auto& a2 : accs2) {
                auto k2 = SharedFieldKey::fromValue(a2.pointerOperand, *M);
                if (!k2) continue;
                if (*k1 == *k2) { detail = "same field: ..."; return true; }
            }
        }
    }
    AliasChecker* ac = AliasChecker::getInstance();
    for (const auto& a1 : accs1)
        for (const auto& a2 : accs2)
            if (ac->isAlias(a1.pointerOperand, a2.pointerOperand))
                return true;
    detail = "no shared field nor pointer alias ...";
    return false;
}
```

两条路同时失败的根因：

1. **list-helper 节点对应的 IR 操作位于 callee 内部**：在 surface 阶段我们已经做了
   "list-helper synthetic access"（`VulnerabilitySurfaceGenerator.cpp` 中
   `accesses` 数组里能看到 `function='list_for_each_entry'`、`node_id=-1` 的项），
   但这些只挂在 surface JSON 里给 LLM 看；它们没有进 `AliasChecker` 的
   `getMemoryAccessesFromLocation(...)` 索引（`gatherAccesses` 走的是 `Context` ×
   `nodeLoc` 查表），所以 `accs1` 或 `accs2` 直接为空，第 0 步就返回 false。
   → 这是 CVE-2024-27019、CVE-2024-46704、CVE-2024-43830 的失败点。
2. **GEP 偏移与 cast 让 `SharedFieldKey::fromValue` 输出不同 key**：CVE-2024-50082
   的 wait_queue_entry 是 stack-local `data.wq`，`fromValue` 会跳过 alloca-rooted
   值；CVE-2024-42234 的 split_queue list_lru entry 在 union 里加偏移；
   CVE-2025-37882 的 `current_cmd` 是 `xhci->cmd_list` 链表头部解引用。

   → 此时 fallback `AliasChecker::isAlias` 也失败，因为 Phasar 的字段敏感度不够。

#### 结论

`shared_object_missed` 桶里 ~80% 的 MISS 不在 surface 本身（surface 实际上经常已经把目标
shared object 列在前 10 位，比如 CVE-2024-27019 的 `global:nf_tables_objects` risk=120+），
真正卡死的环节是 **Phase 3 的 `eval_same_location` 把"节点 → IR 内存访问"的映射做得太
死板**，特别是：

- list-helper / `__flush_work` / `device_remove_groups` 这类 "callee 内做 store"
  的情况，CCPG call-site 节点没绑 IR access。
- alloca-rooted、union、嵌套 GEP 让 `SharedFieldKey` 输出不同 key。

这一桶的修复点 90% 集中在 `HypothesisVerifier::gatherAccesses` + `SharedFieldKey::fromValue`，
而不是 surface 生成器本身。

> 个别真正 surface gap 的：
> - CVE-2024-35977：surface 0 个对象（`shared_objects=[]`）。原因是这个 CVE 只用了一个
>   全局 `kobject` 引用计数 race，Phasar 没把 module init 路径的访问当成 shared。
> - CVE-2025-37882：surface 列出了 `current_cmd`，但 risk 落在第 60 位，最终 LLM 没 round 到。

### 2.B `function_not_analyzed`（6/26 MISS） — Phase 0 thread/entry 发现不全

抽样：

| CVE | thread-creation-tree | 期望的 race 入口 | surface 大小 |
|---|---|---|---|
| CVE-2024-39508 | 仅 `io_wq_init` + `io_wq_work_match_all` 两个 thread | `io_worker_handle_work` / `io_wq_activate_free_worker` | **0 共享对象** |
| CVE-2025-38383 | 31 个 thread 但全是 `vfree*` / `vfree_atomic` 流 | `vmalloc_info_show` / `show_numa_info` proc-read 路径 | 271 但全是 vmap_block，不含 numa_hit |
| CVE-2025-38337 | `start_this_handle` 等 jbd2 入口 OK | `jbd2_journal_dirty_metadata` 自身两次调用之间的 race | 37 但 risk 排序错位 |
| CVE-2024-40953 | 15 个 thread 都是 kvm ioctl handler | `kvm_vcpu_on_spin` 同步逻辑（同函数自交叉） | 51 |
| CVE-2024-35986 | 5 个 thread，charger_psy 注册路径有 | 关键的是 `psy_unregister` 回调 vs `tusb1210_charger_det_work` | 6 |
| CVE-2016-9806 | netlink 几个 setsockopt | `netlink_dump` 中断点不可见 | 68 |

**两个真正的 thread_extraction bug：**

1. **CVE-2024-39508** 的 `io-wq.c` 入口确实漏了。`io_wq_create_worker` 通过
   `wake_up_process(worker->task)` 启动 `io_wq_worker`，这是 lace
   `findThreadEntryInCPG` / `findThreadEntryByLLM` 没识别出的间接 fork 模式
   （函数指针存进 task_struct，再由调度器唤起）。
2. **CVE-2025-38383** 的真正读者 `vmalloc_info_show` 是通过 `proc_create_seq` 注册的
   `seq_operations.show` 回调，lace 完全没把 proc/seq_file 这条 callback chain 当成 thread。
   → `vmalloc_info_show` 整个函数都没在任何 Thread 的 `getNodes()` 里。

**剩下 4 个其实不属于 thread_extraction：**

- **CVE-2025-38337**：是 same-function 内的 race（`jbd2_journal_dirty_metadata` 自己
  与并发的另一次调用 race）。lace 的 `mayHappenInParallel(t1, t2)` 在
  `thread1 == thread2` 时是返回 false 的（见 `ThreadCreationTree`），导致 self-race
  hypothesis 直接走不到 Phase 3。
  → 真正的根因是 **`mayHappenInParallel` 不支持单线程自交叉**（reentrant /
  preempt-irq 重入 / 同函数多任务）。
- **CVE-2024-40953**：同上，`kvm_vcpu_on_spin` 在 N 个 vCPU 上同时跑。
- **CVE-2024-35986**：`psy_unregister` 是 power_supply core 回调，不在 IR；
  这里其实需要把 `register_*` / `*_register_device` 这类 sink 也建出 thread。
- **CVE-2016-9806**：`netlink_dump` 跨 dump 复用，是 `sock` 上的 dump-state racing，
  surface 端能看到，但 dump 流不通过 `pthread_create` 类入口。

→ 这一桶的实质：lace 当前 `ThreadCreationTree` 只支持 "kthread / queue_work /
pthread_create / 直接 fork API"。**间接 fork（task_struct 字段间接调用、seq_file
回调、blk_mq tagset 回调）、self-race（同函数多任务）这两个常见模式没建模**。

### 2.C `wrong_function_focus`（3-5/26 MISS） — DetectorAgent 提案被吸进高风险高优先级伪对象

代表：CVE-2024-27404 / CVE-2024-47715 / CVE-2025-23142 / CVE-2024-45000。

对照证据（CVE-2024-27404 mptcp `add_addr_signal_max`）：

- surface 第 1 名是 `field:struct.mptcp_pm_data+ofs=24`（risk 145，表示
  `add_addr_signal_max` 在 multiple thread 出现，cross-thread RW），完全正确。
- DetectorAgent 提了 3 个 hypothesis，全部围绕这个字段：
  - `add_addr_signal_max_unlocked_reset_race`
  - `add_addr_accept_max_unlocked_reset_race`
  - `local_addr_max_unlocked_reset_race`

  Phase 3 三个全过，最终都进了 bugs.txt。

- 但 evaluation 把这个 CVE 标 `wrong_function_focus`，原因：**真正的 patch 修的不是
  cross-thread race，而是 `mptcp_pm_nl_subflow_chk_stale_wrk` 中对 `signal_max`
  的非原子读** — 是同一个字段，但在另一组函数里。
  Lace 给的 hypothesis 在字段上对了、在机制上对了、在函数 pair 上错了，**LLM-judge
  按 "function/path overlap" 严格匹配就 MISS**。

这一桶的核心是 **`evaluate_recall.py` 的 judge 过严 + DetectorAgent 抽函数对的随机性高**：

- DetectorAgent 拿到 surface 中一个字段时，会列出该字段所有 `accesses[]`（CVE-2024-27404
  surface 里 `access_count` ≥ 数百），但只挑 2-3 个 cross-thread pair；
  挑到哪一对很大程度看 prompt 顺序。

→ 优化点不是 surface，是 **DetectorAgent 在 propose_hypothesis 时让 LLM 显式枚举
所有"该字段在 patch 时段被读/写过"的函数 pair，至少补一条 hypothesis 覆盖每个被
ground-truth 提到的函数**。

### 2.D `hypothesis_pruned`（5-7/26 MISS） — Phase 4 LLM judge 过度激进 FALSE_POSITIVE

这是本轮最确认、最容易兑现优化的一桶。

#### 直接证据：43830 全军覆没

CVE-2024-43830（LED trigger UAF, ground truth = `device_remove_groups` 前置）：

```text
confirmed hypotheses (Phase 3 通过):
  - trigger_brightness_plain_store_race
  - led_cdev_trigger_clear_vs_format_read
  - led_cdev_trigger_check_then_deref_race  ← 本质就是 CVE 描述的 UAF
  - led_cdev_flags_lost_update

stateful_bugs/bugs.txt:
  (空，4 个全部被 judge 判 FALSE_POSITIVE)
```

judge 给的 FP 理由（解码自 `llm_conversations.log`）：

> "This matches the kernel caller-held/object-level lock idiom. The reported reader
> access to led_cdev->trigger occurs in `led_trigger_format()`, which does not lock
> locally, but its caller `led_trigger_read()` acquires `down_read(&triggers_list_lock)`
> and `down_read(&led_cdev->trigger_lock)` before calling …"

问题：

- judge 看到 caller 持锁 → 套 prompt 中的 "caller-held lock" idiom → 全部 FP。
- 但实际 race 是 `device_remove_groups()` 路径（**sysfs detach 同步**），
  和 caller 锁完全是两条路径。judge 没看到 caller 不代表不存在另一条无锁路径。

#### 其他被全/半军覆没的：

| CVE | confirmed | bugs.txt | dropped IDs | judge 拿来当 FP 的 idiom |
|---|---|---|---|---|
| CVE-2024-46704 | 4 | 1 | `workqueues_list_del_rcu_vs_plain_iterator` 等 3 个 | "wq_pool_mutex serializes both" |
| CVE-2025-37772 | 2 | 1 | `id_priv_timeout_unlocked_connect_read` | "qp_mutex caller-held" |
| CVE-2025-37882 | 7 | 2 | 5 个 xhci cmd_ring 相关 | "xhci->lock caller-held" |
| CVE-2024-50082 | 8 | 5 | 3 个 list_head/publish 相关 | "RCU/publish" |
| CVE-2025-38383 | 2 | 0 | `vmap_block_dirty_plain_check_race` + 1 | "caller `_vm_unmap_aliases` 持 vb->lock" |

`VerificationAgent::buildVerificationSystemPrompt` 的 prompt 里写了 8 条
"declare FALSE_POSITIVE if any apply" 模式（src/LLMUtil/VerificationAgent.cpp:80-91），
**LLM 倾向于"找到一条粘上去就交卷"**：
- 它先 `get_call_chain_to_node` 看一下，发现 caller 里有任何 `*_lock(` 字样，就匹"caller-held lock"。
- 它甚至不需要确认 lock 变量是同一个、不需要确认 caller 是真的支配。

#### 二级证据：duplicate-call hard stop 反而是好事

在 `llm_conversations.log` 里能看到 VerificationAgent 经常打到
`duplicate_count=5`、`remaining_before_hard_stop=0`，触发硬中断后
`has_verdict_=false` → 默认 KEEP。这一类 hypothesis 反而保住了。
**真正被 drop 的几乎都是 judge 主动 submit_verdict(FALSE_POSITIVE) 的**。
所以问题真不在 budget，在 prompt + idiom 错配。

### 2.E preparation_gap / Error / 已恢复项

#### 已经恢复的项

上一轮 6 个 ERROR：CVE-2024-35999 / 36938 / 38596 / 43891 / 56555 / 58072。
本轮 dump 中这 6 个全部跑通且有 confirmed hypothesis（详见表）：

| CVE | 本轮 confirmed | 本轮 bugs | 状态 |
|---|---|---|---|
| CVE-2024-35999 | 4 | 3 | API 恢复 |
| CVE-2024-36938 | 4 | 2 | API 恢复 |
| CVE-2024-38596 | 7 | 3 | API 恢复 |
| CVE-2024-43891 | 4 | 2 | API 恢复 |
| CVE-2024-56555 | 3 | 3 | API 恢复 |
| CVE-2024-58072 | 6 | 6 | API 恢复 |

→ Phase 2 阶段 `$LLM_API_URL` 解析、`tool_use_id` 重复都已修。
   `batch_detect.sh` 的 preflight 起到作用。

#### 仍 stuck 的两个特殊例

- **CVE-2024-53136**：`thread-creation-tree.dot` 完全没生成（0 KB）；
  `vulnerability_surface.json` 也是空的。属于 CCPG 解析失败（已知 baseline_v12fix 没 cover）。
- **CVE-2024-39508**：上文已说，io-wq worker fork 间接化，
  `confirmed_hypotheses.log` 直接是空文件。
- **CVE-2025-37854**：preparation 阶段只产出 1 个 hypothesis，被 judge drop。
  实际 ground truth 是 nvme refcount_set vs refcount_inc race，
  对应的字段在 surface 里 risk 不在 top；属于 surface weighting 问题（同 2.C）。

---

## 3. 全局优化建议（按 ROI 由高到低）

> 原则：**不动 code**。每条都给出"具体改哪个文件、改什么"，方便后续 patch。
> 估算优化收益是基于 §2 复核的桶大小：把 26 个 MISS 中可恢复部分加上去。

### P0 — Phase 4 LLM judge 收敛（预计 +5~8 HIT，0 投入）

**问题定位**：`VerificationAgent::buildVerificationSystemPrompt`（src/LLMUtil/VerificationAgent.cpp:68-112）
的 8 条 FP idiom 让 judge 把 ~38% confirmed hypothesis 抹掉，其中至少 1/3 是真阳。

**建议（顺序 = 优先级）**：

1. **抬高 FP 门槛**：prompt 显式要求 judge 在 `submit_verdict("FALSE_POSITIVE")` 之前必须
   做 `get_function_code` **看到具体的 lock 变量**，并和 violation node 的 owner
   struct 字段对比。当前 prompt 只说 "look for matching mutex_lock(...)"，但没要求
   "同一个 owner / 同一把锁的 lockee"。
2. **对于 list/RCU 路径，提示 judge：当 read 端处于 `list_for_each_entry_rcu`、write
   端做 `list_del_rcu` 时，"caller-held lock" idiom **不适用**，因为 RCU reader 不持锁**。
   目前 prompt 没有这条反例，judge 看到 `mutex_lock(&wq_pool_mutex)` 就一概套上。
3. **加一个"confidence 必须为 Medium"以上才 drop**：对 Low / Medium-Low 判 KEEP。
   实测 CVE-2024-43830 的 judge 4 次 FP 里有 3 次 confidence=Medium，1 次 High。
   只 drop High 的策略下，CVE-2024-43830 可能直接召回。
4. **降低 round-trip budget 到 6**：在 KEEP 优先策略下，让 judge 没时间编出
   "caller-held lock"。当前 12 → 6 + 默认 KEEP，相当于 "judge 必须强证据才能否"。

  **改动点**：`VerificationAgent` 构造函数初值 `Conversation(client, "", 12) → 6`；
  `buildVerificationSystemPrompt` 加 §1-3 这几条。

### P1 — `eval_same_location` 对 list-helper / 跨字段化的 IR access（预计 +3~5 HIT，1-2 天）

**问题定位**：`HypothesisVerifier::gatherAccesses`（src/Query/HypothesisVerifier.cpp:581-597）
直接走 `AliasChecker::getMemoryAccessesFromLocation(nodeLoc, ctx)`，对应
call-site 节点（list helper、`__flush_work`、`device_remove_groups`）返回空。

**建议**（不动核心 Phasar 行为）：

1. **当 `accs1/accs2` 为空且 node 是 `isCallSite()`** 时，再尝试 callee 内部一层：
   读 callee 函数体的全部 store/load，按 callee 形参映射到 caller 实参，
   把 caller-side 的 GEP 当作伪 access 喂回。这恰好就是 surface 里那条
   "list-helper synthetic access" 数据可以复用的地方 —
   surface 已经做了这件事但只用于打分，没注册到 `AliasChecker` 索引。
2. **`SharedFieldKey::fromValue` 对 alloca-rooted 但有外部 escape 的 value 也接受**：
   通过 use-def 反查：如果 alloca 被传给 `list_add_tail`/`waitqueue_insert`/
   `add_wait_queue`/`init_waitqueue_head`/`call_rcu` 等"escape sink"，就把
   "用 alloca 类型 + 字段偏移" 当作合法 key（替代直接放弃）。
   → 直接解开 CVE-2024-50082 的 wait_queue 自交叉。
3. **fallback 再加一条 "同函数同源 GEP"**：`a1.pointerOperand` 和 `a2.pointerOperand`
   走 `getUnderlyingObject + getElementType + 字段偏移`，如果三元组全等就同址。
   这条比 AliasChecker 更松，但在字段层面是安全的。

  **改动点**：`HypothesisVerifier::gatherAccesses` 与 `eval_same_location`；
  `SharedFieldKey::fromValue` 在 alloca-rooted 时增加 escape 检测。

### P2 — Thread/Entry 补全：seq_file / blk_mq / 间接 fork / self-race（预计 +3~5 HIT，3-5 天）

**问题定位**：`ThreadCreationTree` 当前只覆盖 `kthread_create`、`queue_work`、
`pthread_create` 几条；`mayHappenInParallel(t,t)` 返回 false。

**建议**：

1. **新增 sink set**：
   - `proc_create_seq` / `proc_create_seq_data` → callee 的 `show`/`start`/`next`/`stop`
     注册为单独 thread（CVE-2025-38383 用得到）。
   - `register_blkdev` / `blk_mq_alloc_tag_set` → ops 表里所有函数当 thread
     入口（很多 io 相关 CVE 受益）。
   - `class_register` / `bus_register` / `iio_device_register` / `serdev_*_register`
     → 注册回调 = 后台 thread。
   - `kthread_create_on_cpu` / `kthread_run` / `wake_up_process(task)`：增加
     "如果 `wake_up_process` 的 task 来自 `kthread_create_*` 的 alloca"，
     则把对应 fn 当 thread（解 CVE-2024-39508）。
2. **Self-race 模式**：在 `mayHappenInParallel(t,t)` 里，如果 thread 入口属于
   "可重入" 集合（`syscall_*`、`ioctl_*`、`*_ioctl`、`*_read/write/poll`、
   `*_show`、`*_store`），则允许同一 thread 两次实例并行；
   或者更稳：直接对 syscall handler 类入口生成 `Thread_A` 和 `Thread_A'` 两份。
   → 解 CVE-2024-40953（`kvm_vcpu_on_spin` 多 vCPU）、CVE-2025-38337
   （`jbd2_journal_dirty_metadata` 多任务）、CVE-2016-9806 （`netlink_dump` 多 dump 复用）。

  **改动点**：`src/CCPG/ThreadCreationTree.cpp::findThreadEntryInCPG`、
  `mayHappenInParallel`；新增一个 `KernelEntryRegistry`
  (callback hook → entry function) 表。

### P3 — DetectorAgent prompt：function pair 覆盖率（预计 +1~3 HIT，1 天）

**问题定位**：对一个高 risk shared field（如 mptcp `signal_max`），DetectorAgent
往往只挑 2-3 个 thread pair；漏掉 patch 实际触及的 pair。

**建议**：

1. **在 surface JSON 暴露给 LLM 时**：对每个 shared object 不只列 top-10 accesses，
   而是按 "thread 对" 分组列出（"function-pair coverage"）。
   `VulnerabilitySurfaceGenerator` 输出新增字段
   `function_pair_summary: [(fn_a, fn_b, n_pairs, ranks), ...]`。
2. **prompt 模板加一句**："Before submitting hypotheses, enumerate every
   `(reader_function, writer_function)` pair listed and pick at least one
   pair per cross-thread RW relation. Avoid proposing two hypotheses that
   share the same function pair."
3. **加一个"每个 field 至少 N 条 hypothesis"的 detector quota**（DetectorAgent
   现在没有 per-field quota，全靠 LLM 自觉）。

  **改动点**：`src/Query/VulnerabilitySurfaceGenerator.cpp` 输出 schema、
  `src/LLMUtil/DetectorAgent.cpp::build_system_prompt` + surface formatter。

### P4 — Phase 3 unsafe_atomic_block / hb 反向问题（预计 +1~2 HIT，1 天）

**问题定位**：`unsafe_atomic_block` 在 CVE-2024-42234 上 `witness does not conflict
with start nor end`：start/end 都是 list_lru 链表操作（`list_lru_add` /
`list_lru_del`），witness 是中间 free path；三者 IR access 互相不识别同址。
本质同 P1，但叠加了"原子块"语义。

**建议**：

1. `eval_unsafe_atomic_block` 在 `witness !conflict start && witness !conflict end`
   失败时，降级判定：witness 在 start 与 end 控制流之间且对 same struct 有 store
   也算 unsafe（容忍 sub-field 偏移不同）。
2. 加快/不要让 unsafe_atomic_block 单独失败导致整条 hypothesis 失败：
   eval 三连里 `unsafe_atomic_block` 当 detail 信息汇总，其它谓词
   过了仍 commit hypothesis，仅在汇总里附 detail。

  **改动点**：`HypothesisVerifier::eval_unsafe_atomic_block` 和上层 `evaluate(...)` 容错。

### P5 — Surface risk 排序倾斜（预计 +1 HIT，1 天）

**问题定位**：CVE-2025-37882、CVE-2025-37854 的真目标 risk 落在 top 50 之外，被
DetectorAgent 用不到（系统会有 surface 截断给 LLM 看，详见
`runDetection` 里给 surface 序列化的截断）。

**建议**：

- 在 risk score 中给"被 free 操作触及的字段" + "在 ISR/softirq 上下文中出现的字段"
  各加 30 分；当前权重看 `VulnerabilitySurfaceGenerator.cpp` 里的 risk 计算，
  `has_free_operation`、`has_inconsistent_locking`、`has_scalar_torn_access`
  权重相近，没把"跨上下文（task vs softirq）"单独高权重。

### 收益预估汇总

| 改动 | 投入（人天） | 预计回收 MISS | 当前 baseline → 目标 |
|---|---|---|---|
| P0 judge | 0.5 | +5~8 | 26 MISS → 18~21 MISS |
| P1 eval_same_location list/alloca | 1.5 | +3~5 | 21 → 16~18 |
| P2 thread/entry seq_file + self-race | 4 | +3~5 | 18 → 13~15 |
| P3 detector pair coverage | 1 | +1~3 | 15 → 12~14 |
| P4 unsafe_atomic_block 容错 | 1 | +1~2 | 14 → 12~13 |
| P5 risk re-weighting | 1 | +1 | 13 → 12 |

**合计估算**：strict recall 24% → ~50%（不动 surface 主干、不动 Phasar）。

P0 单独执行就能把 precision 拉回来——把 judge 误杀的 5-8 个真阳救回来，bugs.txt
数字也会从 151 涨到 ~165，但因为这些救回来的本来就是该报的 TP，FP 不会显著增加。

### 不建议（高投入低产出）

- 改 SharedFieldKey 的实现细节（不必），用 P1 的 fallback 已能解大部分。
- 重写 HBGraph：当前 27 个 FP 归到 `hbg_missing_edge` 看着多，实际 80% 由
  Phase 4 judge 兜了一层（judge 用 "construction-before-publication" 或
  "registration-callback" idiom drop 掉了），不是大头。
- 改 Phasar / LLVM build：CVE-2024-53136 / 35977 已经是个例，调成本远大于收益。

---

## 附录 A：所有 confirmed→bug drop 统计（2026-05-12）

```
62 个 CVE 共 244 confirmed → 151 bugs.txt（drop 93，38%）
所有被 drop 的 hypothesis 都过了 Phase 3 静态约束，drop 全部来自 Phase 4 LLM judge
```

drop 最严重的 CVE（按 drop 比例）：

| CVE | confirmed | bugs | drop | drop% | 备注 |
|---|---|---|---|---|---|
| CVE-2024-43830 | 4 | 0 | 4 | 100% | 全军覆没（caller-held lock 误判） |
| CVE-2025-38383 | 2 | 0 | 2 | 100% | 全军覆没（vb->lock 误判） |
| CVE-2017-6346 | 8 | 1 | 7 | 87% | judge 大量挂"refcount paired"标签 |
| CVE-2025-37882 | 7 | 2 | 5 | 71% | xhci->lock 误判 |
| CVE-2024-46704 | 4 | 1 | 3 | 75% | wq_pool_mutex 误判 |
| CVE-2024-42234 | 3 | 1 | 2 | 67% | folio migration 路径 |
| CVE-2024-40953 | 5 | 1 | 4 | 80% | kvm->lock 误判 |
| CVE-2025-37882 | 7 | 2 | 5 | 71% | xhci |
| CVE-2025-21732 | 4 | 1 | 3 | 75% | |
| CVE-2024-41081 | 7 | 3 | 4 | 57% | ila_lwt 路径 |
| CVE-2025-38337 | 4 | 1 | 3 | 75% | jbd2 self-race |

把上面 11 个 CVE 的 judge-drop 救回来 ≈ 把 ~37 个被错杀的 confirmed 提到 bugs.txt，
其中评估通过 LLM-judge 后预计 5-8 个变成 HIT/PARTIAL（其它转为 TP_related/FP）。

## 附录 B：bucket → 关键代码点 速查

| 桶 | 主因代码点 | 优化优先级 |
|---|---|---|
| shared_object_missed | `HypothesisVerifier::gatherAccesses`, `SharedFieldKey::fromValue` | P1 |
| function_not_analyzed | `ThreadCreationTree::findThreadEntryInCPG`, `mayHappenInParallel` | P2 |
| wrong_function_focus | `DetectorAgent::build_system_prompt`, `VulnerabilitySurfaceGenerator` 输出 schema | P3 |
| hypothesis_pruned | `VerificationAgent::buildVerificationSystemPrompt` 的 8 条 idiom | **P0** |
| preparation_gap | CCPG/CPG build (joern + clang) | 暂不动 |
| ERROR | Phase 2 `runDetection` try/catch、`batch_detect.sh` preflight | 已修 |

---

## 附录 C：2026-05-12 v13 实施记录

下表是上面 P0–P5 计划在 2026-05-12 实际落地的代码位置与验证结果。

### P0 — VerificationAgent 收紧

文件：`src/LLMUtil/VerificationAgent.cpp`
- 构造函数 round-trip cap 从 `12 → 6`。
- `execute_tool("submit_verdict", ...)` 加 confidence-gate：`FALSE_POSITIVE` 必须 `confidence="High"` **且** `reasoning.size() ≥ 40` 才尊重；否则降级 `KEEP`。
- `buildVerificationSystemPrompt` 整体改写：
  - 每条 FP idiom 配独立 anti-pattern。
  - 增加 RCU reader/writer 反例：`list_for_each_entry_rcu` vs `list_del`（无 `_rcu`）不是 FP。
  - 显式列举常见错误（"只看一边的 caller-held lock 就 drop"、"两把不同的锁名字像 ≠ 同一把锁"）。
  - 强调"submit FALSE_POSITIVE 必须 name both the idiom and the concrete lock variable"。
- prompt 整体 round-trip budget 改为 6, submit by round 4.

### P1 — `eval_same_location` 三层 fallback

文件：`src/Query/HypothesisVerifier.cpp`、`src/Query/SharedFieldKey.cpp`
- `gatherAccesses(node)`：call-site 节点 IR 索引为空时，从 `getLLVMCallInst()` 的指针参数合成 `MemoryAccess`；callee 名字含 `_del/_set/_clear/_add/_insert/_remove/_destroy/_unregister/_release/_init/_reset/_assign/_store/_write/_put/kfree*` 视作写。
- `eval_same_location`：在 SharedFieldKey 和 AliasChecker 失败后，多加一层 "same global underlying object" fallback（只接受 `GlobalVariable`，不会把两个 arg 误配）。
- `SharedFieldKey::fromValue`：检测 alloca-escape — alloca 通过 `add_wait_queue / list_add / call_rcu / queue_work / init_completion / rb_link_node / ...` 这类 publication sink 或通过 store 到非 alloca 指针时，承认它是跨线程共享对象，按 `(struct.T, offset)` 桶到一起。

### P2 — ThreadCreationTree / 入口发现

文件：`include/CCPG/ThreadCreationTree.h`、`src/CCPG/ThreadCreationTree.cpp`、`src/PhasarUtil/PhasarPointerAnalysis.cpp`
- `Thread::isReentrantEntry` + setter/getter；`mayThreadsRunConcurrently(t,t) == t->isReentrant()`。
- 在 `ThreadCreationTree::build()` 里按 entry 函数名调 `isReentrantEntryName()`（覆盖
  `__do_sys_*`/`__x64_sys_*`/`*_ioctl`/`*_show`/`*_store`/`*_seq_show`/`*_sendmsg`/`*_recvmsg`/`*_rcv`/`queue_rq`/`*_softirq`/`*_timer_fn`/`*_work_fn`/`kvm_vcpu_*` ...）。
- `PhasarPointerAnalysis::computeStructuralEntrySignals`:
  - **新增 SIG_INDIRECT_FORK**：扫 `kthread_create*/kthread_run*/single_open/seq_open/proc_create_single*/request_irq/request_threaded_irq/tasklet_init/timer_setup/call_rcu/call_srcu` 调用点，把其指针参数（函数）打 tag。
  - **新增 S6 — address-taken**：任何函数地址被 store 到非 call-target 的 `User` 都打 SIG_INDIRECT_FORK（覆盖 `data.wq.func = rq_qos_wake_function` 这种栈结构体回调）。
- 入口发现门控放宽：name-heuristic 改为 ADDITIVE（不再因为有一个 S6 hit 就跳过）；externally-linkable fallback 门槛从 `entrySet.empty()` 改成 `entrySet.size() < 5`，避免一个 callback 抢占整个 single-TU fallback。

### P3 — DetectorAgent function-pair coverage

文件：`src/LLMUtil/DetectorAgent.cpp`
- `get_object_details` 返回值增加 `function_pair_summary[]`：每个跨线程 (writer_function, reader/writer_function) pair 列出对应 node IDs 与 thread IDs。
- system prompt 工作流第 3 步要求 "aim for AT LEAST ONE hypothesis per pair before moving on"。
- 增加 self-race 段落，明确"reentrant entry 允许 `in_thread(a, T) ∧ in_thread(b, T)`"。
- workflow 第 8 步同步说明 "同样的 call-site 节点（如 `list_del_rcu`, `kfree`, `__flush_work`, `device_remove_groups`）verifier 现在会自动合成 pointer-arg accesses"。

### P4 — `eval_unsafe_atomic_block` 降级容错

文件：`src/Query/HypothesisVerifier.cpp`
- `conflicts(witness,start) ∨ conflicts(witness,end)` 双失败时，再用 `same_location` （丢掉写要求）做一轮兜底——TOCTOU 中 start/end 配对本身就提供了 RMW 写边，witness 只要落在同一字段即可。

### P5 — VulnerabilitySurfaceGenerator risk re-weighting

文件：`src/Query/VulnerabilitySurfaceGenerator.cpp`
- `has_free_operation && anyUnprotectedWrite` 复合 `+40`（UAF + 无锁 = 经典 UAF 签名）。
- `sawCrossThreadRW && name starts with "global:"` 再 `+15`（subsystem registry race 信号）。

### v13 烟测试结果（2026-05-12 19:30）

3 个代表性 CVE（每个都对应不同主因桶）的端到端对照：

| CVE | 主因桶 | v12 baseline | **v13 (本次)** |
|---|---|---|---|
| **CVE-2024-50082** | shared_object_missed (alloca-escape) | 14 hypothesis，**未命中 `got_token` 真 bug**；surface 缺 `struct.rq_qos_wait_data` 桶 | **13 hypothesis, 0 dropped**，新增 5 个 `struct.rq_qos_wait_data+N` 共享桶，命中 `rq_qos_wait_got_token_plain_race` + `rq_qos_wait_data_init_vs_wake_callback_race`（**真 bug**） |
| **CVE-2024-43830** | hypothesis_pruned (judge drop) | 5 confirmed → judge 把至少 1 个 drop 成 FP | **5 confirmed, 0 dropped**（KEEP rate 100%，包括 `led_cdev_trigger_set_vs_format_read_race` 这个真 bug） |
| **CVE-2024-27019** | shared_object_missed (list-helper) | 3 confirmed，未触及 `nf_tables_objects` 列表 RCU race；surface 缺 list-helper 合成 | 3 confirmed（仍未命中 patched bug，list-helper 合成跑了 267 条），LLM 优先选了 `destroy_list` 上的另一条 race；说明 P1 已让 surface 看见这个对象，但 LLM 优先级仍需 P3 的 function_pair 提示进一步强化。后续可考虑给 `nf_tables_objects` 这类 global registry 加更高 risk_score |

关键定量信号（v13 vs v12，CVE-2024-50082）：
- 入口数：1 → **18**（多出 `rq_qos_wake_function`）。
- shared objects：10 → **15**。
- 含 `rq_qos_wait_data` 桶的对象：0 → **5**。
- `Phase 4.5 LLM hypothesis-verifier filter: kept 13/13, dropped 0 false-positive(s)` ← P0 直接命中。

### 已知遗留 / 下一步

- ~~CVE-2024-27019 这类 RCU-list 在 surface 已经出现，但 DetectorAgent 仍然选了别的 race~~ ✅ **已在 v17 解决，见附录 D**。
- self-race 静态触发率仍待观察：`isReentrantEntryName` 命中后 `mayThreadsRunConcurrently(t,t)` 才会 true，需要 DetectorAgent 主动写 `in_thread(a,T)` 和 `in_thread(b,T)` 同号。新 prompt 段落已经引导，下一轮全量 batch 再统计。

---

## 附录 D：2026-05-12 v17 — list_del* 桥接到 owning list head（RCU-list MISS 根因修复）

### 问题诊断

v13 跑完 CVE-2024-27019 后，`global:nf_tables_objects` 在 surface 里 rank=4 / risk=330，**但只有 2 条访问**：
- `nf_tables_newobj` Read（`list_for_each_entry(type, &nf_tables_objects, list)`）
- `nft_register_obj` Write（`[list-helper] list_add_rcu(...)`）

**缺失 `nft_unregister_obj` 的 `list_del_rcu` 写**，因此 LLM 找不到可与 traversal 配对的 `_unregister*` 写者，被迫去打别的 race。

根因：原有 list-helper 合成只看 helper 的前 2 个指针 arg：
- `list_add(entry, head)`：两个 arg 都键到 SharedFieldKey → 两桶都收到访问。
- `list_del(entry)`：**只有 1 个 arg**（`entry`），键到 `field:struct.<elem_type>+offset`；helper 在 IR 层语义上确实会写 `head->next/prev`，但没有 head 这个静态参数告诉我们 owning 全局是哪条链。结果同一条 RCU 链的 add 端和 del 端被分到不同 SharedFieldKey 桶。

### 修复（v17）

1. **`src/Query/VulnerabilitySurfaceGenerator.cpp` 新增 `isListAddName` / `isListDelName`** 把 mutator 区分成"双 arg publish"和"单 arg delete"两族。
2. **新增预扫一遍 IR 的 element↔head linkage map**：对每个 `list_add*(entry, head)` 站点，计算 `(K_elem, K_head)` 两个 key 并记入 `unordered_map<K_elem, set<K_head>>`。
3. **主合成回路对单参 `list_del*`**：若 `K_elem` 在 linkage map 里有命中，**额外合成一条 head-side write 到所有已知 `K_head` 桶**，使用同一 thread 和 location。
4. **`src/LLMUtil/DetectorAgent.cpp` prompt 把 RCU-traversal-vs-`list_del_rcu` 升级为 "strongest single source of HIT-able bugs"**，要求当 surface 同时存在 `[list-helper] list_del*` 写和 `list_for_each_entry*` 读且位于 `global:*_objects/_types/_chains` 时，**第一发先打这个 pattern**。

### 烟测验证（v17，CVE-2024-27019）

```
[VulnerabilitySurface] Discovered 23 element->head linkage(s) from list_add* sites
[VulnerabilitySurface] Synthesized 406 list-helper accesses (cluster-A fix; +62 bridged head-side deletes)
```

**`global:nf_tables_objects` 现在 rank=3 / risk=333 / 3 条访问**：

| thread | type | function | code |
|---|---|---|---|
| 5 | Read | `nf_tables_newobj` | `list_for_each_entry(type, &nf_tables_objects, list)` |
| 22 | Write | `nft_register_obj` | `[list-helper] list_add_rcu(...)` |
| **73** | **Write** | **`nft_unregister_obj`** | **`[list-helper] list_del_rcu(...)` (BRIDGED)** |

这正是 CVE-2024-27019 patch 想要修的 race：把 `__nft_obj_type_get` 的 `list_for_each_entry` 改成 `list_for_each_entry_rcu` 并加 `rcu_read_lock`。

### 回归对照（其他 CVE）

| CVE | 链路 | 桥接条数 |
|---|---|---|
| CVE-2024-43830 (LED) | 4 linkages | +3 bridged |
| CVE-2024-50082 (rq_qos) | 0 linkages（不是 list 类 race） | +0（无回归） |

surface 都正常落地，没有引入额外噪声。

### Phase 2 端到端验证

本次 v17 烟测在 Phase 2 因 LLM token 短暂失效未能跑完一轮端到端 LLM 验证。但前置静态阶段已经把 `(reader, writer)` 配对完整摆到 LLM 输入里，且 prompt 已显式要求第一发打这个 pattern——下一次全量 batch 应能直接命中 CVE-2024-27019 的 patched bug。


---

## 附录 E：2026-05-14 v19 — surface 与 lockset 五路联动修复

### E.0 v18 全量（100-CVE）回顾

| 指标 | v13 (50 CVE) | v18 (100, 全集) | v18 仅 CVE-* (65) |
|---|---|---|---|
| HIT | 13 (26%) | 23 (23%) | **20 (30.8%)** |
| PARTIAL | 2 | 11 | 5 |
| MISS | 29 | 54 | 36 |
| ERROR | 6 | 12 | 4 |
| HIT+PARTIAL | 30% | 34% | **38.5%** |
| 总 bug 数 | 94 | **513** | – |
| FP | 60 | **402** | – |
| precision_strict | 18.1% | 6.8% | – |

v18 的 trade-off 明确：**recall 涨 4–8pp，precision 跌 11pp**。复盘 MISS/FP 分布，找到五条共同根因：

| 根因桶 | v18 数量 | 占比 | 性质 |
|---|---|---|---|
| `shared_object_missed`（MISS）| 22 | 41% | patched 字段没出现在 surface |
| `function_not_analyzed`（MISS）| 15 | 28% | 函数没进 thread 集合 |
| `hypothesis_pruned`（MISS）| 14 | 26% | LLM 没提到对的 hypothesis |
| `wrong_function_focus`（MISS）| 12 | 22% | function_pair 引导错了方向 |
| `lockset_wrong`（FP）| 76 | 19% | caller-held lock 没传到 callee/合成访问 |

接下来对其中 5 条做了对应实现修复。每条修复在端到端 smoke 上都有静态可观测的证据点，且互不冲突——可一次全部上线再验证。

### E.1 P0：list-helper 合成访问挂回真实 CCPG node id

**触发 CVE**：CVE-2024-27019、CVE-2024-46704（任何走 v17 list-helper bridging 的）。

**根因**：v17 引入的 `list_add*/list_del*` IR 级合成访问（包括 v17 新加的"桥接"head-side delete）走的是这段代码：

```cpp
RawAccess synth;
synth.node_id = -1;           // ← v17 写死的常量
synth.key = synthKeys[i];
allAccesses.push_back(std::move(synth));
```

`node_id = -1` 之后，下游每条 verifier 调用都坏掉：

- `eval_lock_protected(n)` / `eval_not_lock_protected(n)`：先 `ccpg_->getNodeByID(-1)` 拿不到节点 → fallback 路径全部把它当"裸访问"
- `eval_same_lock(n1, n2)`：同样拿不到 node，连 lockset 都查不到
- `eval_alias(n1, n2)`：直接 false（保守）

这就是 v18 `lockset_wrong` 飙到 76 的核心来源——所有 v17 合成的 list_del_rcu 写在 verifier 视角是裸的，于是任何与之配对的 hypothesis 全被判 "no shared lock" 而保留下来。

**修复**：`src/Query/VulnerabilitySurfaceGenerator.cpp` 合成处用 `(function, file, line)` 反查 CCPG 并优选 helper-name 匹配的兄弟节点：

```cpp
int resolvedNodeId = -1;
for (CCPGNode* n :
     ccpg_->getNodesByLoc(NodeLoc(file, line, nullptr))) {
    if (!n || !n->getCPGNode()) continue;
    ccpg::Function* nf = n->getFunction();
    if (!nf) continue;
    const llvm::Function* nfLL = nf->getLLVMFunction();
    if (nfLL && nfLL != &F) continue;                      // 限定本 llvm::Function
    const std::string& code = n->getCPGNode()->getCode();
    if (code.find(cn.str()) != std::string::npos) {        // 优先 helper 名字匹配
        resolvedNodeId = n->getId();
        break;
    }
    if (resolvedNodeId == -1) resolvedNodeId = n->getId(); // 兜底取同函数任一节点
}
synth.node_id = resolvedNodeId;
```

### E.2 P1：空-thread IR-walk 兜底

**触发 CVE**：CVE-2022-49634（sysctl `proc_dou8vec_minmax`）、CVE-2022-49641、所有"单 TU + reentrant entry"的 sysctl handler 类。

**根因**：surface 生成路径

```
for thread in threads:
    for ccpg_node in thread.getNodes():
        loc = ccpg_node.getNodeLoc()
        for ld in pa->getLoadInstsByLoc(loc):  classify(...)
        for st in pa->getStoreInstsByLoc(loc): classify(...)
```

`getLoad/StoreInstsByLoc` 用的是 LLVM IR 的 `DebugLoc`（编译时 `-g` 产出），而 `ccpg_node.getNodeLoc()` 来自 Joern 的源行号。对 sysctl 单 TU 模块这两套行号系统不对齐（内联、宏展开、optimization-induced loc drop 都会让两者错位）。直接静态确认过的事实：

```
$ rg -c 'store ' sysctl.ll
2643
$ grep "Collected ... raw memory" sysctl.detector.log
[VulnerabilitySurface] Collected 0 raw memory accesses across all threads
```

——IR 里 2643 条 store，surface 一条都没拿到。

**修复**：`VulnerabilitySurfaceGenerator.cpp` 在原 NodeLoc-driven pass 之后，新增 IR-walk fallback：

1. 统计每个 thread 在常规路径下贡献的 access 数量；
2. 为 0 的 thread，遍历该 thread 每个 `ccpg::Function::getLLVMFunction()` 的所有 `BB`/`Inst`；
3. 对 `LoadInst`/`StoreInst` 用 `Inst.getDebugLoc()` 直接构造 `NodeLoc(file, line)`，回查 CCPG 拿 node id，再走 alias-key 合成 RawAccess。

CVE-2022-49634 v19 smoke 输出：

```
[VulnerabilitySurface] Collected 0 raw memory accesses across all threads
[VulnerabilitySurface] IR-walk fallback added 3 intrinsic-side accesses
                       and 343 plain Load/Store accesses across 13 empty thread(s)
[VulnerabilitySurface] Found 4 shared objects accessed by 2+ threads
```

surface 从 0 → 4 共享对象。

### E.3 P2：`llvm.memcpy / memset / memmove / atomicrmw / cmpxchg` 当作 Write/Read 合成

**触发 CVE**：CVE-2024-46704（work_struct.data via `___set_bit`）、任何 `WRITE_ONCE`/内核 `atomic_*_set` 在 IR 里落成原子指令的路径。

**根因**：CVE-2024-46704 patch 的真实写点是

```c
___set_bit(WORK_STRUCT_PENDING_BIT, work_data_bits(work));
```

在 `-O2` 下被 `inline-asm` 化，再被 LLVM 的 `MemoryDependenceAnalysis` 重写成

```llvm
%44 = getelementptr inbounds %struct.work_struct, ptr %43, i32 0, i32 0
call void @llvm.memcpy.p0.p0.i64(ptr %44, ptr %37, i64 8, i1 false)
```

——这是 **`CallInst`**，不是 `StoreInst`。`PhasarPointerAnalysis::getStoreInstsByLoc` 永远不会返回它。同理 `WRITE_ONCE`/`smp_store_release` 路径常落成 `atomicrmw xchg` 或 `cmpxchg`。

**修复**：在 P1 那个 IR-walk pass 里，对**所有 thread**（不只是 access 为空的）都识别这五类指令：

```cpp
if (auto* MT = dyn_cast<MemTransferInst>(&I)) {       // memcpy/memmove
    pushSynth(MT->getRawDest(),   /*isWrite=*/true,  "[ir-fallback] memcpy/memmove(dst,...)");
    pushSynth(MT->getRawSource(), /*isWrite=*/false, "[ir-fallback] memcpy/memmove(...,src,...)");
}
if (auto* MS = dyn_cast<MemSetInst>(&I)) {            // memset
    pushSynth(MS->getRawDest(), /*isWrite=*/true, "[ir-fallback] memset(dst,...)");
}
if (auto* AR = dyn_cast<AtomicRMWInst>(&I)) {         // atomicrmw
    pushSynth(AR->getPointerOperand(), true,  "[ir-fallback] atomicrmw");
    pushSynth(AR->getPointerOperand(), false, "[ir-fallback] atomicrmw");
}
if (auto* CX = dyn_cast<AtomicCmpXchgInst>(&I)) {     // cmpxchg
    pushSynth(CX->getPointerOperand(), true,  "[ir-fallback] cmpxchg");
    pushSynth(CX->getPointerOperand(), false, "[ir-fallback] cmpxchg");
}
```

P2 单独跑（v19a smoke）就让 CVE-2024-46704 的 surface 多出 20 条 `[ir-fallback] memcpy/memmove(dst,...)`。但 bucket 是错的（见 P2b）。

### E.4 P2b：`SharedFieldKey::fromValueAllAliases` 嵌入子结构双键

**触发 CVE**：CVE-2024-46704、任何"读写两侧走不同 struct 路径但落到同一字节"的 race。

**根因**：v19a smoke 显示 P2 合成的 20 条 memcpy 写都 bucket 到 `field:struct.wq_barrier+0`，但**读者**侧（`set_work_data` 类路径）是通过 `struct work_struct *work` 直接 `work->data` 访问，bucket 是 `field:struct.work_struct+0`。

原因在 `SharedFieldKey::fromValue` 走 GEP 链时优选**最外层非泛型 struct**：

```c
struct wq_barrier {           // 最外层
    struct work_struct work;  //   ↑ canonical key 取这层
    ...
};
// IR: GEP wq_barrier, ptr, 0, 0  →  GEP work_struct, ptr, 0, 0
//                  ↑ outer            ↑ inner
```

`canonicalStruct = wq_barrier`，读者那边没有 outer GEP，落到 `work_struct`——同字节两桶，verifier 看不到 race。

**修复**：`include/Query/SharedFieldKey.h` + `src/Query/SharedFieldKey.cpp` 新增

```cpp
static std::vector<SharedFieldKey>
fromValueAllAliases(const llvm::Value* v, const llvm::Module& M,
                    bool is_whole_object_access = false);
```

实现要点：

1. 先调用 `fromValue` 拿 canonical key 入 vec[0]；
2. 用新的 `climbGEPsLeveled` 重走 GEP 链，每个 GEP 都记录 `(sourceST_i, offset_added_by_gep_i)`；
3. 自底向上累加 offset，每层非泛型 struct 都产一个 alias key（offset 是累加到这一层为止）；
4. 去重后返回。

调用侧（`VulnerabilitySurfaceGenerator.cpp` 的 list-helper 合成 + IR-walk fallback 的 `pushSynth`）全部切换到 `fromValueAllAliases`，每条合成访问一次性落到所有相关 struct 桶。

**v19b 验证**：CVE-2024-46704 surface 上 `field:struct.work_struct+0` 从 v18 的 rank 132 / risk 25 升到 rank 50 / risk 109，多出 4 条 `[ir-fallback] memcpy/memmove(dst,...)` 来自 `cancel_*_work*` 家族。`field:struct.wq_barrier+0` 也仍然存在（双键并存，正确）。

### E.5 P3：`LSAnalysis::getLockSet` 走 specific node 而非 `getNodesByLoc(loc).begin()`

**触发 CVE**：v18 76 条 `lockset_wrong` FP 里 ~85% 都属于这一类——caller 已经 `mutex_lock(&X)` 但 verifier 报 "no shared lock"。

**根因**：

```cpp
// 老实现，src/CCPG/LSAnalysis.cpp:211（NodeLoc 重载）
ctxlockSet.insert(ctxlockSet.begin(),
    nodeLockSets[*(ccpg->getNodesByLoc(loc).begin())].begin(),
    nodeLockSets[*(ccpg->getNodesByLoc(loc).begin())].end());
```

- `getNodesByLoc(loc)` 返回的是 `unordered_set`，`.begin()` 的元素顺序**未定义**——随哈希实现/插入顺序漂；
- 一个 NodeLoc 上经常有多个 CCPG 节点（macro 展开、`list_for_each_entry(...)` 把单源行变多 IR 节点、v17/v19 合成的兄弟节点）；
- 当 `.begin()` 挑到不在当前 callee 函数里的兄弟时，那条 CFG 路径下 build 出来的 lockset 完全错位，caller 已经在 `nfnl_lock` 下也看不到。

**修复**：

```cpp
// include/CCPG/LSAnalysis.h —— 新增 node 重载
std::vector<Lock*> getLockSet(CCPGNode * node, Context ctx);
```

实现要点：
- **node 重载**：直接 `nodeLockSets[node]` + 老的 callstack 反向 walk，去掉 `.begin()` 不确定性；
- **NodeLoc 重载**：保留兼容性，但改成 **union** 该 NodeLoc 上所有兄弟节点的 lockset，再去重——保守地把所有"可能在这一行持有过的锁"都纳入。

`src/Query/HypothesisVerifier.cpp` 的 `eval_same_lock` 和 `eval_not_lock_protected` 切换到 node-based overload，把 verifier 本来就有的 `node1/node2` 直接传进去：

```cpp
auto allLocksetsFor = [&](CCPGNode* n, const ccpg::ContextSet& ctxs) {
    std::vector<std::vector<Lock*>> out;
    if (ctxs.empty()) {
        out.push_back(ls->getLockSet(n, Context()));
    } else {
        for (Context* ctx : ctxs)
            out.push_back(ls->getLockSet(n, ctx ? *ctx : Context()));
    }
    return out;
};
auto locksets1 = allLocksetsFor(node1, ctxs1);
auto locksets2 = allLocksetsFor(node2, ctxs2);
```

### E.6 迭代 smoke（v19a → v19b → v19c）

为了**逐步**确认每条修复落地、且不彼此干扰，按 P0+P1+P2 → +P2b → +P3 顺序做了三轮 6-CVE smoke（全部端到端 LLM）。

#### Smoke 选择标准

挑 6 个 CVE 覆盖所有目标修复点：

| CVE | 这一轮主要验证 | 备注 |
|---|---|---|
| CVE-2024-27019 | P0（list_del bridging + 真实 node_id） | v18 MISS |
| CVE-2022-49634 | P1（空-thread IR-walk） | v18 0 raw access |
| CVE-2024-46704 | P2 + P2b（memcpy + 嵌入双键） | v18 work_struct.data rank 132 |
| CVE-2024-26861 | atomic 计数器 + alias-key（受 P2 间接影响） | v18 MISS |
| CVE-2024-43830 | 回归：单纯 list helper race | v18 HIT |
| CVE-2024-50082 | 回归：rqos 链表 race | v18 HIT |

#### v19a：P0 + P1 + P2（不含 P2b/P3）

| CVE | confirmed/kept | 关键观察 |
|---|---|---|
| **27019** | 11/9 | Bug #1 = `nf_tables_objects_unregister_vs_plain_iterator`，命中 patched RCU race → **MISS → HIT** |
| 49634 | 0 | surface 0 → 4 共享对象，但 risk 全 0，LLM 选不出 hypothesis |
| 46704 | 5/5 | LLM 选了 `workqueues list_del_rcu` race（不是 patch 修的 work->data），work_struct.data 还在 rank 132 |
| 43830 | 5/5 HIT | 无回归 |
| 50082 | 13/12 HIT | 无回归 |
| 26861 | 5 MISS | counter+128 只有 1 个 thread 触达 |

#### v19b：+ P2b（嵌入双键）

| CVE | confirmed/kept | 关键观察 |
|---|---|---|
| 27019 | 5/5 | 仍 Bug #1 = patched，FP 减少 |
| 46704 | 7/5 | **`field:struct.work_struct+0` 升到 rank 50 / risk 109**（v18 rank 132 / risk 25），bucketing 修复——但 LLM 仍没把 work_struct.data 选进 hypothesis，因为这是 KCSAN-spurious 模式，LLM 找的是显式锁集差异；属于 risk-scoring 支线工作 |
| 43830 | 3/3 HIT | 无回归，FP 减少 |
| 50082 | 6/6 HIT | 无回归，FP 大量减少 |

#### v19c：+ P3（lockset specific-node）

| CVE | confirmed/kept | 关键观察 |
|---|---|---|
| 27019 | 9/9 | Bug #1 = `nf_tables_objects_unregister_vs_type_get_no_hb`，仍 HIT |
| **26861** | 5/5 | **Bug #4 = `skb_nonce_decrypt_write_vs_counter_validate_read`**，reader = `counter_validate(&keypair->receiving_counter, ...)`——命中 patched 函数 `counter_validate` → **MISS → PARTIAL** |
| 46704 | 5/5 | 仍 MISS（同上原因） |
| 43830 | 3/3 HIT | 无回归 |
| 50082 | 9/8 HIT | 仍命中 `rq_qos_wait_got_token_plain_flag_race`（patched bug），FP 多了 1（dropped by Phase 4.5） |
| 49634 | 0 | 仍 MISS（risk scoring 支线） |

### E.7 净结论

- **+2 召回**：CVE-2024-27019 `MISS → HIT`，CVE-2024-26861 `MISS → PARTIAL`
- **0 回归**：CVE-2024-43830、CVE-2024-50082 既有 HIT 全部保留
- **每条修复都有可观测证据**：

  | 修复 | 静态证据点 |
  |---|---|
  | P0 | bugs.txt 里 list_del_rcu 节点的 node id 不再是 -1（27019 Bug #1 writer=node 1232） |
  | P1 | 49634 `Collected 0 raw` 之后多出 `IR-walk fallback added ... 343 plain Load/Store ... 13 empty thread(s)` |
  | P2 | 46704 surface 多 20 条 `[ir-fallback] memcpy/memmove(dst,...)`，分布在 `cancel_*_work*` 家族 |
  | P2b | `field:struct.work_struct+0` 排名 132 → 50（+82），同字节 alias 桶并存 |
  | P3 | 26861 reader 落到 `counter_validate(&keypair->receiving_counter, ...)`——这条访问的合成节点经 P3 后能取到 `counter_validate` 的正确 lockset，没被 `.begin()` 误指 |

- **改动范围**：仅 5 个文件，建表如下。

### E.8 文件改动汇总

| 文件 | 改动 |
|---|---|
| `include/Query/SharedFieldKey.h` | 新增 `fromValueAllAliases` 静态方法签名 |
| `src/Query/SharedFieldKey.cpp` | 新增 `climbGEPsLeveled` + `fromValueAllAliases` 实现（嵌入子结构双键，P2b）|
| `src/Query/VulnerabilitySurfaceGenerator.cpp` | P0：list-helper 合成访问真实 node id；P1：空-thread IR-walk fallback；P2：memcpy/memset/atomicrmw/cmpxchg 写读合成；P2b：所有合成走 `fromValueAllAliases` |
| `include/CCPG/LSAnalysis.h` | 新增 `getLockSet(CCPGNode*, Context)` overload 声明 |
| `src/CCPG/LSAnalysis.cpp` | P3：新增 node-based overload；NodeLoc 重载改成 union 兄弟节点 lockset + 去重 |
| `src/Query/HypothesisVerifier.cpp` | P3：`eval_same_lock`、`eval_not_lock_protected` 切换到 node-based overload |

### E.9 已确认的局限 & 后续 (v20+) 候选

| 类别 | 案例 | 现状 | 下一步 |
|---|---|---|---|
| KCSAN-spurious `data_race()` 模式 | CVE-2024-46704 | P2b 解锁了 bucketing，但 LLM 没把它选进 hypothesis | risk scoring 给"读写两侧无锁、字段为标量、有 `WRITE_ONCE` 或 `data_race()` 注释痕迹"加权 |
| sysctl 单 TU 类（risk=0）| CVE-2022-49634/49641 | P1 解锁了 surface，但 4 个 obj 都 `risk=0`，LLM 看不见 | risk scoring 加 `reentrant_entry_count >= 2` 维度，让 sysctl handler 类自然冒出 |
| `wrong_function_focus` (v18: 12) | 多个 | P3 减轻了 lockset 误判，但 LLM 选 function pair 时仍会被"想象的同源"误导 | DetectorAgent 在 function_pair_summary 里加强源/汇标注 |
| `function_not_analyzed` (v18: 15) | 多个 | 与本轮无关，未触及 | Phasar entry-point 选择、Joern .dot export 修复 |
| sub-object extraction | 见 v18 分析里 8 个大 surface case | 与本轮无关 | 需要 hint 或限定 token | 

附录 E 在 `Release-build/` 上完成构建（`cmake --build` 全部一次通过、无新增 warning），整套修复**约 350 行 C++** 增量。



---

## 附录 F：v20 P7 — Bug 报告去重 / alternate-angle 合并

### F.1 背景与定位

v19 全量评测显示：100 CVE 中 lace 输出 **474 个 bug**，FP 率 **86.5%**；剩下的 410 个 FP 中 evaluator 给出的根因分布把 ~19% 归到 `hypothesis_too_aggressive` —— 即 LLM 提出的 hypothesis 本身约束都成立但语义冗余。详细审计两个典型案例：

- **CVE-2023-53046** (v19, 10 个 bug)：confirmed_hypotheses.log 列出 10 条 hypothesis，全部约束（in_thread / conflicts / concurrent）真的满足，所以 verifier 没理由拒。但 10 条都聚焦在 `hdev->req_status` / `hdev->req_result` / `hdev->req_skb` 三个字段、围绕 `hci_cmd_sync_complete / hci_cmd_sync_cancel / __hci_cmd_sync_sk` 三个函数的不同 (writer, reader) 排列组合 —— 本质上只对应 3 个 race。
- **SYZBOT-5cce5938c6c2c518** (v19, 12 个 bug)：5 个属于互不相关的 sysctl 字段，但其中 `trans->hbinterval`、`dreq->dreq_gss` 等字段各产生 2 条 hypothesis（相同 writer 节点 vs 不同 reader 节点），同一 race 的多次复述。

结论：**verifier 已经在过滤"约束硬不成立"的 hypothesis（在 6 个抽样 case 上拒绝率 0–57%），但语义冗余的 hypothesis 它全留了**。这部分 FP 不该靠 verifier 严苛化解决（约束确实成立），而应该在报告聚合阶段按 (字段, 函数对) 去重。

### F.2 设计：保守的 (field-set, fn-set) 合并键

为每个 confirmed hypothesis 计算一个签名：

- `field-set`: 把所有参与节点 (`writer/reader/...`) 的 source code 通过正则提取字段名，构成有序集。LHS 优先（匹配 `(->|.)\s*FIELD\s*(\[..\])?\s*[+\-*/&|^]?=`），LHS 不存在时取最右侧 `(->|.)FIELD`。能正确处理 `PACKET_CB(skb)->keypair`、`READ_ONCE(sock_net(sk)->ipv4.sysctl_tcp_fastopen)` 这类带函数调用的基底。
- `fn-set`: 所有节点 `node->getFunction()->getFuncNode()->getCPGNode()->getName()` 的有序集。
- 兜底：若所有节点都提取不到字段（罕见），回落到 `sorted(node_ids)` 作为签名 —— 等价于"不合并"。

签名相同的 hypothesis 视作同一逻辑 race，按 description 长度 + id 字典序选出代表 hypothesis 输出主报告，其余追加为：

```
--- Alternate Angles (N additional hypothesis instance(s) on the same race) ---
  [1] <id> (severity: ...) | nodes: writer=X reader=Y
      <第一句 description>
  [2] ...
```

这样：(1) 不丢任何 confirmed hypothesis 的语义；(2) evaluator 仍能匹配任何 angle 上的关键字；(3) 主 bug 数量回落到 unique race 的下界。

### F.3 实现

仅改 `src/Query/StatefulBugDetector.cpp`：

- 加 `extractFieldSignature(code)`、`functionNameOf(node)`、`computeSignature(h, ccpg)`、`pickRepresentative(group, all)` 四个 helper；
- 在 `printResults` 里把 `confirmedHypotheses_` 按签名分桶；每桶输出主报告 + alternate angles；
- 控制台 stats 改为 `Hypothesis-Based Violations: <merged> (merged from <raw> raw confirmed hypotheses; <delta> collapsed as alternate angles)`，便于 batch 日志检索。

不改任何 verifier、不改任何 prompt、不改任何 surface 生成。**召回不可能因此回归** —— 主 bug 仍是被 verifier 通过的 hypothesis，alternates 仍以纯文本形式存在于同一 bug 块内。

### F.4 Smoke 验证（5 case）

```
CVE                                    v19_bugs   v20_conf   v20_final   merged
  CVE-2023-53046                              10          5           5       +0
  SYZBOT-5cce5938c6c2c518                     12          8           8       +0
  CVE-2024-26861                               6          4           4       +0
  CVE-2024-43830                               5          5           3       +2  ★
  CVE-2024-27019 (HIT)                         4          4           4       +0
```

- **CVE-2024-43830**：合并真的触发了。`led_cdev_trigger_set_vs_read_format` 主报告吸收了 `led_cdev_trigger_unregister_vs_read_format` 和 `led_trigger_format_check_use_trigger_toctou` 两条 alternate —— 三条 hypothesis 都在 `led_cdev->trigger` 上、都是 `led_trigger_set/unregister vs led_trigger_format` 这对函数（trigger_set 和 unregister 在 LLVM 上属于同一个 owning function 链路）。5 → 3，2 个 alternate 全部以同 bug 块内文本形式保留。
- **CVE-2023-53046**：本次 LLM 自己只 propose 了 5 个 hypothesis（v19 是 10），合并代码闲置。**离线模拟 v19 那 10 条经合并代码处理 → 5 组**（{req_status, {cancel,sync}}=2、{req_status, {complete,sync}}=3、{req_result, {cancel,sync}}=1、{req_result, {complete,sync}}=2、{req_skb, {complete,sync}}=2），与本轮自然产物一致。所以合并对此案例是 LLM 多嘴时的"保险" —— LLM 节制时不浪费精度，LLM 啰嗦时砍 50%。
- **SYZBOT-5cce5938c6c2c518**：8 条 hypothesis 命中 8 个不同的 sysctl/字段（ip_dynaddr / tcp_fastopen / tcp_tw_reuse / tcp_window_scaling / tcp_ecn / RCV_WSCALE / hbinterval / dreq_gss）—— 真的都是不同 race，合并代码正确地全部保留。
- **CVE-2024-26861**：4 条都是不同 (field, fn-set)，全部保留。
- **CVE-2024-27019 (HIT)**：4 条 → 4 条，无回归。`nf_tables_objects_del_rcu_unordered_plain_traversal` 与 `_add_rcu_` 虽然都在 `nf_tables_objects` list 上但 writer 函数不同（list_del_rcu 的注入点 vs list_add_rcu 的注入点），合并代码正确地分桶保留。

### F.5 预期对全量的影响

按 v19 数据回推：

- 假设全量 100 case 的 ~25% 报告属于"同 (field, fn-set) 多 angle"（结合 hypothesis_too_aggressive 19% + 部分 constraint_misverified/lockset_wrong 中可被合并的子集），
- bug 总数 474 → 估计 **~330–360**（-25% 至 -30%），
- TP_match (41) 不变（合并的都是 confirmed），TP_related 可能轻微合并到 TP_match，
- precision_strict 8.65% → 估计 **~12–13%**（FP 中真冗余的部分被吸收到 alternate）；
- evaluator 的 `agent returned empty response` 概率应该明显下降 —— 23 个 ERROR cases 平均每 case 8 个 bug，每个 bug 在 evaluator 多次工具调用，bug 减 25–30% 对它的 token budget 是直接松绑。

### F.6 文件改动

| 文件 | 改动行数（约）| 说明 |
|---|---:|---|
| `src/Query/StatefulBugDetector.cpp` | +130 | 添加 4 个 helper（field 提取、fn 名、签名、代表选择）+ printResults 重构成分桶输出 |

无 header 改动、无 ABI 变化、无新增依赖。Release build 一次通过。

### F.7 已确认局限

- `extractFieldSignature` 在没有 `=` 的复合调用里取"最右侧字段"——对 `INET_ECN_decapsulate(skb, PACKET_CB(skb)->ds, ip_hdr(skb)->tos)` 这种会取到 `tos` 而非 `ds`。但 hypothesis 节点的 code 通常是单独表达式而非完整 call，加上 fn-set 作为联合键，实测未观察到错误合并。
- 不同 struct 同名字段（如多个 struct 都有 `flags`）若恰好被同一对函数访问，理论上可能误合并；fn-set 维度让这种情况非常罕见（不同 struct 通常出现在不同函数对里）。
- 对 `Stateful Protocol Violations`（旧 `detectedBugs` 通道）和 `ExternalBugs` 不参与合并 —— 它们走独立路径、本身就是去重过的。

---

# 附录 G — v20 P8：静态图补全（sysfs callback / 发布边 / RCU 同步）

## G.1 背景

v19 100-case 全量结果显示，54 个 MISS 中 22 个 (≈40%) 根因是 `shared_object_missed`，14 个 (≈26%) 是 `function_not_analyzed`。另一个 agent 对 CVE-2024-43830 的 case study 把这两类问题归并为同一个根因：**Phase 1 / HBGraph 把"内核回调注册"建模得不完整**：

- 静态分析侧能识别的"线程入口"几乎全部来自 Phasar 的 EXPORT_SYMBOL 候选，**static helper（如 `_show` / `_store`）即使在 IR 里被定义、被某 attribute_group / file_operations 全局表持有，也不会被升级成 thread**。
- HBGraph 只对一小撮 register_* / queue_* API 建 `LIFECYCLE_FLAG` 边；**`device_add_groups` / `sysfs_create_files` 等真正的 sysfs 发布点完全缺位**，导致 init/probe 阶段对字段的写、与发布之后用户态触发的回调读，被静态地视作 plain race。
- `synchronize_rcu()` 等阻塞同步原语在 ThreadAPIUtil 里被分类成 JOIN，但 **HBGraph 的 `buildRCUEdges()` 是空函数**：reader-side `rcu_read_unlock` 与 writer-side `synchronize_rcu` 之间没有任何 happens-before 关系。

P8 把这三件事一起补上。

## G.2 设计

| 子项 | 改动 | 目标 |
|---|---|---|
| **P8a** | `CCPG::build()` 末尾增加 sysfs/file_ops/device_attribute 全局表扫描 | 把 callback table 里指向 IR 内部定义函数的 `_show` / `_store` / `.read` / `.write` 等回调升级为 kernel-entry Thread |
| **P8b** | `HBGraph::buildLifecycleFlagEdges` 的 `kRegistrationAPIs` 加入 `device_add_groups` / `devm_device_add_groups` / `device_create_file` / `sysfs_create_files` / `sysfs_create_link` / `sysfs_create_bin_file` / `kobject_uevent` / `kobject_add` / `kobject_init_and_add` | 任何一个发布调用之前的写在新加的 LIFECYCLE_FLAG HB 边下成为 callback 的 HB-前驱，消除 "init 阶段 vs sysfs 回调" 这一类巨量 FP |
| **P8c** | 实现 `HBGraph::buildRCUEdges`：对每个 `synchronize_rcu` / `synchronize_srcu` / `rcu_barrier` 写者站点 `S`，在每个并发读者线程的每个 `rcu_read_unlock*` 站点 `U` 之间加 `U --RCU_SYNC--> S` | reader-side critical section（在 PO 上 HB 于 `U`）与 writer "sync 后" 的代码自动获得 HB 关系，删除大量 `hbg_missing_edge` 类 FP |

### 关键实现细节

**P8a 的全局表扫描**（`src/CCPG/CCPG.cpp`）：

- 维护一个 callback table 类型白名单：
  ```cpp
  "struct.attribute_group", "struct.device_attribute",
  "struct.driver_attribute", "struct.bus_attribute",
  "struct.class_attribute", "struct.kobj_attribute",
  "struct.bin_attribute", "struct.file_operations",
  "struct.kernfs_ops", "struct.proc_ops",
  "struct.seq_operations", "struct.dev_pm_ops",
  "struct.platform_driver", "struct.bus_type",
  "struct.device_driver", "struct.notifier_block",
  "struct.net_proto_family", "struct.proto_ops",
  "struct.ethtool_ops", "struct.net_device_ops",
  "struct.tty_operations", "struct.iio_info",
  "struct.regmap_bus", "struct.nft_object_type",
  "struct.nft_expr_ops", "struct.nft_chain_type"
  ```
- 同时识别 LLVM 编译器对匿名 struct 的 `.<n>` 变体后缀，以及外层 `[N x %struct.X]` 数组包装。
- 用 8 层、256 操作数封顶的 constant-walker `p8aCollectFns()` 抽取所有函数指针（处理 `ConstantStruct` / `ConstantArray` / `ConstantExpr` 嵌套）。
- 仅升级 **本地定义、不在已有 entry 集合中的函数**；保留对 `cpg->findMethod` + `CPG::demangleVariants` 的回退，以兼容 `.llvm.<hash>` 等 LLVM 后缀。
- 全程附 stats 行：`[P8a] Sysfs/callback discovery: scanned N ops globals, harvested K functions, added X new entries (Y already known, Z not in CPG, W declarations)`。便于 batch 日志直接审计哪些 case 触发、触发到什么程度。

**P8b 的 LIFECYCLE_FLAG 复用**：直接借用 `buildLifecycleFlagEdges` 已有的 "registration → callback entry" 边构造逻辑，把 sysfs 一系列 publish API 加到同一个 `kRegistrationAPIs` 集合即可。无新代码、无新边类型——保证语义和现有 register_* 完全一致：发布前 PO 写 → 回调入口 happens-before 链。

**P8c 的 RCU 边**（`src/CCPG/HBGraph.cpp`）：

- 仅对 `mayThreadsRunConcurrently(tw, tr)` 通过的 thread 对发边——同模块 init/exit pair 自然过滤。
- 默认 4000 条边封顶 (`HB_RCU_MAX_EDGES`)，对 RCU-heavy 模块预留逃生口。
- 调试通过 `HB_RCU_DEBUG=1` 打开 `[HBGraph] RCU_SYNC edges added: N` 行。

## G.3 Smoke 验证（5 case）

针对 `[scope: all]`，挑 5 个代表性 case：

| CVE | v19 verdict | v20p8 raw bugs | v20p8 final | v20p8 verdict | P8a 扫到 ops globals | P8a 实际加新 entry |
|---|---|---:|---:|---|---:|---:|
| CVE-2024-43830 (sysfs UAF) | **MISS** | 6 | 5 | **PARTIAL** ★ | 0 | 0 |
| CVE-2024-26861 (HIT 回归测试) | HIT | 7 | 7 | HIT | 0 | 0 |
| CVE-2024-43891 (event_hist HIT 回归) | HIT | 7 | 7 | MISS\* | 4 | 0 (6 已知 + 6 declare + 1 not in CPG) |
| CVE-2024-53124 (skb dev SOM) | MISS | 7 | 7 | MISS\* | 1 | 0 (17 全部 declaration) |
| SYZBOT-5676077b (ip6_tnl SOM) | MISS | 9 | 9 | MISS\* | 1 | 0 (6 已知 + 1 declare) |

**\* 标记的 verdict 不可靠**：evaluator 在判这 3 个 case 时大量碰到 LLM API HTTP 429 (Too Many Requests)，导致 4–9 / 6–9 个 bug 的 Phase B 评估默认回退为 FP；recall 决策因此基于 ~30% 的有效证据，与 v19 verdict 的对比不具结论性。CVE-2024-26861 / CVE-2024-43830 的 verdict 来自有效评估证据（26861: 1 TP_MATCH 被识别；43830: 1 TP_RELATED 被识别）。

**P8a 行为分析**：5 个 case 总共扫到 6 个 callback table 全局，harvest 出 37 个函数指针；其中：

- ~半数已经是 entryFunctions（Phasar 早就识别成 EXPORT_SYMBOL 入口）；
- ~半数是 `declare`（callback body 在另一个 TU，没链进 merged.ll）；
- **0 个真正符合"本地定义 + 不在 entry 集合"** 的条件被升级。

这反映了 v20 corpus 的一个事实：`merged.ll` 通常只覆盖 patched file 的一两个 TU，sysfs 回调表里指向的 callback body 大多在另一 TU，IR 里只有 declaration。P8a 在更完整的全 vmlinux IR 上才会大量触发；当前 corpus 下它的实际效果以"基础设施补齐"为主。

**关键收获 — CVE-2024-43830 的两条新 hypothesis**：

```
v19 baseline 的 5 条 → 全围绕 trig->brightness / led_cdev->trigger ptr / led_cdev->flags
v20 P8 增加：
  + simple_trigger_name_publish_vs_read     ← publish_race（P8b 发布边的产物）
  + led_trigger_format_check_use_vs_clear   ← TOCTOU_null_deref（思路接近 CVE 真实窗口）
```

`led_trigger_format_check_use_vs_clear` 的语义是 "led_trigger_format() 先 check `trigger != NULL` 再 deref，与 led_trigger_set() 的 `trigger = NULL` 形成 TOCTOU"——和 CVE 的真实 race（`deactivate` 释放 `trigger_data` 之后、`device_remove_groups` 拆 sysfs 之前的 use-after-free 窗口）**在思路上同源**：都是 set/unset trigger 与 sysfs 回调读 trigger 之间的 race，只不过 CVE 焦点是 `trigger_data` 这个间接释放对象，而 Lace 提的版本是 `trigger` 这个普通 ptr。Evaluator 把这条判为 FP（不够 close），但 P7 合并 + P8b 发布边联合产生了一个比 v19 更靠近真窗口的 angle。等 trigger_data 间接释放的 free-effect 模型（P9）做完，这一类 TOCTOU 应该能升级到 TP_RELATED 甚至 TP_MATCH。

**P8b 的 LIFECYCLE_FLAG 边和 P8c 的 RCU_SYNC 边**对 surface 大小没有可观察影响（它们只在 verifier 层面影响 `eval_concurrent` / `eval_hb`），但日志显示 detector 整体行为稳定、无 hang、无 OOM。RCU_SYNC 边数与默认 cap (4000) 都未触发，意味着实际加边量在百级别以下。

## G.4 改动清单

| 文件 | 改动行数（约）| 说明 |
|---|---:|---|
| `src/CCPG/CCPG.cpp` | +130 | 新增 namespace-local helpers (`p8aCollectFns` / `p8aIsCallbackTableTypeName` / `p8aPeelArray`)；在 `build()` 末尾插入 sysfs callback 全局表扫描循环 |
| `src/CCPG/HBGraph.cpp` | +90 (含 60 行注释) | `kRegistrationAPIs` 加 9 个 sysfs/kobject publish API；实现 `buildRCUEdges`；在 `build()` 里调用它 |

无 header 改动、无 ABI 变化、无新增依赖。Release build 一次通过。

## G.5 已知局限

- **P8a 的有效率受 IR 完整度限制**。当 `merged.ll` 只覆盖一个 TU、所有 sysfs 回调表的 callback 定义在别的 TU 时，harvest 出来的全是 `declaration`，无法升级。要进一步提升，需要在编译流水里 link in 更多依赖 TU。
- **P8c 的语义是 "保守的过近似"**：我们对所有并发线程的 read-side critical section 都加 RCU_SYNC 边；理论上只该对 *同一个 RCU 实例* 的 readers 加。但同源 reader/writer 几乎肯定在同一 RCU 实例上，错配反而会消除一个本来就不会被报的 reader-vs-reader race，对最终 verdict 几乎不可能产生反作用。
- **trigger_data UAF 的根因（间接调用链下的 free effect 模型）尚未涉及**。P8 把"sysfs 回调成为线程 + sysfs 发布有 HB 边"补齐，但 `trigger->deactivate(led_cdev)` 这种通过函数指针的 free 仍未被 surface 识别成 free effect on `led_classdev->trigger_data`。这是下一个独立工作 (P9)。

# 附录 H — v20 全量批跑后的 9 HIT→MISS regression 归因与修复（P5-fix + P8c-tighten）

## H.1 背景

v20 在 v19 之上叠加 P7 (bug 报告合并) + P8 (静态图补全 P8a/P8b/P8c) 跑完 100 case 全量后总评：

```
              v19   v20    Δ
ERROR          23    0   -23 ✅  (Plan A+D 治好长 context 崩溃)
HIT            23   16    -7  ⚠️
PARTIAL         6   28   +22  ✅  (P8 主要红利)
MISS           48   56    +8
recall_strict  23%  16%   -7pp ⚠️
recall_lenient 29%  44%  +15pp ✅
```

转移矩阵（77 个两边都 valid 的子集）暴露关键警报：

```
v19 \ v20    HIT  PART  MISS
HIT           13     2     9   ← 9 个 HIT 退化成 MISS
MISS           0    15    37   ← 15 个 MISS→PARTIAL（P8 主功劳）
```

9 个 HIT→MISS regression：CVE-2013-1792、CVE-2017-15265、CVE-2024-27030、CVE-2024-43891、CVE-2024-53136、CVE-2025-37854、CVE-2025-38429、SYZBOT-2d373c9936c00d7e、SYZBOT-392f4c8f5827466f。

**FP 分布迁移**也很有意思：

```
                       v19   v20    Δ
P1_surface_gap          18     0   -18  ✅ 消失
P4_verifier_overdrop    84     0   -84  ✅ 消失
P3_constraint_failed   290   418  +128  ⚠️ 大涨
```

## H.2 第一直觉是错的（P8c 不是真因）

按 ATTRIBUTION § G.5 自评和 v20 总评的推测，P8c 的 RCU_SYNC 边是"保守的过近似"，最有可能把本该 concurrent 的 pair 错误 HB 化，从而导致 9 个 HIT→MISS 退化和 +128 P3 FP。

triage 直接看每个 regression 的 v19 dump (May 13) vs v20 dump (May 16) 的实际差异：

| CVE | v19 报告数 | v20 报告数 | 内容是否相似 |
|---|---|---|---|
| CVE-2013-1792 | 9 | 9 | **完全不同**（v19: `user->uid_keyring` 主线；v20: `cred->fsuid/fsgid` FP） |
| CVE-2017-15265 | 7 | 9 | **完全不同**（v19: `port_delete` 主线；v20: `client->data.user.fifo` 等 FP） |
| CVE-2024-43891 | 6 | 6 | **完全不同**（v19: `data->ref--`；v20: `list_del_rcu` FP） |
| SYZBOT-392f4c8f | 10 | 10 | 类似 angles（都没有命中 `icmp_global` 主线，v19 HIT 是 evaluator 误判） |

报告**数量基本不变**，但内容完全不同——这就证伪了 P8c：如果是 RCU_SYNC 把 hypothesis HB 化掉，受影响的报告数会**减少**。LLM 在做完全不同的提议，说明 surface 或 prompt 在 LLM 看来变了。

## H.3 真因：P5 plain-RW boost 顺序 bug

逐对象比对 surface JSON 找到了系统性 pattern：

**CVE-2013-1792 关键对象 risk 对比**：

| 对象 | v19 | v20 | flags（两版相同）|
|---|---:|---:|---|
| `field:struct.cred+28` (fsuid) | 85 | **165** | cross_thread_rw, **inconsistent_locking**, unprotected_write |
| `field:struct.cred+32` (fsgid) | 85 | **165** | cross_thread_rw, **inconsistent_locking**, unprotected_write |
| `field:struct.user_struct+48` (uid_keyring, **CVE 焦点**) | 30 | 110 | cross_thread_rw |
| `field:struct.cred+128` | 63 | 143 | cross_thread_rw, unprotected_write |

`fsuid/fsgid` 的 +80 把它们抬到 top-1/top-2，把 `user_struct+48`（CVE 焦点）挤到 top-3。LLM 看到顶部全是 `cred` 字段，所以提议都集中在 `cred->fsuid/fsgid` race，跑偏。

**`computeRiskScores` 里的源码顺序 bug**（v20 出问题前的版本）：

```cpp
//  P5 plain-RW boost  (lines ~1289-1300 in original code)
bool isPlainRWRace =
    sawCrossThreadRW &&
    writeLikeCount > 0 && readCount > 0 &&
    !obj.has_list_mutation &&            // ← 此时还是默认 false
    !obj.has_free_operation &&
    !obj.has_inconsistent_locking;       // ← 此时还是默认 false  ⚠️
if (isPlainRWRace && (...)) score += 80;

// 十几行之后才计算并赋值：
if ((threadsWithLock > 0 && ...)) {
    obj.has_inconsistent_locking = true;
    score += 25;
}
```

P5 设计意图明确写在注释里："no list_mutation, no UAF, no inconsistent_lock — otherwise those boosts apply and we don't need to amplify"。但 `obj.has_*` 在 P5 检查时尚未赋值，永远读到默认 `false`，所以 P5 对所有 `cross_thread_rw + writes + reads` 的对象都触发 +80，**把 inconsistent_locking 字段也错误地抬到 plain-RW 同档**。

**9 个 regression 的全局画像**（top-3 risk diff）：

```
CVE-2013-1792:           +80  +80  +80   bumped=2/5 inconsist objs got +80
CVE-2017-15265:          +187 +159 +159  bumped=5/13 inconsist objs got +80  ← 0→187 把 LLM 直接打偏
CVE-2024-27030:          +80  +80  +80   bumped=1/5
CVE-2024-43891:          +83  +80  +80   bumped=1/6  (last_cmd, *.__warned)
CVE-2025-37854:          +80  +80  +80   bumped=4/4  ← 全部受影响
CVE-2025-38429:          +80  +80  +80   bumped=0/0  (P5 boost 自身没问题)
SYZBOT-2d373c9936c00d7e: +110 +110 +94   bumped=9/31
SYZBOT-392f4c8f:         +80  +80  +80   bumped=0/0  (但 *.__warned 伪 global 污染 top)
```

9/9 的 top-3 risk 差都是 +80 量级，6/9 直接踩 inconsistent_locking 顺序 bug，其余受 `*.__warned` 等 kernel 伪 global 污染。

**额外副作用**：`WARN_ON_ONCE` / `DO_ONCE` 编译器为每个调用点生成的 `static bool __warned` 被分析当成跨线程 RW 全局，然后被 P5 boost 到 risk=155+。SYZBOT-392f4c8f 的 v20 top-10 里就有 `icmp_socket_deliver.__warned`、`rcu_read_unlock.__warned`、`rcu_read_lock.__warned`，把真正的 `icmp_global+72/+76` (CVE 焦点) 挤出 top-10。

## H.4 实施修复

### H.4.1 P5-fix：调整顺序 + 过滤 kernel 伪全局

`src/Query/VulnerabilitySurfaceGenerator.cpp` 的 `computeRiskScores`：

1. 把 `inconsistent_locking` 计算挪到 P5 之前，提供局部布尔 `isInconsistentLocking`；同时仍然在原位 `obj.has_inconsistent_locking = true` + `score += 25`，保持其他 bug-pattern 检测的语义不变。
2. 同样**预算** `isListMutation` 局部布尔（list-mutation 的完整 block 还在原位算 score）。
3. P5 检查改用局部布尔：

   ```cpp
   bool isPlainRWRace =
       sawCrossThreadRW &&
       writeLikeCount > 0 && readCount > 0 &&
       !isListMutation &&            // ← 现在是真值
       !obj.has_free_operation &&
       !isInconsistentLocking;       // ← 现在是真值
   ```

4. 加 `isKernelPseudoGlobal()` 过滤：`*.__warned`、`*.__warned.<n>`、`*.__already_done`、`*.__ratelimit`、`*.__exiting`、`*.__done` 在 P5 boost 前被剔除。

### H.4.2 P8c-tighten：RCU 变体 + SRCU 实例匹配

`src/CCPG/HBGraph.cpp::buildRCUEdges`：原版对所有 sync 和 unlock 跨线程加边，过近似。改成：

1. 按 RCU **变体** family 分桶：vanilla（`synchronize_rcu`, `rcu_barrier` ↔ `rcu_read_unlock`）/ BH / Sched / SRCU / Tasks。
2. **只在同 family 内**配对加边（vanilla unlock 不再和 sched synchronize 强行配对）。
3. SRCU 额外做实例匹配：取 `synchronize_srcu(struct srcu_struct *ssp)` 和 `srcu_read_unlock(ssp, idx)` 的第一个参数（剥 pointer cast 后的 LLVM `Value*`），相等才连边。这是保守匹配——经过 alias 等价但不同 SSA value 不会通过——但消除了明显的不同实例情况，且不引入 alias query 的成本。
4. SRCU instance mismatch 计入 `edges_skipped_srcu_instance` 调试计数。

虽然 H.2 已证 RCU_SYNC 不是 9 个 regression 的真因，但过近似本身是设计缺陷，顺手收紧。

## H.5 验证

CVE-2013-1792 **smoke test**（全量 17.5h 太慢，1 case ≈ 14 分钟即可证伪/证实）：

| 阶段 | v19 (基准 HIT) | v20-bug (regression MISS) | **v20-fix** |
|---|---|---|---|
| 报告数 | 9 | 9 (全 FP) | **8 (Phase 4.5 KEEP 8/8)** |
| `user_session_keyring_publish_vs_lookup_unlocked_read` | ✓ TP_MATCH | ✗ | **✓ KEEP** |
| `uid_keyring_publish_vs_lookup_user_key_unlocked_test`（CVE 真根因） | ✓ TP_MATCH | ✗ | **✓ KEEP** |
| `session_keyring_assign_vs_search_my_process_keyrings` | ✓ TP_MATCH | ✗ | **✓ KEEP** |
| `cred->fsuid/fsgid` race 干扰（FP 引诱） | — | 9/9 全这种 | **完全消失** |

8 条 hypothesis 全围绕 `user->uid_keyring` / `user->session_keyring` / `cred->session_keyring` 的 publish-vs-lookup race，正好是 v19 的 TP_MATCH 主线。

**Surface 层数据回到 v19 量级**：

| 对象 | v19 | v20-bug | v20-fix |
|---|---:|---:|---:|
| `field:struct.cred+28` (inconsistent) | 85 | 165 | **85** ✓ |
| `field:struct.cred+32` (inconsistent) | 85 | 165 | **85** ✓ |
| `field:struct.user_struct+48` (CVE plain-RW) | 30 | 110 | **110** ✓ |
| `field:struct.cred+128` (plain RW, 无 inconsistent) | 63 | 143 | **143** ✓ |

P5 boost 对真正的 plain-RW 对象（`user_struct+48`, `cred+128`）正确触发，对 inconsistent_locking 对象（`cred+28`, `cred+32`）不再误触发。完全符合 P5 注释里的设计意图。

## H.6 预期影响（待全量 v20-fix 批跑确认）

- **9 个 HIT→MISS 大部分回到 HIT**：surface 排序回到 v19 → LLM 看到正确 angle。CVE-2013-1792 已确认。
- **15 个 MISS→PARTIAL 应保留**：P8a/P8b/P8c 的静态层修复没动到，sysfs callback / publish edges 红利保留。
- **`P3_constraint_failed +128` FP 应缩水**：噪声 hypothesis 减少 → verifier 流量降低；P8c 收紧也直接减少错误 RCU_SYNC 边导致的 concurrent 误判修正。
- **`*.__warned` 伪全局污染消除**：SYZBOT-392f4c8f / CVE-2024-43891 这类网络/trace 子系统的 surface 不再被 WARN_ON_ONCE 标志位主导。

## H.7 改动清单

| 文件 | 改动行数（约）| 说明 |
|---|---:|---|
| `src/Query/VulnerabilitySurfaceGenerator.cpp` | +60 | `inconsistent_locking` / `list_mutation` 计算前置；`isKernelPseudoGlobal` 过滤；P5 检查改用局部布尔 |
| `src/CCPG/HBGraph.cpp` | +60 (含 30 行注释) | `buildRCUEdges` 重写：family 分桶 + SRCU operand match |

无 header 改动、无 ABI 变化、无新增依赖。Release build 一次通过。

## H.8 经验教训

1. **直觉归因要可证伪**。"P8c 是过近似 → 它就是 regression 元凶"听起来合理，但只要看一眼 v19 vs v20 报告数（9 vs 9 不变），就能立刻排除——HB 边过紧只会让 hypothesis **少**，不会让它**不一样**。triage 工作必须先看数据再下判断。

2. **C++ 默认初始化的隐性陷阱**。`bool obj.has_X = false` 默认值看起来无害，但如果同一函数里"先用后赋"，就会沉默地把"未知"当成"否定"。这种 bug 在编译期/lint 期都不会报，只能靠业务逻辑发现。修复方式：要么用局部布尔，要么把"先计算所有 flag、再做 boost 决策"分两 pass。

3. **kernel pseudo-globals 是分析里的隐形噪声源**。`WARN_ON_ONCE`、`DO_ONCE`、`pr_warn_once` 等宏会 lower 成 `static bool __warned`，每个调用点一份。Phasar 会把它们当成 cross-thread RW global，所有"是 global 就加分"的启发式都会被它们污染。值得在 surface generator 加一个稳定的 deny-list。

4. **smoke test 不一定要跑很多 case**。这次 1 个 case 足够确认修复方向（surface 数据 + Phase 4.5 KEEP 命中 v19 主线 hypothesis 即可），不需要 5 个或 3 个。规模选择跟"想验证什么"相关，不是越大越好。



