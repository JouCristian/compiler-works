# 实验一 · 词法分析器设计与实现

**实验一已完成，指定源程序测试通过。** 该实验以简化类 C 语言为输入，完成了词法规则识别、错误定位、Token 输出和有限自动机设计。程序采用 C11 编写，支持终端彩色分类输出与纯文本文件导出。

## 功能实现

| 功能 | 支持内容 |
| --- | --- |
| 关键字与标识符 | `if`、`else`、`for`、`while`、`int`、`write`、`read`；以字母开头的标识符 |
| 无符号整数 | `0` 或非零数字开头的十进制整数 |
| 运算符 | `+` `-` `*` `/` `=` `<` `>` `>=` `<=` `!=` `==` |
| 分界符 | `,` `(` `)` `;` `{` `}` |
| 注释 | 单行 `//`、块注释 `/*...*/`，识别后跳过 |
| 错误报告 | 非法数字词素、未知字符、无效运算符、未闭合注释等 |
| 输出与统计 | 合法 Token 文件、关键字数、错误数及出错行数 |

### 程序结构

```text
src/main.c
  └── lexer_scan_file()                  [src/lexer.c]
        ├── 标识符 / 整数 / 分界符 / 运算符识别
        ├── skip_line_comment()
        ├── skip_block_comment()
        └── emit_token()
              ├── LexerState 统计
              ├── print_token()          → 终端（ANSI 彩色）
              └── write_token()          → docs/lex.txt（合法 Token）
```

## 编译与运行

```bash
make clean && make

./build/lexer tests/instructor_lexer_test.txt
./build/lexer tests/instructor_lexer_test.txt --all-tokens

wc -l docs/lex.txt
```

默认模式显示错误和统计；`--all-tokens` 显示完整彩色 Token；`docs/lex.txt` 保存所有合法 Token，首行为表头。

## 指定样例验收 · 2026-10-29

实际运行 [instructor_lexer_test.txt](../../tests/instructor_lexer_test.txt)，得到：

| Valid Tokens | Keywords | Errors | Error Lines | lex.txt（含表头） |
| ---: | ---: | ---: | ---: | ---: |
| **89** | **15** | **7** | **6** | **90 行** |

七个词法错误位于第 3、4、7、8、13（两个）、20 行；单行注释和已闭合块注释均正确跳过，未闭合块注释在 EOF 报错。

[![词法错误输出与统计](results/screenshots/01-errors-and-statistics.png)](results/screenshots/01-errors-and-statistics.png)

**验收资料：** [完整测试报告及七张实测截图](results/README.md) · [89 个合法 Token 输出快照](results/instructor-lex.txt)。

## 理论设计资料

| 文档 | 主要内容 |
| --- | --- |
| [词法规则与 Token 类别](theory/regex-and-tokens.md) | 七类正则、保留字优先级、错误分类 |
| [NFA 合并与 DFA 确定化](theory/nfa-to-dfa.md) | ε-closure、子集构造与 38 状态 DFA |
| [DFA 最小化](theory/dfa-minimization.md) | 7 → 19 → 25 → 29 → 30 的状态划分 |
| [最长匹配与错误恢复](theory/maximal-munch.md) | `fgetc()`、`ungetc()`、边界和行号管理 |

## 十张自动机图

| 编号 | 自动机 | 原图 |
| --- | --- | --- |
| 01 | 标识符 NFA | [Identifier-NFA.png](figures/Identifier-NFA.png) |
| 02 | 保留字 NFA | [Keyword-NFA.png](figures/Keyword-NFA.png) |
| 03 | 无符号整数 NFA | [Number-NFA.png](figures/Number-NFA.png) |
| 04 | 分界符 NFA | [Delimiter-NFA.png](figures/Delimiter-NFA.png) |
| 05 | 运算符 NFA | [Operator-NFA.png](figures/Operator-NFA.png) |
| 06 | 单行注释 NFA | [Single_line_comment-NFA.png](figures/Single_line_comment-NFA.png) |
| 07 | 多行注释 NFA | [Block_comment-NFA.png](figures/Block_comment-NFA.png) |
| 08 | 合并 NFA | [Seven_lexical_analysis_sum-NFA.png](figures/Seven_lexical_analysis_sum-NFA.png) |
| 09 | 确定化 DFA | [Full-DFA.png](figures/Full-DFA.png) |
| 10 | 最小化 DFA | [Min-DFA.png](figures/Min-DFA.png) |

图 09 与图 10 对部分重复的公共转移采用了压缩标注；具体转移条件与状态划分过程在上面的理论文档中完整说明。

## 测试集与课程实验边界

`tests/` 保留了指定样例，以及数字、运算符、注释、未知字符等专项输入，便于回归测试。当前实现遵循课程规定的简化词法规则，不负责完整 C 语言词法处理或语法正确性判定。理论自动机描述合法词素的识别过程；手写 Lexer 另实现了 `TOKEN_WRONG` 错误聚合和统计逻辑。

[返回文档首页](../README.md) · [返回项目主页](../../README.md)
