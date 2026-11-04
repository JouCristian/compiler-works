# 实验二 · 递归下降语法分析

本实验针对简化类 C 语言设计递归下降语法分析程序。分析器读取 Lexer 生成的 Token 序列，依据改造后的文法检查声明、语句和表达式，并在发现第一处语法错误时停止分析。

## 功能实现

| 模块 | 实现内容 |
| --- | --- |
| 文法设计 | 18 组原始文法改造为 22 组非终结符产生式；计算 FIRST / FOLLOW 集并检查 LL(1) 条件 |
| 词法与语法衔接 | Lexer 将合法 Token 保存至 `TokenBuffer`，词法错误存在时跳过 Parser |
| 递归下降 | 识别声明、赋值、`read/write`、算术与布尔表达式、`if-else`、`while`、`for` 和复合语句 |
| 错误处理 | 首错停止，输出错误行号和期望/实际 Token |
| 错误记录 | 跨多次执行将语法错误追加至独立日志，`make clean` 可重置 |

### 实现结构

```text
main()
  ├── lexer_scan_file()         → TokenBuffer
  ├── 词法错误检查              → 有错误则停止
  └── parser_program()
        ├── declaration_list() → 声明
        └── statement_list()   → 各类语句和表达式
```

Parser 按源程序起始产生式 `{ declaration_list statement_list }` 执行分析，结束时要求 Token 全部消费。悬挂 `else` 按就近匹配原则处理。

## 理论设计

| 文档 | 内容 |
| --- | --- |
| [改造后的语法文法](theory/grammar-specification.md) | 22 组文法、左递归消除及左公因子提取 |
| [FIRST 与 FOLLOW 集](theory/first-follow-sets.md) | 44 个候选式的右部 FIRST 集、22 个非终结符 FOLLOW 集与冲突说明 |
| [理论文档索引](theory/README.md) | 语法设计资料导航 |

## 编译与使用

从仓库根目录执行：

```bash
make clean && make

# 基础合法和非法样例
./build/compiler tests/parser/valid.c
./build/compiler tests/parser/invalid.c

# 教师提供的原始测试源文件（包含故意设置的语法错误）
./build/compiler tests/parser/source_code_for_testing_syntaxanalyzer.txt

# 查看跨运行追加的语法错误
cat docs/syntax-analysis/results/syntax-errors.txt
wc -l docs/syntax-analysis/results/syntax-errors.txt
```

`make clean` 会删除构建目录和运行时累计日志；普通 `make` 不会删除既有日志。需要连续测试并累积错误记录时，不应在两次执行之间运行 `make clean`。

每次语法分析只报告第一处错误，因此要统计不同错误，需要在修正源程序后重复运行。日志记录格式为 `源文件:行号:错误描述`。原始教师测试文件保持未修正状态，**不能直接通过语法分析**。

## 验收结果

使用教师指定样例逐轮检查并修正后，归档了 **16 条语法首错记录（涉及 15 个不同的错误行号）**。最后一次成功运行的终端截图显示 **340 个合法 Token、35 个关键字、0 个词法错误和 0 个错误行**，语法分析成功。

结果基于逐轮修改后的测试版本；中间部分错误由人工直接修正，并未逐项单独运行截图，不将归档的 16 条记录视作原始文件的全部语法错误。

- [测试记录与错误明细](results/README.md)
- [已归档的错误日志快照](results/syntax-errors-snapshot.txt)
- [终端截图索引](results/screenshots-manifest.md)

[返回文档首页](../README.md) · [返回仓库主页](../../README.md)
