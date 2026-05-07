# 实验交接说明 (HANDOFF)

> **读者：在新机器（字节开发机）上接手这个项目的下一个 LLM / 工程师。**
> 这个文档让你在 5 分钟内 grok 全貌、避开所有已踩过的坑、并精确知道下一步要做什么。
> 详尽的历史背景、五阶段优化记录、模型架构在 [`README.md`](./README.md) 和
> [`M7_IMPLEMENTATION_PLAN.md`](./M7_IMPLEMENTATION_PLAN.md) 里，本文档不重复。

---

## 0. TL;DR

- **是什么**：Lace = LLM-Enhanced Concurrency Vulnerability Detector，针对 **Linux 内核真实 CVE** 的并发 bug 检测系统。把传统静态分析（CPG + Phasar 指针分析 + Happens-Before 图）和 LLM 假说生成 / 验证闭环结合。
- **你的目标**（业务层面）：**提高真实内核 CVE 上的检出召回率与精确率**——不要被 milestone 名字（M7）牵着走。M7 只是手段。
- **判定标准**（务必内化）：LLM-judge 看 **root cause 是否匹配 patch**，而不是 `bug_category` 字面相等。如果一个 UAF 被报成 `data_race` 但描述对了同一个字段、同一组线程、同一个 race window，那就是 HIT。**不要为了字面命中而往 prompt 里堆 hard template 规则**——那会让模型扭曲描述去贴标签，反而降召回。

---

## 1. 当前实验进度（继承自上一台机器）

跑过 50 个 CVE 中的 20 个，hypothesis 累计 ~156 条；剩 30 个未跑（其中 3 个因 jeniya 配额耗尽 fail，27 个根本没跑到）。

**之所以迁移到字节开发机**：上一台机器只能走 jeniya 网关（境外代理），(a) 配额按量计费已耗尽，(b) clash-yunlong 代理频繁挂掉，运维成本高。字节内网网关 `search.bytedance.net` 用 ByteDance AK 鉴权，配额和稳定性都更好。

**已成功的 18 个 FOUND CVE 与 hypothesis 数**（数据保留在 git 历史 commit `4e043e0` 之前的 batch 日志中，但**实验产物已 .gitignore，不会跟着代码迁移**——新机器上需要从头跑）：

```
CVE-2013-1792:19  CVE-2015-7550:6   CVE-2016-9806:21  CVE-2017-15265:8
CVE-2017-6346:14  CVE-2024-26974:7  CVE-2024-27019:2  CVE-2024-27030:7
CVE-2024-27404:15 CVE-2024-35898:2  CVE-2024-35977:2  CVE-2024-35986:7
CVE-2024-35999:9  CVE-2024-36938:12 CVE-2024-38596:9  CVE-2024-39503:7
CVE-2024-40953:9
```

外加 2 个 CLEAN：CVE-2016-7911、CVE-2024-39508（有 ground truth 但 detector 没出假说，可能是入口点解析问题）。

---

## 2. Critical Pitfalls（避坑指南，按真实踩坑顺序）

下面每一条都对应过几小时到一整天的浪费。**先看完再开跑**。

### 2.1 `$LLM_API_URL` 这个环境变量代码不读

`src/llm_main.cpp:401-404` 的 `base_url` 只读 `--llm-url` CLI 参数或 config 文件 `base_url` 字段，**不读** `$LLM_API_URL` env。如果你 `export LLM_API_URL=...` 然后没传 `--llm-url`，请求会发到 hardcoded 默认 `https://jeniya.cn/v1/chat/completions`——上一次就是这个 bug 让一整批跑全错（火山方舟 token 发到 jeniya 端点 → 全 47 个 API_ERROR）。

`scripts/batch_detect.sh` 已扩展支持 `--base-url <url>` 和 `LLM_BASE_URL` env var（通过 sh 脚本透传到 detector 时仍是 `--llm-url`）。**在字节开发机上务必显式传 base_url，详见 §4.5**。

### 2.2 `batch_detect.sh` 默认 model 历史上是 `gpt-5.4`（虚构模型名）

已修成 `claude-sonnet-4-6`。如果你想换模型，从命令行 `--model` 显式传，不要依赖默认。

### 2.3 LLM 复用 `tool_use_id` → Anthropic 拒绝整条 conversation

`src/LLMUtil/LLMClient.cpp` 已加 `sanitize_messages_for_tool_calls()`，会把所有 `tool_call.id` 重写成 `lacecall_<N>` 全局唯一格式，并 FIFO 配对 TOOL.tool_call_id。这是必须的 fix——上一台机器有 5+ 个 CVE 因为 LLM 在重复调 `propose_hypothesis` 时复用了同一个 `tooluse_xxx` ID 而被 jeniya/Anthropic 报错 `unexpected tool_use_id found in tool_result blocks`。**不要回退这个改动。**

### 2.4 `Conversation::prune_history()` 在长对话时可能产出孤儿 tool_use / tool_result

同一份 sanitization 也覆盖了这个孤儿场景：`pending_renames` 没消费完时会注入 placeholder TOOL response；orphan TOOL（没有对应 ASSISTANT.tool_calls）会被丢弃。也不要回退。

### 2.5 don't 为了 metric 在 prompt 里加 hard template 规则

`src/LLMUtil/DetectorAgent.cpp::build_system_prompt()` 现在用 **soft** 指引：「三个 template 是工具箱，bug_category 是 free-form，judge 评 root cause」。曾经短暂出现过一版 hard 规则（`Template-Selection Rule (HARD)` + `Anti-pattern to avoid`），让 LLM 担心被 judge 降级反而扭曲描述去贴 template，**已撤回**。如果你要重新调 prompt，请遵守这条原则。

### 2.6 `cmake --build` 不一定 rebuild 你刚改的 .cpp

`scripts/CMakeLists.txt` 用 `file(GLOB_RECURSE)` 收集源文件，新加 `.cpp` 必须 `cmake .` 重 configure。修改既有 `.cpp` 后如果 cmake 没检测到（比如 mtime 比 .o 旧），用 `touch <file.cpp>` 强制 + `cmake --build . -j$(nproc)`。**编译完务必 `nm Release-build/llm_detector | grep <你的新符号>` 确认 binary 真的链进去了**。上一台机器有过一次「以为修了 bug 实际没编进去」浪费几小时。

### 2.7 `batch_detect.sh` SKIP 逻辑会跳过 API_ERROR 的 log

判断条件是「日志包含 `hypotheses confirmed` 或 `Bug_Detection.*COMPLETED`」，**API_ERROR 的 log 也含后者**（detector 在 LLM 失败后还跑了 Phase 4）。重跑前要 `rm -f` 清掉。已在 §5.3 的脚本里处理。

### 2.8 git 2.25.1 + 某个 env var → `commit` 报 `unknown option 'trailer'`

发现于上一台机器，根因没定位但确认在 **clean env** 下能跑：

```bash
env -i HOME=$HOME PATH=$PATH bash -c "cd /home/LLM4Con && git commit -F /tmp/msg.txt"
env -i HOME=$HOME PATH=$PATH bash -c "cd /home/LLM4Con && git push origin phasar"
```

新机器上如果你 `git commit` 直接报这个错，照上面办法绕。（更优解：升级 git ≥ 2.32 或者找出注入 `--trailer` 的工具——可能是 cursor 自身的 hook、husky、或某个 IDE 集成。）

### 2.9 `kernel_experiment/README.md` 历史 commit 里有 hard-coded jeniya key

commit `0c71ef7` 老版本里 `src/llm_comparison.cpp:277` 有 hard-coded key `sk-Y5PXh...QafbVRoK`。**已在 commit `4e043e0` 改成 env-driven**（`LLM_EVAL_API_KEY` + `LLM_EVAL_BASE_URL`），但 GitHub 历史还在。**用户应该到 jeniya 后台撤销该 key**——如果还没办，提醒一次。

### 2.10 syscall 名称归一化没做 → 部分 CVE 入口点缺失

`SyS_*` / `__do_sys_*` / `__se_sys_*` / `__arm64_sys_*` 是 `SYSCALL_DEFINEN` 宏展开后的产物，IR debug 里是这些名字，但 Joern CPG 里看到的是宏原名（如 `add_key`）。`CPG::findMethod()` 已加了 demangling，但 syscall 系列还没覆盖。已知影响 CVE-2015-7550, CVE-2016-7911, CVE-2024-41005, CVE-2024-42234, CVE-2024-43830 等的入口点解析（`Total entry functions: 0` → 没有 thread → 没有 hypothesis）。**这是 §7 待办里的高 ROI 项**。

---

## 3. 项目核心组件速览（5 句话）

```
.ll (LLVM bitcode)  ─┐
src/ (C source)     ─┤─→ CPG (Joern) ─┐
                     │                  ├─→ CCPG ─→ HBGraph ─┐
                     └─→ Phasar pointer analysis ─┘            │
                                                              ↓
                            VulnerabilitySurfaceGenerator (静态风险面)
                                                              ↓
                            DetectorAgent (LLM 单会话多轮，5+3 DSL)
                                                              ↓
                            HypothesisVerifier (谓词验证 conflicts/concurrent/hb/...)
                                                              ↓
                            confirmed_hypotheses.log → evaluate_recall.py (LLM-as-judge)
```

- **DetectorAgent prompt** 在 `src/LLMUtil/DetectorAgent.cpp::build_system_prompt()`，定义 LLM 能用的 5+3 DSL 谓词（5 primitives：`same_location` / `op_kind` / `in_thread` / `reachable` / `hb`，3 sugars：`conflicts` / `concurrent` / `unsafe_atomic_block`）。
- **Verifier 谓词实现**在 `src/Query/HypothesisVerifier.cpp`，每个 `eval_*` 方法对应一个 DSL 谓词。
- **HBGraph 当前 MVP**：只产 `PROGRAM_ORDER` + `CALL_RETURN` 边；`LOCK_RELEASE_ACQUIRE`、`FORK_TO_ENTRY`、`COMPLETION` 等都是 stub。对内核「显式定义多 thread 入口点」场景 OK（`concurrent(a,b)` 通过 `¬hb(a,b) ∧ ¬hb(b,a)` 仍能正确识别），但如果未来发现 FP 高，要去 `src/CCPG/HBGraph.cpp` 把那些 `build*Edges()` 实装。

---

## 4. 在新机器上 bring up

### 4.1 Clone 仓库

```bash
# 你（接手的 LLM/工程师）应该已经 clone 了；如果没有：
git clone -b phasar git@github.com:yxscc/LLM4Con.git /home/LLM4Con
cd /home/LLM4Con
```

注意：默认分支 `master` 是老的；**当前活跃分支是 `phasar`**。

### 4.2 Clone Linux kernel git

`scripts/batch_prepare.sh` 写死 `LINUX_REPO=/home/ConCord/targets/linux.git`。新机器上要么按这个路径放 linux.git，要么改脚本里的常量。完整 clone（≈ 4-5 GB，含全 history，因为要 `git checkout <fix_commit>~1`）：

```bash
mkdir -p /home/ConCord/targets
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git /home/ConCord/targets/linux.git
```

如果字节开发机访问 kernel.org 慢，可以用 mirror：
- `https://github.com/torvalds/linux.git`
- `https://mirrors.tuna.tsinghua.edu.cn/git/linux.git`

### 4.3 系统依赖

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake g++ clang-15 libtinfo5 libz3-dev graphviz \
    libcurl4-openssl-dev libssl-dev \
    libboost-system-dev libboost-thread-dev libcpprest-dev \
    python3 python3-pip
```

**Joern**：装到 `/opt/joern` 并把 `joern-parse`、`joern-export` 加到 PATH，参考 https://joern.io/docs/installing。
**Phasar**：从源码 build，依赖 LLVM-15。详见根目录 [`README.md`](../README.md) §Prerequisites。
**llvm-link / clang-15**：用于 prepare 阶段把 .c 编成 .ll 并合并多文件。

### 4.4 Build LLM4Con

```bash
cd /home/LLM4Con
./build.sh                     # Release，输出到 build/
# 或者：
mkdir -p Release-build && cd Release-build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -j$(nproc)
# 二进制：Release-build/llm_detector  和  Release-build/llm_comparison
```

Smoke test：

```bash
nm Release-build/llm_detector | grep -c sanitize_messages_for_tool_calls
# 应输出 ≥ 1（确认 tool-call sanitization fix 链进去了）
```

### 4.5 配置 LLM endpoint（**字节内网，与上一台机器关键差别**）

字节开发机走内网 endpoint：

```
URL: https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=<AK>
Body: 标准 OpenAI Chat/Responses 协议
鉴权: AK 拼在 URL query string 里，不是 Authorization header
```

**LLMClient 当前发请求的方式**（`src/LLMUtil/LLMClient.cpp:540-549`）：

```c++
// OpenAI path:
"curl -X POST -H \"Authorization: Bearer $AK\" \"$BASE_URL\" -d @body.json"
```

字节网关把 AK 放在 query 里，但**通常也接受额外的 Bearer header**（多重认证一般不冲突）。**先按"AK 同时在 URL 和 Bearer header"试**：

```bash
export LLM_API_KEY="<your_GPT_AK>"
export LLM_BASE_URL="https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=$LLM_API_KEY"

# 预检：
bash scripts/batch_detect.sh --api-key "$LLM_API_KEY" --base-url "$LLM_BASE_URL" --model claude-sonnet-4-6
# 看到 "[Preflight OK]" 就行。如果 Preflight FAIL 报 "duplicated auth"，就需要走 §4.5.1 的方案 B。
```

#### 4.5.1 方案 B：如果字节网关拒绝双重认证

`src/LLMUtil/LLMClient.cpp::chat()` 第 543-549 行的 OpenAI 分支需要改成「不发 Authorization header」（因为 AK 已经在 URL 里）：

```c++
} else { // OPENAI and compatible APIs
    cmd = "curl -sS -k" + timeout_opt +
          " -X POST -H \"Content-Type: application/json\"" +
          // ↓ 字节内网：AK 已在 URL 的 ?ak= 里，不要再加 Bearer header
          " '" + sh_escape_single_quotes(base_url_) + "'" +
          " -d @" + tmp_filename +
          " 2>&1";
}
```

更优雅的做法是加一个 `--auth-style query|bearer` CLI option 让 LLMClient 选择，二者切换。**当前没实装**——如果方案 A 通了就别动。

#### 4.5.2 字节支持的 model 名

按用户给的分析，bytedgpt 走方舟、bytedgpt_responses 走 Responses API。可用模型示例：
- `claude-sonnet-4-6`（继续与上一台机器同 model 保证可比性）
- `gpt-5.3-codex-2026-02-24`
- `gpt-5-2025-08-07`
- `doubao-pro-32k`

**强烈建议沿用 `claude-sonnet-4-6`** 直到把这一轮 50 CVE 跑完，避免引入"换模型"和"换实现"的混淆变量。换模型应该是**单独的对照实验**，不在 M7 验收里做。

---

## 5. 跑实验

### 5.1 Prepare CVE bitcodes（如果还没准备）

```bash
cd /home/LLM4Con
bash scripts/batch_prepare.sh
# ≈ 几小时，单线程串行 checkout + make allyesconfig + emit-llvm
# 输出：kernel_experiment/CVE-*/{*.ll, src/, ground_truth.json}
```

**注意**：`batch_prepare.sh` 已集成 `scripts/patch_expander.py`（M7 Phase F），会根据 patch 自动扩展 compile 文件集到所有 patch-touched 文件，确保多文件 CVE 的完整可见性。

### 5.2 验证 prepare 结果

```bash
ls /home/LLM4Con/kernel_experiment/ | grep ^CVE- | wc -l
# 应该是 50（活跃 CVE）

# 抽查一个：
ls /home/LLM4Con/kernel_experiment/CVE-2024-43891/
# 应该有 *.ll、src/、ground_truth.json
```

### 5.3 跑 detect

**重跑前清旧 log**（包括 API_ERROR 的，见 §2.7）：

```bash
cd /home/LLM4Con/kernel_experiment
for d in CVE-*/; do
  if [ -f "$d/detection_hypothesis_batch.log" ]; then
    [ ! -f "$d/detection_hypothesis_batch.prev.log" ] && \
      mv "$d/detection_hypothesis_batch.log" "$d/detection_hypothesis_batch.prev.log" || \
      rm "$d/detection_hypothesis_batch.log"
  fi
done
```

**启动批跑**：

```bash
cd /home/LLM4Con/kernel_experiment
nohup bash /home/LLM4Con/scripts/batch_detect.sh \
    --api-key "$LLM_API_KEY" \
    --base-url "$LLM_BASE_URL" \
    --model claude-sonnet-4-6 \
    > batch_run.m7.log 2>&1 &
echo $! > batch_run.pid
disown
```

预估时长：50 CVE × 平均 8-15 分钟 = **6-12 小时**。`batch_detect.sh` 内置：
- **Preflight**：上来就 curl 一次 ping，端点/key/model 任一不通立即 abort
- **Fail-fast**：连续 3 个 API_ERROR 自动 abort（避免重蹈前一台机器跑 10 小时全错的覆辙）

### 5.4 中途观察

```bash
# 总进度
tail -20 /home/LLM4Con/kernel_experiment/batch_run.m7.log

# 当前活跃 detector
ps -ef | grep llm_detector | grep -v grep

# 抽看某个 CVE 是否有 sanitize 触发
tail -50 /home/LLM4Con/LLM_dump/CVE-*_<latest_timestamp>/llm_conversations.log | grep -E "sanitize|Phase 2 ERROR"
```

如果 fail-fast 触发，**先看错误是 schema bug 还是 quota / network**。jeniya 时代的 quota 用尽错是 `local:insufficient_quota`；字节内网应该没这个，但可能有 `quota`/`rate limit`/`unauthorized`。

### 5.5 跑 evaluate（LLM-as-judge）

```bash
python3 /home/LLM4Con/scripts/evaluate_recall.py \
    --api-key "$LLM_API_KEY" \
    --base-url "https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=$LLM_API_KEY" \
    --model claude-sonnet-4-6
# 输出：/home/LLM4Con/kernel_experiment/evaluation_report.json
```

**关键点**：judge prompt 评的是 root cause，**不要**在 detector 端再加 hard template 规则（见 §2.5）。如果你发现 judge 自身的 prompt 偏严（非语义匹配，而是字面 bug_category 比对），**改 judge prompt** 而不是 detector prompt。

---

## 6. 评估指标

| 维度 | 含义 | 当前数据（M5 baseline） | M7 目标 |
|---|---|---|---|
| **FOUND%** | 至少产出 1 条 confirmed hypothesis 的 CVE 占比 | 50.0% (26/52) | ≥ 60% |
| **recall@broad** | judge 认为命中文件层 | 89.8% | 维持 |
| **recall@strict** | 三维（文件 + 函数 + 语义）全命中 | 18.4% | ≥ 35% |
| **precision** | 假说中真正是漏洞的比例 | ≈ 12% | ≥ 25% |

`recall@strict` 是核心提升目标——M7 设计 DSL 就是为它。

---

## 7. 你接手后建议的下一步（按 ROI 排序）

1. **跑通字节内网 endpoint**（§4.5）→ 跑完 50 CVE batch → evaluate → 把结果填进 [`README.md`](./README.md) 的 §M7 验收表
2. **Syscall 名称归一化**（§2.10）：`src/CPG/CPG.cpp::findMethod()` 已有 `demangleVariants()` 框架，加上 syscall 系列；预计能多救活 5 个 CVE 的入口点解析 → 直接拉高 FOUND%
3. **VulnerabilitySurface shared-object 口径统一**：`src/Query/VulnerabilitySurfaceGenerator.cpp` 里 `conflicting pairs` 和 `sharedObjects` 用了不同聚合键。已知影响 CVE-2024-35977 / CVE-2024-36938 / CVE-2024-41081（threads > 0、conflicting pairs > 0、但 shared object = 0 → 0 hypothesis）。统一后可救 3 个
4. **HBGraph 实装剩余 build*Edges**：当前只有 PROGRAM_ORDER + CALL_RETURN。如果 evaluate 显示 FP 多，在 `src/CCPG/HBGraph.cpp` 把 `buildLockEdges()` / `buildRCUEdges()` / `buildCompletionEdges()` 实装
5. **Core dump 修复**：CVE-2025-22050、CVE-2025-37772 仍崩。挂 gdb 取堆栈

**不要**做的事：
- ❌ 不要为了字面命中数据集的 bug_category 而往 prompt 加 hard rule（§2.5）
- ❌ 不要回退 `sanitize_messages_for_tool_calls`（§2.3、§2.4）
- ❌ 不要切到一个全新模型（如 doubao）然后比 M5 baseline——会引入 confounding variable

---

## 8. Reference Files

精读顺序：

| 优先级 | 文件 | 看什么 |
|---|---|---|
| ★★★ | [`HANDOFF.md`](./HANDOFF.md) | 你正在看 |
| ★★★ | [`README.md`](./README.md) | 完整实验背景、M0-M5 优化记录、评估流程 |
| ★★ | [`M7_IMPLEMENTATION_PLAN.md`](./M7_IMPLEMENTATION_PLAN.md) | M7 六阶段设计与验收标准 |
| ★★ | [`HYPOTHESIS_DSL_DESIGN.md`](./HYPOTHESIS_DSL_DESIGN.md) | 5+3 DSL 的理论基础与 8 个 bug family 映射 |
| ★ | `src/LLMUtil/DetectorAgent.cpp::build_system_prompt()` | 当前 prompt 全文 |
| ★ | `src/Query/HypothesisVerifier.cpp` | 谓词求值实现 |
| ★ | `src/LLMUtil/LLMClient.cpp::sanitize_messages_for_tool_calls` | 关键 schema fix |
| ★ | `scripts/batch_detect.sh` | 批跑入口（含 preflight + fail-fast） |
| ★ | `scripts/evaluate_recall.py` | LLM-as-judge |

---

## 9. 失联场景的 fallback

如果你（接手的 LLM）卡住了，在向用户求助之前先确认：

- [ ] 是不是 §2 里的某个已知坑？
- [ ] `nm Release-build/llm_detector | grep <symbol>` 确认你的代码改动真的链进去了？
- [ ] 你看的是不是上一次失败留下的 `.prev.log`？最新跑的 log 是 `detection_hypothesis_batch.log`（无后缀）
- [ ] git push / commit 失败时试过 §2.8 的 `env -i` 招吗？
- [ ] LLM 端点不通时，**先用 curl 直接打**（绕开 detector），确认是 key 问题、URL 问题、还是网络问题？

只有以上都不解释你的问题时，才向用户描述「你做了什么 → 期待什么 → 实际看到什么」，并给出**最小复现步骤 + 完整错误输出**（不是 paraphrase）。
