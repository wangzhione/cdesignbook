# 第1章-流派-入我华山§学我剑法

        流派等同于代码范式(套路). 开发中流传已久的范式有很多, 其中 winds 武当流派,
    linux 昆仑流派代表性最强. 但不管什么样流派范式最基础一点就是自成方圆. 本书最终选择
    在昆仑流派的基础上精进. 并会分析出招的所以然. 
        眼神望见目标, 套路决定手法. 这里是 C 领域, 主要围绕 C 范式. 这年代一招鲜吃遍
    天有点无聊了, 也很有挑战. 所以常遇到不熟悉的领域, 一种有意思方式是, 忘记以前知道的,
    立即训练需要现在知道的, 学而不思. 特定的步伐能让你游刃有余, 百步穿杨. 但不管怎么扯,
    宗旨追求简求美 ~ 
        本书重点讲解思路和设计, 解读那些被遗忘 C 的故事. 代码搞不完, 会展示较少的部分
    用于充数哈.

## 1.1 缘起 main

        很久很久以前认识一个被无数前辈大佬称之为 main 的函数. 如果要仔细扯它的话, 估计
    能说个几天. 这里只引述 C11 标准中仅存的两种规定正确写法.

```C
The function called at program startup is named main. The implementation
declares no prototype for this function. It shall be deﬁned with a return
type of int and with no parameters:
  int main(void) { /* ... */ }
or with two parameters (referred to here as argc and argv, though any 
names may be used, as they are local to the function in which they are 
declared):
  int main(int argc, char *argv[]) { /* ... */ }
or equivalent; or in some other implementation-deﬁned manner.
```

    以上标准要牢记, 一切美好从这开始. C 的学习历程, 推荐从 Brian W. Kernighan / 
    Dennis M. Ritchie 的 The C Programming Language 起步, 再到 C 标准文献, 随
    后熟悉操作系统. 和人扯皮搬上面的标准文献, 站在起源山峰中, 够终结一切问题. 
    
    不知道有没有人会想起起 void main(){} or main(){} 写法呢?
    对于 void main() {} 引述 C++ 之父 Bjarne Stroustrup 在其 FAQ 中说的 

```C
The definition void main() { /* ... * / } is not and never has been C++,
nor has it even been C.
```

    是编译器老母亲纵容了我们的"错误". 而 main() {} 最初出现在 K&R C 年代中, 那时候函
    数缺省定义(默认返回 int)不报警告. 当 warning 甩过来的时候, 应该听老母亲的话. 此刻
    希望下面是个好的开头 -_-

```C
#include <stdio.h>

int main(int argc, char * argv[]) {
    fprintf(stderr, "I'm here!\n");
    return 0;
}
```

## 1.2 括号布局

        C 是个极其强调细节的语言. 因而在用 C 开发的时候都会有点神经嘻嘻. 难以接收铺场
    浪费. 恶心的是, 也很容易在细节处栽跟头. 叶孤城通过一片被西门吹雪剑削过的叶子看出其
    实力. 同样一个 { } 布局就能见证你的起手式. 本门华山剑法 { } 遵循的如下规则和演示
        a) 左 { 右对齐
        b) 右 } 上对齐
        c) 右 } 是完整的一段代码才另起一行

```C
#include <stdbool.h>

// 1. for or while
for(;;) {     while(true) {
    ...
}             }

// 2. if ... else
if(true) {
    ...
} else {
    ...
}
```

    多说一点 for(;;) 和 while(true) 最终生成的汇编代码是一样的, 但是推荐前者.
    因为它表达了语法层面美好的愿望, 执行死循环的时候省略一步条件判断.
    对于 if else 为什么这么写, 先看下面对比.

```C
if(true)          if(true) {          if(true) {
{                     ...                 ...
    ...           }                   } else if(false) {
}                 else if(false) {        ...
else if(false)        ...             } else {
{                 }                       ...
    ...           else {              }
}                     ...
else              }
{
    ...
}
```

    第一种写法是 winds 推崇的标准. 第三种是 linux 上面常见写法. 第二种是两者的
    一种过渡. 对于上面三种范式, 都写过好久. 最后基于 C 而言, 这里推荐第三种. 更
    加紧凑飘逸正统! C 本身就是个精简的语言, 强调最小意外原则. 第一种直接 pass, 
    门一旦打开了, 就难关上了. 第二种还是有点不极致. 同样对于 switch 语法推荐下面
    写法

```C
switch(type) {
case SBase:
  ...
  break;
...  
default:
  ...
}
```
    C 中的标签都放在作用域最左边.

## 1.3 C 中三大战神 - 函数 § 帝释天

        面向过程变成两个主旋律是内存结构和函数封装. C 中函数的一个小小的 extern 声明
    都能搞晕一批人. C 函数分为声明和定义两部分. 华山剑法将其细化为 3 部分. 函数声明: 
    确定函数地址; 函数定义: 确定函数地址和实现; 函数声明加定义: 确定函数地址和实现并且
    限制作用域.

```C
// 1' 函数声明
extern void hoge(void);

// 2' 函数定义 (返回值单独占一行, 用于强调, 标准套路)
void
hoge(void) {
  ...
}

// 3' 函数声明加定义 (extern 默认缺省定义)
extern void hoge(void) {
  ...
}
```

    上面函数返回值另起一行为了突出函数定义. 在同一行表示声明加定义. 隐含的一个细节是 
    hoge(void), 因为在 C 中函数声明 void hoge(); 表示的意思是函数 hoge 参数是不
    确定的 ( __cdecl 入栈方式决定的). 加上 (void) 会在语法层面约束函数参数为空! 
        当我们写库的时候, 下面两个函数声明, 表达出的意思就不一样. 华山剑法添加的一种
    新语义.

```C
// 函数声明 显示声明加上 extern
extern void hoge(void);

// 函数声明 缺省 extern. 意图告诉别人, 这是个 low level api 用的话要理解源码
void hoge(void);
```

    二者没啥区别, 但传达的意图不一样, 前者是外部可以调用. 后者是外部不推荐调用, 调用的
    时候需要小心, 推荐了解源码细节. 当然了如果希望存在 inline 行为, 由于编译器之间的
    差异, 声明的时候必须加上 extern. 或者声明和定义一块写 (常见 cl 编译器). 当然了 
    inline 和 register 很相似, 是书写者意愿表达, 具体看编译器大佬'心情'了! 请同学们
    多实战演练. 函数的套路至关重要.
        同 extern 相对的还有个关键字 static 特别神奇. 它描述的变量和函数表示文件或函
    数内私有的. 请看下面演示代码 ->

```C
static void heoo(int arr[static 64]) {
  static int cnt;
  ...
}
```

    上面局部代码中包含的 static 套路和潜规则剖析如下:
    a) 第1个 static 表示当前函数是私有的, 被包含的文件所有, 作用域是具体文件中这个函数
       以下范围.
    b) 第2个 static 是 C99 出现的语法, 告诉编译器这个"数组"至少有 64 个数据单元, 您
       可以放心优化.
    c) 第3个 static 表示声明一个私有静态变量 cnt, 作用域属于 heoo 函数域. 
    d) 另一个细节是, 生命周期是未初始化全局的变量. 默认内存都是以0填充的, = 0 是缺省的.

> 科普 C 中 0, 00, 0x0, .0, '\0', '0', "0", NULL, false, EXIT_SUCCESS 是什么鬼!  

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

    从上可以看出围绕 C 生态部分 NULL, false, EXIT_SUCCESS 都是宏. NULL 是个特殊的空
    指针 (void *)0, 后两个 false, EXIT_SUCCESS 具体定义是一样的. 有时候函数退出使用 
    exit(EXIT_SUCCESS); 或者 return EXIT_SUCCESS; 本书推荐采用后者标准装波写法. (
    年少不装, 以后还能再记得雄起吗).
        前面几种 0, 00, 0x0, .0, '\0' 都是零, 区别在于 00 是八进制, 0x 是十六进制,
    '\0' 是字符常量. 0 值都是一样的, 写者想表达出来的语义不同. .0 是 double双精度的 0,
    对于 "0" 是个字符串常量, 等同于 char [2] { '0', '\0' } 这里也可以看出来 '0' 和 
    '\0' 不同字符常量 '0' == (int)48; 同样需要让人记忆的是 C 中 sizeof '0' == 4.
    希望大家以后再遇到这些, 能了然于胸, 乘云破雾. 
    
    顺带再次为函数声明定个基调.
        a) 给其它模块用的函数, 推荐 extern 声明, 然后定义其实体
        b) 给自己模块用的函数, 推荐 static 静态(私有) 声明加定义
        c) inline 和各种缺省函数声明和定义, 理性使用

> 顺带说说 exit(EXIT_SUCCESS); 写法居然木有警告?

```C
#include <stdlib.h>

// 常在 man 手册中看见下面模式代码
int main(void) {
  ...
  exit(EXIT_SUCCESS);
}
```

> 你是否好奇[**1' 为啥不 return EXIT_SUCCESS;**] 和 [**2' 而且编译器没有警告**]

    1' 这要从小伙伴可能踩过坑说起. 
       exit(EXIT_SUCCESS); 和 return EXIT_SUCCESS; 操作系统多数情况是相同的.
       在 main 函数最后 return 返回到系统的启动函数中, 会再次执行 exit. 这就存在一
       个执行依赖的问题了, 如果我们的主函数 return 了没有触发 exit, 那么二者就不一样.
       看下面例子

```C
#include <stdio.h>
#include <stdlib.h>

int test(void) {
  puts("main not here");
  return EXIT_SUCCESS;
}

// build : gcc -g -Wall -O2 -nostartfiles --entry=test -o test.exe test.c
// run   : ./test.exe
``` 

    程序启动后, 退出时会崩溃. 因为基础库准备的善后操作执行异常. 这时候 return 替换为 exit
    就可以解决! 因为直接退出, 会走编译器为 exit 开通的善后流程. 为了安全起见有时候直接 exit
    粗暴高效.

    2' 编译器没有警告, 先看截取部分源码

```C
#if defined(__GNUC__) || defined(__clang__)
    #define NORETURN(func) func __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define NORETURN(func) __declspec(noreturn) func
#endif

// C11 中为其构造了新的关键字 _Noreturn

_Noreturn void suicide() {
    abort(); // Actually, abort is _Noreturn as well
}
```

    脉络比较清晰了, 因为 exit 经过 '_Noreturn' 类似声明(标识), 永不返回, 因此编译器优
    化后就不再抛出无返回值警告. 
    

> 分享 Over, 赠送个打桩小技巧. 在协同开发时, 先快速设计好接口给对接方. 随后来个空实现

### 1.4 C 中三大战神 - 指针 - 达姿·奥利哈刚

        C 中一个有争议的命题是一切皆内存. 而指针就是内存的魔法杖. C is free and unsafe. 
    强调

**程序员是万能的**

    隐含的一层意思是你的程序你负责. 所有的一切都因指针而起, 它无所不能在当前的系统的世界里.
    本小节里我们只表述华山剑法中 C 指针的写法范式. 首先看下面小段演示 Demo.  

```C
// 第1种: 传统写法, * 靠右
void *piyo, *hoge;

// 函数定义
void *
free(void *block) {
    ...
}

// 第2种: * 靠左, 模仿上层语言的类型
void* piyo, *hoge;

void*
free(void* block) {
    ...
}
```

    微发现上面两种写法不统一和自然. 由于 C 本身缺陷就不少, 上面两种写法都不完美. 第1种, 
    函数定义的时候 * 号就不知道该放在那里了. 这种用法比较广, 但也不是最好的选择. 第2种写法
    ,多数是有过面向对象的编程经验, 重温一下 C. 这种写法在定义多个变量指针的时候基本就废了.
    而且本身是变量指针声明, 却被强幻想成指针类型. 第2种写法强烈不推荐. 这里采用下面写法, 
    在追求自然和美过程中, 脚步不要停歇!

```C
// 第3种: 变量声明, * 全部放中间
void * piyo, * hoge;

void *
free(void * block) {
  ...
}

// 补充说明, 多维指针, 函数指针声明
int ** arrs = NULL;
typedef void (* signal_f)(int sig);
```

        关于指针范式基调主要如上. 简单扯一点关于 C 中变量声明. C 一共才44个关键字, 推荐
    命名要短小精干. 强烈不推荐驼峰规则, 因为在远古时期 C 是不区分大小写的. 而且 C 代码也
    不容易读懂. 推荐遵从内核源码古法. 命名语义区分用 _ 分隔. C 也许不能让工资飞高, 但是可
    以让你在面对大佬的时候看见希望 ~

### 1.5 C 中三大战神 - 宏 - 封神记·天

      宏有点疯狂有点好用. 但切记能用 inline内联, 就不要用宏! (目前而言 inline不太好用哈) 如果说
    指针是自由, 那宏就是噩梦. 我们从 struct.h 来认识一下这个偏执狂 ->

```C
#ifndef _H_SIMPLEC_STRUCT
#define _H_SIMPLEC_STRUCT

//
// 数据结构从这个头文件集成开始
// 构建统一的标识文件, 构建统一的注册函数, 构建统一基础头文件
//
// author : wz
//

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <errno.h>
#include <limits.h>
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

#undef	CERR
#define CERR(fmt, ...) \
	fprintf(stderr, "[%s:%s:%d][%d:%s]" fmt "\n", __FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)

#define CERR_EXIT(fmt, ...) \
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
	do { \
		CERR(fmt, ##__VA_ARGS__); \
		return val; \
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
// int - 函数返回值全局状态码
// >= 0 标识 Success状态, < 0 标识 Error状态
//
enum flag {
	ErrParse    = -8,  //协议解析错误
	ErrClose    = -7,  //句柄打开失败, 读取完毕也返回这个
	ErrEmpty    = -6,  //返回数据为空
	ErrTout     = -5,  //操作超时错误
	ErrFd       = -4,  //文件打开失败
	ErrAlloc    = -3,  //内存分配错误
	ErrParam    = -2,  //输入参数错误
	ErrBase	    = -1,  //错误基础类型, 所有位置错误都可用它

	SufBase     = +0,  //基础正确类型
};

//
// 定义一些通用的函数指针帮助, 主要用于基库的封装.
// 有构造函数, 析构函数, 比较函数, 轮询函数 ... 
// icmp_f	- int icmp(const void * ln, const void * rn); 标准结构
// each_f	- int <-> int, each循环操作, arg 外部参数, node 内部节点
// start_f	- pthread 线程启动的辅助函数宏, 方便优化
//
typedef int    (* icmp_f )( );
typedef void * (* vnew_f )( );
typedef void   (* node_f )(void * node);
typedef int    (* each_f )(void * node, void * arg);
typedef void * (* start_f)(void * arg);

#define _ENUM_FLAG
#endif // !_ENUM_FLAG

#endif // !_H_SIMPLEC_STRUCT
```

      这里我们主要讲解一下宏的命名准则. 以下对于宏态度的演示, 多感受其中范式

* 克制使用, 要用就要表述最清晰

```C
#define _H_SIMPLEC_SCHEAD
#define _STR_LOGDIR       "logs"
#define _UINT_LOG         (2048u)
#define _INT_MQ           (1 << 6)
#define _FLOAT_ZERO       (0.000001f)	
```

    上面是常量的宏命名流式, 统一大写. _ 开头表达'私有', 慎重使用. STR, H, UINT ...一眼看过去就
    知道其宏常量的类型. 第一个是头文件宏, 采用 _H_[项目名称]_[文件名称] 的命名规范, 防止重复. 还
    有字符串常量方便宏拼接, 不用加(). 数值常量防止意外拼接加括号. 当你不清楚什么意外的时候, 加括
    号解决方案是最保险的. 

* 函数宏另起一行写, 局部使用宏直接在一行写

```C
// 置空操作, v必须是个变量
#define BZERO(v) \
	memset(&(v), 0, sizeof(v))

// 添加双引号的宏
#define _STR(a) #a
#define CSTR(a)	_STR(a)
```

    例如上面, BZERO(v) 可以独立存在, 另起一行加上 \. 对于 _CSTR(a)可以作为一个子语句, 直接一行写.

* 有些宏, 想被人认为是函数, 那就小写

```C
// 下面是依赖GCC编译器实现
#ifdef __GNUC__

#include <unistd.h>
/*
 * sh_msleep - 睡眠函数, 时间颗粒度是毫秒.
 * m      : 待睡眠的毫秒数
 * return : void
 */
#define sh_msleep(m) \
    usleep(m * _INT_STOMS)

#elif _MSC_VER

#include <windows.h>
#define sh_msleep(m) \
    Sleep(m)	

#else
	#error "error : Currently only supports the Visual Studio and GCC!"
#endif    
```

    写的人意图希望 sh_sleep是个'函数', 使用了小写. 对于两个内置宏,  __GNUC__ 是 GCC编译器自带的,
    表示当前是使用 GCC来编译项目. _MSC_VER 表示使用 M$的 CL来编译项目. 做为开发人员, 推荐用最新
    的编译器. 因为保持活力和新鲜感很有趣, 一次新的尝试说不定就是一次机遇(多数还是坑多...).

> 命名规范简单总结  

    a ). 变量命名, 函数命名
      -> 小写, 语义分隔使用 _ ; 
      -> 全局的使用 g_ 开头(强烈不建议使用全局量, 太污了);
      -> static 声明的, 开头加 _; 
      -> 宏中局部变量使用 $ 开头, 防止变量污染.

```C
// 简单的time时间记录宏
#define TIME_PRINT(code) \
	do { \
		clock_t $st, $et; \
		$st = clock(); \
		code \
		$et = clock(); \
		printf("The current code block run time:%lf seconds.\n", \
		 (0.0 + $et - $st) / CLOCKS_PER_SEC); \
	} while (0)
```

    b ). 宏命名
      -> 大写, 语义分隔使用 _ ; 
      -> 宏私有常量采用 _[类型简写]_[名称]
      -> 宏对外常量或类型 [类型简写]_[名称]
      -> 文件头宏采用 _H_[项目名称]_[文件名称]
      -> 跨平台相关工作, 且希望被当成函数使用, 可以用小写命名

    c ). 枚举声明, 类型声明
      C 枚举同INT宏常量很同质化, 默认类型 int. 有时候很显得多余, 请用参照下面模式仿照

```C
#ifndef _ENUM_FLAG

enum flag {
  ...
	SufBase   = +0,	//结果正确的返回宏
	SufExist  = +1,	//希望存在,设置之前已经存在了.
  ...
};

typedef int	(* each_f )(void * node, void * arg);

// 定义的字符串类型
typedef struct tstr * tstr_t;

#define _ENUM_FLAG
#endif // !_ENUM_FLAG
```
    -> 类型声明 xxxx_[类型缩写], 很直观用于区分. 
    -> 为了区分枚举和INT宏常量, 枚举采用首字母大写, 驼峰规则, 没有使用 _ 分割.

> 
到这里基本把 C的华山流派总纲讲的有小些了  
按照流派范式去写, 你会有更多的时间去学心法, 然后再出来构建你自己的独孤飞箭.  
同是不年轻的穷屌丝, 要学会适应, 不流血那就流水. 孙子古法一直是博大精深.
程序猿多看看墨子很贴切  

### 1.6 修炼剑法自然需要神兵

      万般皆自然, 一通俩顺, 难在真通了ヾ(๑╹◡╹)ﾉ" 需要一定时间. 这里的神兵指的是你的
    编程环境. 硬件方面要是可以的话买最快, 最美, 最便宜的套装. 软件方面, 分下面流派描述

**winds 武当流派**  

    a ). Install Best New Windows Profession ISO 
    b ). Install Best New Visual Studio Enterprise
    c ). Install Best New Visual Studio Code

    学习的时候推荐用最新的环境, 这样躺坑会多, 锻炼的机会也多. 这里多说一点, 没有一款 IDE比
    VS猛. 那怕 C++被 M$打压了十多年, VC++仍然宇宙中最屌的编辑器(笔者此刻用的是 VS2017). 
    winds的缺陷在于封闭和 api毫无章法. 导致后期提升艰难.  阻挡了开发人员的持续投入的兴趣. 
    同样其无敌的IDE, 为其引来无数上层商业应用. 总而言之 winds 比 linux难, 同级别工资比
    linux工资低. 服务器开发领域就是是个鸡肋(虽强但不通用, 所以不深用)! 

**linux 昆仑流派**  

    本人是在虚拟机上跑的, 推荐安装环境如下
    a). Install Best New VMware Workstation Pro
    b). Install Best New Ubuntu Desktop ISO
    c). Install Best New Xshell
    d). Install Best New GCC, GDB, VIM

    linux 业界标杆, 简单高效优美. vi gcc make 能够搞定一切. 无数的一手资料, 强大的 man手册.
    题外话 APPLE 的系统作为程序开发更加带感. 毕竟是 unix内核, 外观好看. 会 linux平移很轻松.
    就是做为盗版游戏机难点. 做为服务器开发环境, 有条件同学可以首选.

**你的剑就在前方**  

    市面上多数服务器书籍是 GCC弄的, 没啥意思, 同质化严重. 本文主打 Visual Studio CL 搞起. 
    先将 GCC的一些命令翻译到 VS 上面, 其它的用到时自行查手册 Serach.  

```bash
# gcc -E -> 生成预编译, 调试宏的时候用到. VS上面默认关闭
(项目)属性 -> 配置属性 -> C/C++ -> 预处理器 -> 生成预处理文件 -> [自行修改]

# gcc -D -> 添加预编译阶段的全局宏, 但VS上面默认值为1
(项目)属性 -> 配置属性 -> C/C++ -> 预处理器 -> 预处理器定义 -> [自行添加]

# gcc -I -> 添加包含目录
(项目)属性 -> 配置属性 -> VC++目录 -> 包含目录 -> [自行添加]

# gcc -l -> 添加引入的库
(项目)属性 -> 配置属性 -> VC++目录 -> 库目录 -> [自行添加]
(项目)属性 -> 链接器 -> 输入 -> 附加依赖库 -> [自行添加]

# 额外的 make 目录操作
(项目)属性 -> 生成事件 -> 后期生成事件 -> 命令行 -> [自行添加]

# ... 多写代码, 多看书, 经验条自然上涨了, 档升级了 ~~~

```

    对于CL编译器, 这里说一下项目经常用的预处理宏和系统库的作用
    -> 预处理器定义
      _DEBUG                          -> Debug 模式标识宏
      WIN32_LEAN_AND_MEAN             -> winds 上屏蔽一些不常用的 api 优化应用程序
      _CRT_SECURE_NO_WARNINGS         -> 方便使用 scanf, strcpy ... 这类无辜的老函数
      _CRT_NONSTDC_NO_DEPRECATE       -> 开启 windows 的 POSIX 支持部分, mkdir, ...
      _WINSOCK_DEPRECATED_NO_WARNINGS -> 开启 Winsock2.h 中新的socket函数替代老的

    -> 附加依赖库
      ws2_32.lib                      -> Winsock2.h 库使用必须引入的库文件
    飘逸一点有时候也不在 VS CL中附加依赖库, 直接写成下面这样

```C
#ifdef _MSC_VER

#include <ws2tcpip.h>

# pragma comment(lib, "ws2_32.lib")
  WSADATA wsad;
  WSAStartup(WINSOCK_VERSION, &wsad);

#endif
```

    个人感悟, 跨平台是在浪费生命. 有些人单纯的用 linux, 顺带配上 apple. 感觉很好, 毕竟越纯
    粹越强. linux是个回报率很高的平台. 但是学了C/C++ 真是天高地厚不知路长~ 岁月不尽然呀...  

### 1.7 夜太黑正是练剑的好时光

      上面聊的有些多, 很多细节需要自己亲身建构. 回想起13年看<<自制编程语言>>那本书, 惊为天人.
    感觉作者实力好强. 因为看不明白, 强行撸, 狂看最后懂了点, 收益良多(也都忘了). 在编程的世界里, 
    不需要啥, 只要 wo are 正在舞剑! 
      如果咱们一样只是为了, 更好的生存和活着. 那么学起来就更随意了, 君子当善假于物! 熟悉工具, 
    帮人实现梦想, 就成了~ 然后回家开个水果店挺好的.

如果你没有对象, 那就使劲打代码

如果你实在无聊, 那就跑步加看书

如果你还不甘心, 那就呵呵闷闷哒

> 最后推荐一首喜欢的小诗

***
    <<也许>> 
    - 汪国真
    也许，永远没有那一天
    前程如朝霞般绚烂
    也许，永远没有那一天
    成功如灯火般辉煌
    也许，只能是这样
    攀援却达不到峰顶
    也许，只能是这样
    奔流却掀不起波浪
    也许，我们能给予你的
    只有一颗
    饱经沧桑的心
    和满脸风霜
***

*练剑的你, 终于有一天将满身是坑* --<-<-<@

***

![笑傲江湖](img/笑傲江湖.jpg)