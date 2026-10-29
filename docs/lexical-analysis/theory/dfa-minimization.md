# DFA 状态最小化与等价划分

## 1. 目标与等价判定

从 [图 09 的 38 状态 DFA](../figures/Full-DFA.png) 出发，考虑词法分类输出语义：两个状态只有**输出类别相同**，并且对每个输入字符的后继状态最终属于同一个等价类，才可以合并。

不能把不同类别的双圆接受状态混成一个状态。初始按照非接受和六种接受类别划分：

| 组别 | 状态类别 | 状态数 |
| --- | --- | ---: |
| P0 | 非接受 | 5 |
| P1 | IDENTIFIER | 18 |
| P2 | KEYWORD | 7 |
| P3 | NUMBER | 2 |
| P4 | DELIMITER | 1 |
| P5 | OPERATOR | 3 |
| P6 | COMMENT | 2 |
| **总计** | | **38** |

## 2. 状态等价类的逐轮细分

对同组内的每个状态，比较其在完整 ASCII 输入字符集下转向哪一个**当前分组**，以转移签名不同为依据拆分。重复到组数不变：

| 轮次 | 分组数量 | 描述 |
| --- | ---: | --- |
| 初始 | 7 | 按接受类别分组 |
| 第 1 轮 | 19 | 非接受、数字、运算符、注释、部分保留字前缀按后继类别分裂 |
| 第 2 轮 | 25 | 继续分离不同保留字前缀 |
| 第 3 轮 | 29 | 持续细分 |
| 第 4 轮 | 30 | 达到最终等价类数量 |
| 第 5 轮复核 | 30 | 无新分裂，稳定 |

为使结果能够复核，列出每轮**非单元素分组**（未列出的状态在对应轮次已单独成组）：

- **初始：** 非接受 5、IDENTIFIER 18、KEYWORD 7、NUMBER 2、DELIMITER 1、OPERATOR 3、COMMENT 2。
- **第一轮：** `{QM,QX}`；`{K_els,K_whil,K_writ}`；`{K_if,K_int,K_else,K_for,K_while,K_write,K_read}`；`{K_e,K_el,K_f,K_r,K_re,K_w,K_wh,K_whi,K_wr,K_wri,QID}`。
- **第二轮：** `{K_els,K_whil,K_writ}`；七个完整关键字终态集合；`{K_e,K_r,K_w,K_wh,K_wr,QID}`。
- **第三轮：** `{K_w,QID}`；`{K_els,K_whil,K_writ}`；七个完整关键字终态集合。
- **第四轮：** `{K_els,K_whil,K_writ}`；七个完整关键字终态集合。
- **第五轮：** 与第四轮相同。

这是根据 [38 状态 DFA 的完整转移规则](nfa-to-dfa.md) 计算的划分；错误词素聚合规则不属于该 DFA。

## 3. 两组必须合并的状态

### A. 七个关键字完整终态

`{K_if, K_int, K_else, K_for, K_while, K_write, K_read}`

合并成唯一的 **K_FINAL（KEYWORD 接受状态）**，减少 `7−1=6` 个状态。所有状态都输出 KEYWORD；后续遇字母或数字均进入 `QID`，其余转死状态。

### B. 三个等价前缀

`{K_els, K_whil, K_writ}`

合并为 **K_E（IDENTIFIER 接受状态）**，减少 `3−1=2` 个状态。这三者都在 `e` 上进入 `K_FINAL`，其余字母或数字进入 `QID`，其他字符至死状态。

因此：

```math
38-(7-1)-(3-1)=30
```

**最小 DFA 共 30 个状态，包括死状态 QX。**

## 4. 最终状态全集

基础 14 个：

`Q0, QID, QZ, QN, QD, QO, QC, QB, QS, QL, QM, QMS, QME, QX`。

保留字相关 16 个：

`K_i, K_in, K_e, K_el, K_f, K_fo, K_w, K_wh, K_whi, K_wr, K_wri, K_r, K_re, K_rea, K_E, K_FINAL`。

关键字主要路径：

```text
Q0 --i--> K_i --f--> K_FINAL
           └--n--> K_in --t--> K_FINAL
Q0 --e--> K_e --l--> K_el --s--> K_E --e--> K_FINAL
Q0 --f--> K_f --o--> K_fo --r--> K_FINAL
Q0 --w--> K_w --h--> K_wh --i--> K_whi --l--> K_E
           └--r--> K_wr --i--> K_wri --t--> K_E
Q0 --r--> K_r --e--> K_re --a--> K_rea --d--> K_FINAL
```

`K_E` 和 `K_FINAL` 在最终自动机中分别**只有一个状态节点**，多条路径指向同一状态。所有关键字前缀的非续接字母数字转向 `QID`；`K_FINAL` 后跟字母或数字也转向 `QID`。其他 14 个基础状态沿用原转移规则。

[图 10：最小化 DFA](../figures/Min-DFA.png) · [返回实验一主页](../README.md)
