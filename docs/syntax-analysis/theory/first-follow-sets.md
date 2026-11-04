# FIRST 与 FOLLOW 集计算

本文依据[改造后的语法文法](grammar-specification.md)计算各产生式**右部符号串的 FIRST 集**和各**非终结符的 FOLLOW 集**，用于递归下降分析的预测选择及 LL(1) 条件检查。

## 符号约定

- `ID`：标识符；`NUM`：无符号整数；`ε`：空串；`$`：输入结束标记。
- `S = { if, while, for, read, write, "{", ID, ";" }`：语句的起始符集合。
- `P = { "(", ID, NUM }`：算术表达式的起始符集合。
- `R = { ">", "<", ">=", "<=", "==", "!=" }`：关系运算符集合。

## 产生式右部符号串的 FIRST 集

### 程序与声明

| 非终结符 | 产生式右部 α | FIRST(α) |
| --- | --- | --- |
| `<program>` | `{ <declaration_list> <statement_list> }` | `{ "{" }` |
| `<declaration_list>` | `<declaration_stat> <declaration_list>` | `{ int }` |
| `<declaration_list>` | `ε` | `{ ε }` |
| `<declaration_stat>` | `int ID <variable_list> ;` | `{ int }` |
| `<variable_list>` | `, ID <variable_list>` | `{ "," }` |
| `<variable_list>` | `ε` | `{ ε }` |
| `<statement_list>` | `<statement> <statement_list>` | `{ if, while, for, read, write, "{", ID, ";" }` |
| `<statement_list>` | `ε` | `{ ε }` |

### 语句与控制结构

| 非终结符 | 产生式右部 α | FIRST(α) |
| --- | --- | --- |
| `<statement>` | `<if_stat>` | `{ if }` |
| `<statement>` | `<while_stat>` | `{ while }` |
| `<statement>` | `<for_stat>` | `{ for }` |
| `<statement>` | `<read_stat>` | `{ read }` |
| `<statement>` | `<write_stat>` | `{ write }` |
| `<statement>` | `<compound_stat>` | `{ "{" }` |
| `<statement>` | `<assignment_stat>` | `{ ID }` |
| `<statement>` | `;` | `{ ";" }` |
| `<if_stat>` | `if ( <bool_expression> ) <statement> <else_part>` | `{ if }` |
| `<else_part>` | `else <statement>` | `{ else }` |
| `<else_part>` | `ε` | `{ ε }` |
| `<while_stat>` | `while ( <bool_expression> ) <statement>` | `{ while }` |
| `<for_stat>` | `for ( <assignment_expression> ; <bool_expression> ; <assignment_expression> ) <statement>` | `{ for }` |
| `<write_stat>` | `write <arithmetic_expression> ;` | `{ write }` |
| `<read_stat>` | `read ID ;` | `{ read }` |
| `<compound_stat>` | `{ <statement_list> }` | `{ "{" }` |
| `<assignment_expression>` | `ID = <arithmetic_expression>` | `{ ID }` |
| `<assignment_stat>` | `<assignment_expression> ;` | `{ ID }` |

### 表达式

| 非终结符 | 产生式右部 α | FIRST(α) |
| --- | --- | --- |
| `<bool_expression>` | `<arithmetic_expression> <relop> <arithmetic_expression>` | `{ ID, "(", NUM }` |
| `<relop>` | `>` | `{ ">" }` |
| `<relop>` | `<` | `{ "<" }` |
| `<relop>` | `>=` | `{ ">=" }` |
| `<relop>` | `<=` | `{ "<=" }` |
| `<relop>` | `==` | `{ "==" }` |
| `<relop>` | `!=` | `{ "!=" }` |
| `<arithmetic_expression>` | `<term> <arithmetic_tail>` | `{ ID, "(", NUM }` |
| `<arithmetic_tail>` | `+ <term> <arithmetic_tail>` | `{ "+" }` |
| `<arithmetic_tail>` | `- <term> <arithmetic_tail>` | `{ "-" }` |
| `<arithmetic_tail>` | `ε` | `{ ε }` |
| `<term>` | `<factor> <term_tail>` | `{ ID, "(", NUM }` |
| `<term_tail>` | `* <factor> <term_tail>` | `{ "*" }` |
| `<term_tail>` | `/ <factor> <term_tail>` | `{ "/" }` |
| `<term_tail>` | `ε` | `{ ε }` |
| `<factor>` | `( <arithmetic_expression> )` | `{ "(" }` |
| `<factor>` | `ID` | `{ ID }` |
| `<factor>` | `NUM` | `{ NUM }` |

## 非终结符的 FOLLOW 集

| 非终结符 | FOLLOW 集 |
| --- | --- |
| `<program>` | `{ "$" }` |
| `<declaration_list>` | `{ if, while, for, read, write, "{", ID, ";", "}" }` |
| `<declaration_stat>` | `{ int, if, while, for, read, write, "{", ID, ";", "}" }` |
| `<variable_list>` | `{ ";" }` |
| `<statement_list>` | `{ "}" }` |
| `<statement>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<if_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<else_part>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<while_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<for_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<write_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<read_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<compound_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<assignment_expression>` | `{ ";", ")" }` |
| `<assignment_stat>` | `{ if, while, for, read, write, "{", ID, ";", "}", else }` |
| `<bool_expression>` | `{ ";", ")" }` |
| `<relop>` | `{ ID, "(", NUM }` |
| `<arithmetic_expression>` | `{ ";", ")", ">", "<", ">=", "<=", "==", "!=" }` |
| `<arithmetic_tail>` | `{ ";", ")", ">", "<", ">=", "<=", "==", "!=" }` |
| `<term>` | `{ ";", ")", ">", "<", ">=", "<=", "==", "!=", "+", "-" }` |
| `<term_tail>` | `{ ";", ")", ">", "<", ">=", "<=", "==", "!=", "+", "-" }` |
| `<factor>` | `{ ";", ")", ">", "<", ">=", "<=", "==", "!=", "+", "-", "*", "/" }` |

## LL(1) 条件检查

改造后的文法不存在直接左递归，各非终结符的不同候选式均可根据 FIRST 集区分，**唯一的预测冲突位于 `else_part`**。

```text
else_part → else statement | ε
```

由于 `else ∈ FOLLOW(else_part)`：

```text
FIRST(else statement) ∩ FOLLOW(else_part) = { else } ≠ ∅
```

因此，该文法整体**不属于严格 LL(1)**。语法分析实现需对悬挂 `else` 采用就近匹配策略；这属于明确的解析约定，不应被描述为已消除文法的歧义。

除 `else_part` 外，`declaration_list`、`variable_list`、`statement_list`、`arithmetic_tail` 和 `term_tail` 的非空候选式 FIRST 集均与对应 FOLLOW 集不相交。

[返回语法分析总览](../README.md) · [理论文档索引](README.md)
