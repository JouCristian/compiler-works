# 词法规则、正则表达式与 Token 分类

## 1. 基础约定

依据《编译原理》实验指导书第 1.2 节，本项目的源语言是简化类 C 语言：

- `L = [A-Za-z]`：英文字母。
- `D = [0-9]`：十进制数字。
- `NZ = [1-9]`：非零数字。
- `Σ`：输入字符集合；`NL`：换行符 `\n`。
- 正则表达式中 `|` 表示选择，`*` 表示零次或多次重复；必要时使用反斜杠转义标点。

## 2. 七类词法规则

| 规则 | 正则表达式 | 接受类别或动作 | 示例 |
| --- | --- | --- | --- |
| 标识符 | `[A-Za-z][A-Za-z0-9]*` | IDENTIFIER | `abc123` |
| 保留字 | `if|else|for|while|int|write|read` | KEYWORD | `int` |
| 无符号整数 | `0|[1-9][0-9]*` | NUMBER | `0`、`1024` |
| 分界符 | `,|\(|\)|;|\{|\}` | DELIMITER | `,`、`;`、`}` |
| 运算符 | `\+|-|\*|/|=|<|>|>=|<=|!=|==` | OPERATOR | `/`、`<=`、`!=` |
| 单行注释 | `//[^\n]*` | COMMENT，跳过 | `// hello` |
| 多行注释 | `/\*([^*]|\*+[^*/])*\*+/` | COMMENT，跳过 | `/* hello */` |

注释不支持嵌套；单行注释在换行或 EOF 结束，块注释在首次 `*/` 结束。块注释未闭合时报告词法错误。

## 3. TokenType 与理论接受类别的区别

代码中的 `include/lexer.h` 目前定义：

| 类型 | 语义 | 写入 `docs/lex.txt` | 计数 |
| --- | --- | --- | --- |
| TOKEN_KEYWORD | 七个保留字 | 是 | 合法 Token + 关键字 |
| TOKEN_IDENTIFIER | 非保留字标识符 | 是 | 合法 Token |
| TOKEN_NUMBER | 无符号整数 | 是 | 合法 Token |
| TOKEN_OPERATOR | 十一个运算符 | 是 | 合法 Token |
| TOKEN_DELIMITER | 六种分界符 | 是 | 合法 Token |
| TOKEN_WRONG | 非法词素 | 否 | 错误及错误行数 |

理论自动机有 COMMENT 接受类别，但代码没有对应的 `TOKEN_COMMENT`：`skip_line_comment()` 和 `skip_block_comment()` 直接消费注释，不调用合法 Token 输出。未闭合块注释则产生一个 `TOKEN_WRONG`。

## 4. 优先级与最长匹配

- **保留字优先：** `int` 同时符合 IDENTIFIER 和 KEYWORD，最终分类为 KEYWORD；`int123` 是 IDENTIFIER。当前实现先扫描完整标识符，再调用 `is_keyword()`。
- **最长匹配：** `>=`、`==`、`!=` 优先于可接受的较短前缀；例如 `===` 被划分为 `==` 与 `=`。
- **错误聚合：** 当前实现对以数字开头的连续字母数字串整体判断，`012`、`02zjy`、`12abc` 作为一个非法词素报错。这是额外的错误恢复约定，并非合法 NUMBER NFA 的接受语言。
- **长度限制：** `MAX_TOKEN_LEN=128`，单个词素最多保存 127 个字符再加 `\0`；超长词素被判为错误，显示时可能截断。
- **未知字符：** 不属于空白和合法词素起始类别的字符，产生 WRONG Token。

## 5. 输出格式

程序将合法 Token 写入 `docs/lex.txt`，列为 `type`、`character`、`line`，第一行为表头，内容不包含 ANSI 控制符。默认终端仅显示错误和统计，`--all-tokens` 额外展示全部 Token。

## 6. 对应图纸

[01 标识符](../figures/Identifier-NFA.png) · [02 保留字](../figures/Keyword-NFA.png) · [03 整数](../figures/Number-NFA.png) · [04 分界符](../figures/Delimiter-NFA.png) · [05 运算符](../figures/Operator-NFA.png) · [06 单行注释](../figures/Single_line_comment-NFA.png) · [07 块注释](../figures/Block_comment-NFA.png)

[返回实验一主页](../README.md)
