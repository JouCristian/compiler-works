# Compiler Works

**基于 C11 的编译原理课程实验工程**。采用单仓库、分阶段的开发方式，完整保留词法分析器实现、测试输入、有限自动机设计及运行结果，为后续语法分析与语义处理提供基础。

**实验一 · 词法分析：已完成并通过指定样例验收（2026-10-29）**

## 实验一成果

| 验收指标 | 实测结果 |
| :--- | ---: |
| 合法 Token | **89** |
| 保留字 Token | **15** |
| 词法错误 | **7** |
| 出错行数 | **6** |
| 合法 Token 文件 | **90 行（含表头）** |

本次实验使用 [指定测试源程序](tests/instructor_lexer_test.txt) 验证了保留字、标识符、整数、分界符、运算符、两类注释、最长匹配、错误定位及文件输出。详见 [测试报告与全部七张实测截图](docs/lexical-analysis/results/README.md)。

[![词法分析器错误检测及统计结果](docs/lexical-analysis/results/screenshots/01-errors-and-statistics.png)](docs/lexical-analysis/results/screenshots/01-errors-and-statistics.png)

## 功能与实现

- **词法识别**：标识符、七个保留字、无符号整数、六个分界符、十一个运算符。
- **注释处理**：识别并跳过单行注释 `//` 与块注释 `/*...*/`，同步维护行号。
- **错误检测**：识别非法字符、非法数字词素、超长词素及未闭合块注释；分别统计错误数与错误行数。
- **输出模式**：默认只显示错误和统计；指定 `--all-tokens` 可查看 ANSI 彩色 Token 流。
- **文件输出**：合法 Token 的类别、词素和行号写入 `docs/lex.txt`，不含颜色控制码或非法词素。
- **工程组织**：C11、GCC、Makefile、模块化头文件与独立测试集。

## 构建与运行

运行环境为 Linux，使用 GCC（C11）与 GNU Make。**命令从仓库根目录执行**：

```bash
make clean && make

# 指定测试源程序：错误与统计
./build/lexer tests/instructor_lexer_test.txt

# 完整彩色 Token 输出
./build/lexer tests/instructor_lexer_test.txt --all-tokens

# 输出文件核验
cat docs/lex.txt
wc -l docs/lex.txt
```

也可执行 `make run` 使用默认 `tests/test.c`，或执行 `make run TESTS=tests/operatorsTest.c` 运行专项用例。

## 仓库结构

```text
compiler_works/
├── src/                         # main.c、lexer.c
├── include/                     # Lexer API、Token、ANSI 及公共宏
├── tests/                       # 指定测试与专项回归用例
├── docs/
│   ├── README.md                # 课程实验文档导航
│   ├── lexical-analysis/
│   │   ├── README.md            # 实验一设计与成果展示
│   │   ├── figures/             # 10 张 NFA / DFA 图
│   │   ├── theory/              # 4 份理论推导文档
│   │   └── results/             # 报告、输出快照、7 张实测截图
│   └── lex.txt                  # 运行时生成，不纳入版本控制
├── Makefile
└── .gitignore
```

## 设计文档

| 资料 | 内容 |
| --- | --- |
| [实验一完整说明](docs/lexical-analysis/README.md) | 程序架构、功能、10 张图与验收 |
| [正则表达式与 Token](docs/lexical-analysis/theory/regex-and-tokens.md) | 词法规则与分类优先级 |
| [NFA → DFA](docs/lexical-analysis/theory/nfa-to-dfa.md) | 合并 NFA、子集构造、38 状态 DFA |
| [DFA 最小化](docs/lexical-analysis/theory/dfa-minimization.md) | 划分迭代及 30 状态结果 |
| [最长匹配与错误恢复](docs/lexical-analysis/theory/maximal-munch.md) | 向前看、回退与错误处理 |
| [正式测试记录](docs/lexical-analysis/results/README.md) | 89/15/7/6、七张实测截图、完整输出 |

## 开发阶段

| 阶段 | 内容 | 当前状态 |
| --- | --- | --- |
| 实验一 | 词法分析 | **完成，指定样例验收通过** |
| 实验二 | 语法分析与递归下降 | 后续开发 |
| 实验三 | 语义分析与中间代码生成 | 后续开发 |

**设计说明：** 本项目针对指导书定义的简化类 C 语言，不是完整的 C 编译器。当前 `lexer.c` 通过字符分类、`fgetc()` 与 `ungetc()` 实现手写扫描；文档中的 NFA、DFA 和状态最小化用于对应词法规则的理论推导，并非代码运行时使用的 DFA 表。
