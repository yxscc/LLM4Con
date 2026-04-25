# CPG Facts Extractor

用于从 LinConVul 数据集中提取可验证的并发事实，支持 GRPO 训练的 Process Reward。

## 概述

FactsExtractor 是一个独立的工具，它利用 LLM4Con 的 CPG（代码属性图）、CCPG（并发代码属性图）和 Phasar 指针分析能力，从 C/C++ 并发代码中提取结构化的、可验证的事实。

这些事实可用于：
- 训练 LLM 进行并发漏洞检测
- 作为 GRPO 训练的 Process Reward 验证器
- 构建并发漏洞检测的基准数据集

## 文件结构

```
LLM4Con/
├── include/FactsExtractor/
│   └── FactsExtractor.h          # 头文件
├── src/FactsExtractor/
│   └── FactsExtractor.cpp        # 实现文件
├── src/facts_extractor_main.cpp  # 主程序入口
├── scripts/
│   └── extract_all_facts.sh      # 批量处理脚本
└── FACTS_EXTRACTOR.md            # 本文档
```

## 构建

```bash
cd /home/LLM4Con
./build.sh debug
```

构建完成后，可执行文件位于 `Debug-build/facts_extractor`。

## 使用方法

### 单个样本提取

```bash
./Debug-build/facts_extractor \
    --input-src /home/LLM4Con/LinConVul/CVE-2017-15265 \
    --input-bc /home/LLM4Con/LinConVul/CVE-2017-15265/CVE-2017-15265.ll \
    --output /home/LLM4Con/LinConVul/CVE-2017-15265/facts.json \
    --cve-id CVE-2017-15265
```

### 批量提取

```bash
./scripts/extract_all_facts.sh
```

批量脚本会：
1. 遍历 `LinConVul/` 下所有 CVE 目录
2. 自动编译生成 LLVM bitcode（如果不存在）
3. 运行 facts_extractor 提取事实
4. 将所有结果合并到 `extracted_facts/all_facts.json`

## 输出格式

每个样本生成一个 `facts.json` 文件：

```json
{
    "id": "CVE-2017-15265",
    "code_file": "CVE-2017-15265.c",
    "ground_truth": {
        "bug_type": "UseAfterFree",
        "bug_lines": [124, 131],
        "description": "Vulnerability: CVE-2017-15265"
    },
    "verifiable_facts": {
        "shared_variables": [
            {
                "var_name": "port",
                "threads": ["creator_thread_entry", "deleter_thread_entry"],
                "evidence": "accessed by 2 threads"
            }
        ],
        "memory_accesses": [
            {"line": 113, "var": "port", "type": "WRITE", "thread": "creator_thread_entry", "code": "..."},
            {"line": 131, "var": "port", "type": "FREE", "thread": "deleter_thread_entry", "code": "..."}
        ],
        "alias_pairs": [
            {"var_a": "port", "var_b": "found", "is_alias": true}
        ],
        "lock_operations": [
            {"line": 100, "lock": "ports_mutex", "op": "acquire", "thread": "...", "code": "..."},
            {"line": 104, "lock": "ports_mutex", "op": "release", "thread": "...", "code": "..."}
        ],
        "protected_regions": [
            {"start": 100, "end": 104, "lock": "ports_mutex", "thread": "..."}
        ],
        "thread_contexts": [
            {"function": "creator_thread_entry", "start_line": 159, "end_line": 165, "thread_id": 1},
            {"function": "deleter_thread_entry", "start_line": 167, "end_line": 177, "thread_id": 2}
        ],
        "parallel_pairs": [
            {"line_a": 159, "line_b": 167, "thread_a": "creator_thread_entry", "thread_b": "deleter_thread_entry", "can_parallel": true}
        ],
        "happens_before": [
            {"line_a": 209, "line_b": 159, "relation": "before", "type": "fork"}
        ],
        "unprotected_accesses": [
            {"line": 124, "var": "port", "type": "WRITE", "thread": "creator_thread_entry", "locks_held": []}
        ]
    }
}
```

## 提取的事实类型

| 事实类型 | 说明 | 对应 Claim |
|---------|------|-----------|
| `shared_variables` | 被多个线程访问的变量 | `assert_shared(var)` |
| `memory_accesses` | 每行代码的内存读/写/释放操作 | `assert_access(line, var, type)` |
| `alias_pairs` | 指针别名关系 | `assert_alias(var_a, var_b)` |
| `lock_operations` | 锁的获取/释放位置 | `assert_lock_acquire/release(line, lock)` |
| `protected_regions` | 临界区范围 | `assert_protected(line, var, lock)` |
| `thread_contexts` | 线程入口函数及范围 | `assert_parallel(line_a, line_b)` |
| `parallel_pairs` | 可能并行执行的线程对 | `assert_parallel(line_a, line_b)` |
| `happens_before` | Happens-Before 关系（fork/join） | `assert_happens_before(line_a, line_b)` |
| `unprotected_accesses` | 未被锁保护的共享变量访问 | 组合验证 |

## Claim 验证器

提取的事实可用于构建 CPGVerifier，验证 LLM 推理过程中的 Claims：

```python
from cpg_verifier import CPGVerifier

verifier = CPGVerifier("facts.json")

# 验证共享变量断言
verifier.verify_shared("port")  # True

# 验证内存访问断言
verifier.verify_access(124, "port", "WRITE")  # True

# 验证锁保护断言
verifier.verify_protected(124, "port", "ports_mutex")  # False (未保护)

# 验证并行执行断言
verifier.verify_parallel(124, 131)  # True
```

## Reward 计算

```python
def compute_process_reward(claims, verifier):
    reward = 0.0
    for claim in claims:
        result = verifier.verify(claim)
        if result == True:
            reward += 0.5   # 事实正确
        elif result == False:
            reward -= 2.0   # 幻觉惩罚
    return reward
```

## 依赖

- LLM4Con 完整构建环境
- Phasar（指针分析）
- Joern（CPG 生成）
- LLVM/Clang（bitcode 编译）

## 开发状态

- [x] FactsExtractor.h 头文件
- [x] FactsExtractor.cpp 实现
- [x] facts_extractor_main.cpp 主程序
- [x] CMakeLists.txt 构建配置
- [x] extract_all_facts.sh 批量脚本
- [x] 单样本测试 (CVE-2017-15265 成功)
- [ ] 批量提取

## 测试结果 (CVE-2017-15265)

```
Shared Variables: 3
Memory Accesses: 32
Alias Pairs: 3
Lock Operations: 15
Protected Regions: 9
Thread Contexts: 3
Parallel Pairs: 1
Happens-Before: 4
```

## 相关文件

- 计划文档: `.cursor/plans/cpg_facts_extractor_*.plan.md`
- LinConVul 数据集: `LinConVul/CVE-*/`
- 提取结果: `extracted_facts/`
