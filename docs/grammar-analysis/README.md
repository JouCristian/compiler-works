# 实验二 · 递归下降语法分析

本实验面向课程指导书定义的简化类 C 语言，采用 C11 实现递归下降语法分析。程序通过词法分析获得 Token 序列，在无词法错误的前提下验证源程序的语法结构，并记录第一处语法错误。

## 功能实现

| 模块 | 实现内容 |
| --- | --- |
| 文法处理 | 消除直接左递归、提取左公因子；完成 FIRST / FOLLOW 集及 LL(1) 条件检查 |
| 数据交接 | Lexer 将合法 Token 保存到 `TokenBuffer`，由 Parser 顺序读取 |
| 语法识别 | 声明、赋值、读写、算术与布尔表达式、`if-else`、`while`、`for`、复合语句 |
| 错误处理 | 检测第一处语法错误并输出行号与错误描述；词法错误时跳过 Parser |
| 错误归档 | 将各次运行发现的语法错误以追加方式写入文本日志 |

### 程序结构

```text
src/main.c
  ├── lexer_scan_file()                 [src/lexer.c]
  │     └── 合法 Token → TokenBuffer
  ├── 检查词法错误
  └── parser_init() / parser_program()  [src/parser.c]
        ├── declaration_list()
        └── statement_list()
              └── statement() → 各类语句及表达式
```

`parser_program()` 是语法分析入口。分析过程对声明、语句、表达式按文法递归匹配，并在程序结束时检查是否存在多余 Token。对于悬挂 `else`，采用就近匹配规则。

## 文法与理论资料

| 文档 | 内容 |
| --- | --- |
| [改造后的语法文法](theory/grammar-specification.md) | 22 组非终结符产生式，共 44 个候选式 |
| [FIRST 与 FOLLOW 集](theory/first-follow-sets.md) | 产生式右部 FIRST 集、非终结符 FOLLOW 集与 LL(1) 分析 |
| [理论文档导航](theory/README.md) | 文法与预测分析资料 |

## 编译与运行

在仓库根目录执行：

```bash
make clean && make

# 合法语法
./build/lexer tests/parser/valid.c

# 语法错误：第一处错误输出到终端并追加写入日志
./build/lexer tests/parser/invalid.c

# 查看累计语法错误
cat docs/grammar-analysis/results/grammar-errors.txt
```

程序遇到词法错误时停止进入语法分析；发现语法错误时报告第一处错误，并返回非零退出码。完整语法测试用例的运行结果见[测试记录](results/README.md)。

## 错误日志管理

- **输出位置**：`docs/grammar-analysis/results/grammar-errors.txt`。
- **写入方式**：追加模式（`fopen(..., "a")`）；每次发现语法错误追加一行，包含输入文件路径、行号和错误描述。
- **统计方法**：`wc -l docs/grammar-analysis/results/grammar-errors.txt`，统计当前累计记录数。
- **清理方式**：`make clean` 删除日志和构建产物；普通 `make` 不清除历史记录。
- **版本管理**：运行时日志由 `.gitignore` 排除，`results/` 目录及其测试说明保留在仓库中。

由于分析器采用首错停止策略，日志中的每条记录对应**一次运行发现的首个语法错误**，不代表单个输入文件内全部潜在的语法错误数量。

[查看测试记录](results/README.md) · [返回文档首页](../README.md) · [返回项目主页](../../README.md)
