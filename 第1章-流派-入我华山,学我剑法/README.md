### 第1章-流派-入我华山,学我剑法
      流派可以理解为代码套路. 程序江湖中流传最广的套路就是 winds流派, linux流派.  
    流派套路存出不穷自成方圆.本书在昆仑流派的基础上精进.并且会分析出招的所以然.  
    视野决定格局, 套路决定手法. 说多一点当你熟悉了一种套路之后, 喜欢追求统一. 这种
    做法容易封闭, 例如 C, C++, C#, Lua... 开发的环境不同, 设计理念不同, 强撸会很丑.  
    这时候好的做法同张无忌学太极剑一样, 都忘了. 融入此刻环境, 体会大佬们的传下的意志.  
    特定领域套路如定式, 能让你游刃有余, 百步穿杨. 但不管怎么扯, 一定要求简求美~  

#### 1.1 缘起 main
      认识main函数, 哈哈, 是个太久远的故事了. 如果要扯估计可以说个几天.这里只引述C11标准
    中两种最准确的写法.
```C
The function called at program startup is named main. The implementation declares no
prototype for this function. It shall be deﬁned with a return type of int and with no
parameters:
  int main(void) { /* ... */ }
or with two parameters (referred to here as argc and argv, though any names may be
used, as they are local to the function in which they are declared):
  int main(int argc, char *argv[]) { /* ... */ }
or equivalent; or in some other implementation-deﬁned manner.
```
    最强烈词语是紧记切记, 有些东西是有规矩的.感觉最好的C入门方式就是,学C语言之父写的书.
    当你扯皮时候就用C标准委员会出的[Programming languages — C]标准文献.
    希望这里是个好的开始 -_-
```C
#include <stdio.h>

int main(int argc, char * argv[]) {
  fprintf(stderr, "I am here!");
  return 0;
}
```

#### 1.2 括号的布局
      抛开汇编,没有一个语言像C一样抠门到底.这种态度影响了用C的设计师们.很难容忍铺场浪费的写法.
    叶孤城能通过一片叶子看出西门吹雪的实力.同样一个 { } 写法就能见证你的大智慧布局~
    遵循的规则如下
      a. 左 { 右对齐
      b. 右 } 后面另起一行
    不妨观看下面的 Demo展示
```C
#include <stdbool.h>

// 1. for or while
for(;;) {     while(true) {
  ...
}             }

// 2. if ... else
if(true) {
  ...
}
else {
  ...
}
```
    这里多说一句 for(;;) 和 while(true)最终生成的汇编代码是一样的, 但是推荐前者.
    因为它表达了一个程序员(语法层面)美好的愿望, 执行死循环的时候省略一步条件判断.
    对于 if else 为什么这么写, 先看下面对比.
```C
if(true)      if(true) {    if(true) {
{               ...           ...
  ...         }             } else {
}             else {          ...
else            ...         }
{             }
  ...
}
```
