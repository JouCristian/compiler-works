# 课程实验文档

本目录收录 Compiler Works 的实验设计、理论推导、测试数据及验收证据。项目入口与构建方式见[仓库 README](../README.md)。

## 实验一 · 词法分析

| 文档 | 内容 |
| --- | --- |
| [实验总览](lexical-analysis/README.md) | 功能、词法规则、实现结构及测试摘要 |
| [验收报告](lexical-analysis/results/README.md) | 89 个合法 Token、7 个词法错误及七张实测截图 |
| [词法规则与 Token](lexical-analysis/theory/regex-and-tokens.md) | 正则规则与分类方式 |
| [NFA → DFA](lexical-analysis/theory/nfa-to-dfa.md) · [DFA 最小化](lexical-analysis/theory/dfa-minimization.md) | 确定化及状态最小化推导 |
| [最长匹配与错误处理](lexical-analysis/theory/maximal-munch.md) | 扫描、回退及错误策略 |
| [合法 Token 输出快照](lexical-analysis/results/instructor-lex.txt) | 指定样例的 Token 文件归档 |

## 实验二 · 语法分析

| 文档 | 内容 |
| --- | --- |
| [实验总览](syntax-analysis/README.md) | 文法改造、Parser 实现及错误记录 |
| [验收报告](syntax-analysis/results/README.md) | 16 条首错报告、最终通过结果及 17 张实测截图 |
| [改造后的文法](syntax-analysis/theory/grammar-specification.md) | 22 组非终结符产生式 |
| [FIRST 与 FOLLOW 集](syntax-analysis/theory/first-follow-sets.md) | 44 个候选式的 FIRST 集、FOLLOW 集及 LL(1) 条件检查 |
| [终端截图索引](syntax-analysis/results/screenshots-manifest.md) | 原始测试 PNG 及对应行号 |
| [语法错误日志快照](syntax-analysis/results/syntax-errors-snapshot.txt) | 16 次运行的原始首错记录 |

## 文件与验收约定

- `theory/` 存放文法、数学推导和算法设计说明；`results/` 存放正式测试记录和截图。
- `docs/lexical-analysis/lex.txt` 为 Lexer 运行产物，不纳入版本控制。
- `docs/syntax-analysis/results/syntax-errors.txt` 是可追加、可由 `make clean` 清除的运行时日志；`syntax-errors-snapshot.txt` 是保留的验收快照。
- 教师原始语法测试文件保留错误，以便复现首错停止行为；最终成功结果来自逐轮修正版本。

[返回项目主页](../README.md)
