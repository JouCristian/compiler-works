# 实验二 · 语法分析器设计与实现

**当前进度：已完成文法改造、Lexer → TokenBuffer → Parser 数据传递与词法错误阻断；递归下降主体和语法验收待实现。**

本实验面向指导书规定的简化类 C 语言，使用递归下降方法检查 Token 序列的语法结构。

## 当前实现

| 模块 | 进度 |
| --- | --- |
| Lexer 与 TokenBuffer | 已完成；缓冲区仅保存合法 Token |
| 词法错误阻断 | 已完成；存在词法错误时跳过 Parser |
| Parser 基础接口 | 已实现初始化、查看/前进 Token、`match()`、`expect()` |
| 文法改造 | 已消除直接左递归、提取左公因子 |
| FIRST / FOLLOW | 正在整理 |
| 完整递归下降与语法错误统计 | 待开发 |

### 程序结构

```text
src/main.c
  ├── lexer_scan_file()              [src/lexer.c]
  │     └── emit_token()
  │           ├── 终端输出与统计
  │           ├── 写入 lexical-analysis/lex.txt
  │           └── 合法 Token → TokenBuffer
  ├── 检查 LexerState.error_count
  │     ├── > 0：跳过 Parser
  │     └── = 0：parser_init()       [src/parser.c]
  └── token_buffer_free()
```

目前 `parser_init()` 仅接收 Token 数组、初始化分析状态，尚未判断整份源程序的语法合法性。

## 改造后的文法

以下直接移植 [grammar-list 原稿](theory/grammar-list)，**保留原有编号、排列、符号和缩进**，不重新改写文法：

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

文法改造包括：消除声明序列、语句序列、算术表达式及项的左递归；提取 `if` 语句和布尔表达式的左公因子。**注意：** `else_part` 仍存在悬挂 `else` 的 LL(1) 冲突；后续实现采用“`else` 匹配最近的未匹配 `if`”原则，而不是宣称整个文法满足严格 LL(1)。

## 理论资料

| 文件 | 内容 |
| --- | --- |
| [grammar-list](theory/grammar-list) | 改造后的文法原稿 |
| [FISRT-FOLLOW-list](theory/FISRT-FOLLOW-list) | FIRST / FOLLOW 计算草稿（尚未完成） |

## 构建与阶段验证

从仓库根目录执行：

```bash
make clean && make

# 无词法错误：测试 TokenBuffer → Parser 初始化
./build/lexer tests/parser/valid.c

# 有词法错误：验证跳过 Parser
./build/lexer tests/lexer/instructor_lexer_test.txt
```

当前测试只验证词法分析及接口，不代表递归下降语法分析已通过。后续补充语法错误测试、统计结果及实验截图。

[返回文档首页](../README.md) · [返回项目主页](../../README.md)
