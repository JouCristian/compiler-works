# 实验二 · 语法分析理论资料

本目录收录递归下降分析器的文法改造、FIRST/FOLLOW 集及 LL(1) 分析结果。

| 文档 | 内容 |
| --- | --- |
| [改造后的文法](grammar-specification.md) | 从 18 组原始文法改造为 22 组非终结符产生式；处理左递归与左公因子 |
| [FIRST/FOLLOW 集与 LL(1) 分析](first-follow-sets.md) | 44 个候选产生式右部的 FIRST 集、非终结符 FOLLOW 集及悬挂 `else` 冲突 |

分析器对悬挂 `else` 采用就近匹配原则，不将改造后的整个文法称为严格 LL(1)。

[实验二总览](../README.md) · [实验二验收报告](../results/README.md) · [课程文档索引](../../README.md)
