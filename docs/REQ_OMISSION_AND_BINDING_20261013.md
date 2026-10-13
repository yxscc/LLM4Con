# Req 遗漏轨迹分析与源码对应修复

2026-10-13。分析对象为 `c4f4695` 的七例文件接口原生实验，全部已经结束。
这是人工读源码、最终契约、报告和 SDK 工具返回后的判断；没有用自动 GT/阶段分类。
本轮不改变 Req/Gua 类型、E 的消解规则、实验输入或原生结果，不启动真实模型实验。

## 轨迹能说明什么

不能仅从没有目标 Req 推断模型不掌握某项知识。当前日志没有模型的隐藏推理；可以核实的是返回到上下文的源码、查询范围、模型明确提交的逻辑，以及尚未调查的依赖。
旧提示词已经包含 scalar、首次 admission、child resource 和 hardware consumer 等问题。单纯增加这些关键词不能解决具体关系没有串起来的问题。

| Case | 实际轨迹和契约证据 | 人工判断与知识问题 |
| --- | --- | --- |
| CIFS / CVE-2024-35999 | 初始搜索以锁、work、RCU 等 API 为主，并用 head/页截断。后续 channel 搜索集中在 sess.c/connect.c。完整工具返回未出现 `cifs_pick_channel` 或其锁外 `return ses->chans[index].server`。req:11 选 channel 管理路径，没有 transport.c:1062 的实际读。 | 首先是阅读/参与者遗漏，不能归因于不知道锁外读的风险。需要从已发现 channel 资源反查整个模块的读、写、辅助函数和锁后访问；静态图中真实读节点 123360 存在。 |
| KVM / CVE-2024-40953 | sdk_source_tools 的 event 9 已返回 kvm_main.c:2589 的 `last_boosted_vcpu` 读、2623 写及 approximate round-robin 注释；event 66 再次返回这些访问。19 条最终 Req 没有该字段；同一 yield 代码的 pid 生命周期 Req 是另一资源。 | 属于已读访问没有形成 Req。本批没有显式放弃理由，不能声称已证明“模型因 approximate 而排除”。旧批曾这样概括，当前它是合理待验证的推断：容忍旧值与容忍任意编译器变换/撕裂值不同，需要分析实际值消费和验证。模型中真实读/写 39630/39819 存在。 |
| serdev / CVE-2024-35977 | event 5/40 返回 uart.c:266 open 先于 272/273/294/302 初始化。event 7 的 pinned 查询返回 ttyport_receive_buf 的 SERPORT_ACTIVE 检查与 dispatcher；保存的 repository_evidence 还有 open 激活和 ops 解引用实现。req:6 却明确把后来的 set_client_ops 当首次 callback admission。 | 相关知识/实现已经可查且被返回，主要是证据组合和边界判断错了；只补“注意初始化”无效。必须从完整 dispatcher 链确定第一次准入，并检查进入用户回调之前的 ops/context 解引用。新增候选仍可粗相关，不能掩盖这个逻辑缺口。 |
| ipset / CVE-2024-39503 | event 3 返回 __list_set_del_rcu:146 `e->set`、148 `ip_set_ext_destroy(set,e)`、160 call_rcu。event 9 返回 barrier、cancel_gc、destroy 的顺序。req:7 是 timer 关系，req:14 是 e->id 引用的另一个 child set；没有 owner `e->set` 的回调使用→owner 释放关系。本批没有 pinned 依赖读取。 | 并非只知道/不知道 RCU 的二选一。已生成的 RCU Gua 能解释成员读者→成员释放，却没有继续追踪 callback 自身对 owner 的使用，也没有建立 producer→另一个 executor 的依赖。缺少具体的传递生命周期推导和必要的 API 语义核查。取消 timer 不等于等待它已经排入 RCU 的回调。 |
| GPU / CVE-2025-37854 | event 9/121/123 返回 release worker 的 dequeue、fence signal、BO free；event 18 返回异步 reset worker。16 条 Req 主要覆盖 CPU lookup/ref、pdds、alloc_idr、evict/restore 等，未形成硬件使用/异步 reset 完成→BO 回收关系。本批 repository_evidence 为空。notes 明确称部分 DQM/GPUVM 实现在输入外，契约避免依赖内部语义。 | CPU 对象生命周期分析没有扩展到设备执行和 reset 完成；依赖缺失已经被看到，却没有调用现有 pinned 查询继续追。可能包含领域知识不足，但可观察的流程问题是跨域使用链调查不足，不能把 CPU 引用归零当硬件访问完成。 |

以上区分的是源码调查和契约推导缺口，不另设更严格的召回门槛。
MACSEC/NF objects 的对应 Req 仍是正面对照；CIFS/serdev 的粗相关候选与真实逻辑问题分别保留。
未知 E 状态、请求数量和 pass 正常结束均不证明代码覆盖或 Req 完备。

原始证据根目录：

- 本地 `../analysis_outputs/lace-source-contract-artifacts-pilot-20261013/native/<case>/artifacts/<run>/`。
- `sdk_source_tools.jsonl` 是 shell 调用与真实返回；上表 event 编号为 JSONL 从 0 开始的事件位置。
- `module_contracts.json` 保存真实 Req/Gua、pinned repository_evidence 和 E 检查；`agent_workspace/contracts/` 保存源码层契约。
- 仅为人工定位提取的命令列表保存在本地 `../analysis_outputs/lace-source-contract-artifacts-pilot-20261013/diagnostics/req-omission-traces/`；不作为自动阶段标签。

## 通用实现修改

**绑定**：文件接口的 read/write expression 往往是一整条赋值或增减表达式，后端此前只接受精确 lvalue。
现在只对 `statement_binding` 操作中的简单 write 赋值、复合赋值和前/后增减派生 lvalue，再用原有编译器 access_paths/类型/位置匹配。
原 expression、source ID 和契约均保留；输出 `source_effect_selector` 记录转换。
原完整 selector 仍须出现在原源码，不能用派生字段掩盖虚构表达式。
条件、连续赋值、多个语句、带额外增减的右侧、复杂 lvalue、API 调用均不靠该规则改写。
多 IR 效果继续歧义，已知不同字段不被行号覆盖；没有 LLVM atomic 标志改写。
旧账本没有 statement_binding 时保持原选择逻辑。

**反馈**：发现声明的 function 与该源码坐标的编译器函数均不一致时，返回 `function_selector_mismatch` 和真实 `coordinate_functions`。
例如 CIFS 3958–3961 被写为 cifs_tree_connect，实际在 cifs_setup_session；这是错误限定，不是缺 IR。
后端继续未知，Agent 可据证据修订文件；不擅自忽略函数或替换节点。
格式示例也明确字段、条件和 copy/helper API 应如何定位。

**Req 推导**：文件模式提示增加 source-independent 推导规则，而非新的契约类型或 GT 专属线索：

1. 从已发现资源追所有实际访问与参与者，包含 unlock/return 后的操作；API grep/第一页不能代替这一步。恢复按需函数导航，仍不要求逐函数记账或以函数充当线程入口。
2. 从 deferred callback 的具体使用追 captured owner/payload/code 的退役；区分 producer cancellation、已排入另一 executor 的任务、grace period 与 callback completion。Req 的 A 是真实 use、B 是对应 retire。
3. 沿 dispatcher 找首个准入事件及进入回调前的解引用；不能把后来的赋值当已存在的准入屏障。
4. 把 CPU lifetime 与 device/DMA/reset completion 分开；查询必要同版本实现，以真实源码 effect 表示 Req，不制造 hardware IR 节点。
5. 对 approximate state 分析允许的错误和值的实际消费/验证；需要访问完整性时写同字段 MustBeMediated，不强加 freshness 全序，也不把 READ_ONCE 当 acquire/release 或 RMW 原子性。

这些知识仅指导调查和结构化 Req/Gua 生成，不直接注入同步边、不从 notes 猜规则、不新增 finish gate。
第二次源码调查也明确按上述具体关系检查遗漏，而非重复已有协议总结。

Linux 官方文档提供通用语义依据，具体 API 应继续核对输入 pinned 版本：

- [RCU callbacks 与 rcu_barrier](https://docs.kernel.org/RCU/rcubarrier.html)：先停止新的 callback 提交，再等待已有 callback；grace-period 等待不等于 callback completion。
- [Access marking](https://docs.kernel.org/dev-tools/lkmm/docs/access-marking.html)：错误容忍 heuristic 需分析其能容忍什么值/编译器行为，不能因标记或不报 KCSAN 就断言正确。

## 验证

验证根目录为远端 `lace-req-binding-validation-20261013`，原 c4f4695 二进制单独保存。
所有离线输入读取原始 native 冻结模型/账本，只写独立输出；不会修改或覆盖原实验。
本轮真实模型调用为 0。绑定改善是工程对应改善，不能当作新模型 Req 召回结果。

完整构建和 6 项 CTest 已通过，包括实际 clang 编译的复合赋值对应，以及错误字段、虚构源码、函数不匹配、多效果/连续赋值的反例。
SDK/workspace/repository 的 pytest 为 25 项通过；一次误用 unittest 发现 0 tests，已改为实际 pytest 执行，原日志保留。
localhost 原生 SDK 两次 Phase A→E 正常完成，11 次模拟请求，0 真实模型调用。
七例原冻结输入以原 c4f4695 二进制重放，checks 与 native 全部相同；再使用本轮 binder/E 独立重放。
所有原来 bound 的操作保持原节点；没有把 unknown 自动填为一个同源码行的任意节点。

| Case | Req/Gua 引用的去重操作 | 原 bound | 新 bound | 函数限定不匹配诊断 |
| --- | ---: | ---: | ---: | ---: |
| NF objects | 171 | 139 | 139 | 0 |
| serdev | 52 | 29 | 44 | 0 |
| CIFS | 164 | 150 | 150 | 7 |
| ipset | 176 | 91 | 109 | 0 |
| KVM | 204 | 165 | 165 | 0 |
| GPU | 153 | 132 | 132 | 0 |
| MACSEC | 207 | 169 | 169 | 0 |

新增 33 个对应均由源操作派生字段与编译器效果对应取得，没有手工提供节点。
例如 serdev 的 `ec_dev->priv = ec_uart` 对应 6838 的实际 store，ipset `set->elements--` 对应 20504。
serdev 的现有初始化 Req 从 9 个绑定组合增加到 12 个，首次 admission 的原错误仍存在；不是修复 GT。
独立 E 的候选计数 serdev 4→6、ipset 2→6，其余不变；这里没有新增目标 Req，因此不当成召回提升。
剩余 unknown 包含条件/遍历宏的多个效果、helper 与 effect kind 不匹配、函数限定错误、实际无候选等，继续保留。

工程结果保存在验证根的 `replays/<case>/`，包含 original/updated checks、全部定位对应、binding-changes、summary 与冻结输入 hash。
最终实现再次用同一输入重放，核验与上述 updated 对应/检查一致，保存在 `final-replays/` 和 `final-verification.json`。
新提示对 Req 遗漏的效果尚未经过真实模型验证；本轮没有新的模型召回率。
