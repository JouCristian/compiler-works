# 改造后的语法文法

本文件给出编译原理实验二所采用的语法文法。源语言为教学用途的简化类 C 语言，原始文法包含 18 组规则；经左递归消除及左公因子提取后，文法以 22 组非终结符产生式表示。

## 文法定义

```text
(1) <program> -> 
	{<declaration_list><statement_list>}
(2) <declaration_list> -> 
	  <declaration_stat><declaration_list> 
	| ε
(3) <declaration_stat> -> 
	int ID<variable_list>;
(4) <variable_list> -> 
	  ,ID<variable_list> 
	| ε
(5) <statement_list> -> 
	  <statement><statement_list> 
	| ε
(6) <statement> ->
	  <if_stat>
	| <while_stat>
	| <for_stat>
	| <read_stat>
	| <write_stat>
	| <compound_stat>
	| <assignment_stat>
	| ;
(7) <if_stat> ->
	  if(<bool_expression>)<statement><else_part>
(8) <else_part> ->
	  else <statement>
	| ε
(9) <while_stat> ->
	  while(<bool_expression>)<statement>
(10) <for_stat> ->
	  for(<assignment_expression>;<bool_expression>;<assignment_expression>)<statement>
(11) <write_stat> ->
	  write <arithmetic_expression>;
(12) <read_stat> ->
	  read ID;
(13) <compound_stat> ->
	  {<statement_list>}
(14) <assignment_expression> ->
	  ID=<arithmetic_expression>
(15) <assignment_stat> ->
	  <assignment_expression>;
(16) <bool_expression> ->
	  <arithmetic_expression><relop><arithmetic_expression>
(17) <relop> ->
	  >
	| <
	| >=
	| <=
	| ==
	| !=
(18) <arithmetic_expression> ->
	  <term><arithmetic_tail>
(19) <arithmetic_tail> ->
	  +<term><arithmetic_tail>
	| -<term><arithmetic_tail>
	| ε
(20) <term> ->
	  <factor><term_tail>
(21) <term_tail> ->
	  *<factor><term_tail>
	| /<factor><term_tail>
	| ε
(22) <factor> ->
	  (<arithmetic_expression>)
	| ID
	| NUM
```

## 文法改造说明

| 改造类型 | 涉及非终结符 | 处理方法 |
| --- | --- | --- |
| 消除直接左递归 | `declaration_list`、`statement_list` | 改写为右递归列表 |
| 消除直接左递归 | `arithmetic_expression`、`term` | 引入 `arithmetic_tail`、`term_tail` |
| 提取左公因子 | `if_stat` | 引入 `else_part` |
| 提取左公因子 | `bool_expression` | 提取关系运算符 `relop` |

`ID` 表示标识符，`NUM` 表示无符号整数，`ε` 表示空串。其余关键字、分界符与运算符由词法分析器提供相应 Token。

## LL(1) 分析

左递归与公共前缀处理后，`else_part → else <statement> | ε` 仍存在悬挂 `else` 的 LL(1) 冲突。实际递归下降分析采用 `else` 与最近的未匹配 `if` 结合的约定。

详细集合计算与冲突检查见 [FIRST 与 FOLLOW 集](first-follow-sets.md)。

[返回语法分析总览](../README.md) · [理论文档索引](README.md)
