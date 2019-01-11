# 第1章-流派-入我华山§学我剑法

        流派等同于代码范式(套路). 开发中流传至今的范式有很多, 其中 winds 武当流派,
    linux 昆仑流派代表性最强. 但不管什么样流派范式最基础一点就是自成方圆. 本书最终选择
    在昆仑流派基础上精进. 并会分析出招的所以然. 
        眼神望见目标, 套路决定手法. 这里是 C 领域, 主要围绕 C 范式. 如今这时代一招鲜
    吃遍天总觉得有点无趣, 也很有挑战. 成长中时常遇到不熟悉的领域, 一种有意思方式是, 忘
    记以前知道的, 立即训练需要现在知道的, 学而不思. 特定的步伐能让你游刃有余, 百步穿杨.
    但不管怎么扯, 宗旨一直是求简求美 ~ 
        本秘籍重点讲解思路和设计, 解读那些被遗忘 C 的故事. 代码搞不完, 会展示较少的部
    分用于打桩充数哈.

## 1.1 缘起 main

        很久很久以前进入一个被无数前辈巨擘称之为 main 的函数. 如果要仔细扯它的话, 估计
    能说个几天. 这里只引述 C11 标准中规定的两种标准写法.

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

    标准练气要诀要牢记, 一切美好从这诞生. C 的学习历程, 推荐从 Brian W. Kernighan / 
    Dennis M. Ritchie 的 The C Programming Language 起步, 再到 C 标准文献, 随
    后熟悉操作系统. 和人斗法搬标准文献, 站在起源山峰上, 够终结一切问题. 
    
    不知道有没有人会想起 void main(){} or main(){} 写法呢?
    对于 void main() {} 引述 C++ 之父 Bjarne Stroustrup 在其 FAQ 中说的 

```C
The definition void main() { /* ... * / } is not and never has been C++,
nor has it even been C.
```

    是编译器老母亲纵容了我们的"错误". 而 main() {} 最初出现在 K&R C 年代中, 那时候函
    数缺省定义(默认返回 int)不报警告. 当前 warning 甩过来的时候, 应该听老母亲的话. 此
    刻希望下面代码是个好的开头 -_-

```C
#include <stdio.h>

int main(int argc, char * argv[]) {
    fprintf(stderr, "I'm here!\n");
    return 0;
}
```

## 1.2 括号布局

        C 是个极其强调细节的语言. 多数在用 C 开发的时候都会有点神经嘻嘻. 难以接收铺场
    浪费. 恶心的是, 细节之处往往充满了妖魔鬼怪. 叶孤城通过一片被西门吹雪剑削过的叶子看出
    其实力. 同样一个 { } 布局就能见证你的起手式. 本门华山剑法 { } 遵循的如下规则和演示
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

    多说一点 for(;;) 和 while(true) 二者最终生成的汇编代码是一样的, 但是推荐前者.
    因为它表达了语法层面的美好愿望, 执行死循环的时候省略一步条件判断.
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

        面向过程编程两个主旋律是内存结构和函数封装. C 中函数的一个小小的 extern 声明
    都能搞晕一批人. C 函数分为声明和定义两部分. 华山剑法将其细化为 3 部分. 函数声明: 
    确定函数地址; 函数定义: 确定函数地址和实现; 函数声明加定义: 确定函数地址和实现, 并
    且限制作用域.

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

    2' 中函数返回值另起一行为了突出函数定义. 在同一行表示声明加定义. 隐含的一个细节是
    hoge(void), 因为在 C 中函数声明 void hoge(); 表示的意思是函数 hoge 参数不确
    定(由 __cdecl 从右向左入栈方式决定). 加上 void 会在语法层面约束函数参数为空!
        当我们写库头文件的时候, 下面两个函数声明, 表达出的意思就不一样. 华山剑法为其
    添加的一种新语义.

```C
// 函数声明 显示声明加上 extern
extern void hoge(void);

// 函数声明 缺省 extern 意图告知别人, 这是个 low level api 用的话要理解源码
void hoge(void);
```

    编译层面二者没啥区别, 阅读层面传达的意图不一样, 前者是外部可以调用. 后者是外部不推
    荐调用, 调用的时候需要小心, 推荐了解源码细节. 当然了如果函数定义存在 inline 行为,
    由于编译器对 inline 实现的差异, 为了照顾编译器差异, 声明时候必须加上 extern, 用
    于确定函数定义地址. inline 和 register 很相似, 是书写者意愿表达, 具体看编译器大
    佬'心情'了! 请同学们多实战演练. 函数套路至关重要, 陪伴咱们度过整个修真岁月.
        同 extern 还有个关键字 static 特别神奇. 它描述的变量和函数表示在文件或函数
    内私有(静态)的. 请看下面演示代码 ->

```C
static void heoo(int arr[static 64]) {
    static int cnt;
    ...
}
```

    局部代码中包含的 static 套路和潜规则剖析如下:
    a) 第1个 static 表示当前函数是私有的, 被包含的文件所有, 作用域是具体文件中这个函
       数定义行以下范围.
    b) 第2个 static 是 C99 出现的语法, 告诉编译器这个"数组"至少有 64 个数据单元, 您
       可以放心优化.
    c) 第3个 static 表示声明一个私有静态变量 cnt, 作用域属于 heoo 函数域. 
    d) 另一个细节是 cnt 生命周期同未初始化全局的变量. 默认内存都是以 0 填充的. 即这里
       cnt = 0 是缺省的.

***

> 科普 C 中 
> **0, 00, 0x0, .0, '\0', '0', "0", NULL, false, EXIT_SUCCESS** 
> 是什么鬼!  

        我们首先看源码中能找见的部分

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

    可以看出围绕 C 生态部分 NULL, false, EXIT_SUCCESS 都是宏. NULL 是个特殊的空指针
    (void *)0, 后两个 false, EXIT_SUCCESS 具体定义是一样的数值零. 有时函数退出使用 
    exit(EXIT_SUCCESS); 或者 return EXIT_SUCCESS; 本书推荐采用后者标准装波写法. (
    年少不装, 以后还能雄风依旧吗?)
        前面 0, 00, 0x0, .0, '\0' 这几种都为零, 区别在于 00 是八进制, 0x 是十六进制,
    '\0' 是字符常量. 零值都是一样的, 写者想表达出来的语义不同. .0 是 double 双精度的零, 
    对于 "0" 是个字符串常量, 等同于 char [2] { '0', '\0' } 这里也可以看出来 '0' 和 
    '\0' 不同字符常量 '0' == (int)48; 同样需要让人意外的是 C 中 (sizeof '0' == 4)!
    希望大家以后在 C 中看到这些数字, 能了然于胸, 乘云破雾. 

***

> 顺带说说 **exit(EXIT_SUCCESS);** 写法居然木有警告?

```C
#include <stdlib.h>

// 常在 man 手册中看见下面套路代码
int main(void) {
    ...
    exit(EXIT_SUCCESS);
}
```

你是否好奇[**1' 为啥不 return EXIT_SUCCESS;**] 和 [**2' 而且编译器没有警告**]

    1' 这要从小伙伴可能踩过坑说起. 
       在 main 函数中 exit(EXIT_SUCCESS); 和 return EXIT_SUCCESS; 操作系统多数
       情况是相同的. 在 main 函数 return 会返回到系统的启动函数中, 会再次执行 exit. 
       这就存在一个执行依赖的问题了. 如果我们的主函数 return 了没有触发 exit, 那么二者
       就不一样. 看下面例子

```C
#include <stdio.h>
#include <stdlib.h>

int test(void) {
    puts("main not here");
    return EXIT_SUCCESS;
}

// $ gcc -g -Wall -O2 -nostartfiles --entry=test -o test.exe test.c
// $ ./test.exe
``` 

    程序启动后, 退出时会崩溃. 因为基础库准备的善后操作执行异常. 这时将 return 替换为 exit
    就可以解决! 因为直接退出会走编译器为 exit 开通的善后流程. 为了安全起见有时候直接 exit
    粗暴高效.

    2' 编译器没有警告原因先看截取部分源码

```C
#if defined(__GNUC__) || defined(__clang__)
    #define NORETURN(func) func __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define NORETURN(func) __declspec(noreturn) func
#endif

// C11 中为其构造了新的关键字 _Noreturn

_Noreturn void suicide(void) {
    abort(); // Actually, abort is _Noreturn as well
}
```

    脉络比较清晰了, 因为 exit 经过 '_Noreturn' 类似声明(标识), 永不返回, 因此编译器
    优化后就不再抛出无返回值警告. 分享 Over 哈哈哈. 赠送个打桩小技巧. 在协同开发时, 先
    快速设计好接口给对接方. 随后来个空实现!

***
    
    此刻再次为函数声明定个弱基调 :)
        a) 给其它模块用的函数, 推荐 extern 声明, 然后定义其实体
        b) 给自己模块用的函数, 推荐 static 静态(私有) 声明加定义
        c) inline 和各种缺省函数声明和定义, 理性使用

## 1.4 C 中三大战神 - 指针 - 达姿·奥利哈刚

        C 中一个有争议的命题是一切皆内存. 而指针就是指向内存的魔法杖. C is free and
    unsafe. 至高奥义就是

**程序员是万能的**

    隐含的一层意思是你的程序你负责. 所有的一切都因指针而起, 它无所不能在当前的系统世界里.
    本小节我们只表述华山剑法中 C 指针的写法范式. 首先看下面小段演示 Demo.

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

    微发现上面两种写法不自然和统一. 由于 C 本身缺陷就不少, 上面两种写法都不完美. 第1种, 
    函数定义的时候 * 号就不知道该放在那里了. 这种用法比较广, 但也不是最好的选择. 第2种
    写法, 多数是有过面向对象的编程经验, 重温一下 C. 这种写法在定义多个变量指针的时候基本
    就废了. 而且本身是变量指针声明, 却被"幻想"成指针类型. 第2种写法强烈不推荐. 这里采用
    下面写法, 在追求自然和美过程中, 脚步不要停歇!

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

        关于指针范式基调主要如上. 简单扯一点关于 C 变量声明. C 目前共有44个关键字, 推
    荐命名要短小精悍. 强烈不推荐驼峰规则, 因为在远古时期 C 是不区分大小写. 而且 C 代码
    本身不容易读懂, 要懂的人会懂. 推荐遵从内核源码古法. 命名语义区分用 _ 分隔. C 也许不
    能让工资飞高, 但是可以让你在面对大佬的时候看见希望 ~

    小节最后不妨借花献佛. 引述个<<C 语言问题>>书中让人豁然开朗, 关于 C 命名经典问题.

```C
问: 如何判断哪些标识符可以使用, 那些被保留了?
答:
  1' 标识符的3个属性: 作用域, 命名空间和链接类型.
　　[*] C 语言有4种作用域(标识符声明的有效区域): 函数, 文件, 块和原型. (第4种类型
       仅仅存在于函数原型声明的参数列表中)
　　[*] C 语言有4种命名空间: 行标(label, 即 goto 的目的地), 标签(tag, 结构, 联
       合和枚举名称), 结构联合成员, 以及标准所谓的其它"普通标识符"(函数, 变量, 类
       型定义名称和枚举常量). 另一个名称集(虽然标准并没有称其为"命名空间")包括了预
       处理宏. 这些宏在编译器开始考虑上述4种命名空间之前就会被扩展.
　　[*] 标准定义了3中"链接类型": 外部链接, 内部链接, 无链接. 对我们来说, 外部链接
       就是指全部变量, 非静态变量和函数(在所有的源文件中有效); 内部链接就是指限于
       文件作用域内的静态函数和变量; 而"无链接"则是指局部变量及类型定义(typedef)
       名称和枚举常量.

  2' ANSI/ISO C标准标识符标准建议规则:
   规则1: 所有下划线大头, 后跟一个大写字母或另一个下划线的标识符永远保留(所有的作
         用域, 所有的命名空间).
   规则2: 所有以下划线打头的标识符作为文件作用域的普通标识符(函数, 变量, 类型定义
         和枚举常量)保留(为编译器后续实现保留).
   规则3: 被包含的标准头文件中的宏名称的所有用法保留.
   规则4: 标准中所有具有外部链接属性的标识符(即函数名)永远保留用作外部链接标识符.
   规则5: 在标准头文件中定义的类型定义和标签名称, 如果对应的头文件被包含, 则在(同
         一个命名空间中的)文件作用域内保留.(事实上, 标准声称"所有作用于文件作用域
         的标识符", 但规则4没有包含标识符只剩下类型定义和标签名称了.)
```

    以上关于标识符问答推荐 write and read 一遍 :)

### 1.5 C 中三大战神 - 宏 - 封神记·天

        宏有点爽也有些疯狂. 但切记教条[能用 inline 内联, 就不要用宏!] (目前而言 
    inline 不太好用) 如果说指针是自由, 那宏就是噩梦. 我们从 struct.h 来认识一下
    这个偏执狂 ->

```C
#ifndef _H_STRUCT
#define _H_STRUCT

#include <math.h>
#include "alloc.h"
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>
#include <inttypes.h>

//
// cmp_f - 比较行为 > 0 or = 0  or < 0
// : int add_cmp(const void * now, const void * node)
//
typedef int (* cmp_f)();

//
// new_f - 构建行为
// : void * rtree_new(void * node)
//
typedef void * (* new_f)();

//
// node_f - 销毁行为
// : void list_die(void * node)
//
typedef void (* node_f)(void * node);

//
// start_f - pthread create func
// : int * run(int * arg)
//
typedef void * (* start_f)(void * arg);

//
// each_f - 遍历行为, arg 外部参数, node 是内部结点
// : int echo(void * node, void * arg) { return 0; }
//
typedef int (* each_f)(void * node, void * arg);

//
// DCODE - DEBUG 模式下的测试宏
// DCODE({
//     puts("debug start...");
// });
//
#ifndef DCODE
#   ifdef _DEBUG
#       define DCODE(code)  do code while(0)
#   else
#       define DCODE(code)  
#   endif //  ! _DEBUG
#endif  //  ! DCODE

//
// CERR - 打印错误信息
// EXIT - 打印错误信息, 并 exit
// IF   - 条件判断异常退出的辅助宏
//
#define CERR(fmt, ...)                                                   \
fprintf(stderr, "[%s:%s:%d][%d:%s]" fmt "\n",                            \
    __FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)

#define EXIT(fmt, ...)                                                   \
do {                                                                     \
    CERR(fmt, ##__VA_ARGS__);                                            \
    exit(EXIT_FAILURE);                                                  \
} while(0)

#define IF(cond)                                                         \
if ((cond)) EXIT(#cond)

//
// RETURN - 打印错误信息, 并 return 返回指定结果
// val      : return 的东西, 当需要 return void; 时候填 , 就过 or NIL
// fmt      : 双引号包裹的格式化字符串
// ...      : fmt中对应的参数
// return   : val
// 
#define RETURN(val, fmt, ...)                                           \
do {                                                                    \
    CERR(fmt, ##__VA_ARGS__);                                           \
    return val;                                                         \
} while(0)

#define NIL
#define RETNIL(fmt, ...)                                                \
RETURN(NIL , fmt, ##__VA_ARGS__)

#define RETNUL(fmt, ...)                                                \
RETURN(NULL, fmt, ##__VA_ARGS__)

#endif//_H_STRUCT
```

    这里主要讲解华山剑法中宏的命名基本准则. 以下关于宏态度和演示, 多感受其中范式!

* 克制使用, 要用就要用的最清晰

```C
#define _H_STRUCT
#define STR_RMRF        "rm -rf '%s'"
#define UINT_LOG        (2048u)
#define INT_Q           (1<<6)
#define FLOAT_ZERO      (0.000001f)	
```

    上面是常量的宏命名准则, 统一大写. 如果是 _ 开头表达'私有'的, 慎重使用. H, STR, UINT 
    INT, FLOAT ... 一眼看过去就知道其宏常量的类型. 第一个是头文件宏, 采用 _H_[文件名称] 
    的命名规范, 防止重复. 如果项目比较大可以加上项目名称, 工程名称等等, 这就是命名空间由来.
    其中字符串常量宏, 为了方便字符串拼接, 不用加(). 数值常量宏防止意外拼接, 加了(). 宏中当
    你不清楚会有什么意外的情况, 加括号是最保险的解决方案. 

* 函数宏另起一行写, 局部使用宏可以放一行写

```C
// BZERO - 变量置零操作
#define BZERO(v)            \
memset(&(v), 0, sizeof(v))

//
// STR - 添加双引号的宏 
// v    : 待添加双引号的量
//
#define STR(v) S_R(v)
#define S_R(v) #v
```

    例如上面 BZERO(v); 可以独立存在, 另起一行加上 \. 对于 STR(v) 可以作为一个子语句, 
    直接写在一行就很清晰明了.

* 有些宏想被认为是函数, 那就小写

```C
typedef volatile long atom_t;  

#ifdef __GNUC__

#define atom_lock(o)        while(__sync_lock_test_and_set(&(o), 1))

#define atom_unlock(o)      __sync_lock_release(&(o))

#endif

#ifdef _MSC_VER
    ...
#endif
```

        写的人意图是希望 atom_lock 和 atom_unlock 被人当'函数'去使用, 使用了小写. 
    对于内置宏, __GNUC__ 是标识 GCC 编译器, 表示当前用 GCC 编译项目会进入这个分支. 
    同样 _MSC_VER 是标识 M$ 的 CL 编译器. 做为开发人员, 推荐用最新的编译器. 因为保持
    活力和新鲜感很有趣, 一次新的尝试说不定就是一次机遇(多数就是坑...)

> 命名规范简单总结  

    a) 变量命名, 函数命名
       -> 小写, 语义分隔使用 _
       -> 全局变量使用 g_ 开头(强烈不建议使用全局量, 太污了)
       -> 宏中局部变量使用 $ 开头, 防止变量污染

```C
//
// TEST - 单元测试宏, 并打印执行时间
// ftest    : 测试函数
// ...      : 可变参数
//
#define TEST(ftest, ...)                                            \
do {                                                                \
    extern void ftest();                                            \
    clock_t $s = clock();                                           \
    ftest (##__VA_ARGS__);                                          \
    double $e = (double)clock();                                    \
    printf(STR(ftest)" run time:%lfs\n", ($e-$s)/CLOCKS_PER_SEC);   \
} while(0)
```

    b) 宏命名
       -> 大写, 语义分隔使用 _
       -> 宏常量采用 [类型简写]_[名称]
       -> 头文件宏采用 _H_[文件名称]_{项目名称}_{工程名称}
       -> 希望被当成函数使用的宏, 可以用小写命名

    c) 枚举声明, 类型声明
       C 枚举同 INT 宏常量很同质化, 默认类型 int. 有时候很显得多余, 请参照下面模式

```C
// enum 状态, >= 0 is Success 状态, < 0 is Error 状态
//
enum {
    SBase       =   +0, // 正确基础类型

    EBase       =   -1, // 错误基础类型
    EParam      =   -2, // 输入参数错误
    EFd         =   -3, // 文件打开失败
    EClose      =   -4, // 文件操作关闭
    EAccess     =   -5, // 没有操作权限
    EAlloc      =   -6, // 内存操作错误
    EParse      =   -7, // 协议解析错误
    EBig        =   -8, // 过大基础错误
    ESmall      =   -9, // 过小基础错误
    ETimeout    =  -10, // 操作超时错误
};

//
// start_f - pthread create func
//  : int * run(int * arg)
//
typedef void * (* start_f)(void * arg);

#ifndef TSTR_CREATE

struct tstr {
    size_t len;   // 长度
    size_t cap;   // 容量
    char * str;   // 字符池
};

typedef struct tstr * tstr_t;

//
// TSTR_CREATE - 栈上创建 tstr_t 结构
// TSTR_DELETE - 释放栈上 tstr_t 结构
// var  : 变量名
//
#define TSTR_CREATE(var)                                    \
struct tstr var[1] = { { 0, 0, NULL } }

#define TSTR_DELETE(var)                                    \
free((var)->str)

#endif//TSTR_CREATE
```
     -> 类型声明 [name]_[类型缩写] 例如 start_f tstr_t 等, 让区分什么类型变得
        很简单.
     -> 为了区分枚举和 INT 宏常量, 枚举采用首字母大写, 驼峰规则, 没有使用 _ 分割.

    到这基本把 C 的华山剑法的总纲讲的有小些了. 按照流派范式去写, 你会有更多的时间去学
    高深心法, 年岁久了再出来构建你自己的独孤飞溅. 同是不年轻的穷"屌丝", 要学会适应, 
    不流血那就流水. 都不简单, 也不难 ~

### 1.6 绝世好剑

        万般皆自然, 一通都顺, 魔鬼在踏实的大道上ヾ(๑╹◡╹)ﾉ" 需要时间才会出现. 这里
    的绝世好剑指的是你的编程环境. 硬件方面要是可以的话买最快, 最美, 最便宜的套装. 软件
    方面, 分下面流派描述

**武当流派**  

    a) Install Best New Windows Profession ISO
    b) Install Best New Visual Studio Enterprise
    c) Install Best New Visual Studio Code

    学习的时候推荐用最新的环境, 这样资料少, 躺坑多, 锻炼的机会多. 多说一点, 没有一款
    IDE 比 VS 猛. 那怕 C 和 C++ 被 M$ 打压十多年, VC++ 仍然宇宙中最屌的编辑器(笔者 
    winds 机器上用的是 VS2017). winds 缺陷挺多, 例如封闭, api毫无章法... 导致后期
    技术提升艰难. 阻挡了开发人员飞升天梯. 同样其无敌的 IDE, 为其引来无数上层商业应用. 
    个人觉得 winds 比 linux 难, 同级别待遇比 linux 待遇低. 服务器开发领域很鸡肋, 虽
    强但不通用, 这方面技术韩国同行研究分享的较多. 

**昆仑流派**  

        刚开始用虚拟机上跑的话, 推荐安装环境如下

    a) Install Best New VMware Workstation Pro
    b) Install Best New Ubuntu Desktop ISO
    c) Install Best New Visual Studio Code
    d) Install Best New GCC, GDB, VIM

    完全也可以跳过 a) 直接用老式电脑安装 Ubuntu. 办公效率也许更高(没啥娱乐消耗)

    linux 是业界标杆, 简单高效优美. vi gcc make 能够搞定一切. 无数的一手资料, 强大
    的 man 手册. 题外话, 以前推荐开发用 MAC, 但随着全员 MAC 后. 自己用老式 linux 
    机器也很快之后, 思路有些变化. 发现草木皆可为剑. 什么环境都行, 怎么舒服怎么来, 但
    都要醇厚精通 ~ 

**剑在前方**  

        多数服务器书籍是 GCC 弄的, 这里简单介绍下 Visual Studio CL 相对应操作. 
    先将 GCC 的一些常用命令翻译到 VS 上面, 其它没有说的. 用到时自行查手册 Serach.  

```bash
# gcc -E -> 生成预编译, 调试宏的时候用到. VS 上面默认关闭
(项目)属性 -> 配置属性 -> C/C++ -> 预处理器 -> 生成预处理文件 -> [自行修改]

# gcc -D -> 添加预编译阶段的全局宏, 但 VS 上面默认值为1
(项目)属性 -> 配置属性 -> C/C++ -> 预处理器 -> 预处理器定义 -> [自行添加]

# gcc -I -> 添加包含目录
(项目)属性 -> 配置属性 -> VC++目录 -> 包含目录 -> [自行添加]

# gcc -l -> 添加引入的库
(项目)属性 -> 配置属性 -> VC++目录 -> 库目录 -> [自行添加]
(项目)属性 -> 链接器 -> 输入 -> 附加依赖库 -> [自行添加]

# 额外的 make 后续命令操作
(项目)属性 -> 生成事件 -> 后期生成事件 -> 命令行 -> [自行添加]

# ... 多写代码, 常看书, 经验条上涨佷猛, 宕宕宕 升级了 ~~~
```

    对于 CL 编译器, 说一下项目经常用的预处理宏和系统库的作用
    -> 预处理器定义
       _DEBUG                          -> Debug 模式标识宏
       WIN32_LEAN_AND_MEAN             -> winds 屏蔽不常用 api 优化应用程序
       _CRT_SECURE_NO_WARNINGS         -> 方便用 scanf, strcpy 等无辜的老函数
       _CRT_NONSTDC_NO_DEPRECATE       -> winds 开启 POSIX 支持部分 mkdir ...
       _WINSOCK_DEPRECATED_NO_WARNINGS -> 开启 winsock2.h 中扩展 api

    -> 附加依赖库
       ws2_32.lib                      -> 使用 winsock2.h api 必须引入的库
    

    年轻人有时候喜欢做很多事情, 例如通过下面代码帮 VS CL 编译器附加链接依赖库.

```C
#ifdef _MSC_VER

#include <ws2tcpip.h>

// socket_init - 初始化 socket 库初始化方法
inline void socket_init(void) {
#   pragma comment(lib, "ws2_32.lib")
    WSADATA wsad;
    WSAStartup(WINSOCK_VERSION, &wsad);
}

#endif
```

    这种写法没有语法错误, 实战也可以用. 但累人, 毕竟链接是编译器事情, 倒不如全权交给它. 
    这才真潇洒 ~ 库中直接 #pragma comment 做法不妙. 个人感悟, 跨平台是在浪费生命. 有
    些人单纯的用 linux. 感觉很好, 毕竟越纯粹越强. linux 是个回报率很高的平台. 但是学了
    C/C++ 真是天高地厚不知路长~ 岁月不尽然呀...

### 1.7 夜太黑练剑好时光

        上面聊的有些多, 细节部分需要自己亲身建构. 回想起2013年看<<自制编程语言>>那本书,
    惊为天人. 感觉作者实力好强. 因为看不明白, 强行撸, 狂看猛打最后懂了点, 收益良多(也都
    忘了). 在编程的世界里, 不需要太多前缀, 只要 wo are 正在舞剑! 
        如果咱们一样只是为了, 更好的生存和活着. 那么学起来就更随意了, 君子当善假于物! 熟
    悉工具, 帮人实现梦想, 就成了 ~ 然后回家开个水果店, 想想也挺好.

如果你没有对象, 那就使劲敲代码

如果你觉得无聊, 那就跑步加看书

如果你真不甘心, 那就呵呵萌萌哒

        书归正转, 前面 struct.h 头文件中引入了 alloc.h 头文件, 不知道小伙伴是否还记得.
    这个 alloc.h 是我们对 malloc / free 等内存操作接口包装层. 它基本原理是替换我们使用
    系统申请内存释放内存相关函数. 并且携带了少量平台通用宏. 先看接口设计

```C
#ifndef _H_ALLOC
#define _H_ALLOC

#include <stdlib.h>
#include <string.h>

// :) 高效内存分配, 莫名伤感 ~
//
#ifndef OFF_ALLOC

#undef  free
#define free    free_

#undef  strdup
#define strdup  strdup_

#undef  malloc
#define malloc  malloc_
#undef  calloc
#define calloc  calloc_
#undef  realloc
#define realloc realloc_

#endif//OFF_ALLOC

// enum 状态, >= 0 is Success 状态, < 0 is Error 状态
//
enum {
    SBase       =   +0, // 正确基础类型

    EBase       =   -1, // 错误基础类型
    EParam      =   -2, // 输入参数错误
    EFd         =   -3, // 文件打开失败
    EClose      =   -4, // 文件操作关闭
    EAccess     =   -5, // 没有操作权限
    EAlloc      =   -6, // 内存操作错误
    EParse      =   -7, // 协议解析错误
    EBig        =   -8, // 过大基础错误
    ESmall      =   -9, // 过小基础错误
    ETimeout    =  -10, // 操作超时错误
};

//
// free_ - free 包装函数
// ptr      : 内存首地址
// return   : void
//
extern void free_(void * ptr);

//
// malloc_ - malloc 包装函数
// size     : 分配的内存字节
// return   : 返回可使用的内存地址
//
extern void * malloc_(size_t size);

//
// strdup_ - strdup 包装函数
// s        : '\0' 结尾 C 字符串
// return   : 拷贝后新的 C 字符串
//
extern char * strdup_(const char * s);

//
// calloc_ - calloc 包装函数
// num      : 数量
// size     : 大小
// return   : 返回可用内存地址, 并置 0
//
extern void * calloc_(size_t num, size_t size);

//
// realloc_ - realoc 包装函数
// ptr      : 首地址, NULL 等同于 malloc
// size     : 重新分配的内存大小
// return   : 返回重新分配的新地址
//
extern void * realloc_(void * ptr, size_t size);

#endif//_H_STDEXIT
```

    通过 OFF_ALLOC 宏配置来替换全局 free / malloc. 使用时候对用户无感知的. 目前采用了近代
    软件编程中最后免费午餐 jemalloc 来包装我们的 alloc.h 层. jemalloc 科普可以搜查资料, 
    对于如何编译成静态库并使用, 可在 jemalloc github 主页获取官方方法. 如果其中遇到困难, 
    可以搜索和翻看作者相关博客. alloc.c 实现全在这里, 代码即注释 ~

```C
#include <stdio.h>

#define OFF_ALLOC
#include "alloc.h"

#define JEMALLOC_NO_DEMANGLE
#include <jemalloc/jemalloc.h>

// check - 内存检测并处理
static inline void * check(void * ptr, size_t size) {
    if (NULL == ptr) {
        fprintf(stderr, "check memory collapse %zu\n", size);
        fflush(stderr);
        abort();
    }
    return ptr;
}

//
// free_ - free 包装函数
// ptr      : 内存首地址
// return   : void
//
inline void free_(void * ptr) {
    je_free(ptr);
}

//
// malloc_ - malloc 包装函数
// size     : 分配的内存字节
// return   : 返回可使用的内存地址
//
inline void * malloc_(size_t size) {
    void * ptr = je_malloc(size);
    return check(ptr, size);
}

//
// strdup_ - strdup 包装函数
// s        : '\0' 结尾 C 字符串
// return   : 拷贝后新的 C 字符串
//
inline char * strdup_(const char * s) {
    if (s) {
        size_t n = strlen(s) + 1;
        char * ptr = malloc_(n);
        return memcpy(ptr, s, n);
    }
    return NULL;
}

//
// calloc_ - calloc 包装函数
// num      : 数量
// size     : 大小
// return   : 返回可用内存地址, 并置 0
//
inline void * calloc_(size_t num, size_t size) {
    void * ptr = je_calloc(num, size);
    return check(ptr, size);
}

//
// realloc_ - realoc 包装函数
// ptr      : 首地址, NULL 等同于 malloc
// size     : 重新分配的内存大小
// return   : 返回重新分配的新地址
//
inline void * realloc_(void * ptr, size_t size) {
    void * ntr = je_realloc(ptr, size);
    return check(ntr, size);
}
```

    对于 check 函数思路, 内存不足时, 直接 abort. 非常粗暴有效, 管不了那么多了, 快上车.
    而引入 jemalloc 库很自信扛住内存方面瓶颈, 又给华山剑法注入了一道自信秘术 :0 练习的人
    不知道是否感到身体中兴奋 ~ 颤抖 ~ 原来这么简单, 就可以这么强. 哈哈. 此刻只想说三个字, 
    屌 屌  屌 ~ (仔细看 alloc.h 会发现, 实现是可插拔的.)

***

> 在心田种颗草 ----<<@

***

    也许 - 汪国真

    也许, 永远没有那一天
    前程如朝霞般绚烂
    也许, 永远没有那一天
    成功如灯火般辉煌
    也许, 只能是这样
    攀援却达不到峰顶
    也许, 只能是这样
    奔流却掀不起波浪
    也许, 我们能给予你的
    只有一颗
    饱经沧桑的心
    和满脸风霜

***

-----<--*练剑的你, 终于有一天将坑满天下*

***

![笑傲江湖](img/笑傲江湖.jpg)
