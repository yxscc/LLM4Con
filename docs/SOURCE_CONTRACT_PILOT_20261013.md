# 源码契约文件接口：七例真实模型重跑

2026-10-13，北京时间14:37:13–15:00:13。全部结束，没有模型队列待接管。

代码提交 `c4f4695210918f8341adfd5fc3ad29879c080d7e`，分支 `codex/source-contract-artifacts`，远端工作树 `LLM4Con-req-first-workflow`。
模型 `gpt-5.5-2026-04-24`、Chat Completions、reasoning provider default，SDK openai 3.26.0 / openai-agents 0.23.1。

与上一轮 `2c9fa779` 的相同七个 development 输入逐一核验源码、IR 和 autoconf hash，无输入改动。
jobs=7、max-turns=0、requirement-sweep=on、contract-input=artifacts、Phase C 关闭。
全部 native exit0、两次抽取及 E 结束、implementation_unchanged=true；没有上下文超限或抽取异常。

## 成本与人工对照

| 模块 | 旧→新分钟 | 旧→新请求 | 最终 E 报告 | 人工对照 |
| --- | ---: | ---: | ---: | --- |
| KVM | 29.5 → 18.0 | 150 → 62 | 16 | 未有目标对应 |
| MACSEC | 47.7 → 13.1 | 212 → 58 | 15 | 对应候选 |
| CIFS | 50.6 → 23.0 | 211 → 75 | 12 | 同类粗相关，逻辑有缺口 |
| serdev | 17.1 → 12.1 | 103 → 62 | 4 | 同类粗相关，逻辑有缺口 |
| GPU BO | 36.1 → 18.0 | 166 → 70 | 13 | 未有目标对应 |
| ipset | 50.4 → 7.7 | 231 → 29 | 2 | 未有目标对应 |
| NF objects | 34.5 → 14.8 | 155 → 51 | 13 | 对应候选 |

总请求 **1,228→407（-66.9%）**；累计 input **272,586,263→41,909,577（-84.6%）**，output 501,032→291,283。
平均单例 38.0→15.2 分钟，新中位 14.8 分钟。

input 包含反复发送历史，不是独立代码量或已确认计费 token。当前配置仍没有实际启用 native compaction。
旧批最初 jobs=2 后扩至7，新批全程7；接口、提示/结束门槛和 compiler facts 同时改变，模型也有随机性。
不是单变量消融，不能把所有时间差归给单一接口变化。

明确对应保住 MACSEC 和 NF objects 两例；serdev/CIFS 有同类粗相关报告，按宽松口径单独列出，但仍未正确建模目标 admission/reader。
这里不要求根因、类型或全部 GT 操作精确相同，也不将同类资源提示当成真实 Req/Gua 逻辑正确的证据。不是72例总体 benchmark recall。

## Req/Gua/E 的实际问题

- **MACSEC**：req:15 / finding:8 对应同一 net_device 的普通 rx_dropped/tx_dropped/rx_errors 更新与读取。
  10个唯一源码操作均自动绑定，真实 read/write 与 write/write 组合存在。
  相比上一轮补了 rx_errors 读取和 write/write 对侧；三个独立字段仍合并，42组合中14个同字段、28个跨字段，需要拆分操作域。
- **NF objects**：req:6 / finding:5 单独表达 nf_tables_objects 的 register/unregister、无保护遍历、type->type 读取和 try_module_get。
  5个操作全绑定、6组合，gua:7仅保护 writer，eligible 但未用于消解 reader 冲突。
  没有沿用旧将三表合并的 Req 或默认 NFNL 保护 reader 的解释，目标 Req 组织比上一轮清晰。
- **serdev**：新 req:6 / finding:1 确实提初始化 drvdata/waitqueue/priv/client_ops 先于 receive 回调，按宽松口径与 GT 初始化竞态同类相关。
  仍将 set_client_ops 当成首个发布点，scope 排除了真正早于它的 open 激活，未表达 init 必须先于 open；这不是已正确定位 GT 顺序。
  priv 初始化又用完整赋值表达式，未绑定。已查询 serdev-ttyport、core、serdev.h 并读到 SERPORT_ACTIVE，仍未追清 admission。
- **CIFS**：新 req:11 / finding:10 涉及同一 ses channel 表/count/slot，有同类通道表竞争提示。
  Req只覆盖add、reconnect、disable等，未覆盖 cifs_pick_channel 锁外读 slot。
  其列举的操作大多处于 chan_lock 内；gua:10 因4个未知定位不能采用，导致泛化候选，不能当成 GT 锁外读取已经被检查。
  SDK源码工具结果未读到 picker 函数体；实际 load 节点123360仍在模型，且是缺命名的窄单行效果。
- **KVM**：19条Req及16条最终报告仍无 last_boosted_vcpu。
  SDK源码返回已包含2589读取及2623更新，模型中读39630/写39819存在。pid 生命周期与其他列表竞态不覆盖这个共享索引。
- **GPU BO**：16条Req及13条报告包含process/event、alloc_idr、suspend/reset admission counter等，但没有 GPU 仍访问 BO 先于 release worker 回收 BO 的需求。
  已读 release worker 与 free_outstanding_kfd_bos；CPU lookup/IDR保护不是硬件消费者的 reset 完成等待。
- **ipset**：2条最终报告是 timer 终止/重排与 child set 引用释放。
  未表达 __list_set_del_rcu 通过 e->set 使用 owner，与 kfree(owner set) 的冲突；timer 停止不能等价于已排队 RCU callback 结束。
  已读取143–160源码，读20554及 ip_set_ext_destroy 调用20558在模型。
  大量整语句定位未知是另一个通用实现/格式问题，不是这例 GT 未提出 Req 的替代归因。

## 文件流程与定位诊断

七例完成时 artifact_errors 均为空，校验失败与源码/IR未知被区分；没有强制逐个未知定位机械查询。
source IO 通过原生 SDK 工具，文件改动在原 tool 返回中增量导入；旧原始实验不覆盖。

ipset 模型多次以整条赋值/RMW/条件作为 read/write expression，例如 `set->elements--`、`set->elements = 0`、`s->ref || s->ref_netlink`。
格式说明要求具体 lvalue/callee，现有 binder 按 compiler access path 精确对比，导致许多本来存在的效果未绑定。
memcpy 等 copy 调用又有被写成 write 操作的情况。这是需要统一的接口/实现问题，不能靠随便选同一行指令解决。

独立零模型诊断：仅将157行既有 source locator 的 symbol 从 `set->elements--` 改为 `set->elements`，它从 operation_not_resolved 变为 compiler_access_path 绑定节点20504。
原 Req/Gua 逻辑和原生结果不改，派生账本单存。这不是模型实际输出、新召回或正式 binder 修复。
多读操作的复合条件、多效果宏和分支仍不能被简单压成一个节点。

下一步优先统一 source expression 语义和编译器效果对应，再解决首次 admission、协议外 reader、子 callback/hardware 关系遗漏。
暂不恢复机械结束门槛，不引入 surface/入口裁剪或 AtomicOp 重构。

## 原始记录

远端 `/mlx_devbox/users/mayunlong.39/playground/lace-source-contract-artifacts-pilot-20261013`；
本地 `/Users/bytedance/Documents/并发缺陷检测/analysis_outputs/lace-source-contract-artifacts-pilot-20261013`。
完整 native 数据395个文件、约318MB；下载 archive SHA256：`47e685532ff27cdfe096f8cb44344b23176de8228e2e2f3f64657874511f128a`，所有清单文件hash已核验。

`native/batch.json`、`completed_bundle_manifest.json`、原始 Req/Gua/模型/SDK轨迹/最终报告均保留。
`manual_review.json` 为人工归因，`cost_comparison.json` 仅计算usage/时间；派生定位诊断在 `diagnostics/lvalue-ipset`。
GT和历史修复只在本地事后人工阅读，未提供给实验Agent。
