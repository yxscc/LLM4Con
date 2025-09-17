# ConCodeQL 对比实验数据整理报告

## 整理信息
- **整理时间**: 2025年1月7日
- **源目录**: `/home/ConCodeQL/ConCodeQL-comparison-output`
- **目标目录**: `/home/ConCodeQL/ConCodeQL-comparison-organized`

## 目录结构
```
ConCodeQL-comparison-organized/
├── deepseek-v3.1/           # DeepSeek v3.1 模型结果
│   ├── CVE-2009-3547/
│   │   └── run_01/          # 运行结果
│   ├── CVE-2011-2183/
│   │   └── run_01/
│   └── ... (共56个CVE)
└── gemini-2.5-pro/          # Gemini 2.5 Pro 模型结果
    ├── CVE-2009-3547/
    │   └── run_01/
    ├── CVE-2011-2183/
    │   └── run_01/
    └── ... (共56个CVE)
```

## 统计信息
- **LLM模型数量**: 2个
  - deepseek-v3.1
  - gemini-2.5-pro
- **CVE总数**: 56个
- **每个LLM的CVE数量**: 56个
- **总运行次数**: 112个 (56 CVE × 2 LLM)

## 数据内容
每个CVE目录下的run_01包含以下文件:
- `CCPG.dot`: 控制流图
- `functions/`: 函数分析结果
- `llm_conversations.log`: LLM对话日志
- `llm_evaluation_report.md`: LLM评估报告
- `llm_simplified_trace.log`: 简化的跟踪日志
- `stateful_bugs/`: 状态相关漏洞
- `temporal_rules.log`: 时间规则日志
- `thread-creation-tree.dot`: 线程创建树
- `threads/`: 线程分析结果
- `zero_shot_analysis.txt`: 零样本分析结果

## 对比实验说明
此目录包含基于两个不同LLM模型的对比实验结果，可以用于:
1. 比较不同LLM在相同CVE上的分析能力
2. 评估模型性能差异
3. 分析模型在特定类型漏洞上的表现

