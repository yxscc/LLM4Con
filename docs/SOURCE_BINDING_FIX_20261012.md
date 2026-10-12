# 源码到 IR 对应修复：2026-10-12

实现提交：`19e779d4b8408cb0d59e18a02641ef17071138fa`。本轮只修改编译器事实、绑定和窄范围 IR 查询；Req/Gua 定义、E 消解规则、提示词、AtomicOp 和 Phase C 设置均未改动。

## 实现

- 从 DWARF 内联链提取 API 调用位置和具体内存效果。源码 API 调用可对应实际 load/store/RMW/fence/内联汇编效果，而不要求保留直接 call。
- 排除局部栈临时读写；普通嵌套调用可能是插桩或不透明辅助调用，不仅凭内联帧将它认作 API 的内存效果。
- 恢复未逃逸、只有一次指针写入且该写入支配读取的 spill 来源。保留外层函数的源码变量名，避免把 `iobj`、`imem`、`tc6` 等局部别名改写成传入参数名；内联参数临时变量才沿来源恢复。
- 间接调用记录 callee pointer 的访问表达式。`psy->desc->get_property` 据此绑定到实际间接 call，不声称证明具体动态调用目标或对象实例相同。
- 绑定和 `get_operations` 查询共用包含内联链的源码位置匹配。
- 只有唯一对应才 bound；多个调用或多个效果仍为 ambiguous。LLVM `atomic`、volatile 和 memory_ordering 保持实际指令语义，不因源码 API 名而赋予原子性或同步边。

## 零模型调用验证

新增 `module_fact_refresh input.ll frozen_model.json derived_model.json`，对旧账本进行离线编译器事实刷新。它校验全部 IR 函数、指令序号及指令内容，拒绝不匹配的输入，并保留节点 ID、原始指令和图边；输出不得覆盖输入。

七例使用原批次 `run.json` 记录的 IR 路径与 SHA-256。原模型和契约文件 SHA-256 在运行前后核验；刷新前后逐项核验函数、CFG、调用边、节点 ID、指令和 LLVM atomic 标志相同。随后执行现有 `contract_replay`，没有模型调用，也没有修改原始契约。

最终结果：

| Case | 原成功 Req/Gua 绑定 | 新增成功绑定 | E 候选变化 |
| --- | --- | --- | --- |
| Nouveau | 全部保留 | 0 | 无 |
| CAN | 全部保留 | 0 | 无 |
| PHY/power | 全部保留 | 3 个 Req 操作 | req:10 从未评估变为两个失败组合、产生候选 |
| ioprio | 全部保留 | 0 | 无；部分多效果操作从未解析变为歧义 |
| KVM | 全部保留 | 1 个 Gua 证据操作 | 无；仍没有 last_boosted_vcpu Req |
| OA TC6 | 全部保留 | 0 | 无 |
| netfilter | 全部保留 | 0 | 无 |

人工核对新增节点：

- PHY `atomic_read` → 7279，实际 `load volatile i32`，LLVM atomic=false。
- PHY 属性回调 → 7299，通过 `%37` 的间接 call；callee 表达式为 `psy.desc.get_property`。
- PHY `atomic_dec_return` → 5915，实际 `lock; xaddl` 内联汇编；LLVM atomic=false。
- 原 `device_unregister` → 5979 保持绑定。原 req:10 的步骤为 [7279,7299]，干扰为 [5915,5979]，现有 E 评估两个组合并产生候选；因原 assumptions 保留，状态仍为 requires_semantic_review。
- KVM Gua 的 `atomic_inc` 证据 → 36566，实际 `lock; incl` 内联汇编，LLVM atomic=false。它不涉及 last_boosted_vcpu 漏提 Req 的修复。

这是原账本的派生重放结果，没有新的模型抽取或最终原生报告，不替换此前七例结果，也不直接宣称新实验召回提升。

## 回归与记录

LLVM 16.0.6 构建通过，3 项 CTest 通过：完整图、现有 Req 检查器、新增编译器对应测试。新增 fixture 检查内联 volatile read、局部临时变量与插桩排除、内联汇编、间接 callee、外层变量别名、同一行两次调用/字段读取的歧义，以及逃逸函数指针不被推定为原始字段。

第一次离线诊断发现沿 spill 来源越过外层变量名，导致 Nouveau/OA 等已有字段绑定丢失。外层变量作用域修复及其回归加入后重新完成七例验证，所有原成功 Req/Gua 绑定均保留。早期诊断不作为最终实现结果。

- 最终远端目录：`/mlx_devbox/users/mayunlong.39/playground/lace-source-binding-audit-20261012-v2`。
- 本地镜像：`analysis_outputs/lace-source-binding-audit-20261012-v2`。
- `audit.json` 保存 IR/原始输入/二进制哈希与完整状态；每例保存派生模型、原/新 E 检查、绑定变化及日志；`audit_driver.py` 为执行脚本，`binding_comparison.json` 为原绑定保留核对。
- 早期诊断另存远端 `lace-source-binding-audit-20261012`；原七例 uncapped 数据未覆盖。

## 当前范围与后续约定

没有明确编译器展开出处的宏、多个 IR 效果对应一个源码事件、一般别名与对象实例证明仍可能 unknown。本次没有手工指定节点或按 API 名构造同步保障。

用户确认 use→free 的端到端 Req 本身合理，Gua 端点设计暂缓；循环 occurrence 问题暂缓。第三项讨论沿用 `MustBeAtomic(S,I)`：同一作用域与同次事务执行中的有序步骤 S，不允许已识别会破坏该事务的干扰 I 插入步骤之间。观察依赖用于说明步骤选择，不新增同步边，也不在本轮更改 Req 公式或 E。
