# 实验一 · 指定源程序测试结果

**测试日期：** 2026-10-29  
**提交版本：** C11 手写 Lexer（`src/main.c`、`src/lexer.c`）  
**测试输入：** [instructor_lexer_test.txt](../../../tests/instructor_lexer_test.txt)  
**运行环境：** Ubuntu，GCC / make，使用 `-std=c11 -Wall -Wextra -g -O0` 构建。

## 测试命令

```bash
make clean && make
./build/lexer tests/instructor_lexer_test.txt
./build/lexer tests/instructor_lexer_test.txt --all-tokens
wc -l docs/lex.txt
```

## 实际运行结果

| 指标 | 用户实测 | 当前程序规则下的独立核对 | 结论 |
| --- | ---: | ---: | --- |
| 合法 Token | **89** | 89 | 一致 |
| 关键字 | **15** | 15 | 一致 |
| 词法错误 | **7** | 7 | 一致 |
| 错误行数 | **6** | 6 | 一致 |
| `docs/lex.txt` 行数 | **90** | 90（含表头） | 一致 |

终端运行截图由用户提供；本报告记录的是**本次样例**的真实运行结果，并不宣称覆盖所有可能输入。

## 七个词法错误

| 行号 | 错误词素 | 说明 |
| ---: | --- | --- |
| 3 | `2a` | 非法数字开头字母数字串 |
| 4 | `@` | 未知字符 |
| 7 | `.` | 未支持的浮点分隔符 |
| 8 | `0100` | 前导零整数字面量 |
| 13 | `|` | 非法字符（第 1 个竖线） |
| 13 | `|` | 非法字符（第 2 个竖线） |
| 20 | `/*` | 块注释没有结束标记 */ |

错误行号集合为 **{3, 4, 7, 8, 13, 20}**，共六行；第 13 行有两个错误，所以总错误数比错误行数多一。

## 行为核查

- 第 1 行中文单行注释被跳过；第 10 行单行注释和第 17—19 行闭合块注释也被跳过。
- `read 2a;` 将连续词素 `2a` 作为一个 WRONG。
- `10.23` 分别输出合法 `10`、错误 `.`、合法 `23`，而非支持浮点数。
- `0100` 因前导零错误作为一个 WRONG。
- `a || b2` 中两个 `|` 分别计为错误。
- `>=`、`==`、`!=` 被正确识别为双字符运算符。
- 第 20 行未闭合的 `/*` 在 EOF 时报错，行号为注释起始行。
- 已输出的合法 Token 可在 [instructor-lex.txt](instructor-lex.txt) 逐行查看：89 条数据，另有一行表头，**不包含 ANSI 转义序列或 WRONG Token**。

## 截图归档说明

本次用户提供七张原始截图，分别覆盖默认错误输出、完整 Token 输出的开始/中间/结束，以及合法 Token 文件的起始和末尾部分。截图二进制文件须单独添加至 `results/screenshots/`；本 Markdown 测试记录及输出快照均可在 GitHub 直接查看，截图尚未在此版本中作为仓库资产链接。

推荐保存文件名：

```text
screenshots/
├── 01-errors-and-statistics.png
├── 02-all-tokens-start.png
├── 03-all-tokens-middle-a.png
├── 04-all-tokens-middle-b.png
├── 05-all-tokens-end.png
├── 06-lex-output-first.png
└── 07-lex-output-last.png
```

## 结论

**实验一指定样例验收通过。** 词法分类、中文注释跳过、最长匹配、错误检测、错误行数统计及 `lex.txt` 生成均与本次样例的预期一致。后续实验二可在此版本基础上开发语法分析功能。

[实验一主页](../README.md) · [项目主页](../../../README.md)
