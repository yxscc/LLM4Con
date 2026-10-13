# 源码契约文件接口与坐标绑定

2026-10-13。实现位于远端 `LLM4Con-req-first-workflow` 的
`codex/source-contract-artifacts` 分支，基于 `2c9fa779`。原实验、原二进制和输入均保留。

此前七例轨迹中，操作注册、未知锚点逐条查询和反复诊断占了大量请求；第一次尝试结束之后还继续了大量机械流程。用户同意让 Agent 直接维护源码层契约文件，把 ID 和一般对应工作交给后端。本次没有改 Req/Gua 类型或 E 的消解规则。

## 接口

默认 `--contract-input artifacts`，旧接口可显式用 `--contract-input tools` 对照。
SDK 原生 `exec_command/write_stdin` 继续负责文件访问、命令和 PTY；模型/工具循环仍归官方 Runner。
输入源码 `module/`、格式说明、`contract_state.json` 和 `contract_status.json` 只读；只有 `contracts/` 可写。
完整源码、AST、图节点表、GT、修复和整个 Linux 均不注入初始请求。

Agent 读 `contract_format.json` 后，增量写 `contracts/**/*.json`，每个文件一条 Req 或 Gua。例如：

```json
{
  "kind": "requirement",
  "form": "MustBeMediated",
  "resource": "p->field",
  "scope": "same p",
  "a": [{"path":"example.c","line":10,"kind":"write","expression":"p->field"}],
  "b": [{"path":"example.c","line":20,"kind":"read","expression":"p->field"}],
  "actors": ["updater activation", "reader activation"],
  "assumptions": [],
  "note": "由实际源码说明冲突、重叠和安全需求；此例只展示格式。"
}
```

`MustBeAtomic` 的 `a` 是有序 S，`b` 为空，`interference` 是 I。
Gua 的 `a/b/evidence` 同样使用源码定位；`regions` 的 `start/end` 是获取/释放定位。
每个定位含 `path,line,kind,expression`，可加 `function/end_line`。
`expression` 是具体 lvalue 或 callee，路径不含 `module/`。
Req/Gua 的逻辑直接来自 JSON；不会从 note、笔记或自然语言总结反推规则。

SDK shell 返回后发现 JSON 内容变化，即通过已有 C++ 工具通道通知后端。
同一次工具返回包含变更反馈，不增加一次模型调用。
后端自动生成源码操作和契约 ID，调用现有规范校验与账本逻辑。
同名文件修订保留 ID；删除撤回当前契约；已接受版本、修订和删除事件保留。
非法文件/字段/源码定位给出错误并保留上次合法版本；错误未修复不能结束。
某个文件非法不妨碍其他合法文件接收。

`get_operations(operation,offset,limit)` 和 `bind_source_operation(operation,node_id,reason)`
仅在具体歧义需要调查时使用，不要求先注册和抄 ID。
`get_contract_diagnostics(files=[])` 返回紧凑状态，指定文件才构建详细局部 CFG 反馈。
默认结束门槛不再要求逐条 pattern 记账、全量诊断重读或每个未知锚点查询。
两次源码调查仍在同一 SDK 历史中、E 之前；不承诺抽取完备。
中断后尽力导入当前合法文件并运行部分 E，保留非零退出、错误和 incomplete。
Phase C 如启用，契约文件只读。

## 对应边界

后端首先用路径、行范围和函数限定候选，再结合真实 effect kind、callee/access path、DWARF 内联信息对应。
新增 `inline_calls` 表示源码包装 API 与实际嵌套调用的编译器对应，不改写真实 callee、memory ordering 或 atomic 标记。
一个内联 API 同时包含内存效果和诊断 helper 时，优先匹配具体内存效果；多个同类效果继续歧义。

文件定位可启用 `statement_binding`：精确单行、表达式确在源码、唯一函数上下文且只有一个目标类型的可执行效果时，可对应缺少编译器名称的效果。
已知但不匹配的字段路径不会被坐标覆盖；嵌套源码调用、内联调用伴随其他内存效果和多效果不使用该 fallback。
atomic API 仍要求编译器效果对应，不能把 `atomic_read` 的 volatile load 改成 LLVM atomic。
原账本没有此标志时保留原选择逻辑；重新生成 compiler facts 的独立变体可以使用新增内联对应。
Agent 恢复仍要求先看指令、限定坐标/函数、记录理由和快照；真实无 IR 或未解决歧义保留 unknown。
坐标/效果对应不是同实例证明，也不会产生同步关系。

## 验证记录

构建和零模型验证保存在远端
`/mlx_devbox/users/mayunlong.39/playground/lace-source-contract-artifacts-validation-20261013`。
该目录保存修改前的三份二进制副本、构建日志、合成 SDK 请求、失败尝试及后续修复结果。
后续已完成七例真实模型重跑，见[SOURCE_CONTRACT_PILOT_20261013.md](SOURCE_CONTRACT_PILOT_20261013.md)；本节的工程验证和派生账本仍不能当成模型召回/耗时实验。

回归覆盖结构化翻译、未知定位、内联对应、稳定 ID、SDK 工作区权限和增量回调。
localhost 全流程覆盖创建、修订、非法修改回滚、结束拦截、删除、嵌套文件、跨 pass 历史、部分 E 和可选 C 的只读文件。
旧七例账本的冻结图重放与 CIFS 独立内联事实变体另存；原账本和原模型不可覆盖。

实际结果：完整构建、6 项 CTest、25 项 Python 测试通过。
新接口 localhost A→E 为 11 次模拟请求，失败部分 E 为 3 次、A→E→C 为 15 次；都没有真实模型调用。
七例冻结模型与原账本的 E JSON 与原生 checks 完全一致。
CIFS 仅刷新编译器对应事实后，原来 82 个未知 spin_lock/spin_unlock 定位均由内联信息绑定到实际 raw 调用；没有手工填写节点。
这是独立派生图+旧账本的绑定诊断，不是 Agent 新抽取或新的缺陷召回。
实际 localhost 请求中，同一实现的 legacy/artifacts 初始 schema 为 17,729/5,803 字节（不含仓库工具）；工具为 16/7 个。
字节数不等于 token 或耗时，实际速度和召回仍待真实模型实验。
legacy 工具全流程回归也通过。第一次合成输入按字典序选到未编译的 extra.c，不满足旧测试要求“宽锚点有 IR 候选”，随后用同一已编译 fixture.c 单独验证；失败记录保留，未据此修改检测器。
