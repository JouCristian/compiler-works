# 最长匹配与错误恢复

## 1. 最长匹配原则

从未处理的源码位置开始，尽可能识别当前可接受的最长合法词素。当再读一个字符无法继续匹配当前 Token 时，保留最长已接受前缀，再扫描后续输入。

当前 `lexer.c` 不使用通用 DFA 状态表，而是通过 `fgetc()` 读取、条件分支识别和 `ungetc()` 回退实现。

## 2. 运算符与注释示例

| 输入 | 输出 Token / 动作 | 理由 |
| --- | --- | --- |
| `>=` | OPERATOR(>=) | 双字符优先 |
| `==` | OPERATOR(==) | 比单独 `=` 更长 |
| `!=` | OPERATOR(!=) | 单独的 `!` 不合法 |
| `===` | OPERATOR(==) + OPERATOR(=) | 先匹配最长合法前缀 |
| `>>` | OPERATOR(>) + OPERATOR(>) | 未定义 `>>` |
| `/` | OPERATOR(/) | 非注释 |
| `//hello` | 跳过单行注释 | `/` 后续出现 `/` |
| `/*hello*/` | 跳过块注释 | `/` 后续出现 `*`，以首次 `*/` 结束 |

例如扫描到 `>` 后，先用 `fgetc(fp)` 查看下一字符是否为 `=`；如不是，用 `ungetc()` 将多读字符交回，避免吞掉下一词素。

## 3. 保留字与标识符

标识符扫描直到连续字母数字串结束，再使用 `is_keyword()` 判别：

- `int` → KEYWORD
- `int123` → IDENTIFIER
- `while2` → IDENTIFIER

不能遇到 `int` 便立即输出，因为后面可能继续跟数字或字母。

## 4. 错误恢复约定

合法整数表达式为 `0|[1-9][0-9]*`，但实验指导书没有规定数字后跟字母时应如何聚合错误词素。当前 Lexer 特别约定**以数字开始的连续字母数字串**整体检查：

- `012` → WRONG(012)
- `02zjy` → WRONG(02zjy)
- `12abc` → WRONG(12abc)

它不同于纯合法词素正则式的最长前缀匹配。应区分“合法语言 DFA”与“错误输入的恢复策略”。

其他 WRONG 可能来自单独的 `!`、未知字符（`@`、`#` 等）、超长词素，或 EOF 时未闭合的 `/*` 注释。错误词素不写入 `docs/lex.txt`。

## 5. 行号和统计

- 外层扫描读取 `\n` 时行号加一。
- `skip_line_comment()` 在内部消耗换行后更新行号，外层不重复加。
- `skip_block_comment()` 消耗块注释内的换行并更新行号。
- 每遇到一个 WRONG，错误数加一；与 `last_error_line` 不同时，错误行数加一。
- 注释不计入合法 Token，合法 Token 输出到 `docs/lex.txt`。

## 6. 回归与验收

```bash
make clean && make
make run TESTS=tests/operatorsTest.c
./build/lexer tests/numbersTest.c --all-tokens
./build/lexer tests/commentsTest.c --all-tokens
./build/lexer tests/unknownTest.c
wc -l docs/lex.txt
```

目前输出文件有**一行表头**，若没有写入错误，`文件行数 = 合法 Token 数 + 1`。

还建议测试 `/**/`、单行注释遇 EOF、块注释未闭合、`a/*comment*/=2`、连续运算符 `===` 与 `>>`、超长标识符及超长数字词素。

指定源程序已于 2026-10-29 完成测试，实测结果为：**89 个合法 Token、15 个关键字、7 个错误、6 个错误行**。详见 [正式验收报告与截图](../results/README.md) 和 [合法 Token 输出快照](../results/instructor-lex.txt)。

[返回实验一主页](../README.md)
