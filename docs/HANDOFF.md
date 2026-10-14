# LACE 当前交接：源码契约 Agent / E-only 全量基准

更新：2026-10-14。**此文是当前交接入口，以本轮全量加4例补跑的最终结果为准。**
结果报告见 [FULL72_BASELINE_20261014.md](FULL72_BASELINE_20261014.md)，可移植逐例记录见
[results/source_contract_full72_20261014.json](results/source_contract_full72_20261014.json)。
此前交接全文已归档到 [history/HANDOFF_20261013_BEFORE_FULL72.md](history/HANDOFF_20261013_BEFORE_FULL72.md)；
[SOURCE_LED_HANDOFF.md](SOURCE_LED_HANDOFF.md) 仅保留重构历史。旧文中的“当前实现”“未启动”“最新结果”均是当时状态。

## 1. 已确认的当前状态

- 当前实现分支：`codex/source-contract-artifacts`。代码提交 `18ab94cdac000190e30f935e7c347a533ccc6aaa`；实验 manifest 的 HEAD 是 `4ab053aaece564ff4fb09a016bf9f35ca7dabf2b`，其相对代码提交只增加文档。此次发布继续保持这份实现，只整理文档、输入计划与人工结果基准。
- 全72例中71例可运行，71例最终选定运行全部 exit 0、两次源码抽取完成、E finished、implementation_unchanged。4例首轮网关429中断在独立目录补跑，原始失败结果和首轮判断均保留。
- 按用户约定的大致匹配口径：**13明确对应 +17粗粒度对应 =30/71（42.3%），41例漏报**。按全72例分母为30/72（41.7%）；不把未运行例伪装成已评估漏报。
- `CVE-2017-15265` 的 `snd_core.h` 与 pinned revision 不一致、0个canonical字节匹配。用户选择保留原输入、记未完成；不替换源码，不补造IR或判断其GT在此输入中一定存在/消失。
- 最终采用的71份运行共847条E候选、1070条Req。报告数不是召回数。粗粒度对应也计召回，但Req操作集、Gua、绑定及E关系的科学性缺口另行保留。
- 全部E-only，Phase C关闭；当前**没有仍在运行的模型队列**。不需要通过重跑恢复上下文。

## 2. 用户目标与约定

目标是静态分析 + LLM 的并发缺陷检测工具，再用于真实漏洞发现。以源码主导构建传统静态分析难处理的Req/Gua，保持统一 `Req + Gua + CCPG → E` 流程。

- 不要求完美同步建模或找到所有问题；召回允许大致匹配真实缺陷，不要求根因、类型、全部GT操作完全一致。仅同模块/字段而安全需求不同的候选不计。
- 科学性检查仍关注相关节点是否存在/正确绑定、Gua真实同步语义与作用域、Req真实逻辑/操作集、E构建与检查的关系。不能因计入召回而略过这些问题。
- Agent可读完整供应模块及同版本Linux依赖。**不把源码正文、AST或整个Linux直接注入初始提示词**；源码经SDK文件工具读取，IR经窄范围工具按需查询。
- 不使用入口识别、共享对象surface、旧TCT/HB来限制分析；图覆盖全部supplied IR定义/指令及Joern保留的源码方法。不存在任意对象/参与者/代表采样上限。
- Req是并发安全需求；Gua是代码实际提供的同步。普通本地顺序/API前提不是同步Gua。结构化Req/Gua直接保存，不从自然语言notes反推规则。
- 优先分析漏报和通用问题，误报调优暂缓；多案例验证，人工读报告、轨迹、源码和修复，**不用自动GT匹配/阶段标签代替判断**。
- 删除Gua后候选仍存在是预期行为，不据此否定E。不能因缺少完整形式化证明而推翻已接受的方法。
- `MustBeAtomic(S,I)`沿用现有类型：同实例、同次事务执行中，指定干扰不得插入事务步骤之间。观察依赖说明为什么选S/I，不生成同步边；互斥分支不能硬串成事务。
- `AtomicOp`暂保留，应表达单次操作的实际原子性，不改造成需求或泛协议归属。Gua端点设计和一般循环occurrence问题仍暂缓，不声称本轮已解决。
- 默认 `--max-turns 0`，无硬性轮数cap；可显式使用正数。中断仍检查已接受契约、保留非零退出和partial，不伪装完整。
- 当前Agent负责实现，不交给Cursor/Opus。用户已授权逐例并行审阅，本轮复用了三个审阅子任务；这不等同于要求后续任意新开实现子任务。

## 3. 实际工作位置

本地镜像：`/Users/bytedance/Documents/并发缺陷检测/lace2-module-agent`，**不是独立Git工作树**；不要在父项目提交全部研究资料。

SSH：`122353`。当前远端Git工作树：
`/mlx_devbox/users/mayunlong.39/playground/LLM4Con-req-first-workflow`，分支 `codex/source-contract-artifacts`。
origin为 `git@github.com:yxscc/LLM4Con.git`。原 `LLM4Con-v2-module-agent` / `lace-v2-module-agent` 留作历史，不作为当前实现。

运行/构建前静默加载：

```sh
source /mlx_devbox/users/mayunlong.39/playground/LLM4Con/setup_env.sh >/dev/null 2>&1
export PATH="$PWD/external/joern-cli:$PWD/external/joern-cli/bin:$PATH"
```

Python：`/mlx_devbox/users/mayunlong.39/playground/LLM4Con/.venv-lace3/bin/python`。
LLVM/编译器：LLVM16、`/usr/bin/clang++-16`。二进制：`Release-build/llm_detector`、`Release-build/contract_replay`。
实验detector SHA256：`5ef7883c13e02e0483b83b41fa72ca61ee22d788af98bf41e91df48616877c04`。

Pinned Linux仓库：`/mlx_devbox/users/mayunlong.39/playground/linux.git`（非bare目录，使用`git -C`）；已核验官方origin。
当前受控输入：`/mlx_devbox/users/mayunlong.39/playground/lace-controlled-dataset72-20261012/dataset`。
旧数据/GT：`.../LLM4Con/kernel_experiment`。GT与修复仅用于事后复核，不传给检测Agent。
SSH偶发认证失败可重试相同读/复制请求，不打印凭据或setup_env内容。

## 4. 输入范围与当前流程

输入是**受控完整文件功能单元**：按核验的源码种子/直接组件成员选择完整C文件及相应头文件，再构建这些翻译单元的IR；不是按入口/GT节点裁剪，也不是整个驱动文件夹、所有Kconfig组合或整个Linux。
71对源码/IR及编译配置已发布，案例范围、构建尝试与hash保存在受控数据集manifest；仍不能将字节核验和一次构建称为所有配置/外部依赖的完整语义覆盖。
冻结运行计划为 [../tests/source_contract_full72_plan.json](../tests/source_contract_full72_plan.json)。

1. 构建`module_model.json`：全部输入IR定义/指令、精确函数内CFG、源码方法、直接调用导航及compiler-derived访问/内联事实。外部Linux可查询，但不全部编译进模型。
2. 初始请求给目录索引、图覆盖、任务与可扩展协议提示，不含源码正文/AST/全图节点表。
3. SDK原生`exec_command/write_stdin`在隔离工作区读源码；仅`contracts/`可写。一文件一条Req/Gua JSON，操作内联`path,line,kind,expression`，可加`function,end_line`。
4. 后端增量导入、结构校验、自动ID、一般绑定，同一次shell返回附紧凑反馈。修订/删除保留历史；非法编辑保留最后接受版本并阻止结束。notes不解析为规则。IR查询/显式对应恢复可选，真正无IR或无法确定仍unknown。
5. 默认两次Phase A：先从共享状态/使用与失效提出契约，再在同一SDK历史做源码需求复查，继续提交/修订。第二次不是Phase C、不看E/GT，也不要求固定数量新增Req。`--requirement-sweep off`可显式对照。
6. E绑定源码操作并逐条检查Req；假设/部分绑定未知仍检查有依据的组合，保留状态与gaps。E-only直接输出`static_contract_candidate`、origin=`deterministic_E`。
7. 完整模式可显式继续同一SDK历史的Phase C；**当前基准没有启用**。工作流完成不是全源码读完、Req抽取完备或安全证明。

官方SDK：openai `3.26.0`、openai-agents `0.23.1`；模型 `gpt-5.5-2026-04-24`，Chat Completions、provider-default reasoning。
`LACE_CONTEXT_MANAGEMENT=auto`保留原生压缩支持，但此网关的Responses兼容性不足，**本轮未实际使用compaction**。没有自制摘要丢弃历史；71例完成不等于所有未来输入都无上下文风险。

## 5. 实现与检查边界

- `ModuleGraph.cpp` / `IRSourceFacts.cpp`：全输入图与DWARF/DataLayout访问、callee、inline/effect对应。
- `OperationBinding.cpp`：源码坐标+kind/symbol/function+compiler facts；唯一语义匹配才bound。窄statement可使用唯一类型效果，不忽略已知字段冲突、复杂/多效果或错误函数限定；错误限定给出实际compiler函数名供修订。
- `ModuleContractAgent.cpp` / SDK worker：文件契约增量导入、修订历史、原生源工具、紧凑反馈、同历史源码复查。
- `ModulePrompt.h`：状态到Req推导、captured资源、首个dispatcher准入、软件/硬件完成域、stale容忍与访问完整性。
- `RequirementChecker.cpp`：纯JSON E，使用绑定、Gua适用性和局部CFG，不从任意可达关系恢复跨线程HB。
- `ModuleAnalysis.cpp` / `run_module.py` / `run_batch.py`：抽取→E→可选C，中断部分E、输入核验、冻结manifest。

Req形式仍是`MustPrecede`、`MustBeMediated`、`MustBeAtomic`；Gua仍是`Order`、`Wait`、`Exclude`、`AtomicOp`。当前Gua不依赖某个Req ID，但采用时resource/scope需精确匹配，要求unconditional及操作/evidence可绑定。
E**不修改原始CCPG**：Order/Wait以虚拟有向关系查询，Exclude以token/连续区域查询，AtomicOp检查个体效果兼容性。一般独立同步图投影仍是设计，不冒充已实现。
MustPrecede必须有同步handoff；普通同线程CFG顺序不能排除不同激活。连续区域内不透明调用需要effect summary。一般跨过程must-order、匹配返回、循环occurrence、独立别名/实例证明与弱内存形式化检查仍有限。

`18ab94c`发布前验证：6 CTest、25 Python测试、localhost SDK流程通过；七例冻结账本离线重放保留旧bound节点，serdev新增15/ipset新增18一般绑定。这些是工程/零模型验证，不是新召回。本次仅整理文档和结果，未改变检测实现。

## 6. 最新基准与原始记录

首轮：`lace-source-contract-full72-20261013`，北京时间2026-10-13 20:50:33至10-14 00:17:30，jobs7；67完整、4网关429中断，71例均执行E。
补跑：`lace-source-contract-full72-retry4-20261014`，10-14 14:53:52至15:38:28，jobs2；四例37882/37920/38078/38242全部完整，参数与源码/IR不变。两批实现均核验未修改。

两批远端入口分别在`/mlx_devbox/users/mayunlong.39/playground/`下同名目录，**真实存储是独立NVMe盘`/tmp/`下同名目录**，入口是symlink。打包时用真实目录，不能只tar顶层symlink。
本地完整副本在`/Users/bytedance/Documents/并发缺陷检测/analysis_outputs/`下同名目录，均已解压、逐文件SHA256核验。

- 首轮3800原始文件、4,124,847,400 bytes；归档SHA256 `fa8b36d37926fbc92e6e72c3fda5b7a478cc50fac8ace2dbd50c6ac2455a7edc`。
- 补跑204原始文件、337,379,413 bytes；归档SHA256 `93a9fba995d08b6ab43876aa924d147c716bcb07b1d03acfcf8b546d48da0f77`。
- 首轮的`manual_recall_review_20261014/final_recall_review.md/.json`保存完整逐例人工判断、Req/Gua/E/源码/修复证据，4例`first_round_review`保留原partial判断。Git内摘要不替代完整原始档案。

每例读`run.json`、`artifacts/<run>/stateful_bugs/reports.json`、`module_contracts.json`、`module_model.json`、`llm_simplified_trace.log`、`sdk_source_tools.jsonl`与`agent_workspace/module/`。先读最终报告再沿问题查证，不只数契约/unknown。`coverage.unknown_positions`也受关闭C影响，不是自动阶段归因。

71份选定完整运行共4099请求、输入448,943,380 tokens、输出2,753,231；平均单例17.29分钟。**未包括4个失败首轮的完整usage**，不是实际全部调用/成本；输入累计包含重复历史，不是独立源码量或确认计费token。

## 7. 已保留的主要问题与后续起点

不要先重跑全量。先利用最新原生契约/轨迹离线核验通用问题，再冻结改动做多案例E-only实验。

- Req遗漏：KVM spin index、GPU reset/BO、flowtable类型表、失败/复用世代及不同使用域仍有漏提；有些关键代码没被读取，有些读到却未提，不能统一猜测为知识不足。
- 绑定：workqueue统计的枚举数组下标与compiler数值路径、位域共享字节、复杂赋值、部分宏/内联访问仍有通用缺口。实际IR存在不等于选择器已正确绑定。
- 粗召回的形式化：CAN长度依赖、CIFS锁后reader、PHY持ref而driver数据退役、shared XSK pool对FILL的具体关系、OSS buffer准入等均在逐例记录注明；召回不证明这些Req/E已完整正确。
- Gua真实同步域、位置和连续保护检查仍需核验；不能把父对象引用当所有子资源的生存保障，也不能把writer锁推广给所有reader。
- 端到端use→free Req按用户约定保留。OA退出Gua端点与stop/free反序诊断、循环事务局部顺序问题仍是独立已定位事项，见历史语义审计；不要把它们混成waiting-slot目标Req的一次测试。

不现在重做AtomicOp、恢复surface/入口裁剪、引入自动GT标签或要求逐case补标注。历史结果、IR/GT、用户轨迹与研究资料均保留，不能当作可随意删除日志。
