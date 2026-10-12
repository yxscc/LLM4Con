# 契约抽取残留修复与验证：2026-10-12

用户授权继续处理上轮残留。本轮范围是事务互斥分支混串、宽源码锚点及对侧接收操作遗漏、作用域内有效 Gua 被泛化标为 conditional。循环规则、Gua 端点设计、AtomicOp 类型体系继续暂缓；没有新增 Req/Gua 类型或更改 E 消解规则。

实现及三例模型实验提交：`d95026cbbd02b1e0dae504b52375385833ca84e0`。实验结束后的效果身份诊断补丁：`3ce3e18c091ef4cfcda5ff50480aca18eacbee5f`。**三例原生结果使用 d95026c，不能说它们已使用后续身份诊断补丁。**

## 实现

1. 提示词与协议目录 v1.3 明确：Atomic 的 S 应沿同次执行的一个可行分支；不同命令模式、成功/失败分支不能串成步骤。并发接收的不同消息类型也是对侧激活，不能只有 timer 关系。允许新消息覆盖同一共享槽位，使旧激活的观察失效，不能用“同消息 generation”提前排除。
2. 收窄源码锚点：多个语句产生多个语义匹配时重新注册具体语句；一个精确语句仍对应多种编译器效果时保留歧义。真正实施拷贝/修改的调用可以用该调用表示，不把传入指针假装成一个直接 store。不挑同一行的任意节点。
3. `unconditional` 解释为**声明作用域内的效应已成立**。同实例、合法调用域和未覆盖其他路径本身不是未知证据；真正未确认的匹配、调用保护、返回或 admission/rearm 前提仍可 conditional。源代码同步语义与 IR 是否能绑定分开判断。后端没有按 API 名或自然语言自动翻转标志。
4. 新 `ContractFeedback` 在接受 Req/Gua 时返回绑定状态/语义匹配数、Atomic 相邻步骤的局部路径诊断和声明的 applicability；`get_contract_diagnostics` 可查询已登记契约。它不返回 AST/源码操作对应的原始 IR 记录，不构造 HB、不消解 Req、不证明语义。
5. `add_requirement/add_guarantee` 可用 `replace_id` 和 `correction_reason` 提交完整修正版，保持 ID、替换活动账本，原版和修正版保存在 `contract_revisions`。不能靠另加一个正确副本留下已知错误的原 Req。类型、字段、来源和操作验证仍在修订前执行；错误 ID/缺少修订理由不修改账本。

后续身份补丁把 `same_resolved_operation` 与 `cyclic_local_path` 分开，并明确返回相邻操作是否为同一个绑定 IR 操作。同一行的 read 和 store 可以是不同效果；循环反向可达不能当成重复步骤证据。它没有放宽 E 的循环顺序或推断动态 occurrence。

## 工程及零模型验证

- LLVM 16 构建通过，4 项 CTest 通过；官方 SDK/repository 的 15 项 pytest 通过。身份补丁后重建及 4 项 CTest 再次通过。
- 新回归检查互斥路径、循环、跨过程/不完整 CFG、宽锚点歧义、同一行不同效果与同一个效果的不同锚点、反馈不建立同步、显式修订/历史/稳定 ID、非法修订不修改账本、conditional 不被后端从说明自动改为 true。
- 原三例完整账本的零模型诊断准确指出：OA 控制事务的写回复检查→读结果拷贝没有局部路径；消费事务是循环路径；CAN 宽锚点有多个语义匹配；ioprio Gua 的 false 是 Agent 的声明，后端不擅自覆盖。
- 单独的 ioprio 派生 probe 只把原 gua:5 的 unconditional 改为 true。E 的 eligibility 随即成立，但事务内部调用/未覆盖的锁外 free 仍阻碍排他消解，候选保留。这不否定真实 task_lock 保障，也不能把翻转标志当作完成整个 Req 消解。
- 后续效果身份诊断对原 OA 三步账本 `[9092,9095,9098]` 重查，两个相邻对均为 distinct operations + cyclic_local_path，没有把同一行 read/store 合并。

派生诊断保留副本、原始输入 SHA-256 和零模型调用标记；不改原契约、原模型、原报告，不作为新模型召回或漏洞证据。

## 新三例 E-only 实验

使用原三例完整 development 输入，源代码经 pinned Linux revision 字节核验。模型只收到模块与同版本公开依赖，不收到 GT/历史修复或本轮人工诊断。CPG 复用缓存；IR 构建配置/版本来源仍未独立验证。

模型 `gpt-5.5-2026-04-24`、Chat Completions、provider default reasoning、jobs=2、max-turns=0、Phase C disabled。北京时间 14:01:23–14:20:48，三例均 exit 0、generation/E finished、implementation_unchanged=true。

| Case | 请求 | Req | 最终 E 报告 | 人工核对结果 |
| --- | ---: | ---: | ---: | --- |
| OA TC6 | 56 | 8 | 4 | 原互斥步骤没有留在活动 Atomic 中；控制事务使用调用对调用的 mediation；槽位候选保留，消费 Atomic 仍受循环限制。 |
| CAN | 48 | 11 | 8 | FF↔CF 实际接收状态组合进入 E 候选；CF Atomic 的 5 个步骤和 6 个干扰全部绑定，循环内更新到循环后检查的前提未建立。 |
| ioprio | 51 | 7 | 6 | task_lock 在所列路径上标为 true，同时排除无锁读函数；观察/解引用及 use/free 候选保留。 |

累计 155 次请求，input tokens 11,102,664、output tokens 97,893；重复历史计入累计输入，不是独立代码量或已确认计费量。报告数不是正确漏洞数。本轮是多个工具/提示改动的 development 验证，有模型随机性，不是单变量消融或正式召回提升证明。

### OA：错误分支组合已移出活动 Atomic，原问题保留

原控制 SPI Atomic 把 300 写回复检查与 307 读结果拷贝串联，源代码 299–300 的写分支直接 return。本轮没有继续使用这个不可能序列；新 req:0 以公开 read/write API 内的 `oa_tc6_perform_ctrl` 调用 `[6740,6880]` 表示共享控制事务的 mediation。没有验证读写各分支的完整 Atomic 覆盖，不能把这一点说成“所有事务都已抽取”。

gua:0 正确描述两个 API 持 `spi_ctrl_lock`，unconditional=true，并注明其他路径不在保护范围。E 因 mutex_lock 锚点/区域边界未绑定拒绝采用，仍有控制事务候选；不是实际缺少这把 mutex。

消费槽位 req:1 最终 S=[9092 读 waiting,9098 清 waiting]，I=6972 发送写，全部绑定。E 保留 sequence_order_not_established，独立 req:3 的 mediation 评估 6 个组合并输出对应槽位候选。

原生轨迹有 3 次修订。一次理由把同一行的读 waiting/写 ongoing 说成重复步骤，这与实际两个 IR 节点不符；原历史没有被人工改写。修订后两个端点仍表达有效的读/清空非干扰需求，但不能认可该“节点相同”解释。随后补效果身份诊断与回归，避免把循环诊断误读为效果重复。

### CAN：对侧接收与窄锚点有实际改善

req:2 的 mediation 中，FF state 写 7754 与 CF state 读 7797 已作为对侧组合进入 E 失败组合；9 个组合被检查，最终报告对应并发接收重组缺保护的范围。按用户宽松标准保留粗粒度召回，仍不要求精确覆盖历史 rx.len/alloc_skb/skb_put 的所有操作。

新 req:3 明确 S 为 CF 的 state 检查、SN 检查/更新、idx 更新、后续 idx 检查 `[7797,7892,7916,7950,7973]`；I 为 FF 的 state/idx/SN 初始化、timer 和另一个 CF 的 idx 写 `[7552,7710,7750,7754,10654,7950]`。全部通过具体语句绑定，没有把 FF/CF 步骤混成同一事务。

E 未检查成 Atomic 候选：7950→7973 有前向路径、没有反向路径，但 CFG 可零次迭代到达 7973，7950 不支配它。不是此次绑定失败，也不能一律归为反向回边；需要以后处理局部执行/循环前提。该规则本轮未改。

发送事务的 memcpy_from_msg 已用实际调用绑定到 6420，tx.len/idx 的单行写也绑定。wait_event/cmpxchg 宏仍未绑定；没有凭 API 名赋予 LLVM atomic=true，当前 AtomicOp 类型体系保持原样。

### ioprio：作用域声明更准确，真实缺陷候选仍在

新 gua:0 对 blk-ioc.c 的 task_lock 保护 slot 访问标为 true，明确不含 ioprio.c 无锁 get_task_ioprio。原来仅因“同一 task/只覆盖指定路径”而统一标 false 的问题，在这个保障上已修正。没有拿它消解不持锁的读取。

req:0 的非空观察→字段解引用 `[6266,6273]` 与退出清空/put `[8148,8158]` 全部绑定，E 产生 2 个组合和 Atomic 候选；req:2 的 use 6273→free 6478/7144 也进入最终候选。它们与原生命周期问题对应。

gua:1 通过修订把 RCU grace period 从 Wait 改为 Order，并注明 pre-existing reader 的实际范围。关联锁 Gua 的调用者持锁/完整区域前提仍有 conditional，其他 IR 绑定与 effect summary 缺口仍在；没有把所有 false 统一翻为 true，也没有宣称所有已保护路径被 E 完整消解。

## 原始记录和后续边界

- 远端：`/mlx_devbox/users/mayunlong.39/playground/lace-contract-repair-20261012`。
- 本地：`analysis_outputs/lace-contract-repair-20261012`。
- `native/` 保存 batch manifest、三个完整案例和 runner 日志；37 个远端原始文件（native 与 conditional probe）全部下载并核对 SHA-256，见 `archive_verification.json`。
- `offline_diagnostics/`、`conditional_probe/`、`offline_identity_followup/` 分别为初版反馈、单 flag 派生 E 和后续身份反馈诊断。
- 前两轮原生实验与七例派生重放保留，没有覆盖或清理。

已处理的是通用抽取规则、及时反馈及可追溯修订，并在三例看到所述实际改善。仍保留循环 occurrence/路径前提、锁宏对应、不透明调用 effect summary、调用者上下文及具体契约选择的已知限制。用户目标不是完备同步证明；也不能因为仍有这些限制，就把不成立的操作集或 Gua 声明错误视为可以忽略。
