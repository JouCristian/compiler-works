# Compiler Works

西南科技大学《编译原理》课程实验工程，以 **C11** 实现简化类 C 语言的词法分析器，并在同一工程中逐步扩展语法分析、语义处理和中间代码生成。当前**实验一（词法分析）已完成指导样例验收**。

## 当前进度

| 阶段 | 状态 | 说明 |
| --- | --- | --- |
| 实验一：词法分析 | **已完成指定样例验收** | Lexer、Token 输出、错误统计、理论图与文档 |
| 实验二：语法分析 | 尚未开始 | 将复用 Lexer，并实现语法分析器 |
| 实验三：语义分析和代码生成 | 尚未开始 | 随实验二完善后开发 |

## 项目结构

```text
compiler_works/
├── src/                            # main.c、lexer.c
├── include/                        # Token/API、ANSI 配色、公共宏
├── tests/                          # 指定样例和专项回归用例
├── docs/
│   ├── lexical-analysis/
│   │   ├── figures/                # 10 张词法自动机图
│   │   ├── theory/                 # 正则规则、DFA 构造与最小化等
│   │   └── results/                # 正式验收记录及 Token 输出快照
│   └── lex.txt                     # 运行时输出，Git 忽略
├── build/                           # 编译产物，Git 忽略
└── Makefile
```

## 编译与使用

要求 Linux、GCC（C11）和 make。**请在仓库根目录运行**，以正确生成 `docs/lex.txt`。

```bash
make clean && make

# 指定样例：仅显示词法错误及统计
./build/lexer tests/instructor_lexer_test.txt

# 指定样例：显示全部彩色 Token
./build/lexer tests/instructor_lexer_test.txt --all-tokens

# 查看输出文件
cat docs/lex.txt
wc -l docs/lex.txt
```

可执行 `make run` 使用默认 `tests/test.c`，或 `make run TESTS=tests/numbersTest.c` 指定专项测试。

## 指定样例验收结果（2026-10-29）

| 合法 Token | 关键字 | 词法错误 | 错误行 | lex.txt 行数（含表头） |
| ---: | ---: | ---: | ---: | ---: |
| **89** | **15** | **7** | **6** | **90** |

错误分别位于第 3、4、7、8、13（2 次）、20 行。注释均按规则跳过；第 20 行未闭合块注释被报告为错误。

- [测试输入（原样保留）](tests/instructor_lexer_test.txt)
- [完整验收记录](docs/lexical-analysis/results/README.md)
- [合法 Token 输出快照](docs/lexical-analysis/results/instructor-lex.txt)

## 文档与设计

- [文档目录](docs/README.md)
- [实验一设计与实现说明](docs/lexical-analysis/README.md)
- [词法规则与 Token 分类](docs/lexical-analysis/theory/regex-and-tokens.md)
- [NFA 合并与 DFA 确定化](docs/lexical-analysis/theory/nfa-to-dfa.md)
- [DFA 最小化](docs/lexical-analysis/theory/dfa-minimization.md)
- [最长匹配及错误恢复](docs/lexical-analysis/theory/maximal-munch.md)

## 设计边界

代码采用 `fgetc()`、`ungetc()`、字符分支以及辅助函数执行扫描；NFA、DFA、最小化文档用于理论推导，**并非运行时自动机表的实现**。 `TOKEN_WRONG` 的数字串聚合等行为是代码的错误恢复约定；注释识别后跳过，不计入合法 Token。课程实验的程序正确性只在已列测试与规则范围内确认，不表示具有完整 C 语言词法能力。
