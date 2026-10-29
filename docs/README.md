# 课程实验文档

本目录集中保存 **编译器设计说明、自动机图、理论推导与实测证据**。源代码位于仓库 `src/`，测试输入位于 `tests/`，构建产物位于 `build/`。

## 文档导航

| 实验阶段 | 内容 | 状态 |
| --- | --- | --- |
| [实验一：词法分析](lexical-analysis/README.md) | 正则、七类 NFA、合并 NFA、确定化 DFA、最小化 DFA、Lexer 与测试 | **指定测试通过** |
| 实验二：语法分析 | 文法改造、FIRST/FOLLOW、递归下降分析器 | 待开发 |
| 实验三：语义处理和中间代码 | 语义分析、翻译与运行结果 | 待开发 |

## 实验一成果

- [理论文件（四份）](lexical-analysis/README.md#理论设计资料)
- [自动机图（十张）](lexical-analysis/README.md#十张自动机图)
- [指定源程序测试报告](lexical-analysis/results/README.md)
- [合法 Token 输出快照](lexical-analysis/results/instructor-lex.txt)

## 版本控制约定

- `docs/lex.txt` 为当前运行产生的文件，不上传到 Git；结果快照统一保存在 `lexical-analysis/results/`。
- 理论图纸使用 PNG，必要时可另外保存可编辑的矢量源文件。
- 特定测试仍保留在 `tests/`，避免只留一种用例导致后续修改缺乏回归验证。

[返回仓库主页](../README.md)
