# 课程实验文档 · Compiler Works

本目录集中展示编译器课程实验的设计依据、自动机模型、推导过程与实测结果。源代码位于仓库 `src/`，测试输入位于 `tests/`。

## 实验目录

| 阶段 | 实验内容 | 成果 |
| --- | --- | --- |
| **[实验一 · 词法分析](lexical-analysis/README.md)** | 正则规则、七类 NFA、合并 NFA、DFA 确定化与最小化、C11 Lexer | **已完成指定样例验收** |
| 实验二 · 语法分析 | 文法分析、FIRST/FOLLOW、递归下降分析器 | 后续开发 |
| 实验三 · 语义分析与翻译 | 语义处理与中间代码生成 | 后续开发 |

## 实验一成果导航

| 分类 | 内容 |
| --- | --- |
| [词法分析总览](lexical-analysis/README.md) | 系统结构、支持的词法规则、运行命令与实验结论 |
| [理论推导](lexical-analysis/theory/nfa-to-dfa.md) | 子集构造与 DFA 状态转换；另见 [最小化](lexical-analysis/theory/dfa-minimization.md) |
| [自动机图](lexical-analysis/README.md#十张自动机图) | 7 张规则 NFA、合并 NFA、确定化 DFA 与最小化 DFA，共 10 张 |
| [验收报告](lexical-analysis/results/README.md) | 完整错误表、89 个 Token 的统计及 7 张终端截图 |
| [合法 Token 输出快照](lexical-analysis/results/instructor-lex.txt) | 指定源程序产生的 89 条合法 Token（另含表头） |

### 指定样例验收摘要

| Valid Tokens | Keywords | Errors | Error Lines | lex.txt |
| ---: | ---: | ---: | ---: | ---: |
| **89** | **15** | **7** | **6** | **90 行** |

[![指定样例错误报告](lexical-analysis/results/screenshots/01-errors-and-statistics.png)](lexical-analysis/results/screenshots/01-errors-and-statistics.png)

## 文件组织

- `lexical-analysis/figures/`：10 张已归档的自动机设计图。
- `lexical-analysis/theory/`：四份可在线阅读的数学推导和算法说明。
- `lexical-analysis/results/`：正式验收报告、输出快照、实际终端截图。
- `docs/lex.txt`：程序运行时生成的纯文本文件，由 Git 忽略；可复现的正式输出快照已另行归档。

[返回项目主页](../README.md)
