# Compiler Works

**基于 C11 的编译原理课程实验工程**。面向课程规定的简化类 C 语言，实现词法分析、递归下降语法分析及后续编译阶段。实验一已通过指定样例验收；实验二已完成语法识别与错误日志功能。

**实验一 · 词法分析：已完成并通过指定样例验收（2026-10-29）**  
**实验二 · 语法分析：递归下降及错误日志已实现，指导样例逐轮修正后通过**

## 实验一成果

| 验收指标 | 实测结果 |
| :--- | ---: |
| 合法 Token | **89** |
| 保留字 Token | **15** |
| 词法错误 | **7** |
| 出错行数 | **6** |
| 合法 Token 文件 | **90 行（含表头）** |

本次实验使用 [指定测试源程序](tests/lexer/source_code_for_testing_lexicalanalyzer.txt) 验证了保留字、标识符、整数、分界符、运算符、两类注释、最长匹配、错误定位及文件输出。详见 [测试报告与全部七张实测截图](docs/lexical-analysis/results/README.md)。

[![词法分析器错误检测及统计结果](docs/lexical-analysis/results/screenshots/01-errors-and-statistics.png)](docs/lexical-analysis/results/screenshots/01-errors-and-statistics.png)

## 功能与实现

- **词法识别**：标识符、七个保留字、无符号整数、六个分界符、十一个运算符。
- **注释处理**：识别并跳过单行注释 `//` 与块注释 `/*...*/`，同步维护行号。
- **错误检测**：识别非法字符、非法数字词素、超长词素及未闭合块注释；分别统计错误数与错误行数。
- **输出模式**：默认只显示错误和统计；指定 `--all-tokens` 可查看 ANSI 彩色 Token 流。
- **文件输出**：合法 Token 的类别、词素和行号写入 `docs/lexical-analysis/lex.txt`，不含颜色控制码或非法词素。
- **语法分析**：基于改造后的 22 组文法，识别声明、赋值、输入输出、控制语句和嵌套复合语句。
- **错误记录**：每次运行遇到第一处语法错误后停止分析，并将错误位置和信息追加到日志文件。
- **工程组织**：C11、GCC、Makefile、模块化头文件与独立测试集。

## 构建与运行

运行环境为 Linux，使用 GCC（C11）与 GNU Make。**命令从仓库根目录执行**：

```bash
make clean && make

# 指定测试源程序：错误与统计
./build/compiler tests/lexer/source_code_for_testing_lexicalanalyzer.txt

# 完整彩色 Token 输出
./build/compiler tests/lexer/source_code_for_testing_lexicalanalyzer.txt --all-tokens

# 输出文件核验
cat docs/lexical-analysis/lex.txt
wc -l docs/lexical-analysis/lex.txt

# 实验二：合法与非法语法输入
./build/compiler tests/parser/valid.c
./build/compiler tests/parser/invalid.c

# 指导书语法测试源程序（原始文件包含故意设置的语法错误）
./build/compiler tests/parser/source_code_for_testing_syntaxanalyzer.txt

# 查看跨运行追加的语法错误日志
cat docs/syntax-analysis/results/syntax-errors.txt
```

也可执行 `make run` 使用默认 `tests/lexer/test.c`，或指定 `TESTS=tests/lexer/operatorsTest.c` 运行专项用例。`make clean` 会同时删除构建产物与累计语法错误日志；普通 `make` 不会清空日志。

## 仓库结构

```text
compiler_works/
├── src/                         # main.c、lexer.c、parser.c
├── include/                     # Lexer / Parser API、Token、公共定义
├── tests/                       # lexer/ 和 parser/ 测试输入
├── docs/
│   ├── README.md                # 课程实验文档导航
│   ├── lexical-analysis/
│   │   ├── README.md            # 实验一设计与成果展示
│   │   ├── figures/             # 10 张 NFA / DFA 图
│   │   ├── theory/              # 4 份理论推导文档
│   │   ├── results/             # 报告、输出快照、7 张实测截图
│   │   └── lex.txt              # 运行时生成，不纳入版本控制
│   └── syntax-analysis/
│       ├── README.md            # 实验二总览
│       ├── theory/              # 文法定义、FIRST/FOLLOW 与理论索引
│       └── results/             # 错误快照、测试记录与运行时日志
├── Makefile
└── .gitignore
```

## 设计文档

| 资料 | 内容 |
| --- | --- |
| [实验一完整说明](docs/lexical-analysis/README.md) | 程序架构、功能、10 张图与验收 |
| [实验二语法分析](docs/syntax-analysis/README.md) | 系统结构、开发进度及递归下降设计 |
| [改造后的语法文法](docs/syntax-analysis/theory/grammar-specification.md) | 22 组产生式与文法改造说明 |
| [FIRST 与 FOLLOW 集](docs/syntax-analysis/theory/first-follow-sets.md) | 预测分析集合与 LL(1) 冲突检查 |
| [实验二测试记录](docs/syntax-analysis/results/README.md) | 指导样例首错记录、16 条测试快照和最终验收 |
| [正则表达式与 Token](docs/lexical-analysis/theory/regex-and-tokens.md) | 词法规则与分类优先级 |
| [NFA → DFA](docs/lexical-analysis/theory/nfa-to-dfa.md) | 合并 NFA、子集构造、38 状态 DFA |
| [DFA 最小化](docs/lexical-analysis/theory/dfa-minimization.md) | 划分迭代及 30 状态结果 |
| [最长匹配与错误恢复](docs/lexical-analysis/theory/maximal-munch.md) | 向前看、回退与错误处理 |
| [正式测试记录](docs/lexical-analysis/results/README.md) | 89/15/7/6、七张实测截图、完整输出 |

## 实验二验收摘要

依据教师提供的源程序逐轮定位并修正语法错误；归档的测试日志共有 **16 条首错记录（涉及 15 个不同的错误行号）**。最终修正版本的终端截图显示 **340 个合法 Token、35 个关键字、0 个词法错误**，语法分析通过。原始带错文件保留在 `tests/parser/`，与最终修正版本的结果应加以区分。

详见 [实验二测试报告](docs/syntax-analysis/results/README.md) 和 [错误记录快照](docs/syntax-analysis/results/syntax-errors-snapshot.txt)。

## 开发阶段

| 阶段 | 内容 | 当前状态 |
| --- | --- | --- |
| 实验一 | 词法分析 | **完成，指定样例验收通过** |
| 实验二 | 语法分析与递归下降 | **实现完成；指导样例逐轮修正后验收通过** |
| 实验三 | 语义分析与中间代码生成 | 后续开发 |

**设计说明：** 本项目实现指导书定义的简化类 C 语言，并非完整 C 编译器。`lexer.c` 使用手写扫描逻辑；NFA / DFA 图为理论推导，不是运行时 DFA 表。Parser 遵循首错停止策略；多次运行的语法错误日志采用追加方式记录。
