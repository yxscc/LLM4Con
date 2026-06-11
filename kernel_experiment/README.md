# 真实内核源码并发漏洞检测实验

## 背景

Lace（LLM-Enhanced Concurrency Vulnerability Detector）原先在 LinConVul 数据集上验证，该数据集从 Linux 内核中提取简化后的并发漏洞代码。审稿人指出这种简化不具备真实检测场景的代表性。本实验的目标是直接在真实的内核源码模块上运行 Lace，验证端到端检测能力。

## 目录结构

```
kernel_experiment/
├── README.md                          ← 本文件
├── batch_run.log                      ← 批量检测运行日志
├── CVE-YYYY-XXXXX/                    ← 每个 CVE 一个目录
│   ├── src/                           ← 从内核源码拷贝的 .c/.h 文件（给 Joern/CPG）
│   ├── *.ll                           ← LLVM bitcode（给 Phasar/CCPG）
│   ├── merged.ll                      ← 多个 .ll 合并后的结果（如适用）
│   ├── snd-seq.ll                     ← 预链接 bitcode（CVE-2017-15265 特有）
│   ├── ground_truth.json              ← CVE 描述、CWE、patch、分析摘要
│   ├── entry_points.txt               ← 手动指定的入口点（可选，空则自动发现）
│   ├── detection_hypothesis_batch.log ← 开放假说检测日志
│   └── *_compile.log                  ← clang 编译日志
```

## 数据来源

| 来源 | 路径 | 用途 |
|------|------|------|
| Linux 内核 git | `/home/ConCord/targets/linux.git` | checkout 漏洞版本并编译 |
| CVE 报告 | `/home/ConCord/concurrency_cve_reports/linux_kernel/` | 提取 ground truth |
| CVE 索引 | `/tmp/cve_survey.csv` | 所有 CVE 列表（CVE, HAS_PATCH, FILES, FIX_COMMIT）|

## 数据集统计

工作目录 `kernel_experiment/` 的原始 prepared 数据集包含 **100 个 case**（`CVE-*` 和 `SYZBOT-*`）。
截至 2026-06-11，active benchmark 采用以下排除策略：

| 类别 | 数量 | 说明 |
|------|------|------|
| 原始 prepared case | 100 | 见历史实验与 `dataset_benign_exclusions.json` |
| benign annotation-only case | 28 | 已移动到 `backup_benign_excluded_20260611/` |
| 顶层 active case 目录 | 72 | glob `CVE-*` / `SYZBOT-*` 可见 |
| dual-thread 不适合 case | 3 | 仍在顶层，但 runner 根据 `dataset_exclusion_candidates.json` 排除 |
| 当前有效评估集 | 69 | 见 `dataset_active_manifest.json` |

benign case 的排除依据：纯 `READ_ONCE` / `WRITE_ONCE` / `data_race` 注解修复，且 racy value 不影响内存安全、对象身份或生命周期。完整列表和理由见 `dataset_benign_exclusions.json`。被移动的目录保存在：

```
backup_benign_excluded_20260611/
├── moved_cases_manifest.json
└── <CVE-* | SYZBOT-*>/
```

漏洞类型分布（按 ground truth `cwes` 字段）：

| 类型 | 数量 |
|------|------|
| DataRace | ≈ 22 |
| UseAfterFree | ≈ 25 |
| DoubleFree | 1 |
| NullPointerDereference | 1 |
| unknown | 3 |

## 核心脚本

| 脚本 | 路径 | 功能 |
|------|------|------|
| 批量编译 | `scripts/batch_prepare.sh` | 遍历 survey，checkout 漏洞版本，编译为 LLVM bitcode，收集 ground truth |
| 批量检测 | `scripts/batch_detect.sh` | 对每个已编译的 CVE 运行 `llm_detector --agent-mode`（开放假说模式） |
| 召回评估 | `scripts/evaluate_recall.py` | 用 LLM-as-judge 比对检测结果与 ground truth |

## 编译流程（batch_prepare.sh）

对每个 CVE：
1. `git clean -fdxq` 清除构建残留
2. `git checkout <fix_commit>~1 --force` 切到漏洞版本
3. `make allyesconfig` 启用所有 CONFIG 选项（确保子系统头文件完整）
4. `make modules_prepare` 生成 `autoconf.h`、`asm-offsets.h` 等
5. `clang -S -emit-llvm` 编译涉及的 `.c` 文件为 LLVM bitcode
6. 多文件时 `llvm-link -S` 合并为 `merged.ll`
7. 拷贝源文件到 `src/` 目录（供 Joern 生成 CPG）
8. 从 CVE 报告中提取 `ground_truth.json`

编译涉及的文件来自 `cve_survey.csv` 的 `FILES` 列（即补丁 fix commit 修改的源文件）。

## 检测架构：开放假说驱动（Open Hypothesis）

### 整体流程

```
Input: *.ll (LLVM bitcode) + src/ (source code)
  │
  ├─ CPG Generation (Joern，带内核注解 --define)
  ├─ Pointer Analysis (Phasar)
  ├─ 入口点自动发现（内核模块无 main）
  ├─ Thread Creation Tree
  ├─ LockSet Analysis
  │       ↓
  │   CCPG (Concurrent Code Property Graph)
  │
  ├─ Phase 0: Thread API Discovery (LLM)
  │   └─ 发现自定义的 lock/thread wrapper 函数
  │
  ├─ Phase 1: Vulnerability Surface Generation (静态)
  │   └─ 分析共享对象、访问模式、锁保护 → vulnerability_surface.json
  │
  ├─ Phase 2: DetectorAgent (LLM, 单会话多轮)
  │   └─ LLM 阅读 surface，自由提出 bug 假说 + 约束条件
  │       ↓ propose_hypothesis tool call
  │       HypothesisVerifier 即时验证 → 反馈 pass/fail → LLM 迭代
  │
  └─ Phase 4: Report
      └─ 输出确认的假说为检测结果
```

### 入口点自动发现

无需手动配置 `entry_points.txt`。当 bitcode 中不存在 `main` 函数时（内核模块场景），自动执行：

- **策略 1 — 全局结构体函数指针扫描**：扫描全局变量初始化器（如 `file_operations`、`net_device_ops`、`nfnl_subsys_table` 等），提取其中的函数指针作为回调入口。此策略由 `PhasarPointerAnalysis::discoverCallbackEntryPoints()` 实现。
- **策略 2 — 名称模式匹配**：按优先级识别 `sys_*`（系统调用）、`init_module`/`cleanup_module`、`*_init`/`*_exit`、`*_handler`/`*_callback`/`*_thread` 等内核入口模式。

两策略合并去重后，所有发现的入口点被视为潜在的并行执行上下文。

### 开放假说机制

与传统的预定义 Rule 模式不同，LLM 可以自由描述任意并发 bug 并用 6 个**静态分析谓词**表达验证条件：

| 谓词 | 含义 |
|------|------|
| `in_thread(node, thread)` | 节点可在指定线程中执行 |
| `may_run_concurrently(t1, t2)` | 两线程可并发运行 |
| `reachable(from, to)` | 线程内 from 可达 to |
| `not_lock_protected(node)` | 节点不在任何锁保护区间内 |
| `same_lock(n1, n2)` | 两节点被同一把锁保护 |
| `alias(n1, n2)` | 两节点操作的内存对象可能指向同一地址 |

LLM 提出假说后，`HypothesisVerifier` 立即逐条验证约束，将 pass/fail 反馈返回 LLM，LLM 可据此调整假说，形成闭环迭代。

### Bitcode 选择策略（batch_detect.sh）

当一个 CVE 目录下有多个 `.ll` 文件时，按以下优先级选择：

1. `merged.ll` — `llvm-link` 合并后的完整模块
2. `snd-seq.ll` — 预链接的 ALSA 子系统（CVE-2017-15265 特有）
3. 体积最大的 `.ll` — 通常是主模块或包含更多代码的文件
4. 字母序第一个 `.ll` — fallback

## 批量运行的工程调整

在初次 50 个 CVE 批量运行中，绝大多数 CVE 出现 "0 hypotheses confirmed"。通过逐层定位，发现了五个核心问题并分别修复。**修复均保持非侵入性：不修改待测内核源码，只修改检测器。**

### 问题 1：CPG 缓存错配（所有 CVE 共享同一张 CPG）

**现象**：批量运行中每个 CVE 的 CPG 结构都相同，看似 CPG 缓存生效，实则所有 CVE 都读到了同一张张冠李戴的图。

**根因**：`batch_detect.sh` 通过 `cd $cve_dir` 后传入 `--input-src src` 的相对路径。`TargetPath::setTargetAbsolutePath` 对相对路径不做解析，直接调用 `filesystem::path("src").filename()`，得到空的 `targetProjectName`，所有 CVE 的 CPG 都被写入到根目录 `/home/LLM4Con/cpg_dot/export.dot`。

**修复**：`include/Util/TargetPath.h`，在设置路径前先 `fs::absolute` + `fs::canonical` 解析相对路径：

```cpp
void setTargetAbsolutePath(const std::string& TargetAbsolutePath){
    fs::path targetAbsolutePath = fs::path(TargetAbsolutePath);
    if (targetAbsolutePath.is_relative()) {
        targetAbsolutePath = fs::absolute(targetAbsolutePath);
    }
    if (fs::exists(targetAbsolutePath)) {
        targetAbsolutePath = fs::canonical(targetAbsolutePath);
    }
    ...
}
```

### 问题 2：LockSet 分析崩溃（Signal 6 / getArgOperand 断言）

**现象**：`CCPG_Analysis` 阶段随机崩溃，报错 `llvm::CallBase::getArgOperand: Assertion 'i < arg_size()'`。

**根因**：`src/CCPG/AliasChecker.cpp` 在处理 `FORK`/`JOIN`/`LOCK` 节点时直接 `getArgOperand(0)`，未检查参数个数。内核中某些宏展开产生零参数的 CallInst（如 `rcu_read_lock`），触发断言。

**修复**：在所有 `getArgOperand(0)` 前加 `arg_size()` 边界检查。

### 问题 3：LLM 入口解析死循环导致超时

**现象**：`CVE-2017-6346` 等 CVE 在 `CCPG_Analysis` 阶段被批量脚本 SIGTERM（timeout 600s）。

**根因**：`FindingThreadEntryAgent` 对 `mod_timer` 等间接回调 API 触发 LLM fallback。LLM 正确判断"entry 无法从参数直接确定"并调用 `report_entry_point_id(-1)`，但 Agent 中的 guardrail 强制要求必须先调用其它工具后才接受 `-1`，导致 LLM 反复被打回→再次报告→再次被打回。

**修复**：
1. `src/LLMUtil/FindingThreadEntryAgent.cpp`：放宽 guardrail，允许 LLM 在已经调用 `get_function` 查看过上下文后报告 `-1`。
2. `scripts/batch_detect.sh`：用户明确指出不想使用 600s 超时机制 —— 移除 `timeout` 命令；同时把 `set -eo pipefail` 改为 `set -o pipefail`，避免单个 CVE 失败导致整个脚本退出。

### 问题 4：Joern 无法解析内核注解（丢失入口点）

**现象**：CPG 中某些函数明明存在于源码中，却被解析为 `UNKNOWN` 或 `IDENTIFIER` 节点而非 `METHOD`，导致入口点匹配失败。

**根因**：Joern 的 C frontend 无法识别 `__user`、`__init`、`__net_init`、`__acquires` 等内核 GCC 属性，将其当作类型或变量，破坏了函数声明的解析。

**修复**（非侵入，不改内核源码）：`src/CPG/CPGGenerator.cpp` 向 `joern-parse` 传递 `--frontend-args` + 一系列 `--define` 把这些注解定义为空：

```cpp
static const std::vector<std::string> kernelDefines = {
    "__user", "__kernel", "__iomem", "__rcu", "__percpu",
    "__force", "__cold", "__read_mostly", "__ro_after_init",
    "__init", "__exit", "__initdata", "__exitdata", "__initconst",
    "__net_init", "__net_exit", "__net_initdata",
    "__devinit", "__devexit", "__devinitdata",
    "__acquires", "__releases", "__must_hold",
    "__maybe_unused", "__always_inline",
    "asmlinkage", "notrace", "noinline",
    "__bitwise", "__randomize_layout", "__aligned",
    "__cacheline_aligned", "__cacheline_aligned_in_smp",
    "__packed", "__weak", "__visible",
};
std::string defineArgs;
for (const auto& def : kernelDefines) defineArgs += " --define " + def;
std::string cmd = "joern-parse -J-Xmx40G " + dir + " --frontend-args" + defineArgs;
```

### 问题 5：NodeLoc 路径对齐失败（"0 raw memory accesses"）

**现象**：修复前四个问题后，大量 CVE 仍显示 `Collected 0 raw memory accesses across all threads`，即使 TCT 已识别出多个线程。

**根因**：IR debug info 与 Joern CPG 使用不同的路径前缀：
- IR debug info（Phasar 解析）：`/home/ConCord/targets/linux.git/net/unix/af_unix.c`
- Joern CPG 节点：`/home/LLM4Con/kernel_experiment/CVE-XXXX/src/net/unix/af_unix.c`

`NodeLoc::NodeLocHash` 用 `getBaseFileName()` 生成哈希（映射到同一桶），但 `NodeLoc::operator==` 调用 `arePathsLikelySameFile` 做 segment-by-segment 反向比较。两路径反向匹配到 `net/unix/af_unix.c` 后，下一个目录是 `linux.git` vs `src`，不等，直接返回 false。结果：哈希相等但 operator== 不等 → map 永远找不到对应 key。

**修复**：`include/CCPG/CCPGNode.h`，在路径规范化比较失败时，回退到 basename 比较：

```cpp
bool operator==(const NodeLoc& other) const {
    if (lineNumber != other.lineNumber) return false;
    if (arePathsLikelySameFile(normalizedFileName, other.normalizedFileName))
        return true;
    return getBaseFileName() == other.getBaseFileName();
}
```

**验证**（CVE-2024-26974，修复前后对比）：

| 指标 | 修复前 | 修复后 |
|------|--------|--------|
| Threads 识别 | 5 | 5 |
| Raw memory accesses | 0 | 23 |
| Shared objects | 0 | 1 |
| Hypotheses confirmed | 0 | 1 |
| Bugs found | 0 | **1 (double_free)** |

## 检测命令

```bash
# 单个 CVE（自动发现入口点）
./Debug-build/llm_detector --input-bc nf_tables_api.ll --input-src src \
    --agent-mode --llm-key $KEY --llm-model claude-sonnet-4-6

# 单个 CVE（手动指定入口点）
./Debug-build/llm_detector --input-bc snd-seq.ll --input-src src \
    --entry-config entry_points.txt --agent-mode \
    --llm-key $KEY --llm-model claude-sonnet-4-6

# 批量检测（全部 53 个 CVE，无超时机制）
./scripts/batch_detect.sh --api-key $KEY --model claude-sonnet-4-6
```

> **注意**：`batch_detect.sh` 会跳过存在 `detection_hypothesis_batch.log` 的 CVE 以支持断点续跑。如需完全重跑，先 `rm -f CVE-*/detection_hypothesis_batch.log` 并清理 `cpg_dot/CVE-*` 缓存。

## 本次完整批量实验（2026-04-17）

### 复现命令

```bash
# 1. 清除上次缓存（可选，但重跑时需要）
rm -f /home/LLM4Con/kernel_experiment/CVE-*/detection_hypothesis_batch.log
rm -rf /home/LLM4Con/cpg_dot/CVE-*

# 2. 后台启动批量检测
cd /home/LLM4Con/kernel_experiment
nohup bash /home/LLM4Con/scripts/batch_detect.sh \
    --api-key "$LLM_API_KEY" \
    --model "claude-sonnet-4-6" \
    > batch_run.log 2>&1 &
echo $! > batch_run.pid
```

### LLM 配置

| 项 | 值 |
|---|---|
| 模型 | `claude-sonnet-4-6` |
| API Key | `$LLM_API_KEY` |
| 接入方式 | LLM4Con 内置的 OpenAI 兼容接口（`src/LLMUtil/LLMClient.cpp`） |

### 日志与结果位置

| 内容 | 路径 |
|---|---|
| 批量脚本顶层日志 | `/home/LLM4Con/kernel_experiment/batch_run.log` |
| 单个 CVE 检测日志 | `/home/LLM4Con/kernel_experiment/CVE-*/detection_hypothesis_batch.log` |
| 确认假说（机器可读） | `/home/LLM4Con/LLM_dump/CVE-*_<timestamp>/confirmed_hypotheses.log` |
| 漏洞面快照 | `/home/LLM4Con/LLM_dump/CVE-*_<timestamp>/vulnerability_surface.json` |
| 违规检测输出 | `/home/LLM4Con/LLM_dump/CVE-*_<timestamp>/stateful_bugs/bugs.txt` |
| CPG 缓存 | `/home/LLM4Con/cpg_dot/CVE-*/export.dot` |
| 批处理 PID | `/home/LLM4Con/kernel_experiment/batch_run.pid` |

### 总体数据

| 指标 | 值 |
|---|---|
| 数据集规模 | 53 个 CVE |
| 成功运行完成 | 45 / 53 (85%) |
| **检测到假说（FOUND）** | **20 / 53 (37.7%)** |
| 总共确认假说数 | **123 个** |
| CLEAN（未触发假说） | 17 / 53 |
| API_ERROR（LLM 侧失败） | 8 / 53 |
| 运行时崩溃（core dumped） | 5 / 53 |
| SKIP（无 `.ll` 文件） | 2 / 53 |
| 总检测耗时 | 8329 秒（≈ 2.3 小时，串行） |
| 总 LLM API 调用 | 850 次 |
| 总 Token 消耗 | ~31.56 M |

### 检测到假说的 CVE 明细

| CVE | 假说数 | 主要 bitcode |
|---|---|---|
| CVE-2013-1792 | 1 | process_keys.ll |
| CVE-2016-9806 | 5 | af_netlink.ll |
| CVE-2017-15265 | 6 | snd-seq.ll |
| CVE-2024-26974 | 3 | adf_aer.ll |
| CVE-2024-27019 | 9 | nf_tables_api.ll |
| CVE-2024-35898 | 7 | nf_tables_api.ll |
| CVE-2024-35986 | 4 | phy-tusb1210.ll |
| CVE-2024-39503 | 11 | merged.ll |
| CVE-2024-43891 | 12 | merged.ll |
| CVE-2024-46704 | 8 | merged.ll |
| CVE-2024-53124 | 5 | merged.ll |
| CVE-2024-56555 | 8 | merged.ll |
| CVE-2024-56788 | 13 | merged.ll |
| CVE-2025-23142 | 6 | merged.ll |
| CVE-2025-23151 | 3 | merged.ll |
| CVE-2025-37854 | 5 | kfd_process.ll |
| CVE-2025-37920 | 6 | merged.ll |
| CVE-2025-38037 | 3 | vxlan.ll |
| CVE-2025-38217 | 4 | ftsteutates.ll |
| CVE-2025-38250 | 4 | hci_core.ll |

### CLEAN（0 假说）原因分类

共 17 个，分两大类：

**A. 0 threads（入口点/TCT 无并发结构）**
- CVE-2015-7550、CVE-2016-7911、CVE-2024-41005、CVE-2024-42234、CVE-2024-43830 等
- 根因：IR debug info 里的函数名（如 `SyS_add_key` 由 `SYSCALL_DEFINE5` 宏展开）与 Joern CPG 里的方法名（展开为 `add_key` 或保留宏原名）对不上，入口点从 CPG 映射失败 → `Total entry functions: 0` → 没有构建任何 thread
- 这个问题需要另外一轮修复：增加 syscall 宏系列（`SyS_*` ↔ `SYSCALL_DEFINE*`、`COMPAT_SYSCALL_DEFINE*`）的名称归一化匹配

**B. threads > 0 但 0 shared objects（共享判定过严）**
- CVE-2024-35977（4 threads / 22 accesses / 15 conflicting pairs / 0 shared objects）
- CVE-2024-36938（2 threads / 17 accesses / 55 conflicting pairs / 0 shared objects）
- CVE-2024-41081（8 threads / 66 accesses / 28 conflicting pairs / 0 shared objects）
- 根因：`VulnerabilitySurfaceGenerator` 判定 "shared object" 的逻辑与 "conflicting pairs" 的口径不一致——`conflicting pairs` 识别到了跨线程访问对，但 `sharedObjects` 的聚合键可能把它们拆成了不同对象（指针、别名分析颗粒度问题），导致最终 `shared object == 0`

### API_ERROR（LLM 侧失败）

共 8 个：CVE-2017-6346, CVE-2024-27404, CVE-2024-35999, CVE-2024-40953, CVE-2024-45000, CVE-2024-58072, CVE-2025-22050, CVE-2025-38078

- 主要原因：prompt 过长超过 context window，或 claude API 瞬时速率限制/网络错误
- 可在 `CVE-*/detection_hypothesis_batch.log` 中 `grep "Phase 2 ERROR"` 定位

### Core dumped（5 个）

CVE-2024-27030, CVE-2024-38596, CVE-2024-39508, CVE-2025-37772, CVE-2025-38383

- 说明 CCPG/LockSet 分析在某些极端 bitcode 结构上仍有断言触发。需单独复现，挂 gdb 取堆栈。

### SKIP（2 个）

- CVE-2011-2183（编译失败，无 `.ll` 输出）
- CVE-2024-53160（编译失败，无 `.ll` 输出）

### 后续工作

1. **Syscall 宏名称归一化**：让 `CPG::findMethod` 对 `SyS_xxx`/`__do_sys_xxx`/`__se_sys_xxx`/`__arm64_sys_xxx` 做变体匹配，恢复类别 A 的 5 个 CVE 的 threads。
2. **VulnerabilitySurface shared-object 口径统一**：把 `conflicting pairs` 的聚合键和 `sharedObjects` 对齐，恢复类别 B 的 3 个 CVE。
3. **Core dump 复现与修复**：对 5 个崩溃 CVE 挂 gdb 取堆栈。
4. **API_ERROR 降级处理**：在 `DetectorAgent` 侧检测到 prompt 过长时自动裁剪 surface 列表，或拆分多会话。
5. **召回评估**：用 `scripts/evaluate_recall.py` 对 20 个 FOUND 的 CVE 逐一 LLM-as-judge 与 ground_truth 比对，给出 true-recall / false-positive 数据。

## 优化后完整批量实验（2026-04-19, M5）

在 2026-04-17 的基线快照之后，我们完成了 Lace 检测器五阶段优化（详见 `/root/.cursor/plans/lace_detector_optimization_e985ba2b.plan.md`），
覆盖 `SharedFieldKey` 字段级聚合、`LSAnalysis` 全链路接入、Syscall 入口归一化与生命周期过滤、DetectorAgent 假说去重与
context 截断、以及跨函数 BFS 可达性分析等改动。随后用相同 53-CVE 数据集重跑 `scripts/batch_detect.sh` 并用升级后的
`scripts/evaluate_recall.py` 做三维评分。

### 复现命令

```bash
# 完全重跑（清缓存 + 重新编译 + 重新评估）
rm -f /home/LLM4Con/kernel_experiment/CVE-*/detection_hypothesis_batch.log
rm -rf /home/LLM4Con/cpg_dot/CVE-*
cd /home/LLM4Con/Debug-build && cmake --build . -j$(nproc)

cd /home/LLM4Con/kernel_experiment
nohup bash /home/LLM4Con/scripts/batch_detect.sh \
    --api-key "$LLM_API_KEY" \
    --model "claude-sonnet-4-6" \
    > batch_run.log 2>&1 &

# 批量跑完后用 LLM-as-judge + 三维评分
python3 /home/LLM4Con/scripts/evaluate_recall.py \
    --api-key "$LLM_API_KEY" \
    --base-url "https://jeniya.cn/v1" \
    --model "claude-sonnet-4-6"
```

### 总体数据（M5 快照）

| 指标 | 基线（M0, 2026-04-17）| 优化后（M5, 2026-04-19）| 变化 |
|---|---|---|---|
| 数据集规模 | 53 | 53 | — |
| **FOUND（产出确认假说）** | **20 / 53 (37.7%)** | **26 / 52 (50.0%)** | **+6** |
| 总共确认假说数 | 123 | 130 | +7 |
| CLEAN（0 假说） | 17 / 53 | 14 / 52 | −3 |
| API_ERROR | 8 / 53 | 8 / 52 | 0 |
| 运行时崩溃（core dumped）| 5 / 53 | **2 / 52** | **−3** |
| SKIP（无 `.ll`） | 2 / 53 | 2 / 52 | — |

> 说明：M5 批次总数按脚本实际遍历的 52 个 CVE 目录计数（`CVE-2009-3547`、`CVE-2016-1972`/`1973` 等始终没有 `.ll`）。
> 剩下的 2 个 core dump 集中在 `CVE-2025-22050`、`CVE-2025-37772`。

### 三维召回评估（M5）

用 `scripts/evaluate_recall.py` 对 49 个有 `ground_truth.json` 的 CVE 做 LLM-as-judge + 文件 / 函数层面机械匹配：

| 维度 | 命中 | 比例 |
|---|---|---|
| `file_locality_match`（文件粒度） | 44 / 49 | **89.8%** |
| `func_name_match_with_patch`（函数粒度） | 34 / 49 | 69.4% |
| `llm_judge_semantic_match`（语义粒度） | 9 / 49 | 18.4% |
| **recall@broad**（文件层命中） | 44 / 49 | **89.8%** |
| **recall@strict**（三维全部命中） | 9 / 49 | **18.4%** |

- **MATCH**（LLM 判定与 CVE 根因一致）：`CVE-2016-7911`、`CVE-2017-15265`
- **PARTIAL**（方向一致但目标对象或机制偏差）：`CVE-2013-1792`、`CVE-2024-26974`、`CVE-2024-39503`、`CVE-2024-39508`、`CVE-2024-41081`、`CVE-2025-38037`、`CVE-2025-38078`

三维分解揭示了 broad 与 strict 之间的巨大差距（89.8% vs 18.4%）：Lace 定位到了对的文件/函数，但在 shared object 选择与
竞态机制叙述上仍有大量语义漂移，这是下一步优化的主战场。

### 五阶段 Ablation（M1 → M5）

| 里程碑 | 主要改动 | 预期验收 | 实际数字 |
|---|---|---|---|
| **M1 (Phase 1)** | `SharedFieldKey` 字段级聚合 + 栈局部过滤 | `CVE-2024-35977` 出现 ≥ 1 shared object | Canary 通过（详见 Phase 1 canary）|
| **M2 (Phase 2)** | `LSAnalysis` 接入 Verifier/Surface/Agent + 25 + 条内核锁 API | `CVE-2024-27019` 假说数 9 → ≤ 4 且命中真 race | Canary 通过；同文件 FOUND 数 20 → 26 |
| **M3 (Phase 3)** | Syscall 名称归一化 + `SYSCALL_DEFINE*` 宏展开 + 生命周期过滤 | `CVE-2024-42234` 入口 > 0；`_init`/`_exit` 不再配对 | 代码落地；受 Joern 对内核宏/条件编译支持有限影响，部分 CVE 仍 CLEAN |
| **M4 (Phase 4)** | `toPromptString` topN 截断 + 假说去重 + "Quality over quantity" prompt | API_ERROR 8 → ≤ 2；FOUND 假说 ≤ 5 / CVE | 去重生效（多数 FOUND CVE 现为 5 条以内）；API_ERROR 维持 8（受远端 429 限流影响，非 token） |
| **M5 (Phase 5)** | `getArgOperand` 加固 + `eval_reachable` 跨函数 BFS + `evaluate_recall.py` 三维评分 | 0 core dump；输出 recall@strict / @broad | core dump 5 → 2；recall@broad 89.8%；recall@strict 18.4% |

### 失败类型分布（M5）

| 类别 | 数量 | 代表 CVE |
|---|---|---|
| FOUND（至少 1 条通过 Verifier 的假说） | 26 | `CVE-2017-15265`, `CVE-2024-27019`, `CVE-2025-38037`, ... |
| CLEAN（0 假说） | 14 | `CVE-2024-35977`, `CVE-2024-42234`, ... |
| API_ERROR | 8 | `CVE-2017-6346`, `CVE-2024-27404`, ... |
| FAIL / core dumped | 2 | `CVE-2025-22050`, `CVE-2025-37772` |
| SKIP（无 `.ll`） | 2 | `CVE-2011-2183`, `CVE-2024-53160` |

### 遗留问题与后续计划

1. **API_ERROR 未能实质下降**：本轮 M5 API_ERROR 数量维持 8，主因是上游远端 `claude-sonnet-4-6` 端点在高并发时 429 限流，而非 prompt token 超限（M4 的 topN 截断已将峰值压到 ≤ 90K）。后续需要在 `LLMClient` 中加入指数退避与 batch sleep。
2. **Joern 宏展开仍有盲区**：`SYSCALL_DEFINE*` + `#ifdef CONFIG_NUMA` 组合在孤立源文件下无法完整展开（Joern 的 C 前端限制），导致 `CVE-2024-42234` 等仍 CLEAN。长期方案是导出真实内核 `build.ninja` 中的预处理产物。
3. **三维评分 strict/broad 落差**：recall@broad 89.8% 但 recall@strict 仅 18.4%，说明 Lace 能锁定到正确文件但对 shared object / 机制描述漂移。下一阶段应引入 "bug-pattern canonicalization"：在 DetectorAgent 侧加一条"如果检测到的对象与 ground-truth patch 描述语义不符则打回重提"的自校准规则（需用 LLM-as-judge 离线训练打分器）。
4. **剩余 2 个 core dump**：`CVE-2025-22050` 与 `CVE-2025-37772` 仍在 PhasarPointerAnalysis 的某条 bitcode 上触发断言，需挂 gdb 复现。

## LLM-only 召回评估（2026-04-24，M6 — claude-opus-4-6 judge）

为了消除 M5 中 "broad/strict 看似差距巨大但其实有大量 PARTIAL 命中" 的灰色地带，本轮**彻底放弃文件名 / 函数名机械匹配**：让 LLM 同时接收 ground truth（CVE 描述、CWE、完整 patch、分析摘要）和检测器实际输出的 N 条 bug 报告，**一次性判定**：

- 总体维度：`HIT` / `MISS` —— 是否有任何一条报告对应到 patch 真正修复的 bug
- 单 bug 维度：`TP_MATCH` / `TP_RELATED` / `FP` —— 与 patch 的语义距离

判定 prompt 与重试逻辑见 `scripts/evaluate_recall.py`（`JUDGE_SYSTEM` 常量）。

### 复现命令

```bash
# 1. 用最新 detector 全量重跑（包含本轮在 ThreadCreationTree 加的 try-catch）
cd /home/LLM4Con/Release-build && cmake --build . -j$(nproc)
rm -f /home/LLM4Con/kernel_experiment/CVE-*/detection_hypothesis_batch.log

cd /home/LLM4Con/kernel_experiment
nohup bash /home/LLM4Con/scripts/batch_detect.sh \
    --api-key "<KEY>" --model "claude-opus-4-6" \
    > batch_run.log 2>&1 &

# 2. LLM-only 三类判定（ALL CVE 一次跑完）
python3 /home/LLM4Con/scripts/evaluate_recall.py \
    --api-key "<KEY>" --model "claude-opus-4-6" \
    --base-url "https://jeniya.cn/v1" \
    --output /home/LLM4Con/kernel_experiment/evaluation_report.json
```

### 总体数据（M6）

| 指标 | M5 (机械匹配 + LLM)| M6 (纯 LLM judge) |
|---|---|---|
| 数据集 | 53 (含 SKIP) | **50（隔离 2 个无 .ll）** |
| 检测器有输出的 CVE | 26 / 52 | **49 / 50** |
| 总 bug 报告数 | 130 | **264** |
| **HIT (LLM 判定召回)** | recall@strict 18.4% | **13 / 50 = 26.0%** |
| TP_MATCH | — | 24 / 264 = 9.09% |
| TP_RELATED | — | 166 / 264 = 62.88% |
| FP | — | 74 / 264 = 28.03% |
| precision (strict TP_MATCH only) | — | 9.09% |
| precision (lenient TP_MATCH + TP_RELATED) | — | **71.97%** |
| FP rate | — | 28.03% |
| LLM judge 调用数 | 49 | 50 + 10（重判截断 patch）|

最新 13 个 HIT：`CVE-2013-1792, 2017-15265, 2024-26974, 2024-27030, 2024-35977, 2024-38596, 2024-39503, 2024-39508, 2024-47715, 2024-53124, 2025-23151, 2025-38037, 2025-38048`。

### 工程修复（让 M6 能跑通的两件事）

1. **检测器侧的 SIGABRT 修复**：M6 第一次全量跑时有 13 个 CVE 在 `CCPG_Analysis` 阶段 exit 134。根因为 `FindingThreadEntryAgent` 调用 LLM 期间 quota / 超时抛出 `std::runtime_error`，未在 `ThreadCreationTree::buildThreadCreationTree()` 中被捕获，直接 std::terminate。修复见 `src/CCPG/ThreadCreationTree.cpp` 中 `findThreadEntryByLLM` 调用处的 `try/catch`：LLM 失败时 entry 视为未解析，继续跑后续 phase，而不是把已经找到的 vulnerability surface 全部丢掉。
2. **Ground truth patch 截断修复**：M5 阶段 13 个 CVE 的 `ground_truth.json` 中 `patch` 字段被截断到 5000 B（旧版 prepare 脚本写入限制），LLM judge 缺信息会误判 MISS。已用 `/tmp/entry_scan/rebuild_gt.py` 从 `/home/ConCord/concurrency_cve_reports/linux_kernel/<CVE>/patch.diff` 全量回灌。重新评估 10 个原 MISS 后，仅 `CVE-2024-39503` 由 MISS 翻为 HIT —— 说明截断不是召回缺口的主因。

### 37 个 MISS 的真实根因分析

对 37 个 MISS CVE 按"检测器到底产出了什么"分桶，再对每桶抽 1–3 个深挖（看 `detection_hypothesis_batch.log` + `LLM_dump/<CVE>_*/stateful_bugs/bugs.txt` + `confirmed_hypotheses.log` + ground truth patch），归纳出 5 类根因：

| 桶 | 数量 | 含义 | 代表 CVE |
|---|---|---|---|
| `zero_reports` | 2 | 完全没产出报告 | CVE-2016-7911, CVE-2024-53136 |
| `all_fp` | 9 | 5 条全部 FP | CVE-2024-27404, CVE-2024-35986, CVE-2024-41081, CVE-2024-42234, CVE-2024-46704, CVE-2025-37882, CVE-2025-38165, CVE-2025-38242, CVE-2025-38383 |
| `mostly_fp` | 2 | FP ≥ TP_RELATED | CVE-2024-40953, CVE-2025-38337 |
| `tp_related_only` | 15 | 只触到旁系 race，没击中真目标 | CVE-2024-43891, CVE-2024-50082, CVE-2025-37920 等 |
| `mixed` | 9 | TP/FP 混合但仍未 HIT | CVE-2024-35898, CVE-2024-56555 等 |

#### 根因 A — CCPG/CPG 入口实体解析失败（直接 0 线程）

只占 2 个 CVE，但完全无法产出任何报告。

- **CVE-2016-7911**：结构化入口扫描出 `blk_ioc_init`，但 `getMain()` 在 CPG 中找不到对应 method（log 中明确：`methods from full path: 0` / `methods from filename only: 0` / `Warning: No methods found in file block/blk-ioc.c or blk-ioc.c`），原因是 IR debug info 路径 `block/blk-ioc.c` 与 CPG 入库时的相对路径 `blk-ioc.c` 不一致，且 basename 也匹配不到。
- **CVE-2024-53136**：结构化入口扫描出 42 个 `shmem_*` ops_table 成员，但 `cpg->findMethod(shortName)` 对 41 个返回 null（log 中刷屏 `- Not found in CPG: shmem_xxx`），最终只剩 `shmem_init` 一个入口；只有 1 个入口无法构成 MHP → `Threads: 0, Conflicting pairs: 262, Shared objects: 0` → "No shared objects found. Skipping LLM analysis."。

#### 根因 B — Verifier 谓词词汇表只覆盖"两节点同字段+无共享锁+可并发"（占 ≈ 50% MISS）

注意：Lace 早已切换到**开放假说**架构 —— `bug_category` 是 free-form string，LLM 可以自由命名和描述任意并发 bug（见 `src/LLMUtil/DetectorAgent.cpp:166`）。所以这一类 MISS **不是因为 hypothesis 类别被枚举死了**。

真正的瓶颈在 `HypothesisVerifier`：它只暴露 6 个静态分析谓词：

```text
in_thread / may_run_concurrently / reachable / not_lock_protected / same_lock / alias
```

这 6 个谓词组合起来表达力等价于"两个内存访问节点，可能并发执行，且不被同一把锁保护"。**这正好就是经典 data race / TOCTOU 的形式化定义**。其他常见并发缺陷的形式化条件超出了这套词汇：

| Patch 真正想说的 bug | 缺失的谓词 |
|---|---|
| "deref 前缺少 NULL 检查" | `must_be_nonnull(node)` / `dominator_check_holds(node, predicate)` |
| "读到的对象已被标记 `FREED`" | `flag_check_dominates(node, flag_field)` / `lifetime_state_at(node)` |
| "在 BH/IRQ 上下文调用了要求 BH-off 的 helper" | `executes_in_context(node, BH_DISABLED \| IRQ_DISABLED)` |
| "store 缺少 release / load 缺少 acquire" | `mem_order_at(node)` / `is_paired_with(load, store)` |
| "竞态作用于一个标量但只需 `READ_ONCE`/`WRITE_ONCE` 注解" | `is_torn_access(node)` |

由于 LLM 必须用这 6 个谓词把假说"喂给"verifier 才能通过验证，它只能把上述 bug **强行包装成"两节点同字段、可并发、无共享锁"**。结果就是：**真正命中是巧合（节点恰好对得上）；多数情况下 LLM 在同结构体内找别的字段去凑一组 race 来满足谓词**。代表对比：

| CVE | Patch 实际修的问题（无法用 6 谓词陈述）| Lace 实际产出的 5 个 hypothesis |
|---|---|---|
| 2024-43891 | `f_show()` 没检查 `EVENT_FILE_FL_FREED` → eventfs UAF | `trigger_data_refcount_race`, `trigger_data_count_race`, `event_file_flags_pid_filter_race`, `trigger_filter_plain_write_race`, `trigger_count_toctou` |
| 2024-41081 | `ila_output()` 缺 `local_bh_disable()` | `lwt_output_race_T0_T2`, `connected_race`, `locator_race`, `params_csum_ident_race` |
| 2024-46704 | `__flush_work()` PENDING bit 多余冗余写 | `pwq_refcnt_race`, `pool_nr_idle_race`, `pwq_nr_active_toctou`, `pwq_flush_color_inconsistent_lock`, `pwq_nr_in_flight_race` |
| 2024-35986 | 长存活 `power_supply` 在 `unregister` 后 deref → UAF | `ulpi_ptr_data_race`, `gpio_reset_data_race`, `gpio_reset_poweroff_race` |
| 2025-37882 | xHCI iso ring TRB 指针为 NULL 时 deref | `cmd_uaf_msi_frees_timeout_uses`, `current_cmd_data_race`, `cmd_ring_state_write_write_race` |
| 2025-38337 | `jbd2_journal_dirty_metadata` 未在 deref 前 `is_handle_aborted(handle)` → NULL deref | `j_transaction_sequence_race`, `h_transaction_race`, `h_journal_null_race_v2` |

5 条 hypothesis 都在**同文件、同结构体**找别的字段去凑一组写写/读写冲突，但完全没触碰 patch 修的那个特定行为。LLM judge 也正是据此把它们标 `TP_RELATED`（同模块相关并发问题）而不是 `TP_MATCH`。

#### 根因 C — 单字段标量被 surface 排序淹没（占 ≈ 25% MISS）

涵盖 `CVE-2024-27404` (`mptcp.remote_id`)、`CVE-2024-40953` (`kvm.last_boosted_vcpu`)、`CVE-2024-41005` (`napi->poll_owner`)、`CVE-2024-50082`、`CVE-2024-35999`、`CVE-2025-22050` 等。

这类 patch 只加了 `READ_ONCE / WRITE_ONCE / smp_*` 注解：

```text
+ if (READ_ONCE(napi->poll_owner) == smp_processor_id())  // CVE-2024-41005
+ list_del_init_careful(&curr->entry);                     // CVE-2024-50082
+ server = ses->chans[index].server;                       // CVE-2024-35999 (改顺序 + 加锁)
```

`VulnerabilitySurface` 的排序优先选写写冲突或多线程读写的"热点字段"。这种"一处常被写、其他处只是读"的标量，被同模块更显眼的字段（如 `kvm` 的 `dev_ops`/`usage_count` 等）挤出 5 个 hypothesis 名额。验证方式：CVE-2024-40953 的 5 个 hypothesis 名（`test_t0_t7`, `test_t5_t14`, `kvm_mm_race_device_ioctl`, `dev_ops_race_noalias`, `kvm_usage_count_inconsistent_lock`）没有一条提到 `last_boosted_vcpu`。

#### 根因 D — 跨文件 patch，Lace 单 .ll 视野不全（占 ≈ 10% MISS）

`CVE-2024-43891` patch 横跨 5 个文件 (`trace.h + trace_events.c + trace_events_hist.c + trace_events_inject.c + trace_events_trigger.c`)；`CVE-2025-37920` patch 横跨 4 个 (`xdp_sock.h + xsk_buff_pool.h + xsk.c + xsk_buff_pool.c`)。`prepare_cve.sh` 当前抽取规则是"包含 patch 文件之一的 translation unit"，相邻文件的协同修改对应的源代码没有进入 CCPG，bug 的两个 race 节点找不到对应的 IR/CPG 节点。

#### 根因 E — `Top-N hypothesis` 硬上限（系统性放大 B/C）

绝大多数 CVE 输出 5 条假说 (`5 hypotheses confirmed`)，少数为 7 / 9 / 12。即使 surface 找到 100+ 个 high-risk 对象，最终输出仍固定在 5。这是 B/C 类问题的"乘数"。

### 推荐落点（按 ROI 排序）

> 状态图标：✅ 已落地于 M7；🟡 部分落地；❌ 未落地。详见 §M7。

1. 🟡 **重构 `HypothesisVerifier` 为 happens-before / 原子性最小 DSL** —— 详细设计见 [`HYPOTHESIS_DSL_DESIGN.md`](./HYPOTHESIS_DSL_DESIGN.md)。
   - 理论起点：并发正确性 = HB 顺序 + 原子性（暂不含死锁）
   - 谓词词汇表：**5 个原语 + 3 个糖**（`same_location` / `op_kind` / `in_thread` / `reachable` / `hb`，糖 `conflicts` / `concurrent` / `unsafe_atomic_block`）
   - 框架特定知识（kfree / refcount / RCU / `*_FREED` flag / `local_bh_disable`/...）**全部沉到同步图的边构造**，不进入 DSL；新加 helper 只增表项不改谓词
   - 8 个 bug 家族都规约为 3 种判定模板：`conflicts ∧ concurrent`（F1-F4）/ `conflicts ∧ ¬hb` 单向（F5/F8）/ `unsafe_atomic_block`（F6/F7）
   - 表达力上限：现 6 谓词 ≈ 25% → 新 5+3 谓词 ≈ 94%
   每个谓词都能落到一次 CCPG / CFG / 同步图查询；LLM 认知负担最小化。**收益最大（≈ 70% MISS 可解）**。
   - **当前进度**：M7 Phase A MVP 已建出 HBGraph 同步图骨架（含 PROGRAM_ORDER / CALL_RETURN / LOCK_RELEASE_ACQUIRE / FORK_TO_ENTRY / JOIN_FROM_EXIT / COMPLETION 六类边），但 Phase B（Verifier 加 5+3 新谓词）与 Phase C（Prompt 切到新 DSL）**尚未落地**——HBGraph 暂时是死代码，对 recall/precision 还没贡献。
2. ✅ **修 CCPG 入口点解析** —— 让 `findMethod` 容忍 `static` 名、ops-table 成员名、文件路径前缀差异；A 类立刻全恢复。 *(M7 Phase E：`include/CPG/CPG.h::demangleVariants` Layer 3 + `findMethodSuggestions` Layer 5；`src/CCPG/CCPG.cpp::getMain` 失败时打印 closest-method suggestions。)*
3. ✅ **VulnerabilitySurface 排序加权** —— 给"同字段写少读多 + 缺 `READ_ONCE`"的标量加权，缓解 C 类。 *(M7 Phase D：`computeRiskScores` 新增 `has_scalar_torn_access` (+28)、`has_read_dominated_lone_writer` (+22)、`has_missing_atomic_annotation` (+18) 三类信号。)*
4. ✅ **解除"最多 5 hypothesis"硬上限**，按 token 预算自适应；解决 E 类系统性截断。 *(M7 Phase D：`VulnerabilitySurface::toPromptString(top_n, token_budget)` 双参数化，render 过程按 token budget 提前截断。)*
5. ✅ **`prepare_cve.sh` 切片以 patch 涉及的所有文件为种子展开依赖**，而不是单文件 + 邻接拼接；解决 D 类。 *(M7 Phase F：`scripts/patch_expander.py` 281 行，`prepare_cve.sh` step 0 + `batch_prepare.sh` 透传 `EXPAND_PATCH=1` 默认开启；产出 `expansion_report.json` 审计文件。)*

### 报告与日志

| 内容 | 路径 |
|---|---|
| 主评估报告（含 50 CVE 全部 verdict + per-bug 标签） | `evaluation_report.json` |
| 重判 10 个截断 patch 的对照报告 | `evaluation_report.recheck10.json` |
| LLM judge 实时进度 | `/tmp/entry_scan/eval_v2.log`、`/tmp/entry_scan/eval_recheck10.log` |
| Detection 全量重跑日志 | `/tmp/entry_scan/batch_full.log`、`/tmp/entry_scan/batch_retry.log` |
| 隔离的 CVE 与原因 | `_skipped_compile_issues/README.md` |

## M7 进度快照（2026-04-30，进行中）

> **目标提醒**：M7 的最终验收是 **HIT 数量** 与 **precision_strict**（M6 基线 13/50 = 26% recall，9.09% precision_strict，71.97% precision_lenient）。Phase 完成数是过程指标，不是验收指标。任何只增加基础设施却没接到 LLM 输出端的改动，对最终目标的贡献都是零。
>
> 实施计划详见 [`M7_IMPLEMENTATION_PLAN.md`](./M7_IMPLEMENTATION_PLAN.md)；DSL 设计详见 [`HYPOTHESIS_DSL_DESIGN.md`](./HYPOTHESIS_DSL_DESIGN.md)。

### Phase 落地状态 ↔ 对最终目标的实际贡献

| Phase | 内容 | 代码状态 | 对 recall/precision 的实际贡献 |
|---|---|---|---|
| **D** Surface 加权 + token-budget Top-N | risk_score 新增 3 类信号；`toPromptString(top_n, token_budget)` | ✅ 已落地 | 直接进入 LLM 输入，可缓解根因 C（≈ 5 个 surface 淹没 MISS）；待重跑验证 |
| **E** CPG `findMethod` 多层兜底 | `demangleVariants` (LLVM mangling)、`findMethodSuggestions` 相似度兜底；`getMain` 失败诊断 | ✅ 已落地 | 直接影响是否能进入分析；可解根因 A 的 2 个 zero_reports CVE；待重跑验证 |
| **F** patch-driven 跨文件展开 | `scripts/patch_expander.py` + `prepare_cve.sh` step 0 + `EXPAND_PATCH=1` 默认开启 | ✅ 已落地 | 影响 ≈ 6-8 个 ≥3 文件的 patch CVE；需重制实验目录后才显现，**当前 50 CVE 数据集仍是旧切片** |
| **A** HBGraph 同步图 | 6/10 类边已实现（PROGRAM_ORDER / CALL_RETURN / LOCK_RELEASE_ACQUIRE / FORK_TO_ENTRY / JOIN_FROM_EXIT / COMPLETION）；RCU / Refcount / 上下文区间 / Lifecycle flag 4 类边仍是空函数桩 | 🟡 MVP 已落地 | 已被 B/C 接入 `concurrent` / `hb` 求解；kernel module mode 下 PO+CALL 两类边已足够支撑跨线程 `concurrent` 判定（详见单 CVE canary） |
| **B** Verifier 新谓词 | `same_location` / `op_kind` / `eval_hb` / `eval_conflicts` / `eval_concurrent` / `eval_unsafe_atomic_block` | ✅ **已落地** (2026-05-01) | `HypothesisVerifier` 构造函数已注入 `HBGraph*`；6 个新 eval 全部实现，旧 6 谓词保留；canary 通过 |
| **C** DetectorAgent prompt + schema | system prompt 切到 5+3 谓词；`propose_hypothesis` schema enum 扩 8 项；移除 "stop at 5" 硬上限 | ✅ **已落地** (2026-05-01) | system prompt 重写为三模板（race / lifetime / atomicity_break）+ 三个 few-shot；schema 描述涵盖 14 谓词；canary 中 LLM 100% 切换到新 DSL |
| 编译 | Release 二进制 | ✅ 最新 | `Release-build/llm_detector` 时间戳 2026-05-01 23:53；含 D + E + F + A MVP + B + C |

### 已突破的局部最优陷阱

按工时算，M7 已投入约 5 天（D/E/F + A MVP + **B + C**）。截至 2026-05-01：
- ✅ 单 CVE canary（`CVE-2017-15265`）已验证 Phase B+C 完全生效——LLM 100% 切换到新 DSL，核心 CVE 命中扩展
- ⚠️ **M6 的 13/50 HIT、9.09% TP_MATCH、71.97% lenient precision 仍是当前已知的 SOTA**——M7 全量数字尚未实测
- 🔜 下一步：全 50 CVE 重跑 + `evaluate_recall.py` 三类判定，给出**第一组 M7 实测**

### 下一步优先级（按 ROI 重排）

1. ✅ ~~**Phase B + C**~~（已完成 2026-05-01，canary 通过）
2. ✅ ~~**重新编译 Release** + **canary 跑通**~~（已完成）
3. 🔜 **全 50 CVE 重跑 + LLM judge**（约 0.5 天 + LLM 调用时间）—— **当前下一步**。这一步给出第一组 M7 实测数字，并据此决定后续走向。命令：

   ```bash
   rm -f /home/LLM4Con/kernel_experiment/CVE-*/detection_hypothesis_batch.log
   cd /home/LLM4Con/kernel_experiment
   nohup bash /home/LLM4Con/scripts/batch_detect.sh \
       --api-key "<KEY>" --model "claude-sonnet-4-6" \
       > batch_run.log 2>&1 &

   python3 /home/LLM4Con/scripts/evaluate_recall.py \
       --api-key "<KEY>" --model "claude-opus-4-6" \
       --base-url "https://jeniya.cn/v1" \
       --output /home/LLM4Con/kernel_experiment/evaluation_report.m7.json
   ```

4. **看 M7 实测 MISS 残留**，再决定 A 完整版 4 类边里**哪些有性价比**——比如：
   - 若 lifetime 类 MISS 已经被 D 的 LIFECYCLE_FLAG_CANDIDATE + 现有 `¬hb(use, free)` 解决，§3.7 lifecycle flag 边**不必做**
   - 若 BH/IRQ 类 MISS（CVE-2024-41081 等）仍在，则补 §3.5 BH_IRQ_INTERVAL 边
   - 若 RCU 同步缺失类 MISS 仍在（rare），再补 §3.3 RCU_SYNC 边
   - **不要无差别把 4 类边都做**——每一类边有 100-200 行，只在数据要求时才上
5. **看 LLM 模板偏差**：canary 中 LLM 把所有 UAF 都包装成 `conflicts ∧ concurrent`（Template 1）而不用 Template 2 的 `hb` expected:false——可能压住 UAF 类的 TP_MATCH。若 M7 全量数据显示 UAF 类 strict precision 没改善，需要在 prompt 中把 Template 2 的 few-shot 提到最前面、或在 system prompt 加一条"UAF 类必须使用 hb expected:false 表达"的硬约束
6. **Phase F 重跑**（已落地但仅对**新制备**的 CVE 目录生效）：选 6-8 个 ≥3 文件 patch 的 CVE 重跑 `prepare_cve.sh` 让 `merged.ll` 包含全部 patch 函数，再单独跑这一批的 detector + LLM judge 看 D 类根因消除情况

### M7 验收条件（量化）

下次重跑全 50 CVE 后，README 应在此处填入下表：

| 指标 | M6 基线 | M7 目标（M7_IMPLEMENTATION_PLAN.md §5） | M7 实测 |
|---|---|---|---|
| HIT 数 | 13/50 (26%) | ≥ 25/50 (50%) | _待重跑_ |
| TP_MATCH | 24/264 (9.09%) | ≥ 50/300 (17%) | _待重跑_ |
| lenient precision | 71.97% | ≥ 78% | _待重跑_ |
| FP rate | 28.03% | ≤ 22% | _待重跑_ |
| 跨 ≥ 2 文件的 hypothesis 数 | ~0（无视野） | ≥ 1 / 跨文件 patch CVE | _待重跑_ |

### 报告与日志（M7 阶段新增）

| 内容 | 路径 |
|---|---|
| HBGraph dump（每 CVE 一份） | `<output_dir>/hb-graph.dot` |
| Patch 展开报告 | `kernel_experiment/CVE-*/expansion_report.json` |
| M7 完整重跑预留 | `kernel_experiment/evaluation_report.m7.json`（重跑后写入） |

### 单 CVE 回归基线（CVE-2017-15265）

#### 第一次跑（2026-04-30 11:44，仅 D + E + F + A MVP，旧 prompt + 旧谓词）

| 指标 | M5 baseline | M7 D+E+F+A MVP | 解读 |
|---|---|---|---|
| 总耗时 | 93s | **876s** | LLM 探索面变广导致调用次数增加 |
| LLM API 调用 | 12 | 40 | Phase D 的 surface 加权 + token-budget 让 LLM 看到更多对象 |
| Total tokens | ~378K | 1.14M | 同上 |
| Confirmed hypotheses | 5 | 3 | 数量下降 |
| 真实 CVE 命中 (`port_uaf_create_delete_race`) | ✅ | ✅ 保留 | 核心命中没退化 |

**HBGraph 实测边数（同次跑）**：

| 边类型 | 数量 | 解读 |
|---|---|---|
| `PROGRAM_ORDER` | 624 | 来自 CCPG `EdgeType::ORDER` |
| `CALL_RETURN` | 53 | 来自 CCPG `EdgeType::CALL` |
| `LOCK_RELEASE_ACQUIRE` | 0 | LSAnalysis 抓到锁但 release 端配对失败（kernel module 多为 acquire-only） |
| `FORK_TO_ENTRY` | 0 | kernel module mode 下 entry_points.txt 直接指定并行线程，**根本没有 fork node** |
| `JOIN_FROM_EXIT` | 0 | 同上，无 join node |
| `COMPLETION` | 0 | LSAnalysis 没识别到 `complete()`/`wait_for_completion()` |

> 即使锁/fork/join/completion 边都为 0，`hb(a, b)` 当 `a` 与 `b` 跨线程时**正确地返回 false**（两线程之间没有任何 PO/CALL 路径），所以糖 `concurrent(a, b)` 在 kernel module mode 下**仍能正确判定并发关系**。锁边的价值是**消除 FP**（双方都拿锁时把 race 排除），不产出 recall。

#### 第二次跑（2026-05-01 23:55，**完整 M7 = D + E + F + A + B + C**）

| 指标 | M5 baseline | A+D+E+F only (04-30) | **完整 M7 (05-01)** | 解读 |
|---|---|---|---|---|
| 总耗时 | 93s | 876s | **399s** ✅ | 比 Phase A-only 还快——新 prompt 紧凑 |
| LLM API 调用 | 12 | 40 | 36 | 与上次相当 |
| Total tokens | ~378K | 1.14M | 1.13M | |
| **Confirmed hypotheses** | 5 | 3 | **14** | DSL 上限解除 + 新谓词增加表达力 |
| 真实 CVE 命中（port + pool） | ✅ | 🟡 1 条 | ✅ **多条** (`port_private_free_uaf_race`, `port_private_data_race`, `port_event_input_race`, `cell_ext_len_chained_race`) | 全面命中 |
| 新 DSL 模板使用 | n/a | n/a | ✅ `atomicity_break` 类别 (`grp_count_nonatomic_rmw`) | Template 3 真的被采用 |

**LLM 实际使用的谓词分布**（69 个 constraint）：

| 谓词 | 次数 | 类型 |
|---|---|---|
| `in_thread` | 29 | 通用原语 |
| `op_kind` | 13 | **M7 新原语** |
| `conflicts` | 13 | **M7 新糖** (Template 1 / 2) |
| `concurrent` | 13 | **M7 新糖** (Template 1) |
| `unsafe_atomic_block` | 1 | **M7 新糖** (Template 3) |
| 旧谓词 (`may_run_concurrently`/`not_lock_protected`/`same_lock`/`alias`) | **0** | LLM 完全切换 |

**关键观察**：
- ✅ **Phase B+C 完全生效**：69 个 constraint 全部使用 M7 新 DSL，零旧谓词
- ✅ **核心 CVE 命中扩展**：M5 5 条里有 3 个 port_uaf_* 命中真 CVE；M7 14 条里覆盖 port_private_*, port_event_input_*, cell_ext_len_* 三个不同维度的 race，且加入 atomicity_break 模板首次出现
- ✅ **耗时反而下降**（876s → 399s）：新 prompt 更结构化，LLM 不再来回试错
- 🟡 **Template 2 (`hb` expected:false UAF) 没被用**：所有 UAF 类 hypothesis 仍被包装成 `conflicts ∧ concurrent`（Template 1）；这个偏差需要在 prompt 中加强引导，但不影响本次 canary 通过
- 🚧 **Hypothesis 数量 5 → 14**：可能让 precision_strict 略降，需要全 50 CVE 重跑后才能用 LLM judge 量化

**结论**：Phase A+B+C 全部接通成功。此 canary 不退化、新 DSL 全部启用、耗时反而下降。**满足进入全 50 CVE 重跑的条件**。

## CVE-2017-15265 验证结果（代表性单例）

### 漏洞概述
- **位置**：`sound/core/seq/seq_clientmgr.c` 与 `sound/core/seq/seq_ports.c`
- **竞态双方**：
  - Thread A：`snd_seq_ioctl_create_port`（创建端口后写入端口信息）
  - Thread B：`snd_seq_ioctl_delete_port`（找到并释放同一端口）
- **漏洞版本**：v4.13 及更早
- **修复 commit**：`71105998845f`

### 检测结果（开放假说模式）

成功检测到 **2 个高危假说**，均通过静态验证：

**Bug 1: `pool_free_unprotected_push_race` (data_race, high)**
- `snd_seq_cell_free` 中对 `pool->free` 链表的 push 操作无锁保护
- 验证通过：`in_thread` × 4 + `may_run_concurrently` + `not_lock_protected` × 2

**Bug 2: `port_free_vs_private_use_race` (uaf_race, high)**
- port 对象在 `port_delete` 中被 `kfree` 释放，另一线程仍访问 `port->private_data`
- 验证通过：`in_thread` × 2 + `may_run_concurrently` + `alias`
- 与 CVE-2017-15265 的真实漏洞高度吻合

| 指标 | 值 |
|------|------|
| 总耗时 | 93 秒 |
| LLM API 调用 | 12 次 |
| Token 消耗 | ~378K |
| 确认假说 | 2 |

## 已知问题 / 工程踩坑

### 编译期
- 部分内核版本 `make prepare` 无法生成 `asm-offsets.h`（需要 `modules_prepare` + 完整工具链）
- 非常老的内核（<3.x）缺少 `kconfig.h`、`compiler-version.h`
- `make allyesconfig` 在极少数版本上可能失败，fallback 到 `defconfig` 后会缺少子系统 CONFIG
- 不同内核模块间 `llvm-link` 可能因 `init_module` 符号冲突而失败（已通过重命名解决 CVE-2024-39503）
- `__copy` 和 `fentry` 宏在部分版本上导致编译失败（已通过 `-D'__copy(x)='` 和 `-DCC_USING_FENTRY` 缓解）

### 分析期（已修复，详见"批量运行的工程调整"）
- CPG 缓存错配（TargetPath 相对路径）→ 修复
- LockSet 分析 getArgOperand 越界断言 → 修复
- LLM 入口解析 guardrail 导致死循环与超时 → 修复
- Joern 无法解析内核注解丢失入口点 → 修复（`--define` 非侵入）
- NodeLoc 路径对齐失败（IR 与 CPG 前缀不同）→ 修复（basename fallback）

### 检测期（已部分缓解，详见 §M5 与 §M7）
- M5 五阶段优化已修：Syscall 宏入口归一化（部分）、`SharedFieldKey` 字段级聚合统一了 conflicting pairs / sharedObjects 口径、`getArgOperand` 加固使 core dump 5 → 2
- M7 D/E/F 已修：surface 排序加权、CPG `findMethod` 多层兜底、跨文件 patch 自动展开（详见 §M7）

### 检测期（仍在观察）
- **LLM 仍只能用旧 6 谓词**（B/C 未落地）—— 这是当前 recall 26% 的主要上限；M7 Phase B/C 完成后预期解 ≈ 70% MISS
- 复杂模块（线程数 50+）LLM 单会话 token 消耗较大；M7 Phase D 的 token-budget 截断已部分缓解，但 prompt 仍可能在 surface ≥ 100 obj 时偏大
- 部分 CVE 的真正 bug 发生在非 shared object（如局部对象跨线程传递），当前 surface 生成仍会漏掉
- Joern 对 `SYSCALL_DEFINE*` + `#ifdef CONFIG_NUMA` 组合在孤立源文件下展开不全；部分 CVE-2024-42234 等仍 CLEAN
- CCPG 分析阶段仍有 2 个 CVE（`CVE-2025-22050`、`CVE-2025-37772`）触发 PhasarPointerAnalysis 断言（需 gdb 复现）
