# 实验二 · 递归下降语法分析

本实验针对课程指导书定义的简化类 C 语言，基于 C11 实现语法分析模块。系统以前一阶段的词法分析结果为输入，按照改造后的文法执行递归下降分析。

## 功能与实现状态

| 功能模块 | 实现状态 |
| --- | --- |
| TokenBuffer 数据传递 | 已完成 |
| 词法错误检查与语法分析阶段控制 | 已完成 |
| Parser 状态初始化与 Token 匹配接口 | 已完成 |
| 文法改造、FIRST / FOLLOW 集与 LL(1) 检查 | 理论设计完成 |
| 递归下降语法识别 | 待实现 |
| 语法错误输出、记录与测试验收 | 待实现 |

### 模块结构

```text
main.c
  ├── lexer_scan_file()                [lexer.c]
  │     ├── 词法识别、终端输出和文件记录
  │     └── 合法 Token → TokenBuffer
  ├── LexerState.error_count 检查
  │     ├── 存在词法错误 → 不进入语法分析
  │     └── 词法检查通过 → parser_init()  [parser.c]
  └── token_buffer_free()
```

当前 Parser 已具备 `parser_init()`、`parser_peek()`、`parser_advance()`、`parser_match()` 和 `parser_expect()` 等基础接口；完整的产生式分析函数尚未实现。

## 文法设计

依据指导书的 18 组原始语法规则完成左递归消除和左公因子提取，形成包含 **22 组非终结符产生式、44 个候选式**的改造文法。

语句文法涵盖声明、赋值、输入输出、条件分支、循环及复合语句；表达式文法涵盖整数、标识符、四则运算及关系比较。悬挂 `else` 保留特殊解析约定，其冲突原因在理论文档中单独说明。

| 理论文档 | 主要内容 |
| --- | --- |
| [改造后的语法文法](theory/grammar-specification.md) | 22 组完整产生式及文法改造 |
| [FIRST 与 FOLLOW 集](theory/first-follow-sets.md) | 44 个候选式的 FIRST 集、全部 FOLLOW 集及 LL(1) 检查 |
| [理论资料索引](theory/README.md) | 语法设计文档导航 |

## 编译与运行

在仓库根目录执行：

```bash
make clean && make

# 词法正确的测试输入：验证 Parser 初始化与 Token 传递
./build/lexer tests/parser/valid.c

# 包含词法错误的指定输入：验证错误阻断
./build/lexer tests/lexer/instructor_lexer_test.txt
```

词法分析结果输出至 `docs/lexical-analysis/lex.txt`。上述命令仅验证已实现的分析流程；**当前版本尚不提供完整的源程序语法判定**。

## 后续开发

递归下降产生式函数、语法错误定位与文件记录、指定源程序的语法错误统计将在后续开发阶段实现。

[返回文档首页](../README.md) · [返回项目主页](../../README.md)
