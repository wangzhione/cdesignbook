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
if(true)          if(true) {          if(true) {
{                   ...                 ...
  ...             }                   } else if(false) {
}                 else if(false) {      ...
else if(false)      ...               } else {
{                 }                     ...
  ...             else {              }
}                   ...
else              }
{

}
```
    第一种写法是winds推荐的, 第三种写法是多数linux上常见的.但是这里推荐第二种.为什么呢.
    第一种好看但太松弛了, C本身就是个精简的语言,强调最小意外原则. 最后一种 } else if(...) { 太
    丑了, 两个 } } 实战环境下对应规则也难看出个所以然. 同样对于switch 语句推荐下面写法.
```C
switch(type) {
case TYPE_ONE:
  ...
  break;
...  
default:
  ...
}
```
    C中的标签都放在最左边.

#### 1.3 C中三大战神 - 函数 - 帝释天
      面向过程编程两个主旋律就是结构(内存)和函数(行为). 而在C中函数的一个 extern声明都能干掉一
    批人. C函数分为声明和定义两部分. 我将其细化为3部分.如下
```C
// 函数声明
extern void hoge(void);

// 函数定义, 返回值单独一行, 强调区分作用
void
hoge(void) {
  ...
}

// 函数声明加定义
void hoge(void) {
  ...
}
```
    扯一点上面函数返回值**另一起一行**表示函数定义, 在一行表示声明加定义. 还有一个细节就是 hoge(void)
    定义, 因为在C中函数声明 void hoge(); 表示的意思是函数 hoge参数是不定的. 加上 void表示函数参数为空!
      当我们写库的时候, 看下面两个函数声明, 表达的意思不一样.
```C
// 显示声明加上 extern
extern void mem_hello(void);

// 缺省声明, 没加 extern
void mem_heoo(void);
```
    二者在表达的意思上有点区别, 前者是外部可以调用. 后者是外部也可以调用, 但不推荐调用或者说调用的时候要
    小心,最好能了解实现细节. 当然了也存在一个特殊情况, 如果定义的函数加了 inline 内联行为, 那边声明的时
    候必须加上extern, 或者声明和定义一块写, 请同学们多实战演练. 函数的套路至关重要.
      说到 extern 外部声明的关键字, 那么就必须说一说另一个神奇的关键字, static 私有的. 通过代码来说话.
```C
// static 私有函数 声明加定义
static void _heoo(int arr[static 10]) {
  static int _cnt;
  ...
}
```
    上面代码中包含的潜规则比较多.
    a. 第一个static 表示当前 函数是私有的, 作用域(归属权)归当前文件独有
    b. _heoo, 第一个字符 _, 它是一种约束. 通过命名一看这个函数是私有的. 因为C很自由, 那么它的一些标准,
      是通过程序员自己来定义.(你可以有更好的策略, 但这是哈哈当前华山剑法的套路) 同样对于上层语言的命名空间,
      函数重载, 无外乎是在编译层面做的命名约束. 因而在C中命名特别讲究, 好的命名能够表达很多信息. 其中有这些
      字符是有潜规则的. _xxx -> 表示私有, 上层程序用; __xxx -> 解释权归底层库, 系统底层用; $xxx -> 表示
      私有, 不会和人有冲突. 再或者一个全局变量声明 extern int g_cnt; 通过 g_开头, 有一条准则, 哪怕用goto
      也别用全局变量, 因为谁也不喜欢赤裸裸的开挂.
    c. [static 10] 是C99 新加的语法, 程序告诉编译器, 这个"数组"(其实就是个指针)至少有10个单元, 你可以放心
      优化
    d. static int _cnt; 表示声明了一个私有的变量 _cnt, 作用域是属于 _heoo函数域的. 生存周期是全局的.
    e. 另一个细节是, 生命周期是全局的变量, 默认内存初始化都是以 0填充的. 上面是个缺省的写法等同于
      static int _cnt = 0;

> 这里我们科普一点, C中 0, 00, 0x0, .0, '\0', '0', "0", NULL, false, EXIT_SUCCESS 是什么鬼!  

    我们首先看有源码的部分
```C
#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif

#ifndef __cplusplus

#define bool	_Bool
#define false	0
#define true	1

#endif /* __cplusplus */

#define EXIT_SUCCESS 0
```
    对于C++部分, 争取不讲, 主要是C生态圈部分. 从上面可以看出来 NULL, false, EXIT_SUCCESS 都是宏.
    NULL 是个特殊的空指针 (void *)0, 后者两个是完全一样的. 其中有时候函数返回使用 exit(EXIT_SUCCESS) 或者
    return EXIT_SUCCESS; 是个标准装逼写法. 本书推荐~(年少不装一样, 以后还能雄起吗)
    前面几种 0, 00, 0x0, .0, '\0' 都是零, 但有区别. 00是八进制的0, 0x是十六进制的0, '\0' 是字符常量的零.
    前面几种0值都是一样的, 写者想表达出来的语义不同 .0 是 double 双精度的0. 对于 "\0" 是个字符串常量,  
    等同于 char [2] { '0', '\0' } 这里也可以看出来 '0' 和 '\0' 不同 字符常量 '0' = (int)48;
    希望以后再遇到这些, 你能了然于胸~ 乘云破雾
    关于函数声明部分主要就这些了, 总的基调是
      a). 给其它模块用的函数, 推荐extern 声明, 然后定义写法
      b). 只有自己模块用的, 推荐 static 声明加定义
      c). static 命令推荐 _xxx 开头, 方便一目了然

#### 1.4 C中三大战神 - 指针 - 达姿·奥利哈刚
      C中一个很有争议的命题就是一切皆内存. 而指针就是内存的魔法杖. C is free and unsafe. 强调
> 程序员是万能的 

    隐含的一层意思是你必须对你的程序负责. 而这一切都是指针带来的, 它无所不能在当前的系统架构的世界里.
    而这里我们来看看C指针的写法. 首先看下面一段演示代码.  
```C
// 第一种传统写法, * 靠右;
void *piyo, *hoge;

// 函数定义
void *
free(void *block) {
  ...
}

// 第二种, *靠左, 引进上层语言类型的写法
void* piyo, *hoge;

void*
free(void* block) {
  ...
}
```
    因为C本身缺陷就很多, 上面两种写法都有弊端, 不自然的地方. 第一种, 函数定义的时候 * 号就不自然了. 丑,
    这种用法比较广. 第二种写法, 多数是有过面向对象的编程经验, 重温一下C. 这种写法很不推荐, 多变量声明丑爆.
    而且本身是指针变量声明, 却被强加指针类型. 华山剑法采用下面写法, 追求自然和美脚本不要停歇!
```C
// 变量声明, * 全部放中间
void * piyo, * hoge;

void *
free(void * block) {
  ...
}

// 写法补充说明, 多维指针, 函数指针声明
int ** arrs = NULL;
typedef void (* signal_f)(int sig);

```
    这里关于指针写法的套路就表述完毕了. 简单扯一点关于C中变量声明. C是个比较精简的语言, 推荐变量名短小精干.
    强烈不推荐, 驼峰规则, 因为C不是骆驼千里奔袭, 它只是个小松鼠, 负重低. 命名语义区分用 _ 分隔. 
    在什么都没有的时候一切从源码开始, 从内核源码开始.

#### 1.5 C中三大战神 - 宏 - 封神记·天
    可以用inline内联, 就不要用宏! 如果说指针是自由, 那宏就是噩梦. 首先通过struct.h来认识一下->
```C
#ifndef _H_SIMPLEC_STRUCT
#define _H_SIMPLEC_STRUCT

//
// 数据结构从这个头文件集成开始
// 构建统一的标识文件, 构建统一的注册函数, 构建统一基础头文件
//
// author : wz
//

#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

// 
// 控制台输出完整的消息提示信息, 其中fmt必须是 "" 包裹的字符串
// CERR       -> 简单的消息打印
// CERR_EXIT  -> 输出错误信息, 并推出当前进程
// CERR_IF    -> if语句检查, 如果符合标准错误直接退出
// 
#ifndef _H_CERR
#define _H_CERR

#define CERR(fmt, ...) \
	fprintf(stderr, "[%s:%s:%d][errno %d:%s]" fmt "\n",\
		__FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)

#define CERR_EXIT(fmt,...) \
	CERR(fmt, ##__VA_ARGS__), exit(EXIT_FAILURE)

#define CERR_IF(code) \
	if((code) < 0) \
		CERR_EXIT(#code)

//
// RETURN - 打印错误信息, 并return 返回指定结果
// val		: return的东西, 当需要 return void; 时候填 ',' 就过 or NIL
// fmt		: 双引号包裹的格式化字符串
// ...		: fmt中对应的参数
// return	: val
// 
#define NIL
#define RETURN(val, fmt, ...) \
	do {\
		CERR(fmt, ##__VA_ARGS__);\
		return val;\
	} while(0)


#endif

/*
 * 这里是一个 在 DEBUG 模式下的测试宏
 *
 * 用法 :
 * DEBUG_CODE({
 *		puts("debug start...");
 * });
 */
#ifndef DEBUG_CODE
# ifdef _DEBUG
#	define DEBUG_CODE(code) \
		do code while(0)
# else
#	define DEBUG_CODE(code) 
# endif	//	! _DEBUG
#endif	//	! DEBUG_CODE

#ifndef _ENUM_FLAG
//
// flag_e - 全局操作基本行为返回的枚举, 用于判断返回值状态的状态码
// >= 0 标识 Success状态, < 0 标识 Error状态
//
typedef enum {
	Success_Exist = +2,			//希望存在,设置之前已经存在了.
	Success_Close = +1,			//文件描述符读取关闭, 读取完毕也会返回这个
	Success_Base  = +0,			//结果正确的返回宏

	Error_Base    = -1,			//错误基类型, 所有错误都可用它, 在不清楚的情况下
	Error_Param   = -2,			//调用的参数错误
	Error_Alloc   = -3,			//内存分配错误
	Error_Fd      = -4,			//文件打开失败
	Error_Tout    = -5,			//超时错误
} flag_e;

//
// 定义一些通用的函数指针帮助, 主要用于基库的封装.
// 有构造函数, 析构函数, 比较函数, 轮询函数 ... 
// cmp_f    - int cmp(const void * ln, const void * rn); 标准结构
// each_f   - flag_e <-> int, each循环操作, arg 外部参数, node 内部节点
// start_f  - pthread 线程启动的辅助函数宏, 方便优化
//
typedef int     (* cmp_f  )();
typedef void *  (* new_f  )();
typedef void    (* die_f  )(void * node);
typedef flag_e  (* each_f )(void * node, void * arg);
typedef void *  (* start_f)(void * arg);

#define _ENUM_FLAG
#endif // !_ENUM_FLAG

#endif // !_H_SIMPLEC_STRUCT
```