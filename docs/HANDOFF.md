# 新对话交接：LACE 源码主导 Agent / E-only 优化

更新：2026-10-12。**本文是当前交接入口**。旧清理版已保存到
[history/HANDOFF_20261011_CLEANUP.md](history/HANDOFF_20261011_CLEANUP.md)；
[SOURCE_LED_HANDOFF.md](SOURCE_LED_HANDOFF.md) 保留重构历史，不能把旧流程当成现状。

### 2026-10-12 初始上下文核验与清理

- 用户最新要求：可用的压缩应启用，不是强制关闭。七例重跑暂未启动，先核实压缩与初始注入。
- 当前同模型/既有网关配置重新测试 Responses：HTTP 400、错误码 -1013；只发送 `Return OK.`，未发送案例源码/GT/修复。原生 SDK Compaction 已接入，但这个配置尚不能实际启用；不要把“代码支持”说成“已启用”。后续保持 `LACE_CONTEXT_MANAGEMENT=auto`，不强制 off。
- 官方原生 compaction 使用 Responses；官方 Session 摘要示例属于应用侧策略，需要适配。尤其不能直接照搬按 user turns 触发的示例来处理 Phase A 内的一次用户输入及长工具循环。当前没有实现或启用 Chat Completions 客户端摘要压缩。
- 源码正文、完整 IR/AST、节点/对象清单、GT/修复均未自动注入初始请求。完整输入文件在工作区，源码与窄范围 IR 证据通过工具获取。后端读取源码用于锚点验证不等于模型初始输入。
- 已清理：Phase A 不再携带 Phase C 复核章节，只有实际启动 C 时切换到复核说明；协议目录的 bibliography/sources 元数据不进入初始消息，但完整目录与来源仍保存在账本。六类问题、Req/Gua 定义、绑定/消解规则未变。
- 实际 localhost 请求保存为 `initial-model-request.json`。OA/CAN 样例：Phase A 系统说明 14,475 字符；首次 user 输入分别 5,000/4,986 字节；16 个工具的 schema 共 17,116 字节；整个请求约 37 KB。这些是字节/字符，不是 token 或计费数；真实运行启用 repository 工具时接口定义会更多。
- 远端构建与 5 项 CTest 通过；OA E-only（含 Agent 绑定恢复）及 CAN A→E→C 的 localhost 全流程通过。C 说明按阶段切换且 SDK 历史延续。离线验证 0 真实模型请求；另有上述 1 次失败的真实接口兼容性请求。没有新的七例模型实验。
- 原始审计记录：远端 `.../lace-initial-context-audit-20261012/`；本地 `../analysis_outputs/lace-initial-context-audit-20261012/`。后续上下文仍会累积已读源码、IR 工具结果及契约轨迹；删除初始源码正文不能代替压缩。

### 2026-10-12 源码→IR 的 Agent 恢复（优先于后面的旧“歧义直接未知”说明）

- 用户授权：自动源码匹配失败但 IR 有候选时，让 LLM 检查并匹配；操作未进入 supplied IR 则保留未知。
- Phase A 新增 `bind_source_operation(source_id,node_id,reason)`。Agent 先用 `get_operations(source_id=...)` 查看候选指令，再明确选择；后端检查节点已经被查询、属于该源码坐标/函数、选择理由非空，不能创建不存在的节点。错误 selector 需重新注册正确源码锚点。
- 映射保存在原源码操作的 `agent_binding`；Req/Gua 保持 source ID。抽取反馈和 E 共用 `bindSourceOperation`，因此 Req、Gua evidence 和 source_regions 端点均可使用恢复后的节点。不是把自然语言理由再解析成规则。
- 保存自动绑定状态、选择理由、源码/指令快照和 `source_binding_revisions`。标明 `binding_origin=agent_selected`，不伪装成编译器匹配或语义/实例证明；不改变 atomic 等编译器事实，不产生同步 Gua。
- 可重新提交选择来修订；`node_id=-1` 撤回后恢复自动结果/未知。重新注册同一源码操作不清除既有选择。E 重放时重新验证快照；失效映射或缺失 CFG 继续未知。真正需要多个 IR 效果的操作不能随意挑一个代替全部操作。
- 当前同步图设计、Req 类型、E 消解规则、循环 occurrence 和 AtomicOp 类型体系未变。恢复提示要求处理已提交 Req/Gua 的匹配失败；IR 不存在或检查后仍无法对应的保留源码操作及 gap。
- 验证：远端完整构建、5 项 CTest 和 24 项 SDK/源码工作区/仓库 Python 测试通过；OA 原输入的 localhost 脚本完成“查看候选→选择→重新注册→撤回→重新选择→Req→E”，并验证抽取异常后仍使用已接受绑定执行部分 E。CAN 原输入的 localhost 完整流程通过。均为 0 真实模型请求，不是新召回实验。
- OA/ioprio 最新 native 账本分别保留副本做手工选择后的离线重放：OA 6735/6875 对应 mutex_lock_nested；ioprio 7808 对应 pointer-sized RCU 读取的 volatile i64 分支。OA gua:0 的绑定 eligibility 从 false 变为 true，但仍要经过原 E 的区域/关系检查；不能把匹配成功当成同步正确性证明。手工变体不是 Agent 实际抽取，原 native 数据及模型未修改。
- 独立完整记录：远端 `.../lace-source-binding-recovery-20261012/`；本地 `../analysis_outputs/lace-source-binding-recovery-20261012/`。原自动结果、选择理由/快照、脚本轨迹、baseline/manual-selected checks 和验证输出均单独保存。

### 2026-10-12 SDK harness 更新（优先于本文后面的旧输入流程说明）

- `d4c9b9d`：所有抽取异常均尽力对已接受 Req/Gua 执行部分 E；保留原始错误、非零退出和未完成状态，C 不启动。
- `cd81270`：复用 SDK `Compaction` 与 Runner 的输入 hook。Responses 模式自动启用，原始审计历史保留；不自建压缩 Agent 或模型/工具循环。
- 当前既有网关对标准 Responses 兼容性请求返回 HTTP 400（错误码 -1013），因此尚不能声称实际原生压缩已启用。待同一模型的可用 Responses 配置；密钥留在服务器，不写入交接。模型不变。
- 初始消息改为全量源码目录索引，不再注入全部源码正文。Agent 使用 SDK 原生 `exec_command/write_stdin` 阅读工作区 `module/`；完整列表在 `module_files.json`。旧 read_file/grep_source/list_dir 不再暴露。
- `harness/source_workspace.py` 只负责允许输入的工作区准备、Unix-local 的 Linux bubblewrap 隔离及原始工具轨迹，不重写 SDK shell、PTY 或 Runner。只暴露输入源码和只读实时契约，不挂载 GT、AST、全 IR、凭据或整个 Linux。外部依赖仍通过同版本 repository 工具查询。
- `contract_state.json` 是实时原始 Req/Gua 账本的只读视图；提交仍使用 add_requirement/add_guarantee。原子替换后的新版本可在下一次查询看到。完整 native 工具观察保存在 `sdk_source_tools.jsonl`，抽取错误也保留。
- `LACE_CONTEXT_MANAGEMENT=auto/native/off`；native 要求 `LACE_MODEL_API=responses`，否则明确报错。auto 在 Chat Completions 下明确记录未启用，不伪装有压缩。`LACE_COMPACT_THRESHOLD` 可选，默认使用 SDK 策略；不改变默认 max-turns=0。
- SDK/源码工作区/仓库 Python 测试、4 项 CTest，以及 Nouveau/CAN/PHY 三例输入的 localhost 全流程回归通过；没有新的真实案例模型实验。复用现有 CPG 缓存，未重新运行全部 Joern。最终测试数以独立验证汇总为准。
- 完整离线数据：远端 `.../lace-sdk-source-offline-20261012/194344`；本地 `../analysis_outputs/lace-sdk-source-offline-20261012/194344`。原始实验和第六项修复的早期回归均保留。
- 第 3/4/5 项设计见 [AGENT_RELATION_DESIGN_20261012.md](AGENT_RELATION_DESIGN_20261012.md)。正式结束条件和 E 的 resource/scope 文字匹配门槛尚未改变；Gua 独立关系层是设计，不是已实现结果。

## 1. 用户目标和最新约定（优先遵守）

目标：静态分析 + LLM 的可开源并发缺陷检测 Agent，随后检测真实漏洞。
当前以 LACE v2 工程为基础继续优化，源码主导抽取 Req/Gua，静态图用于 E。
不要重新使用不可靠的入口识别/共享对象 surface 来限制 Agent 的分析范围。

- Agent 阅读完整输入模块，可查询同版本 Linux 依赖；**不注入 AST**，也不注入整个 Linux。
- 静态图覆盖 supplied IR 的全部定义/指令以及 Joern 留下的源码方法，不按入口裁剪。
- Req 是并发安全需求；Gua 是代码实际提供的同步保障。普通本地顺序/API 前提不算同步 Gua。
- LLM 通过工具直接产生可解析 Req/Gua，不从自然语言 note 再猜规则。
- 维持统一的 `Req + Gua + CCPG → E` 检查流程；不要改成散落的事实清单替代 Req 消解。
- **当前实验关闭最终 Agent 复核（Phase C）**，优先看 E 是否产生对应候选。
- 优先分析漏报和通用实现问题，误报调优暂缓；多案例验证，不能只调一个 case。
- **人工读最终报告、轨迹、源码和历史修复来归因**。用户明确不认可自动 GT 标签/阶段标注代替判断。
- 归因关注：①相关节点是否在模型/是否正确绑定；②Gua 真实语义及作用域；③Req 真实逻辑及操作集；④E 如何构建并检查关系。
- **召回按大致匹配真实缺陷判断，不要求根因/类型/全部操作与 GT 完全一致。**
  仍要检查 Req/Gua 逻辑、节点绑定、同步关系和消解本身是否正确。
- 删除 Gua 后原候选仍存在是预期行为，**不能据此认定 E 无效或“蒙对”**；此前该推断已撤回。
- 用户认为当前模式可以继续。不要因不能给出完整形式化证明而推翻整套流程。
- 最新关于 AtomicOp：它应表达单次操作的实际原子性保障，而不是需求或泛泛的“属于某协议”。
  **用户说暂时保留，先看新实验，不要现在重构 AtomicOp 类型体系。**
- 不让 Cursor/Opus实现；用户希望当前 Agent 自己实现。未授权新开 subagent。
- 不设任意对象/参与者/代表采样上限。48 轮已影响正常完成，当前默认无硬性轮数上限。

2026-10-12 新对话最新约定：**先修源码→IR 对应实现问题**；use→free 的端到端 Req 合理，Gua 端点设计与循环问题暂缓。首次绑定修复与七例离线验证已完成，见 [SOURCE_BINDING_FIX_20261012.md](SOURCE_BINDING_FIX_20261012.md)。随后沿现有 `MustBeAtomic(S,I)` 强化事务抽取并完成三例，见 [TRANSACTION_EXTRACTION_20261012.md](TRANSACTION_EXTRACTION_20261012.md)。用户又授权修残留：互斥分支混串、宽锚点/对侧接收遗漏和作用域内 Gua applicability；反馈/修订接口及再一轮三例已完成，见 [CONTRACT_REPAIR_20261012.md](CONTRACT_REPAIR_20261012.md)。没有新增 Req/Gua 类型或更改 E 消解规则。
全部实验均已结束，没有仍在运行的模型队列需要接管。

## 2. 工作位置、版本和访问

本地镜像：`/Users/bytedance/Documents/并发缺陷检测/lace2-module-agent`。
该目录本身不是独立 Git 工作树；提交、构建在远端进行。不要在父项目误提交全部研究资料。

SSH：`122353`。
远端项目：`/mlx_devbox/users/mayunlong.39/playground/LLM4Con-v2-module-agent`。
分支：`lace-v2-module-agent`。
原七例 uncapped 实验使用的**代码**提交：`2d1b6a7c33cbf0badd50ad64c13f1382e8c34053`。
源码绑定修复提交：`19e779d4b8408cb0d59e18a02641ef17071138fa`，先完成七例旧账本零模型调用重放。
首轮事务抽取三例：`151593eed6e04523e50e99677fddf7061fb33cbe`。
最新残留修复三例实验：`d95026cbbd02b1e0dae504b52375385833ca84e0`（绑定/局部路径反馈、显式契约修订、分支/对侧/作用域提示）。
当前实现：`3ce3e18c091ef4cfcda5ff50480aca18eacbee5f`（实验结束后区分同一绑定 IR 操作与不同效果的循环路径；该诊断补丁只做零模型核验，不冒充已用于上述三例）。原七例版本不改写。
交接文档可能在其后单独提交；以 batch 中的 implementation_commit 标识实验版本。

运行/构建前静默加载：

```sh
source /mlx_devbox/users/mayunlong.39/playground/LLM4Con/setup_env.sh >/dev/null 2>&1
```

Python：`/mlx_devbox/users/mayunlong.39/playground/LLM4Con/.venv-lace3/bin/python`。
LLVM/编译器：LLVM 16，`/usr/bin/clang++-16`。
二进制：项目的 `Release-build/llm_detector`、`Release-build/contract_replay`。
Pinned 源码仓库：`/mlx_devbox/users/mayunlong.39/playground/linux.git`，已核对官方 kernel.org origin。
数据集：`/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment`。

SSH 偶发 `gssapi-with-mic` 失败，相同读/复制请求重试往往恢复；不要打印凭据或 setup_env 内容。
模型使用既有配置网关。已完成的七例只发送经版本核验的公开 Linux 源码/依赖，不发送 GT、修复或私有资料。
本轮曾有自动审批暂拒，核验公开源码和既有目的地后已获准执行；这是已解决状态。

Joern CLI 不在当前远端 PATH；案例重用有效 CPG 缓存。不能宣称本轮重新解析了所有源码。
现有 IR 的完整构建配置/版本来源仍未验证；源码字节已在 pinned revision 核验。

## 3. 当前实际流程

1. 构造 `module_model.json`：全部输入 IR 定义及指令、函数内 CFG、源码方法、直接调用导航和覆盖信息。
   这条路线不初始化 TCT/旧 HB/入口筛选；外部仓库可读，但未全部编译进模型。
2. Agent 接收带行号的完整模块源码、图覆盖概览、六类可扩展协议提示。
   不提供自动静态 Req、对象桶、thread/surface。IR 证据只通过窄范围工具按需查询。
3. Agent 注册源码操作，用 `add_requirement`/`add_guarantee` 写结构化契约。
   `record_protocol` 保存普通协议事实；`record_pattern_review` 是流程记账，不证明抽取完备。
4. E 绑定源码操作到实际 IR，根据有效 Gua 与 CFG 检查每条 Req，保存详细检查依据。
5. `--no-review` 直接由 E 失败组合输出 `static_contract_candidate`，origin=`deterministic_E`。
   完整模式仍保留同一 SDK 历史继续 Phase C，但本轮实验没启用。
6. 原始 Req/Gua、模型、SDK 轨迹、最终 `stateful_bugs/reports.json` 和 `bugs.txt` 均保存。

SDK 是官方 OpenAI Python + Agents SDK，C++ 执行静态工具，Python worker 负责模型/工具循环。
官方 SDK 版本：openai `3.26.0`，openai-agents `0.23.1`。
模型：`gpt-5.5-2026-04-24`；Chat Completions；reasoning 使用 provider default。

## 4. 关键实现位置与已完成修改

- `src/CCPG/ModuleGraph.cpp`：完整输入模型。
- `include/CCPG/IRSourceFacts.h` / `src/CCPG/IRSourceFacts.cpp`：DWARF/DataLayout 派生 memory_role、access_paths、内联源码位置链、memory_ordering、volatile 等。
- `src/LLMUtil/OperationBinding.cpp`：源码行范围 + kind/symbol/function + 编译器访问/调用事实。
  唯一语义匹配才 bound；不存在/模糊匹配保留 unknown。不是仅靠同一 debug 行选节点。
- `src/LLMUtil/ModuleContractAgent.cpp`：工具、契约、覆盖状态、E-only 输出。
- `include/LLMUtil/ContractFeedback.h` / `src/LLMUtil/ContractFeedback.cpp`：抽取期间的绑定/路径/applicability 诊断；显式替换保留 ID 与原始修订历史。不建立同步、不消解 Req。
- `include/LLMUtil/ModulePrompt.h`：明确抽取任务、并发 Req/Gua 定义、事务步骤/干扰区分。
- `src/LLMUtil/RequirementChecker.cpp`：纯 JSON E，source 绑定、Gua 适用性、CFG 关系、失败组合。
- `src/LLMUtil/ModuleAnalysis.cpp`：抽取→E→可选复核；预算耗尽时也检查已接受的部分契约。
- `harness/run_module.py` / `harness/run_batch.py`：实验编排、输入核验、冻结 manifest。
- `config/protocol_catalog.json`：v1.3，六类可扩展启发式问题；包含重复激活、逐个注册表、交接双方、互斥分支与作用域保障的检查提示。

`d90a229` 已实现后端语义绑定、E 解析源码操作、Gua 诊断、E-only、离线 contract_replay、重复源码别名核验去重。
`2d1b6a7` 已实现：

- runner 默认 `--max-turns 0`；0 表示无硬性轮数 cap，由 validated finish tool 结束。
- 正数 cap 仍可显式使用。MaxTurnsExceeded 后对已接受 Req 执行 E，跳过 C，保留 error/nonzero exit/partial_extraction，不能假装完整完成。
- 未解决 Req assumptions 不再跳过整条关系；有绑定的组合继续检查，状态仍可 requires_semantic_review。
- MustBeAtomic 的步骤未绑定、少于两个或顺序未建立时，不再靠剩余步骤产生假事务候选。
- 提示词澄清真实事务操作、对侧干扰、窄锚点；不以一个“代表注册表”替代其他已识别协议。

E 目前**不修改原始 CCPG**：Order/Wait 在查询中是虚拟有向关系，Exclude 是带 token/临界区的保护关系，AtomicOp 是个体原子效果兼容关系。
采用 Gua 前要求 resource/scope 字符串精确匹配、unconditional=true、全部操作与 evidence 可绑定。
Order/Wait 的端点集合按组合解释；Exclude 用 CFG 检查持续区域，内部不透明调用需要 effect summary。
MustPrecede 必须包含同步 Gua；普通同线程 CFG 顺序不能直接排除线程间冲突。

当前不支持一般跨过程 must-order/匹配返回、完整循环 occurrence 分析、独立别名/实例证明、弱内存形式化检查。
候选和局部 CFG 见证不是已复现/已证明缺陷；用户仍接受大致 GT 匹配作为召回。

## 5. 真实实验：原七例与后续两轮三例全部完成

**最新残留修复批次**：`d95026c`，远端 `.../lace-contract-repair-20261012/native`，本地 `analysis_outputs/lace-contract-repair-20261012/native`。北京时间 14:01:23–14:20:48；三例 exit 0、generation/E finished、implementation_unchanged=true；jobs=2、max-turns=0、Phase C disabled。OA 56 请求/4 报告，CAN 48 请求/8 报告，ioprio 51 请求/6 报告，共 155 请求。三个完整案例及独立 conditional probe 的 37 个文件已下载核验 SHA-256。

人工核对：OA 没有继续使用控制读写互斥步骤混串，保留控制事务调用之间的 mediation；消费槽位 Atomic 与对应 mediation 仍在，前者受循环限制。CAN req:2 的 FF 写 state 7754↔CF 读 state 7797 入 E 候选；req:3 的 CF 事务 S 与 FF/CF/timer 干扰全绑定，但循环可零次迭代导致写索引不支配后续读取。ioprio task_lock 的声明标为 true 且排除无锁 get_task_ioprio；其观察/解引用及 use/free 候选保留。

实际修订保存在 contract_revisions。OA 一次修订理由错误地把同一行不同效果说成重复；原始历史保留，之后补 same_resolved_operation 与 cyclic_local_path 区分及回归（`3ce3e18`），没有据此修改原生结果。Gua 正确声明不等于 E 完整消解：锁边界绑定、内部调用 effect summary、调用者前提仍有缺口。具体记录见最新报告，不能把报告数当正确漏洞数或正式召回提升。

**首轮事务抽取三例批次**：`151593e`，远端 `.../lace-transaction-extraction-20261012`，本地 `analysis_outputs/lace-transaction-extraction-20261012`。北京时间 13:19:32–13:39:37；jobs=2、max-turns=0、Phase C disabled，三例 exit 0、E finished、implementation_unchanged=true。183 次模型请求；34 个原始文件全部下载并核对 SHA-256。

OA 新 `req:0` 正确登记消费事务 S=[9092读 waiting,9095写 ongoing,9098清 waiting]、I=6972发送写，全部绑定，但循环反向可达造成 sequence_order_not_established，未进入 Atomic 检查；独立槽位冲突候选仍对应。ioprio 新 `req:0` 的观察→解引用与退出/free 干扰全部绑定，E 产生 4 个失败组合及对应候选。CAN 额外登记状态检查/timer取消事务，但 FF↔CF 分组仍未解决，接收状态报告仍是粗粒度对应。

人工检查还发现：OA 控制事务把互斥读写分支串成 S（旧账本也存在），不是循环问题；ioprio 受 task_lock 保护的取引用事务仍报候选，其匹配 Gua 被标为 unconditional=false 而拒绝采用。这些不能算建模已全面正确或新漏洞。具体绑定、源码、Gua 和 E 依据见上述事务抽取报告。三例都保留粗粒度对应报告，但不能据此声称提示词单变量召回提升。

**原始 48 轮批次**：
远端 `.../lace-e-stage-real-20261012`，commit `d90a229c041004da98c7d01e3e35f6bb06336fbd`。
本地 `analysis_outputs/lace-e-diagnostics-20261012/real/`。
四例（PHY/ioprio/KVM/OA）在 48 轮处停止，没有原生 E；另外三例正常完成。
离线部分账本 replay 是诊断实验，不改写原实验的 incomplete 状态。

**原七例无 cap 批次**：
远端 `/mlx_devbox/users/mayunlong.39/playground/lace-e-stage-uncapped-20261012`。
本地 `/Users/bytedance/Documents/并发缺陷检测/analysis_outputs/lace-e-diagnostics-20261012/uncapped`。
2026-10-12 北京时间 09:36:50–10:16:18；jobs=2；全部 exit 0、E finished、Phase C disabled。
全部确认 implementation_unchanged=true。原始完整数据已下载，不能删或覆盖。

| Case | 模型请求 | 最终 E 候选报告 | 当前人工 GT 对照 | 首要问题/依据 |
| --- | ---: | ---: | --- | --- |
| CVE-2024-26984 Nouveau | 48 | 4 | 对应候选 | req:6 报 maps/refcount 快路径下 map/ptrs 发布可见性。7个关键源码操作绑定；两个包装表达式仍未知。E不是弱内存证明。 |
| CVE-2022-48830 CAN | 42 | 5 | 粗粒度对应，可按用户宽松标准计召回 | req:4 最终报告指出接收回调并发、重组状态缺保护，与修复 rx_lock 的范围对应。但 A 放 FF/CF 写，B 主要放 timer/启动timer，未准确组成 FF↔CF 冲突；需继续检查真实 Req 操作逻辑。 |
| CVE-2024-35986 PHY/power | 62 | 8 | 未有对应最终候选 | 新 req:10 识别属性检查/回调与 unregister 的生命周期冲突，但两个序列操作未绑定，E 未评估成候选。其他 charger 缓存/本地 PHY 移除报告不是外部 provider 退役 GT。 |
| CVE-2016-7911 ioprio | 42 | 4 | 对应候选 | req:1 实际 read 6273 与 free 6478/7144 入最终报告。指针读取锚点仍歧义，但没有丢掉已绑定的生命周期组合。 |
| CVE-2024-40953 KVM | 67 | 6 | 未有对应最终候选 | 无 last_boosted_vcpu Req；图中关键读/写存在。轨迹将部分 directed-yield 字段泛化为近似启发式而不提 Req；不能说67轮完成就覆盖了全部代码。 |
| CVE-2024-56788 OA TC6 | 65 | 3 | 对应候选 | req:5 报 waiting_tx_skb 的发送写入与消费清空冲突。另有 req:1 Atomic，但包含未绑定 wake；该原子约束未完成 E 检查。 |
| CVE-2024-35898 netfilter | 54 | 2 | 未有对应最终候选 | 没生成 flowtable 类型查找/注销 Req。Agent 协议记录把 nfnl 锁、commit_mutex、模块引用过度推广到全局类型表，未追清释放锁后的查找。 |

按目前粗粒度口径：**3例明确对应 + CAN 1例粗粒度对应，3例漏报**。
这不是正式 benchmark recall；CAN 的操作组成问题需说明。不要沿用旧“必须匹配 rx.len 精确交错”的否判门槛。
不可以用此前严格评估的0/3与现在粗粒度结果直接声称算法提升。

总模型请求 380；累计输入 tokens 32,400,908，输出 227,219；单例约6.2–13.0分钟。
累计输入包含反复发送历史，不是独立代码量，也不是已确认的计费 token。
Req 共59条；E状态57条 requires_semantic_review、2条 undischarged。
**不能把状态全是 unknown 理解成完全没检查**：Req assumptions/部分绑定导致这个状态，但已绑定组合仍有候选。
coverage.unknown_positions 也会因关闭C而计入未复核 Req；不是自动阶段归因。

提示词/检查器与预算一起改变，运行随机性也存在，因此不是“只取消48轮”的受控消融。
但 PHY62/KVM67/OA65/netfilter54 的自然完成长度确实表明原 cap 会截断正常流程。

## 6. 已经人工核实的实现问题（不是自动标签）

### A. 源码到 IR 绑定存在通用缺口

PHY 新 req:10 的 source 操作包括 atomic_read、间接 psy->desc->get_property、atomic_dec_return、device_unregister。
实际 IR 已存在：

- 节点7279：atomic_read 内联展开的 `load volatile i32`，DWARF 内联链回到 core.c:1058；它不是 LLVM atomic instruction。
- 节点7299：通过 `%37` 的间接 callback call，core.c:1064；callee pointer 的读取节点7295具有 `psy.desc.get_property` 路径。
- 节点5979：device_unregister，core.c:1458，已成功绑定。

当前 binder 用 API 名+kind 对比直接 callee，不能正确识别内联原子 API/宏效果、间接 callee。
Inline pointer temporaries 的 access_paths 可能只有 `v`，无法直接对应 `psy->use_cnt`。
**不要把 Linux atomic_read 简单标成 LLVM atomic=true 来硬绑；不要随便取同一行候选。**
下一步可做一般 compiler-derived inline/callee/effect correspondence；先离线验证，再多case跑。
此缺口现已通过 `19e779d` 的通用编译器对应实现首次修复：7279/7299/5915 均唯一绑定，5979 保持绑定；原 req:10 离线进入 E，产生两个失败组合。没有手工指定节点，也没有新的模型抽取或 native 报告，不能直接声称新实验召回提升。原七例所有成功 Req/Gua 绑定均保留；宏/多个效果等剩余缺口仍可能 unknown。详见绑定修复记录。

### B. Gua Order/Wait 端点过度包含最终程序关系

详见 [IOPRIO_OA_CONTRACT_AUDIT_20261012.md](IOPRIO_OA_CONTRACT_AUDIT_20261012.md)。
这是原48轮 OA ledger 的**另一条退出生命周期 req:2**，不是 waiting-slot GT Req。
Gua语义在实际代码成立：kthread_stop 确实等线程退出，且在free之前。
但 Gua 的 A 是线程内操作、B已经是最终free；stop节点7891只在 evidence。
E直接采用 uses→free 虚拟边，U→U/F→F 的局部闭包使位置检查退化。

已完成零模型调用的独立诊断：

| Gua 终点 | CFG | E建立的顺序组合 | 未消解组合 |
| --- | --- | ---: | ---: |
| 原始free终点 | 原程序 | 12 | 0 |
| 原始free终点 | 合成CFG把stop放free后 | 12 | 0 |
| 手工改为stop返回节点7891 | 原程序 | 12 | 0 |
| 手工改为stop返回节点7891 | 合成CFG把stop放free后 | 0 | 12 |

模型/账本变体单独保留，**不是重编译后的内核，不是模型实际抽取，不是新漏洞**。
新对话可考虑让 Gua 描述同步 handoff 到返回点 W，CCPG 独立验证 W→F；不要让 Agent 直接把目标 Req 完整结论当同步边。
这一点已经定位但**尚未修改正式实现**。用户认为总体模式仍可用，不需要推翻架构。

### C. 循环内事务顺序过强

手工将 OA 正确消费事务 [9092读槽位,9098清空] 与干扰6972发送写交给现有 E，得到 sequence_order_not_established。
`Graph::before` 拒绝 b→a 可达，包括下一循环迭代；这不代表同一迭代中的 a→b 顺序不成立。
需要 occurrence/iteration-aware 的局部事务规则，不能直接全局放宽 HB。
这个问题已定位、**未修复**。新 OA 原子Req当前主要卡wake绑定，不能把两种现象混为一次测试。

### D. Agent 提前排除/漏提 Req

KVM：compound-state review 泛称 directed-yield spin/preempted 字段近似，没有 last_boosted_vcpu Req。
实际GT读/写节点23237/23426存在；不是静态模型没建出来，也不是E错误消解。

netfilter：Agent已查询 nfnetlink。其证据中 nfnetlink.c:428 已 nfnl_unlock，519 才调用 batch callback。
模块 source 的 type lookup 在 try_module_get 前已遍历；commit_mutex不能作为全局类型注销使用的同一锁。
目前是协议理解导致漏提 Req；没有对应类型注册表 Gua 被E错误消解的证据。

CAN：文字识别并发接收重组冲突，但操作两侧混入 timer API；这是 contract formalization/operand 选择偏差。
粗粒度报告可以算召回，逻辑/绑定检查仍应修，不因“召回了”就认为全部关系构造正确。

## 7. 测试与独立诊断已做什么

`2d1b6a7`：2项CTest通过；SDK/repository Python测试15项通过。
官方SDK测试包含 max_turns=0；单独脚本模拟API流程19次localhost请求正常结束，无真实模型调用。
预算耗尽回归12次localhost请求：Req已写入、E执行、C关闭、exit2和partial/incomplete保留。
这些只是工程测试，不作为缺陷召回。

原七例完整账本已离线 replay，结果单独保存在：

- 本地 `analysis_outputs/lace-e-diagnostics-20261012/replays/corrected_E/`
- 本地 `.../replays/original_E_partial/`
- 远端 `.../lace-e-replay-budget-fix-20261012/`

手工语义/端点/CFG probes：

- 本地 `analysis_outputs/lace-e-diagnostics-20261012/semantic-audit/`
- 远端 `/mlx_devbox/users/mayunlong.39/playground/lace-ioprio-oa-semantic-audit-20261012/`

`Release-build/contract_replay model.json contracts.json output.json` 只运行 binding/E，0模型请求。
输入副本、手工操作选择和模型变体须明确标注，原始native结果不能被它覆盖。
`harness/diagnose_case.py` / `tests/test_stage_diagnosis.py` 仍存在但本轮未作为判断依据；不要扩展自动GT标注方案。

## 8. 原始记录与实验输入

每例目录包含 `run.json`、`run.log`、`artifacts/<run>/module_model.json`、
`module_contracts.json`、`llm_simplified_trace.log`、`stateful_bugs/reports.json`/`bugs.txt`。
先读最终报告，随后按问题查工具轨迹、Req/Gua及E事实；不要只数约束数量。
batch.json 记录源码核验、IR hash、SDK/模型、prompt hash、代码版本、实际usage和完整状态。

`tests/e_stage_batch_plan.json` 固定这七个development输入：

| Case | IR | pinned Linux revision |
| --- | --- | --- |
| CVE-2024-26984 | nv50.ll | 4c08f01934ab67d1d283d5cbaa52b923abcfe4cd |
| CVE-2022-48830 | isotp.ll | 7db788ad627aabff2b74d4f1a3b68516d0fee0d7 |
| CVE-2024-35986 | merged.ll | 4201b8c8f2c32af321fb50867e68ac6c1cbed4be |
| CVE-2016-7911 | merged.ll | 74524955556096a0b2a821a49b4d0abebad3ee16 |
| CVE-2024-40953 | kvm_main.ll | 09b2d2a2267187336b446f4c08e6204c30688bcf |
| CVE-2024-56788 | oa_tc6.ll | b15b0df3967e5de43e1638bb152c575ae60062d1 |
| CVE-2024-35898 | nf_tables_api.ll | 994209ddf4f430946f6247616b2e33d179243769 |

GT在数据集的ground_truth.json，用于人工事后评估，不提供给Agent。
ioprio有flat与block/目录重复源码；只有canonical字节通过pinned校验后才省略重复Agent输入，保留aliases来源记录。

## 9. 下一轮建议顺序

通用对应修复、七例离线验证、事务抽取强化，以及残留修复/反馈/修订和两轮三例实验均已完成。用户明确将端点设计和循环修改暂缓；不能把旧建议当成全部已授权实施任务。

1. 核实远端 HEAD/工作树和保存的原始结果；不要为恢复上下文重跑已完成实验。
2. 继续人工查具体契约逻辑，利用 get_contract_diagnostics 和有历史的 replace_id 修订。最新实验已改善互斥分支、窄锚点、对侧接收及 task_lock applicability；不能宣称所有契约选择/条件已正确。
3. KVM/netfilter 的提前排除与漏提仍未修复；防止“近似字段”“框架有锁”“模块有引用”未经具体域核验就提前排除。
4. 后续改动提交并冻结，再按用户选定方向做多个真实 case 的 E-only 实验，人工读轨迹归因。

若用户恢复端点或循环工作，沿用已保存的正反顺序诊断与 OA/CAN 新原生事务；循环既有下一次迭代反向可达，也有零次迭代绕过步骤的支配问题。应处理同次事务 occurrence/路径前提，不能将全局可达性放宽成跨线程 HB。完整锁消解还需实际锁宏对应和不透明调用 effect summary；不能靠把全部 conditional 翻为 true 或要求最终 free 全部放进 task_lock 来掩盖。

不要现在重做AtomicOp、引入surface/自动入口裁剪、要求用户逐case补标注或给全部Linux预构巨大别名上下文。

## 10. 历史保留

`analysis_outputs/lace-experiment-archive`（本地与远端）包含论文full72、优化full72、旧三例完整输入/输出档案。
旧论文代码 `paper-thread-contract` → f810894，旧2.0 `lace-v2` →3ff95ba；实验精确版本以各manifest为准。
历史72例逐case控制台日志在，但部分更早原始SDK轨迹在清理前已缺失；不能补造“完整模型轨迹”。
当前新七例原始完整数据另存如上，不删除。更早清理过程和空间记录见 history/HANDOFF_20261011_CLEANUP.md。
用户提供的 concurrency_hunt 轨迹、论文、数据集/GT/IR和工具资料均不属于可随意删除日志。
