# 实验一：词法分析器设计与实现

本实验以指导书第 1.2 节的简化类 C 语言为对象：标识符、七个保留字、无符号整数、六个分界符、十一个运算符以及两种注释。

当前代码调用主线：

`src/main.c` → `lexer_scan_file()`（`src/lexer.c`）→ `emit_token()` → 彩色终端 / `docs/lex.txt`。

## 理论文档

1. [正则表达式、Token 类别及输出方案](theory/regex-and-tokens.md)
2. [合并 NFA 与子集构造的 DFA](theory/nfa-to-dfa.md)
3. [DFA 状态最小化与等价划分](theory/dfa-minimization.md)
4. [最长匹配与错误恢复](theory/maximal-munch.md)

## 十张自动机图

| 编号 | 图名 | 图片 |
| --- | --- | --- |
| 01 | 标识符 NFA | [Identifier-NFA.png](figures/Identifier-NFA.png) |
| 02 | 七个保留字 NFA | [Keyword-NFA.png](figures/Keyword-NFA.png) |
| 03 | 无符号整数 NFA | [Number-NFA.png](figures/Number-NFA.png) |
| 04 | 分界符 NFA | [Delimiter-NFA.png](figures/Delimiter-NFA.png) |
| 05 | 运算符 NFA | [Operator-NFA.png](figures/Operator-NFA.png) |
| 06 | 单行注释 NFA | [Single_line_comment-NFA.png](figures/Single_line_comment-NFA.png) |
| 07 | 块注释 NFA | [Block_comment-NFA.png](figures/Block_comment-NFA.png) |
| 08 | 合并 NFA | [Seven_lexical_analysis_sum-NFA.png](figures/Seven_lexical_analysis_sum-NFA.png) |
| 09 | 确定化 DFA | [Full-DFA.png](figures/Full-DFA.png) |
| 10 | 最小化 DFA | [Min-DFA.png](figures/Min-DFA.png) |

图 09、10 为**压缩展示的核心状态结构图**。某些公共转移未全部绘成单独的线，必须结合 [符号化 DFA 转移表](theory/nfa-to-dfa.md) 与 [最小化推导](theory/dfa-minimization.md) 阅读。

## 程序运行和输出

```bash
make
./build/lexer tests/unknownTest.c
./build/lexer tests/unknownTest.c --all-tokens
cat docs/lex.txt
```

- `docs/lex.txt`：类别、词素、行号三列；只有合法 Token，第一行是表头。
- 默认终端：词法错误及行号、合法 Token 总数、关键字数、错误数、错误行数。
- `--all-tokens`：额外显示合法 Token 的彩色调试信息。
- 单行与块注释识别后跳过，不计入合法 Token。

既有 `unknownTest.c` 截图测试记录：合法 Token **21**、关键字 **1**、错误 **6**、错误行数 **4**。该记录并非全部边界测试已自动化通过的声明。

## 验收与限制

- `012`、`12abc` 等数字开头的连续字母数字串按本实现的额外约定整体报告一个 `TOKEN_WRONG`。
- `write;` 一类输入可能词法合法但语法不合法；语法检查在实验二实现。
- 需进一步覆盖最长匹配、超长词素、EOF、未闭合注释等边界。
- 需使用老师**另行提供的指定源程序**运行并保留结果，才能完成正式实验验收。

[返回文档总览](../README.md)
