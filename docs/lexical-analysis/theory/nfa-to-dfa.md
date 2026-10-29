# NFA 合并与 DFA 确定化

## 1. 合并各规则 NFA

从七种词法规则对应的简化 NFA 出发，增加全局初态 `S`，并通过七条 ε 转移分别连接：

```text
S --ε--> I0
S --ε--> K0
S --ε--> N0
S --ε--> D0
S --ε--> O0
S --ε--> L0
S --ε--> B0
```

各自保留接受类别 IDENTIFIER、KEYWORD、NUMBER、DELIMITER、OPERATOR、COMMENT；不同类别不能随意合并。

[图 08：合并 NFA](../figures/Seven_lexical_analysis_sum-NFA.png)

## 2. 子集构造法

定义 `move(I,a)` 为状态集 `I` 输入 `a` 可达的状态集合，`ε-closure(I)` 为其 ε 可达闭包。构造后的 DFA 转移为：

```math
\delta_D(I,a)=\varepsilon\text{-closure}(\mathrm{move}(I,a))
```

初始状态：

`Q0 = ε-closure({S}) = {S, I0, K0, N0, D0, O0, L0, B0}`。

若状态子集同时包含 IDENTIFIER 和 KEYWORD 接受状态，以 KEYWORD 为优先接受类别。

## 3. 典型子集构造结果

| 已读输入 | NFA 状态子集（主要成员） | 接受类别 |
| --- | --- | --- |
| `i` | `{I1, K_i}` | IDENTIFIER |
| `in` | `{I1, K_in}` | IDENTIFIER |
| `int` | `{I1, K_int}` | KEYWORD |
| `a` | `{I1}` | IDENTIFIER |
| `0` | `{N1}` | NUMBER |
| `1` | `{N2}` | NUMBER |
| `+` | `{O1}` | OPERATOR |
| `>` | `{O2}` | OPERATOR |
| `>=` | `{O1}` | OPERATOR |
| `!` | `{O3}` | 非接受 |
| `!=` | `{O1}` | OPERATOR |
| `/` | `{O1,L1,B1}` | OPERATOR |
| `//` | `{L2}` | COMMENT |
| `/*` | `{B2}` | 非接受 |
| `/**/` | `{B4}` | COMMENT |

## 4. DFA 状态全集：38 个

**基础 14 状态：**

`Q0, QID, QZ, QN, QD, QO, QC, QB, QS, QL, QM, QMS, QME, QX`。

**24 个保留字非空前缀状态：**

- `K_i, K_if, K_in, K_int`
- `K_e, K_el, K_els, K_else`
- `K_f, K_fo, K_for`
- `K_w, K_wh, K_whi, K_whil, K_while, K_wr, K_wri, K_writ, K_write`
- `K_r, K_re, K_rea, K_read`

`QX` 是死状态；理论 DFA 对字母表中的每个字符都有已定义的转移。主图为避免密集箭头，以规则表压缩公共转移。

## 5. 主要状态转移（符号化完整描述）

| 起始状态 | 输入 | 目标状态 |
| --- | --- | --- |
| Q0 | `i/e/f/w/r` | 对应 `K_i/K_e/K_f/K_w/K_r` |
| Q0 | 其他英文字母 | QID |
| Q0 | `0` | QZ |
| Q0 | `[1-9]` | QN |
| Q0 | `, ( ) ; { }` | QD |
| Q0 | `+ - *` | QO |
| Q0 | `= < >` | QC |
| Q0 | `!` | QB |
| Q0 | `/` | QS |
| QID | 字母或数字 | QID |
| QN | 数字 | QN |
| QC | `=` | QO |
| QB | `=` | QO |
| QS | `/` | QL |
| QS | `*` | QM |
| QL | 除 `\n` 外任意字符 | QL |
| QM | `*` | QMS |
| QM | 非 `*` 字符 | QM |
| QMS | `*` | QMS |
| QMS | `/` | QME |
| QMS | 除 `*`、`/` 外任意字符 | QM |
| QX | 任意字符 | QX |

**保留字前缀的一般规则：** 对任意 `K_p`，当追加字母 `c` 后 `pc` 仍为七个保留字中的非空前缀，转移至 `K_pc`；遇到其他仍属于字母数字的输入，转移至 `QID`；其他字符转到 `QX`。完整保留字状态亦如此，如 `K_int --1--> QID`。

**其他未列转移均至 `QX`。** 单个完整词素的理论 DFA 不应被误解成直接连续消费整个源程序。手写 Lexer 使用最长已接受前缀、回退多读字符，再开始扫描下一个 Token；因此，例如 `QZ` 无须对后面的运算符再增加转移。

## 6. 各类别状态数

| 状态类别 | 数量 |
| --- | ---: |
| 非接受 | 5 |
| IDENTIFIER | 18 |
| KEYWORD | 7 |
| NUMBER | 2 |
| DELIMITER | 1 |
| OPERATOR | 3 |
| COMMENT | 2 |
| **合计** | **38** |

非接受状态为 `Q0, QB, QM, QMS, QX`；COMMENT 的终态为 `QL, QME`。此计数对应合法词素理论自动机，未纳入代码中独立的 WRONG 错误聚合机制。

[图 09：确定化 DFA](../figures/Full-DFA.png) · [返回实验一主页](../README.md)
