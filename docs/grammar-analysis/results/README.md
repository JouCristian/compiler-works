# 实验二 · 语法分析测试记录

本记录覆盖递归下降语法识别、语法错误报告、跨运行日志追加及清理行为。测试采用 GCC / C11，从仓库根目录执行。

## 测试项目

| 测试场景 | 输入或操作 | 预期与验证结果 |
| --- | --- | --- |
| 合法源程序 | `tests/parser/valid.c` | Lexer 无错误，语法分析成功 |
| 非法算术表达式 | `tests/parser/invalid.c` | 第 3 行的 `*` 触发语法错误，返回非零退出码 |
| 重复执行错误用例 | 连续两次运行 `invalid.c` | `grammar-errors.txt` 累计两条错误记录 |
| 合法输入不追加 | 错误测试后运行 `valid.c` | 原有错误记录条数保持不变 |
| 词法错误阻断 | 含非法字符的输入 | Lexer 报错并跳过 Parser，不追加语法错误记录 |
| 缺少右花括号 | 输入在 `}` 前结束 | 报告预期 `}`、实际 EOF 的语法错误 |
| 控制语句与嵌套 | 含 `if-else`、`while`、`for`、复合语句的合法输入 | 语法分析成功 |
| 日志重置 | `make clean` 后重新运行错误用例 | 日志重新建立，初始仅有本次错误 |

上述项目已在独立构建的对应源码环境中通过验证。GitHub 仓库保留 `valid.c` 与 `invalid.c` 作为基础测试输入；其余情形可通过自定义源程序复现。

## 错误追加与清理复现

```bash
# 清除旧日志并构建程序
make clean && make

# 连续产生两次语法错误
./build/lexer tests/parser/invalid.c
./build/lexer tests/parser/invalid.c

# 输出应为 2；文件保留两条错误记录
wc -l docs/grammar-analysis/results/grammar-errors.txt

# 正确源程序不增加记录数
./build/lexer tests/parser/valid.c
wc -l docs/grammar-analysis/results/grammar-errors.txt

# 清空累计记录
make clean
```

每条记录格式为 `源文件:行号:错误描述`，与终端输出的语法错误位置及描述一致，且不包含 ANSI 颜色控制字符。日志文件为运行产物，不纳入版本控制。

## 验证范围

现有测试证明了主要语法结构的基本识别，以及错误日志的追加和清理机制。实验指导书另外要求对教师提供的指定源语言测试文件进行语法分析，并统计其错误记录；此项结果应在获得该文件并执行测试后单独补充，不以通用样例结果代替。

[返回实验二总览](../README.md) · [返回文档首页](../../README.md)
