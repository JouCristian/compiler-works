# 实验一：词法分析

## 实验目标与实现

根据课程实验指导书，设计并实现简化类 C 语言的词法分析程序，包括：七个关键字、标识符、无符号整数、运算符、分界符、单行注释和块注释，识别词法错误并统计行数。

主要调用路径：

```text
src/main.c
  └── lexer_scan_file() [src/lexer.c]
        ├── 标识符 / 数字 / 分界符 / 运算符识别
        ├── skip_line_comment() / skip_block_comment()
        └── emit_token()
              ├── 错误统计和合法 Token 统计
              ├── print_token()  → 终端 ANSI 彩色输出
              └── write_token()  → docs/lex.txt（纯文本）
```

运行接口：

```bash
make clean && make
./build/lexer tests/instructor_lexer_test.txt
./build/lexer tests/instructor_lexer_test.txt --all-tokens
```

默认终端仅显示词法错误及统计；传入 `--all-tokens` 可显示所有词法单元。合法 Token 始终写到 `docs/lex.txt`，第一行是表头，错误词素不写入。运行时需从仓库根目录执行。

## 指定样例验收（2026-10-29）

[测试文件](../../tests/instructor_lexer_test.txt) 包含非法字母数字串、非法符号、小数点、前导零、未支持的 `||` 与未闭合注释。

| 指标 | 实测 |
| --- | ---: |
| Valid Tokens | **89** |
| Keywords | **15** |
| Errors | **7** |
| Error Lines | **6** |
| `docs/lex.txt` 实际行数 | **90**（表头 + 89 个合法 Token） |

实测数值与当前扫描器规则下的预期一致，终端没有编译警告（使用 `-Wall -Wextra` 的构建截图）。[完整验收记录](results/README.md) · [合法 Token 输出快照](results/instructor-lex.txt)。

## 理论设计资料

| 文档 | 内容 |
| --- | --- |
| [regex-and-tokens.md](theory/regex-and-tokens.md) | 七类正则规则、词法分类、关键字优先 |
| [nfa-to-dfa.md](theory/nfa-to-dfa.md) | ε-NFA 合并、子集构造、38 状态 DFA |
| [dfa-minimization.md](theory/dfa-minimization.md) | 逐轮等价划分、38 → 30 状态 |
| [maximal-munch.md](theory/maximal-munch.md) | 最长匹配、回退、错误恢复和行号统计 |

## 十张自动机图

| 编号 | 名称 | 图片 |
| --- | --- | --- |
| 01 | 标识符 NFA | [Identifier-NFA.png](figures/Identifier-NFA.png) |
| 02 | 保留字 NFA | [Keyword-NFA.png](figures/Keyword-NFA.png) |
| 03 | 无符号整数 NFA | [Number-NFA.png](figures/Number-NFA.png) |
| 04 | 分界符 NFA | [Delimiter-NFA.png](figures/Delimiter-NFA.png) |
| 05 | 运算符 NFA | [Operator-NFA.png](figures/Operator-NFA.png) |
| 06 | 单行注释 NFA | [Single_line_comment-NFA.png](figures/Single_line_comment-NFA.png) |
| 07 | 块注释 NFA | [Block_comment-NFA.png](figures/Block_comment-NFA.png) |
| 08 | 合并 NFA | [Seven_lexical_analysis_sum-NFA.png](figures/Seven_lexical_analysis_sum-NFA.png) |
| 09 | 确定化 DFA | [Full-DFA.png](figures/Full-DFA.png) |
| 10 | 最小化 DFA | [Min-DFA.png](figures/Min-DFA.png) |

图 09 和 10 省略了过密的公共字母数字转移，请结合上述符号化转移表与最小化记录阅读；不能将仅保留核心箭头的图视为全部逐边列出的 DFA。

## 专项回归测试

保留 `tests/numbersTest.c`、`operatorsTest.c`、`commentsTest.c`、`errorsTest.c`、`unknownTest.c`、`test.c` 等已有文件，分别覆盖整数、最长匹配、注释、未知字符与基本用例。老师给出的指定输入是**验收用例**，不能替代所有边界测试。

## 设计范围说明

这是一门课程的简化语言 Lexer，不是标准 C 编译器。注释作为自动机的接受类别被跳过；不在代码中生成 COMMENT Token。 `12abc` 等词素的整体错误恢复与正则自动机的合法接受语言相区分。语法检查与语义处理不属于实验一；部分输入可以词法正确但语法错误。

[返回课程文档主页](../README.md)
