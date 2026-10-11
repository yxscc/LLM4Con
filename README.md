# LACE complete-module concurrency agent

当前分支只有一条检测流程：源码与匹配的 LLVM IR → 静态模型 → 通用 Req → 整模块 Agent 补充 Req/Gua → 静态检查提示 → 同一 Agent 复核所有 Req → 最终报告。

这次整理沿用 LACE 2.0 的静态前端与已验证的官方 SDK 路径。没有重新声称语义约束完备，也没有完成百万行规模验证。历史版本留在 Git 分支；本分支不兼容旧的 `--legacy-workflow`、`--agent-mode`、`--abl-contract` 和手写模型协议。

## 构建与运行

依赖 LLVM 16、Phasar、Graphviz、Joern、Java 17 和 Python。Python 依赖见 `requirements-harness.txt`。

```bash
./build.sh Release
python -m pip install -r requirements-harness.txt
export LLM_MODEL='<explicit model name>'
export LLM_API_KEY='<key>'
# Optional OpenAI-compatible endpoint, never written to the run manifest:
export LLM_BASE_URL='https://your-endpoint.example/v1'
python harness/run_module.py --source /path/to/module \
  --bitcode /path/to/matching.ll --output /path/to/new-run \
  --repository /path/to/linux.git --revision <full-source-commit> \
  --max-turns 96
```

`--repository` 和 `--revision` 成对使用：只读查询固定版本的完整依赖源码，不对整个 Linux 编译或把它全部注入提示词。`--autoconf` 接受构建 IR 时实际使用的配置头；不根据 case 猜配置。运行目录必须不存在。

单独检查静态模型不需要模型或凭据：

```bash
LACE_DUMP_ROOT=/path/to/new-static-output Release-build/llm_detector \
  --input-src /path/to/module --input-bc /path/to/matching.ll --static-only
```

可选 `--entry-config` 提供真实入口配置；默认自动识别，不加载数据集 GT。`LACE_MODULE_MAX_TURNS` 是每阶段运行预算；耗尽表示分析未完成，不会删除约束或采样对象。SDK 会保留完整对话，官方结果的完整输入列表用于下一阶段续接，参见 [Agents SDK results](https://developers.openai.com/api/docs/guides/agents/results)。

## 代码入口

| 文件 | 职责 |
| --- | --- |
| `src/llm_main.cpp` | 输入校验、静态图构建、显式客户端配置、阶段状态 |
| `src/LLMUtil/ModuleAnalysis.cpp` | 唯一工作流编排，直接连接模块约束账本与检查器 |
| `src/Query/VulnerabilitySurfaceGenerator.cpp` | 静态入口、对象和访问，按名称排序；没有风险筛选或伪造线程 |
| `include/LLMUtil/ModuleRequirements.h` | 集合形式的通用安全要求，保留全部操作 |
| `include/LLMUtil/ModulePrompt.h` | 明确的并发 Req/Gua 抽取与复核任务 |
| `src/LLMUtil/ModuleContractAgent.cpp` | 工具、约束校验、覆盖账本、复核和候选 |
| `src/LLMUtil/ModuleRepository.cpp` | 固定版本依赖阅读、源码操作证据、调用导航 |
| `src/LLMUtil/RequirementChecker.cpp` | 可独立测试的三类 Req 检查策略；每条结果均 `proof=false` |
| `src/LLMUtil/Conversation.cpp` / `SDKBridge.cpp` | SDK 完整会话与本地 JSONL 通信，不裁剪历史 |
| `src/Query/ModuleFinding.cpp` | 最终报告；只合并同资源同类型、完全相同位置集合的候选 |

Req 仅包含 `MustPrecede`、`MustBeAtomic`、`MustBeMediated`；Gua 仅包含 `Order`、`Exclude`、`AtomicOp`、`Wait`。类型合法、引用真实不等于语义正确，最终仍须复核。

## 输出与评估

- `stateful_bugs/reports.json`：最终报告的主结构化输出，零候选为 `[]`；保留每个贡献约束及其证据。
- `stateful_bugs/bugs.txt`：同一最终报告的可读版本，兼容已有目录布局。
- `module_contracts.json`：Req/Gua、检查提示、逐组复核、源码证据和未解决缺口；数量不是检出数。
- `llm_simplified_trace.log`：完整 SDK 输入、工具调用与结果历史；源码和工具结果视为不可信数据。
- `run.json`：模型、SDK 版本、代码 revision、输入与覆盖元数据；不保存凭据。

评价先比较最终报告与 GT 的缺陷机制、对象和出错时序，不能用约束数或出现相关函数代替命中。源码支持候选不等于已复现漏洞。

## 验证与继续工作

```bash
ctest --test-dir Release-build --output-on-failure
python -m pytest tests/test_sdk_worker.py tests/test_repository_context.py -q
python tests/offline_module_pilot.py --source /path/to/module \
  --bitcode /path/to/matching.ll --output /path/to/new-offline-run --source-anchors
```

离线 pilot 使用本地脚本模型，验证真实静态前端、SDK 工具循环、跨阶段续接、引用拒绝和最终报告；不评价检测能力。最新真实三个 case 的结果已冻结，不用本次清理的离线输出替代。

完整交接、算法边界、历史与数据缺口见 [docs/HANDOFF.md](docs/HANDOFF.md)。
