# Compiler Works

西南科技大学《编译原理》课程实验工程。项目使用 C11，实现一个简化类 C 语言的编译处理流程。当前完成了实验一词法分析器的主要功能，实验二语法分析、实验三语义分析及中间代码生成将在同一仓库中逐步开发。

## 项目结构

- `src/`：程序入口、词法扫描器源码。
- `include/`：Lexer API、Token 类型、ANSI 颜色与公共宏。
- `tests/`：词法测试输入，部分内容故意包含错误，未必符合语法规则。
- `docs/`：课程实验设计、图纸、理论推导和测试说明。
- `build/`：本地构建产物，不加入版本控制。

## 编译与运行

在仓库根目录运行：

```bash
make
make run
make run TESTS=tests/operatorsTest.c

# 默认仅显示词法错误及统计
./build/lexer tests/unknownTest.c

# 显示所有 Token（包括错误）
./build/lexer tests/unknownTest.c --all-tokens
```

合法 Token 输出在 `docs/lex.txt`（含表头，纯文本，无 ANSI 颜色）。目前代码使用相对路径 `fopen("docs/lex.txt", "w")`，所以请在仓库根目录运行，并保留 `docs/` 目录。

## 实验文档

- [全部课程文档](docs/README.md)
- [实验一：词法分析设计与实现](docs/lexical-analysis/README.md)
- [词法规则及 Token 分类](docs/lexical-analysis/theory/regex-and-tokens.md)
- [NFA 合并与 DFA 确定化](docs/lexical-analysis/theory/nfa-to-dfa.md)
- [DFA 最小化](docs/lexical-analysis/theory/dfa-minimization.md)
- [最长匹配与错误恢复](docs/lexical-analysis/theory/maximal-munch.md)

## 理论模型与代码实现的关系

理论图示描述正则规则对应的 NFA、DFA 及最小化。当前 `src/lexer.c` 是基于字符分类、`fgetc()` 向前看和 `ungetc()` 回退的手写扫描器，**不在运行时执行 NFA 子集构造或加载 DFA 状态表**。

`TokenType` 包含 KEYWORD、IDENTIFIER、NUMBER、OPERATOR、DELIMITER、WRONG；注释识别后跳过，不作为普通合法 Token 输出。

实验一理论资料和主要功能已整理，仍需用老师另行提供的指定输入做最终验收。