# 任务：基于 v22 attribution 数据优化 Lace，下一轮提升 recall / precision

你是接手 Lace（LLM-Enhanced Concurrency Vulnerability Detector）项目的工程师。
上一轮跑完了一次完整的 100-CVE 检测 + 17-step 细粒度归因评估
（tag = `v22`），数据已经摊在磁盘上。你的任务是**根据这份归因数据**，
按 ROI 顺序修 detector，每改一处先在 1–2 个目标 CVE 上做 sanity，
确认信号后再全量重跑。

----------------------------------------------------------------------
## 0. 工作环境 / 一定要先做的事

1. **永远先 source 环境**：
   ```bash
   cd /mlx_devbox/users/mayunlong.39/playground/LLM4Con
   source setup_env.sh
   ```
   这里面有 `EXPERIMENT_BASE`、`LLM4CON_HOME`、`PHASAR_INSTALL_DIR`、
   `LLM_API_KEY/LLM_BASE_URL/LLM_MODEL=gpt-5.5-2026-04-24` 等所有
   关键变量。**禁止改 setup_env.sh，禁止改 LLM_MODEL，禁止改 git config。**
2. **不要重装/重编 Phasar 或 Joern**。它们装在 `LLM4Con/external/`，
   编译参数和 LLVM-16 toolchain 已经搭好，碰一下会塌一片。
3. **build 时只 build 自己改动的部分**：
   ```bash
   cmake --build Release-build -j$(nproc)
   ```
   首次 build 会很久（~10 分钟），之后增量编译大约 30s–2min。
4. **任何 `make defconfig` / `make modules_prepare` 失败都不要 panic**——
   `scripts/prepare_cve.sh` 已经把 KCFLAGS 加好了，让旧内核能在
   bookworm + clang-19 下出 IR。CVE 目录里只要有 `*.ll`（或 `merged.ll`）
   就够 detector 用，不需要 `vmlinux`。

----------------------------------------------------------------------
## 1. 项目代码地图（用 Glob + Read 看，不要 `cat` 整棵树）

### 静态分析后端（C++，最容易出 root cause）

| 文件 | 角色 |
|---|---|
| `src/CCPG/CCPG.cpp` | Code Property Graph 构建 + 入口节点登记 |
| `src/CCPG/ThreadCreationTree.cpp` | Thread set 推断（kthread / workqueue / sysfs / syscall / ioctl / socket-cb …）。**A2 的核心。** |
| `src/CCPG/HBGraph.cpp` | Happens-before 图：lock edge、LIFECYCLE_FLAG edge、RCU_SYNC edge。**D3 的核心。** |
| `src/CCPG/LSAnalysis.cpp` | Lockset 分析（context-aware）。**D4 的核心。** |
| `src/Query/VulnerabilitySurfaceGenerator.cpp` | 从 Phasar pts 结果推 shared object，给 risk score。**B1 / B2 / B3 的核心。** |
| `src/Query/SharedFieldKey.cpp` | struct/global field canonicalization（含 opaque pointer 类型反推、list-helper 合成、`fromValueAllAliases`）。**B1 的另一半。** |
| `src/Query/HypothesisVerifier.cpp` | 8 个静态谓词：in_thread / conflicts / hb / concurrent / same_lock / op_kind / reachable / unsafe_atomic_block。**D1–D5 的核心。** |

### LLM Agent 前端（C++，注意是 prompt + 工具调度，不是 inference）

| 文件 | 角色 |
|---|---|
| `src/LLMUtil/DetectorAgent.cpp` | Phase 2：从 surface 提 hypothesis（target / pattern / threads / sites）。**C1 / C2 / C3 / C4 的核心。** |
| `src/LLMUtil/ParallelAnalysisAgent.cpp` | Phase 2 子任务：判断 "CAN/CANNOT run concurrently"。 |
| `src/LLMUtil/VerificationAgent.cpp` | Phase 4.5：对 confirmed hypothesis 做 LLM KEEP/DROP。**E1 / E2 的核心。** |
| `src/Query/StatefulBugDetector.cpp` | 把上面几个 phase 串起来。 |
| `src/llm_main.cpp` | detector 程序入口。 |

### 评估器（Python）

| 文件 | 角色 |
|---|---|
| `scripts/evaluator_agent.py` | v22 评估器。三阶段 Plan A+D：Phase A 共享事实 → Phase B 每个 bug 一个 sub-conversation → Phase C CVE 合成。**输出 17-step attribution，严格 binary HIT/MISS、TP/FP**。schema 在文件头部 `ATTRIBUTION_STEP_IDS` / `CVE_VERDICT_SCHEMA` / `BUG_VERDICT_SCHEMA` 下面，prompt 在 `_ATTRIBUTION_STEP_GUIDE` + `SYSTEM_PROMPT_PHASE_B` + `SYSTEM_PROMPT_PHASE_C`。**只修 prompt / schema / aggregator，不要重写整个文件**。 |
| `scripts/detect_shard.sh` | 单 worker detector 跑批；**注意 line 69–75 的 "skip if log exists" 规则**：要重跑 detector 必须先把现存 `detection_hypothesis_batch.log` 备份成 `.xxx.log`。 |
| `scripts/run_overnight_v22_parallel.sh` | 4-way 并行 detector + 4-way 并行 evaluator + merge。脚本里已经有 `STALE` 备份逻辑兜底，但还是建议手动先把旧 log rename 干净。 |
| `scripts/batch_detect.sh` | 单进程版本，sanity 用 / 不想并行时跑这个。 |
| `scripts/prepare_cve.sh` / `scripts/batch_prepare.sh` | 把 CVE 源码 → bitcode。**不要轻易动**，旧内核 + bookworm 的 KCFLAGS / header shim 已经调过很多次。 |

### 数据布局

```
LLM4Con/
├── kernel_experiment/
│   ├── CVE-YYYY-NNNNN/             # 100 个 CVE/SYZBOT 之一
│   │   ├── ground_truth.json       # 真值：racy 字段 + patch + 描述
│   │   ├── src/                    # patched-pre 源码
│   │   ├── *.ll / merged.ll        # 给 detector 吃的 IR
│   │   └── detection_hypothesis_batch.log         # ← detector stdout
│   ├── SYZBOT-<hash>/              # 同上，但来自 syzkaller reproducer
│   ├── evaluation_report_agent_v22.json    # ★ 上一轮评估结果（4 MB）
│   ├── ATTRIBUTION_2026-05-12.md   # 历次优化的归因 + 经验教训
│   ├── HANDOFF.md                  # 项目初始 onboarding（环境/工具/已知坑）
│   ├── CASE_STUDY_CVE-2024-43830_MISS.md  # 一个 MISS 的逐步追踪示例
│   └── shards_v22/                 # 上轮 worker 日志 + 分片 JSON
└── LLM_dump/
    └── <CVE>_<ts>/                 # 每次 detector 跑都写一个时间戳目录
        ├── vulnerability_surface.json     # P1 surface
        ├── confirmed_hypotheses.log       # P3 通过的 hypothesis
        └── stateful_bugs/bugs.txt         # P4.5 KEEP 后给用户的 bug
```

----------------------------------------------------------------------
## 2. 上一轮（v22）评估结果速读

**文件**：`kernel_experiment/evaluation_report_agent_v22.json`（4 MB JSON）

顶层结构：
```json
{
  "timestamp": "...",
  "model": "merged_from_shards",
  "summary": { ... 见下 ... },
  "details": [ { "cve_id": "...", "recall": "HIT|MISS",
                 "bug_verdicts": [...],
                 "attribution": { "steps": [...], "primary_blocker_step": "..." },
                 "fp_root_causes": [...], ... }, x100 ]
}
```

### 当前盘面（这是你优化的起点 baseline）

| 指标 | 数值 |
|---|---|
| Recall (binary) | HIT **15** / MISS **85** / ERROR 0 = **15%** |
| Per-bug | total 588, TP **21**, FP **567**, precision **3.57%** |
| ERROR | **0**（Plan A+D 把 long-context collapse 完全压住） |
| 子集对比 | CVE 子集 14/64 = 21.9%；SYZBOT 子集 1/36 = 2.8% ← **SYZBOT 是大坑** |

### 17-step primary_blocker 分布（85 个 MISS 的 root cause 投票）

```
B1.shared_field_extraction      26    surface 漏了 patched 对象
C1.target_object_selection      25    Phase 2 LLM 在 surface 里挑错
A2.thread_set_coverage          15    入口 (sysfs / ioctl / cb) 没识别
C2.hypothesis_pattern_fit        7    Phase 2 pattern 选错
B3.risk_scoring                  5    对象在 surface 但 rank 太低
C4.access_site_correct           5    Phase 2 选错 file:line
B2.cross_thread_aggregation      1
A1.build_pipeline                1
```

### `fixable_at` 票数（FAIL 时 LLM 指认的 Lace 组件）

```
DetectorAgent_prompt            997  ← 主战场
VerificationAgent_prompt        640
HypothesisVerifier              187
HBGraph                         106
LSAnalysis                       99
VulnerabilitySurfaceGenerator    75
ThreadCreationTree               24
AliasChecker                     23
SharedFieldKey                   11
PointerAnalysis                  10
```


## 4. 工作循环（每改一处就走一遍）

### 4.1 选 case + 看证据

```bash
# 例：想看 primary=B1 的 CVE 的 attribution 证据
python3 -c "
import json
d = json.load(open('kernel_experiment/evaluation_report_agent_v22.json'))
for r in d['details']:
    if (r.get('attribution') or {}).get('primary_blocker_step') == 'B1.shared_field_extraction':
        print(r['cve_id'])
        for s in r['attribution']['steps']:
            if s['status']=='FAIL' and s['step_id'].startswith('B'):
                print(f'  {s[\"step_id\"]} | {s.get(\"observed\",\"\")[:120]}')
                print(f'    evidence: {s.get(\"evidence\",\"\")[:140]}')
"
```

证据里 LLM 会引用到 `file:line` / `shared_facts.candidate_surface_objects` /
具体 hypothesis index。**先信证据，再去翻源码。**

### 4.2 实施改动

- C++：改 `src/...`、`cmake --build Release-build -j$(nproc)`。
- Prompt：改 `src/LLMUtil/DetectorAgent.cpp` 的 `build_system_prompt()` 后
  也得 rebuild（prompt 是嵌在 .cpp 里）。

### 4.3 Sanity（**强制 1–2 CVE**）

```bash
# 选一个目标 CVE，备份旧 log
CVE=CVE-2024-26862
mv "$EXPERIMENT_BASE/$CVE/detection_hypothesis_batch.log" \
   "$EXPERIMENT_BASE/$CVE/detection_hypothesis_batch.v22_baseline.log"

# 单跑 detector
cd "$EXPERIMENT_BASE/$CVE"
$LLM4CON_HOME/Release-build/llm_detector \
    --input-bc "$(ls *.ll | head -1)" --input-src src \
    --agent-mode --llm-provider openai \
    --llm-url "$LLM_BASE_URL" --llm-key "$LLM_API_KEY" \
    --llm-model "$LLM_MODEL" 2>&1 | tee detection_hypothesis_batch.log

# 单跑 evaluator（出 v22 attribution）
python3 $LLM4CON_HOME/scripts/evaluator_agent.py \
    --cve "$CVE" --force --max-iterations 60 \
    --output /tmp/sanity_${CVE}.json

# 看 attribution 是不是按预期翻盘
python3 -c "
import json; r=json.load(open('/tmp/sanity_${CVE}.json'))['details'][0]
print('recall:', r['recall'])
print('primary_blocker:', (r.get('attribution') or {}).get('primary_blocker_step',''))
for s in (r.get('attribution') or {}).get('steps',[]):
    print(f\"  {s['step_id']:30s} {s['status']}\")"
```

如果 sanity CVE 的 `primary_blocker_step` 改变了 / `recall` 从 MISS → HIT，
说明改动起作用了。**不要靠"看上去合理"判断，要看证据**。

### 4.4 全量重跑

```bash
# 1) 备份所有旧 detector log（防 detect_shard.sh skip）
for f in $EXPERIMENT_BASE/CVE-*/detection_hypothesis_batch.log \
         $EXPERIMENT_BASE/SYZBOT-*/detection_hypothesis_batch.log; do
    [ -f "$f" ] && mv "$f" "${f%.log}.v22_baseline.log"
done

# 2) 启动 4-way 并行（约 4-5 小时）
cd $LLM4CON_HOME
nohup bash scripts/run_overnight_v22_parallel.sh \
    > $EXPERIMENT_BASE/overnight_v23.log 2>&1 &
echo "PID=$!"
```

**注意脚本里 TAG 写死是 "v22"**——如果你想要新的输出文件名
（`evaluation_report_agent_v23.json` / `shards_v23/`），改一下 `TAG="v22"` → `TAG="v23"`，
不要直接覆盖旧 report。

### 4.5 对比

```bash
python3 - <<'PY'
import json
a = json.load(open('kernel_experiment/evaluation_report_agent_v22.json'))['summary']
b = json.load(open('kernel_experiment/evaluation_report_agent_v23.json'))['summary']
print(f"recall: v22 {a['recall']} → v23 {b['recall']}")
print(f"per-bug: v22 tp={a['per_bug']['tp']} fp={a['per_bug']['fp']} "
      f"→ v23 tp={b['per_bug']['tp']} fp={b['per_bug']['fp']}")
print("primary_blocker 变化:")
m = {r['step_id']:r['primary_blocker'] for r in a['attribution_steps_cve']}
for r in b['attribution_steps_cve']:
    d = r['primary_blocker'] - m.get(r['step_id'],0)
    if d != 0:
        print(f"  {r['step_id']:32s} {m.get(r['step_id'],0):3d} → {r['primary_blocker']:3d}  ({d:+d})")
PY
```

----------------------------------------------------------------------
## 5. 必读历史文档（按优先级）

1. **`kernel_experiment/ATTRIBUTION_2026-05-12.md`** —— 历次优化的根因 + 实施记录。
   - 附录 D (v17), E (v19), F (v20 P7), G (v20 P8), H (v20-fix) 是最近的真改动。
   - **附录 H.8 经验教训必读**（4 条 lesson）。
2. **`kernel_experiment/HANDOFF.md`** —— 项目初始 onboarding（环境、Joern、Phasar、已知坑）。
3. **`kernel_experiment/CASE_STUDY_CVE-2024-43830_MISS.md`** —— 一个完整的 MISS 逐步追踪示例，可学习证据组织风格。
4. **`scripts/evaluator_agent.py`** 文件头部注释 + `ATTRIBUTION_STEP_IDS` /
   `_ATTRIBUTION_STEP_GUIDE` 段——理解 17 step 各自的 PASS/FAIL 边界。

----------------------------------------------------------------------
## 6. 安全护栏

- **不要**改 `setup_env.sh`、`LLM_MODEL`、`scripts/prepare_cve.sh`、
  `external/phasar`、`external/joern-cli`、`.gitignore`。
- **不要**主动 `git commit / git push`，除非用户明确叫你 commit。
- **不要**为了一时方便去 silent fallback——例如把"找不到对象" 默默
  当成"PASS"。Lace 是研究系统，**沉默错误比显式失败更难追**。
- **不要**新增 evaluator 评估类别（HIT/MISS/TP/FP 是 v21 + v22 敲定的二值，
  不要回归 PARTIAL/TP_RELATED）。

