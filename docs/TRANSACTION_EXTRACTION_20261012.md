# 事务 Req 抽取与三例 E-only 实验：2026-10-12

实现提交：`151593eed6e04523e50e99677fddf7061fb33cbe`，包含此前源码到 IR 对应修复 `19e779d`。用户同意在现有类型中补充源代码支持的事务 Req 实例。本轮没有新增 Req 类型、依赖边或 E 消解规则；Gua 端点、循环 occurrence 和 AtomicOp 类型体系仍按约定暂缓。

## 改动及语义

提示词、协议目录 v1.2 和工具参数说明强化了以下抽取要求：检查交接双方各自的事务；区分事务步骤与对侧干扰；使用实际观察、更新/使用的窄源码锚点；说明干扰如何破坏不变量；检查已有验证、重试和允许的中间状态。已有 MustBeMediated 不代替实际存在的事务 Req，独立的生命周期和发布 Req 继续保留。没有加入案例符号、GT 或修复提示。

沿用 `MustBeAtomic(S,I)`：S 是同一实例、同次事务执行中的有序步骤，I 是指定的、会破坏该事务的并发操作。抽象交错含义为每个适用的 i 都不应满足 `s₁ ≺ i ≺ sₙ`，其中 ≺ 表示该执行中的事件先后；这不是把源码观察依赖声明成跨线程 HB。为什么需要保护这些步骤写在说明中；操作集合由结构化字段直接给出，E 不从说明猜规则。

现有工具表达仍为 `a_sources/a_nodes=S`、B 为空、`interference_sources/interference_nodes=I`。E 先检查 S 完整绑定及局部顺序，再对每个已绑定干扰检查同步顺序是否将其放在整个区间之外，或同一排他协议是否连续覆盖事务和干扰。单次 AtomicOp 不能消解多步事务。未绑定或顺序未建立保留 gap，不用剩余步骤冒充完整事务。

## 冻结实验与工程验证

- 计划：`tests/transaction_extraction_plan.json`，OA TC6、CAN、ioprio 三个 development 输入。完整模块与同版本公开 Linux 依赖，GT/历史修复不提供给模型。
- 模型：`gpt-5.5-2026-04-24`，官方 Agents SDK，Chat Completions，provider default reasoning；`jobs=2`、`max-turns=0`、`--no-review`。
- 北京时间 2026-10-12 13:19:32–13:39:37，全部 exit 0、E finished、generation finished、implementation_unchanged=true；Phase C disabled。
- 3 项 CTest 和 SDK/repository 的 15 项 pytest 通过。构建通过。
- 远端：`/mlx_devbox/users/mayunlong.39/playground/lace-transaction-extraction-20261012`。
- 本地：`analysis_outputs/lace-transaction-extraction-20261012`。34 个远端原始文件全部下载并逐文件核对 SHA-256。原七例 uncapped 结果和离线 replay 均保留。

| Case | 模型请求 | Req / Atomic Req | 最终 E 报告 | 本轮人工检查结论 |
| --- | ---: | ---: | ---: | --- |
| OA TC6 | 63 | 9 / 3 | 2 | 正确提取消费端交接事务，4 个操作全部绑定；循环顺序 gap 阻止该 Atomic 进入检查。独立槽位冲突候选仍对应原问题。 |
| CAN | 65 | 13 / 3 | 10 | 提取状态检查与 timer 取消之间的依赖事务；接收状态报告仍可按宽松标准作粗粒度对应，但 FF↔CF 操作分组没有解决。 |
| ioprio | 55 | 8 / 2 | 5 | 无保护的观察→解引用事务全部绑定，E 对清空/引用释放/两个 free 产生 4 个失败组合，最终候选对应原生命周期问题。 |

累计 183 次请求，input tokens 12,606,211、output tokens 115,565。累计输入含反复发送的历史，不是独立代码量或已确认计费量。报告数量不代表正确缺陷数量。本轮同时包含编译器对应修复，且模型抽取存在随机性，不能作为提示词单变量消融或正式召回提升证据。

## OA：目标事务已抽取，E 仍受循环限制

新 `req:0`：

- S：1007 读取 `waiting_tx_skb` → 同行写入 `ongoing_tx_skb` → 1008 清空 `waiting_tx_skb`；IR 节点 `[9092,9095,9098]`。
- I：发送路径 1213 写入 `waiting_tx_skb`；IR 节点 `6972`。
- 三个步骤及干扰均通过唯一 compiler_access_path 对应绑定。没有加入 wake、timer 或其他字段代理。

人工查看了生产侧 1202 的非空检查。消费循环在完成当前 skb（982–989）后，下一迭代可能读取空 waiting 槽位；此时生产侧也可以检查到空槽，随后在消费读取与清空之间写入新 skb，最终被清空覆盖。这是源码推导的交错说明，未运行内核复现，也不是 E 构造的可行性证明。

E 给出两个 `sequence_order_not_established`，evaluated_combinations=0、e_candidate=false。人工检查实际 CFG：相邻步骤有前向路径且前一步支配后一步，但后一步可经过 for 循环回边到下一次前一步；`Graph::before` 因反向可达拒绝顺序。这与此前手工消费事务诊断一致。新事务不存在未绑定步骤，不能再归因于 wake 绑定；本轮没有修改循环规则。

`req:1` 的独立 MustBeMediated 使用发送读/写与消费读/清空，全部绑定，评估 6 个组合并输出槽位候选。条件 wake/wait Gua 没有被当作消费区间的排他保护。

另一个 `req:3` 控制 SPI 事务仍有操作集错误：S 同时包含 300 的写回复检查和 307 的读结果拷贝，但 299–300 的写分支直接 return，二者不能在同次执行中串联。E 拒绝建立该顺序是合理的，不能把它也归为循环问题。这类互斥分支混串在旧账本中也存在，本轮未解决。

## CAN：额外事务有效进入 E，FF↔CF 组成仍不准确

`req:9` 的 S 是 CF 路径的 WAIT_DATA 检查、hrtimer_cancel、随后 rx.idx 更新；I 是接收 timer 将状态写为 IDLE。节点 `[7797,7834,7950]` 与 `10654` 全部绑定，E 评估 1 个组合并输出原子性候选。源码确实在检查之后取消 timer，未在返回后重查状态；timer 的等待保证与保持此前观察是不同的要求。实际允许的超时/接收边界行为仍需协议语义核验，这个候选不等同于 FF↔CF 的历史缺陷。

`req:10` 的 FC/发送 timer 事务使用宽锚点，状态观察和更新均匹配多个节点，未进入 Atomic 检查。`req:1` 的发送等待者事务也因等待宏和 buffer 代理锚点不能完整绑定而保留 gap。

最终 `req:4` 接收状态报告描述同 socket 的并发接收重组与无保护状态，仍可按用户宽松标准计粗粒度对应；结构化 A 却把 FF/CF/SF 都放在同一侧，B 仅为 timer，FF 多个写因宽行范围歧义。没有形成 FF 写入对 CF 观察/拷贝事务的直接关系。不能因报告对应就认定操作选择已修好，也不重新引入必须逐操作匹配 GT 的召回门槛。

## ioprio：正确的无保护事务与受保护事务要分开判断

`req:0` 的 S 是 `get_task_ioprio` 的非空检查、指针重读、ioprio 字段读取，节点 `[6266,6271,6273]`；I 是退出清空槽位 `8148`、put 调用 `8158`、最终 free `6478/7144`。全部正确绑定且局部顺序成立，E 输出 4 个失败组合和最终 Atomic 候选。源代码在这个读取函数中没有 task_lock 或 io_context 引用；与原历史修复的生命周期范围对应。假设和对象最终引用条件仍保留，status=requires_semantic_review 并不表示未检查。

`req:5` 则是已有 task_lock 保护的 `get_task_io_context` 读取→get_io_context 事务 `[6602,6613]`。源码的读取和取引用位于同一临界区，退出路径在同锁下清空后才释放引用。新 Gua `gua:5` 的资源、scope 和操作绑定匹配，但被 Agent 标为 unconditional=false，E 以 unresolved_guarantee_conditions 拒绝采用，随后仍输出候选。这个受保护路径的候选不能算新漏洞或缺少 task_lock；当前直接原因是有效 Gua 声明未被接受。进一步消解最终 free 还需要联系取引用与退役语义，不能要求所有 free 都机械放进 task_lock 区间。本轮未调优此误报。

## 后续边界

本轮目标——让现有 MustBeAtomic 补充交接消费端的真实事务——已在 OA 新模型轨迹中实现；ioprio 也形成了正确的观察/使用事务并进入 E。仍需逐条审查实际操作集，不能把更多 Atomic Req 视作更好建模。

循环、Gua 端点设计和 AtomicOp 重构继续暂缓。若继续优化抽取，值得检查的是互斥分支被串成 S、宽源码锚点、对侧接收激活遗漏以及 scope 内有效保障被泛化标为 conditional；这里先记录，不自行扩展为新一轮实现或模型队列。
