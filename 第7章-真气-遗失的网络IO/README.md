# 第7章-真气-遗失的网络IO

    华山派除了练气的华山剑法, 其实还有一本被遗忘的神功, 紫霞真气. 也许正是靠它,
    当年气宗整体压住了剑宗. 后期随着气宗修习紫霞真气的资格越来越高, 导致气宗衰败. 
    紫霞神功也在江湖中失传. 同样编程中也有个遗失的学习领域, 网络开发. 学习成本陡峭,
    开发成本不低, 阻挡了一批有一批想了解服务器开发的生源. 好在有不少巅峰的网络库吼
    住了互联网的基石. 导致重新搭建网络IO 轮子价值变低. 但如果想飞的更高, 或许 ~ 
    此刻不妨随我翻开远古大能们遗留的书章, 感受下那些年可以吹的NB. 友情提示, 本书
    写到这, 可以发现非常注重实战技巧, 忽略缘由. 因为授之以渔, 你还要有欲望, 才能驱
    动. 倒不如授之以臭鳜鱼, 说不定还能爽一口. 

    文末既有可能, 分享江湖中一个化神前辈的元婴功法, 网络 IO poll 模型. 全章流程
    渐进, 需要反复回溯, 必有豁然开朗醍醐灌顶. 修真, 修炼, 一个不可说的秘密就是,
    有了时间就有了一切基础. 例如 <<无尽剑装>> 中主角成圣无不是靠寿元对磨求道 ~

    不妨扯一下以前修炼的经验, 当初刚做开发时候. 公司朴实的前辈说, 搞个 TCP 估计1
    年就过去了. 后来想想还真是, 虽然到现在还是水经验. 但有一点可以确定, 编程, 修
    炼并不是一个青春饭. 很需要底蕴, 和积累. 投机倒把很难干成一件长时间的事情. 大佬
    赠我越努力越幸运. 我也有个小窍门送给修真路上的道友们 : 当你有一天, 打游戏了累
    了, 刚好没对象. 那就打开记事本, 翻开页面, 搞代码瞎玩吧.    

    山中不知岁月, 一切刚刚开始 ~

## 7.1 筑基回顾, 回忆哈 C

    C 语言老了, 目前而言(2017年9月4日) C 语言中有 32 + 5 + 7 = 44 个关键字. 
    具体如下 O(∩_∩)O 哈 :

-> C89 版本关键字

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
|          |           |          |          |

-> C99 新增关键字

|       |          |            |        |          |
| ----- | -------- | ---------- | ------ | -------- |
| _Bool | _Complex | _Imaginary | inline | restrict |
|       |          |            |        |          |

-> C11 新增关键字

|          |          |         |          |           |                |               |
| -------- | -------- | ------- | -------- | --------- | -------------- | ------------- |
| _Alignas | _Alignof | _Atomic | _Generic | _Noreturn | _Static_assert | _Thread_local |
|          |          |         |          |           |                |               |

    随后会细细分析起具体用法, 每个平台的时间会有些差别. C11 最强, C89 最稳. 

### 7.1.1 C89 32 个关键字

    1) char
    解释 :
        声明变量的时候用! 
        char占1字节, 8bit. 多数系统(cl or gcc)上是有符号的(arm 无符号), 
        范围是[-128, 127]. 在工程项目开发中推荐用

```C
    #include <stdint.h>
    #include <stddef.h>
    int8_t   -> signed char 
    uint8_t  -> unsigned char
```  

        扯淡一点, 程序开发最常遇到的就是自解释问题. 鸡生蛋, 蛋生鸡. 后面再分析 signed 和 
        unsigned.
    演示 :

```C
#include <stdio.h>

char c = getchar();
rewind(stdin);
printf("c = %d, c = %c.\n", c);
```

    2) short
    解释 :
        声明变量的时候用! 
        short 占2字节, 为无符号的. 默认自带signed. 范围[-2^15, 2^15 - 1] 
        2^15 = 32800. 推荐使用 int16_t or uint16_t 类型.

    演示 :

```C
short port = 8080;
printf("port = %d.\n", port);
```

    3) int
    解释 :
        声明变量的时候用! 
        int 声明的变量, 占4字节, 有符号. 范围 [-2^31, 2^31-1].
        推荐用 int32_t 和 uint32_t类型开发. 方便移植

    演示 :

```C
int hoge = 24;
printf("hoge = %d.\n", hoge);
```

    4) unsigned
    解释 :
        变量类型修饰符! 
        被修饰的变量就是无符号的.范围 >= 0.  unsigned 只能修饰整型的变量. 
        当然当你用这个修饰变量的时候. 再使用 - 和 -- 运算的时候一定要小心

    演示 :

```C
unsigned int i = 0;         // 正确
unsigned short s = 0;       // 正确
unisgned float f = 0.11f;   // 错误
```

    5) long
    解释 :
        声明变量的时候用!
        长整型 x86 上四字节, x64 上8字节. 一定不比 int 字节数少.
        C99之后出现long long类型8字节.

    演示 :

```C
long l = 4;
long long ll = l;
printf("l = %ld, ll = %lld.\n", l, ll);
```

    6) float
    解释 :
        声明变量的时候用! 
        四字节. 精度是6-7位左右. 详细精度可以查资料 float与double的范围和精度 

    演示 :

```C
float f = -0.12f;        // 四字节
long float lf = 0;       // 八字节 等同于 double, 不推荐这么写
```

    7) double
    解释 :
        声明变量的时候用!
        八字节, 精度在15-16位左右. 有的时候压缩内存用float代替.

    演示 :

```C
double d = 2e13;               // 8字节
long double ld = -0.99;        // x86也是8字节, 不推荐这么用
long long double lld = 99;     // 写法错误, 不支持
```

    8) struct
    解释 :
        定义结构体, 这个关键字用法广泛, 是大头!
        c 的重要思路就是面向过程编程. 撑起面向过程的大头就是结构体.
        struct 就是定义结构的东西, 可以看看下面演示

    演示 :

```C
// 普通结构体定义
struct node {
    int id;
    struct node * next;   
};

struct node node = { 1, NULL };

// 匿名结构定义
struct {
   int id;
   char * name;   
} per = { 2, "王志" };
```

    9) union
    解释 :
        定义公用体, 用法很花哨!
        常在特殊库函数封装中用到. 技巧性强

    演示 :

```C
// 普通定义
union type {
    char c;
    int i;
    float f;
};

union type t = { .f = 3.33f };

// 匿名定义
union { ... } t = { .... };
    
// 类型匿名定义
struct cjson {
    struct cjson * next;
    struct cjson * child;

    unsigned char type;
    char * key;
    union {
        char * vs;
        double vd;
    };
};
```

    再来一种 union用法, 利用内存对齐.

```C
// 12.0 判断是大端序还是小端序, 大端序返回true
inline bool
sh_isbig(void) {
    static union {
        unsigned short _s;
        unsigned char _c;
    } _u = { 1 };
    return _u._c == 0;
}
```

    还有很久以前利用 union 实现内存字节对齐, 太多了. 关键字用法, 很多, 很意外.

    10) void
    解释 :
        这个是空关键字. 用法很多! 
        也是我最喜欢的关键字. 用在函数声明中, 类型定义中.

    演示 :

```C
// 函数声明
extern void foo();

// 函数参数约束
extern void foo(void);   // ()中加了void表示函数是无参的, 否则是任意的

// 万能类型定义, 指针随便转
void * arg = NULL;
```

    11) enum
    解释 :
        枚举类型, C中枚举类型很简陋! 
        其实就相当于一种变相的 INT宏常量. 估计这也许也是 INT宏常量和枚举并存的原因.

    演示 :

```C
//
// flag_e - 全局操作基本行为返回的枚举, 用于判断返回值状态的状态码
// >= 0 标识 Success状态, < 0 标识 Error状态
//
typedef enum {
    Success_Exist    = +2,           //希望存在,设置之前已经存在了.
    Success_Close    = +1,           //文件描述符读取关闭, 读取完毕也会返回这个
    Success_Base     = +0,           //结果正确的返回宏

    Error_Base       = -1,           //错误基类型, 所有错误都可用它, 在不清楚的情况下
    Error_Param      = -2,           //调用的参数错误
    Error_Alloc      = -3,           //内存分配错误
    Error_Fd         = -4,           //文件打开失败
} flag_e;
```

    枚举变量完全可以等同于 int 变量使用, 枚举值等同于宏INT常量使用. 
    枚举的默认值是以0开始, 1为位单位从上向下递增.

    12) signed
    解释 :
        变量声明类型修饰符. 有符号型!
        对比 unsigned 无符号型. 变量声明默认基本都是 signed, 所以多数别人就省略了.

    演示 :

```C
signed int piyo = 0x1314520;
signed char * str = u8"你好吗";
```

    当然了, 平时不需要刻意加. 会让人嫌麻烦. O(∩_∩)O哈哈 ~

    13) const
    解释 :
        const修饰的变量表示是个不可修改的量!
        和常量有点区别. 可以简单认为 const type val 是个只读的.

    演示 :

```C
// 声明不可修改的量
const int age = 24;

// 修饰指针
const int * pi = NULL;         // *pi 不能修改指向变量
int * const pt = NULL;         // pt 不能指向新的指针 
const int * const pc = NULL;   // *pc 和 pc 都不能动
```

    其实在 c 中基本没有什么改变不了的. 全是内存来回搞, 软件不行硬件~~

    14) volatile
    解释 :
        声明变量修饰符, 可变的!
        当变量前面有这个修饰符. 编译器不再从寄存器中取值, 直接内存读取写入. 
        保证实时性. 常用在多线程代码中.

    演示 :

```C
// 具体轮询器
struct srl {
    mq_t mq;                 // 消息队列
    pthread_t th;            // 具体奔跑的线程
    die_f run;               // 每个消息都会调用 run(pop())
    volatile bool loop;      // true表示还在继续 
};
```

    以后使用 srl::loop 的时候被其它线程修改, 当前线程也能正确获取它的值.

    15) typedef
    解释 :
        类型重定义修饰符, 重新定义新的类型!

    演示 :

```C
// 声明普通类型
typedef void * list_t;

// 声明不完全类型, 头文件中不存在struct tree
typedef struct tree * tree_t;
```

    16) auto
    解释 :
        变量类型声明符!
        auto变量存放在动态存储区，随着生存周期 {开始 }结束 而立即释放. 存放在栈上. 
        默认变量都是auto的. 基本都是不写, 除非装逼!

    演示 :


```C
{
    // 生存期开始
    int hoge = 0;
    auto int piyo = 1;
    // 生存期结束
}
```

    17) register
    解释 :
        变量修饰符! 只能修饰整形变量. 表示希望这个变量存放在CPU的寄存器上. 
        现代编译器在开启优化时候, 能够一定程度上默认启用register寄存器变量.

    演示 :

```C
#include <limits.h>

register int i = 0;
while (i < INT_MAX) {

   ++i;
}
```

    由于CPU寄存器是有限的, 有时候你哪怕声明的寄存器变量也可能只是普通变量. 
    printf("&i = %p\n", &i) 这种用法是非法. 语义层面寄存器变量没有地址.


    18) static
    解释 :
        static 用法很广泛, 修饰变量! 表示变量存在于静态区, 基本就是全局区. 
        生存周期同系统生存周期. static 修饰的变量作用域只能在当前文件范围内. 
        可以看成上层语言的 private. 除了auto 就是 static. static 修饰函数
        表示当前函数是私有的, 只能在当前文件中使用. 更加详细的看演示部分.

    演示 :

```C
// 修饰全局变量, 只对当前文件可见
static int _fd = 0;

// 修饰局部变量, 存储在全局区, 具有记忆功能
{
    static int _cnt = 0;
}

// 修饰函数, 函数只能在当前文件可见
static void * _run(void * arg) {
   ......
   return arg;
}

//
// C99之后加的static新用法, 编译器优化
// static 只能修饰函数第一维,表示数组最小长度, 方便编译器一下取出所有内存进行优化
//
int sum(int a[static 10]) { ... }
```

    19) extern
    解释 :
        extern 关键字表示声明, 变量声明, 函数声明! 奇葩的用法很多.

    演示 :

```C
// 声明引用全局变量
extern int g_cnt;

// 声明引用全局函数
extern int kill(int sig, int val);
```

    当然有时候 extern 不写, 对于变量不行会出现重定义. 对于函数是可以缺省写法. 
    再扯一点

```C
// extern 主动声明, 希望外部可以调用
extern int kill(int sig, int val);

// extern 缺省,不推荐外部调用
int kill(int sig, int val);
```

    20) break
    解释 :
        结束语句!
        主要用于循环的跳转, 只能跳转到当前层级. 也用于 switch 语句中, 跳出 switch 嵌套.

    演示 :

```C
for(;;) {
   // 符合条件跳转
   if(six == 6)    
       break;
}

// break 跳出while循环
int i = 0;
while(i < 6) {
   if(i == 3)
      break;
} 
```

    break 用法主要和循环一块使用, 还有 do while. 但只能跳转当前层循环. 

    21) case
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

    对于case相当于标记点. switch 中值决定 case 跳转到哪里. 再一直往下执行, 
    遇到 break 再结束 switch 嵌套.

    22) continue
    解释 :
        跳过此次循环!
        直接进行条件判断操作. for 和 while 有些局别. for 会执行第三个后面的语句.

    演示 :

```C
// for 循环 continue
for(int i = 0; i < 20; ++i) {
    if(i % 2 == 0)
         continue;
    
     // 上面continue 调到 ++i -> i < 20 代码块
}
```

    23) default
    解释 :
        switch 分支的默认分支!
        假如 case 都没有进入那就进入 default 分支. default 可以省略 break. 
        c 语法中可行.

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

    24) do
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

    25) else
    解释 :
        else 是 if 的反分支! 具体看演示

    演示 :

```C
#include <stdbool.h>

if(true) {
   puts("你好吗?");
}
else {
  puts("我们分手吧.");
}


// 附赠个else 语法
#if defined(__GNUC__)

// 定义了 __GNUC__ 环境, 就是gcc环境

#else

#error "NOT __GNUC__, NEED GCC!";

#enfif
```

    26) for
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

// for 最好的写法, 在于死循环写法
for(;;) {
   // xxxx
}
```

    for(;;) {  } 比 while(true) { } 写法好. 表达不走判断的意图, 虽然汇编代码是一样的.

    27) goto
    解释 :
        goto 可以在当前函数内跳转!
        goto 可以替代所有循环. goto 在复杂业务中用到, 切记不要麻木的用!

    演示 :

```C
_loop:
   // xxx 死循环用法
goto _loop;
_exitloop:
```

    还有就是在工程开发中, goto 常用于复制的业务逻辑.

```C
if ((n = *tar) == '\0') // 判断下一个字符
    goto _errext;    

if(cl % rl){ // 检测 , 号是个数是否正常
_errext:
    SL_WARNING("now csv file is illegal! c = %d, n = %d, cl = %d, rl = %d."
        , c, n, cl, rl);
    return false;
}
```

    28) if
    解释 :
        if 分支语句! 
        用法太多了. 程序语句中分支就是智能.

    演示 :

```C
if(false) {
   puts("想做个好人!");
}
```

    29) return
    解释 :
        程序返回语句太多了, 用于函数返回中!
        单纯返回 void 可以直接 return;

    演示 :

```C
#include <stdlib.h>
int main(int argc, char * argv[]) {
   return EXIT_SUCCESS;
}
```

    30) switch
    解释 :
        条件分支语句!
        很复杂的 if else if 时候, 特定情况可以转为 switch.

    演示 :

```C
#include <unistd.h>

do {
    int rt = write(fd, buf, sizeof buf)
    if(rt < 0) {
       switch(errno) {
       case EINTER
           continue;
       default:
           perror("write error");
       }
    }
} while(rt > 0);
```

    31) while
    解释 :
        循环语句!
        有do while 和 while 语句两种.

    演示 :

```C
#define _INT_CNT (10)

int i = -1;
while(++i < _INT_CNT) {
     // ...... 
}
```

    32) sizeof
    解释 :
        也称为 sizeof 运算符, 计算变量或类型的字节大小, 这个关键字特别好用!

    演示 :

```C
sizeof (main)   -> x86 上四字节

// 获取数组长度,只能是数组类型或""字符串常量,后者包含'\0'
#define LEN(arr) (sizeof(arr) / sizeof(*(arr))) 
```

    到这里 C89 保留的关键字基本解释完毕. 走马观花理解理解只求加深理解.

### 7.1.2 C99 5个新增关键字

    33) _Bool
    解释 :
        bool 类型变量!
        等价于 unsigned char . 只有0和1.

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

    33) _Bool
    解释 :

    演示 :

```C
```

    34) _Complex
    解释 :
        数学中复数类型!
        对于C99 标准定义, 存在 float _Complex, double _Complex, long double _Complex 
        复数类型. 下面先演示gcc 中关于复数的用法.

    演示 :

```C
#include <math.h>
#include <stdio.h>
#include <complex.h>

//
// 测试 c99 complex 复数
//
int main(int argc, char * argv[]) {

    float complex f = -1.0f + 1.0if;
    printf("The complex number is: %f + %fi\n", crealf(f), cimagf(f));
    
    double complex d = csqrt(4.0 + 4.0i);
    printf("d = %lf + %lfi\n", creal(d), cimag(d));

    return 0;
}
```

    其实在复数类型中, gcc 标准实现

```C
#define complex     _Complex
```

    而 CL 中具体实现如下:

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

    总的说, 学习 C 最好的平台就是 *nix 平台上使用 Best new GCC. 
    当然除了科学计算会用到复数, 其它很少. 这里 VS 和 GCC 实现理念不一样. 
    用起来需要注意. 总有一天你会全身心进入开源的怀抱 ~

    35) _Imaginary
    解释 :
        虚数类型, _Complex 复数类型的虚部!
        例如 10.0i, 10.8if 等等.  这个关键字在VS 上没有实现. 个人也觉得没有必要.
        和 _Complex 有重叠.

    演示 :

```C
/* 这个关键字无法在代码中表示. 系统保留, 我们不能使用. */
```

    36)  inline
    解释 :
        内联函数定义符号!
        从 C++ 中引入的概念. 就是将小函数直接嵌入到代码中. C的代码损耗在于函数的进
        出栈. 要是可以推荐用内联函数替代宏. 宏能不用就不用. 函数声明的时候不要加 
        inline 需要加 extern, 定义的时候需要加 inline. 主要是不同平台语义实现不一
        致, 用起来随环境而定.

    演示 :

```C
/*
 * 对json字符串解析返回解析后的结果
 * jstr        : 待解析的字符串
 */
extern cjson_t cjson_newtstr(tstr_t str);

inline cjson_t 
cjson_newtstr(tstr_t str) {
    str->len = _cjson_mini(str->str);
    return _cjson_parse(str->str);
}

// 还有就是和static 一起使用
static inline int _sconf_acmp(tstr_t tstr, struct sconf * rnode) {
    return strcmp(tstr->str, rnode->key);
}
```

    37) restrict
    解释 :
        指针修饰符!
        这是很装逼的关键字用于编译器优化. 关键字restrict只用于限定指针; 该关键字用于告知
        编译器, 所有修改该指针所指向内容的操作全部都是基于(base on)该指针的, 即不存在其
        它进行修改操作的途径; 这样的后果是帮助编译器进行更好的代码优化, 生成更有效率的汇
        编代码.

    演示 :

```C
extern void *mempcpy (void *__restrict __dest,
                      const void *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
```

    上面是摘自 GCC 的 string.h 中. 其实正式用法

```C
// 简单演示用法, GCC 和 VS 都是 __restrict 推荐加在 * 后面
static inline void _strlove(char * __restrict dest) {
    *dest = '\0';
}
```

    Pelles C 编译器可以完整支持 restrict, 可惜非商业力量不足, 后继无人.

### 7.1.3 C11 7个新增关键字

    38) _Alignas
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

    例如一种用法

```C
#include <stdio.h>
#include <stdalign.h>

struct per {
    int age;
    double secl;
    char sex;
};

int main(int argc, char * argv[]) {
    char x[100];
    alignas(struct per) struct per * per = (struct per *)&x;
    printf("per = %p, c = %p.\n", per, c); 

    return 0;
}
```

    将 x 数组以 struct per 对齐方式对齐返回回去. 构造过程同内存申请分离.

    39) _Alignof
    解释 :
        返回由类型标识所指定的类型的任何实例所要求的按字节算的对齐! 
        该类型可以为完整类型, 数组类型或者引用类型. 当类型为引用类型时, 该运算符返回被
        引用类型的对齐; 当类型为数组类型时, 返回其元素类型的对齐要求.

    演示 :

```C
// return size_t , 对齐方式是 8 字节, 值为 8
printf("alignof(struct per) = %zu.\n", alignof(struct per));
```

    40) _Atomic
    解释 :
        原子操作, 原子锁! 
        目前 gcc 上面支持, cl 不支持. 如果编译器都支持那么以后无锁编程更加方便高效.
        详细用法需要查阅相关手册, 无外乎怎么初始化, 设置, 加载等等 ~

    演示 :

```C
#include <stdio.h>
#include <stdatomic.h>

int main(int argc, char * argv[]) {
    // 全局初始化 hoge, or 局部初始化 atomic_init(&hoge, 100)
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

    具体的执行结果, 你也懂就那样. 原子操作, 对于写出高效代码很重要. 
    可惜微软太硬, 有机会下次写书来个全本 linux 版本的 ~ 去 CL 体系构建.

    41) _Generic
    解释 :
        这个比较叼, C的泛函机制. 高级函数宏. 下面来个老套路用法!

    演示 :

```C
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) \
        _Generic((x), int:abs, float:fabsf, double:fabs)(x)

//
// 测试 C11 语法
//
int main(int argc, char * argv[]) {
        int a = 1, b = 2, c = 3;
    
        _Generic(a + 0.1f, int:b, float:c, default:a)++;
        printf("a = %d, b = %d, c = %d\n", a, b, c); 

        printf("int abs: %d\n", ABS(-12));
        printf("float abs: %f\n", ABS(-12.04f));
        printf("double abs: %f\n", ABS(-13.09876));

        return EXIT_SUCCESS;
}
```

    宏泛型真的很给力. 宏又能玩上天了.

    42) _Noreturn
    解释 :
        修饰函数, 绝对不会有返回值!
        _Noreturn 声明的函数不会返回. 引入此新的函数修饰符为了解决, 
        消除编译器对没有 return 的函数的警告, 还有允许某种只针对不返回函数的优化.

    演示 :

```C
_Noreturn void suicide(void) {
    abort(); // Actually, abort is _Noreturn as well
}
```

    再扯一点, GCC 中等同于 __attribute__((__noreturn__)), 在 VC 中相似功能是 
    __declspec(noreturn). 它不是说函数没有返回值, 而是说一旦你调了这个函数, 它存在
    永远不会返回的情况. 一些函数是永远不会返回的, 比如 abort 或者 exit 之类, 调用它
    们就意味着结束程序. 所以 warning 就显得没有必要.

    43) _Static_assert
    解释 :
        编译器期间断言符!
        当 #if #error 搞完毕(预编译)之后, 编译器断言. 
        assert 是运行时断言. 用的时候看具体的需求.

    演示 :

```C
_Static_assert(__STDC_VERSION__ >= 201112L, "C11 support required");
// Guess I don't really need _Static_assert to tell me this :-(
```

    44) _Thread_local
    解释 :
        到这里快扯完了, 其实 C11标准是个很好的尝试. 为C引入了线程和原子操作. 
        各种安全特性补充. 可以说 C强大了. 但是还远远不够, 因为越来越丑了. 
        C11 为 C 引入了线程在头文件 threads.h 中定义. 一如既往也允许编译可以不实现.
        _Thread_local 是新的存储类修饰符, 限定了变量不能在多线程之间共享.

    演示 :

```C
_Thread_local static int i;
// Thread local isn't local!
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

    pthread_key_create 创建私有变量, 且需要注册销毁行为. 随后是删除, 设置, 获取
    操作. 从实现而言, 线程私有变量其实是公有变量模拟的. 开发中推荐少用线程私有变量,
    有数量限制. 到这里关于 C修炼的筑基期, 语法层面的回顾就完毕了. 
    不知不觉新的呼喊已经到来, 期待笑和痛, 收拾行囊 ~ 

***
    游子吟
    孟郊 * 唐代

    慈母手中线，游子身上衣。
    临行密密缝，意恐迟迟归。
    谁言寸草心，报得三春晖。

![真正男子汉](./img/心痛.jpg)
***

## 7.2 C 语言 SOCKET 编程指南

    首先把思路切换到 linux 上来, 毕竟 socket 还是它的强! 随我一举破筑基, 拿下基础
    socket api 常见面庞. 为下次一鼓作气搞定 winds socket 套路. 



    
