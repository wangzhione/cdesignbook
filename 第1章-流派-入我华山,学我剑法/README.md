### 第1章-流派-入我华山,学我剑法
      流派等同于代码范式(套路). 开发中流传最广的范式就是 winds武当流派, linux昆仑流派.  
    流派范式存出不穷自成方圆. 本书在昆仑流派的基础上精进. 并且会分析出招的所以然.  
    眼睛决定路子, 套路决定手法. 这里是C领域, 主要围绕C范式. 这年头一招鲜吃遍天难度大. 
    最好的思路是到了不熟悉的领域, 第一件事忘记以前知道的, 立即训练需要现在知道的. 特定的
    路子能让你游刃有余, 百步穿杨. 但不管怎么扯, 一定要求简求美~  
    本书重点讲解思路和设计, 解读那些被遗忘的故事. 代码搞不完, 只会展示很少的部分充数一下.

#### 1.1 缘起 main
      很久很久以前认识一个被后人称之为 main的函数. 如果要扯估计能说个几天. 这里只引述C11
    标准中两种最准确的写法.
```C
The function called at program startup is named main. The implementation declares 
no prototype for this function. It shall be deﬁned with a return type of int and 
with no parameters:
  int main(void) { /* ... */ }
or with two parameters (referred to here as argc and argv, though any names may 
be used, as they are local to the function in which they are declared):
  int main(int argc, char *argv[]) { /* ... */ }
or equivalent; or in some other implementation-deﬁned manner.
```
    上面比较重要, 因为是一切的开始. C的学习历程, C语言之父的C语言入门, 再到C标准文献.
    扯皮就搬上面的标准文献, 站在起源山峰中. 此刻希望下面是个好的开始 -_-
```C
#include <stdio.h>

int main(int argc, char * argv[]) {
  fprintf(stderr, "I am here!");
  return 0;
}
```

#### 1.2 括号的布局
      C是个抠门到底的语言. 因而用C的开发的都很小心翼翼. 难接收铺场浪费, 且容易栽跟头.
    叶孤城通过一片被剑削过的叶子看出西门吹雪的实力. 同样一个 { } 布局就能见证你的起手式.
    本门华山剑法 { } 遵循的如下规则和演示
      a ). 左 { 右对齐
      b ). 右 } 后面另起一行
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
    因为它表达了语法层面美好的愿望, 执行死循环的时候省略一步条件判断.
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
    第一种写法是 winds推荐的, 第三种写法是多数 linux上常见的. 但是这里推荐第二种.为什么呢.
    第一种好看但太松弛了, C本身就是个精简的语言, 强调最小意外原则. 最后一种太丑了 } () { 和
    两个 } } 实战环境下对应规则也难看出个所以然. 同样对于 switch语句推荐下面写法
```C
switch(type) {
case Success_Base:
  ...
  break;
...  
default:
  ...
}
```
    C中的标签都放在最左边.

#### 1.3 C中三大战神 - 函数 - 帝释天
      面向过程编程两个主旋律就是内存结构和函数封装. 在C中函数的一个小小的 extern声明都能搞晕
    一层人. C函数分为声明和定义两部分. 华山剑法将其细化为3部分. 声明确定函数地址; 定义确定函数
    地址和实现; 声明加定义确定函数地址加实现还有就是作用域.
```C
// 函数声明
extern void hoge(void);

// 函数定义(强调返回值, 单独占一行)
void
hoge(void) {
  ...
}

// 函数声明加定义
void hoge(void) {
  ...
}
```
    上面函数返回值另一起一行强调这是函数定义. 在通一行表示声明加定义. 还有一个细节就是 hoge(void),
    因为在C中函数声明 void hoge(); 表示的意思是函数 hoge参数是不定的. 加上 void表示函数参数为空! 
      当我们写库的时候, 看下面两个函数声明, 表达的意思不一样.
```C
// 显示声明加上 extern
extern void mem_hello(void);

// 缺省声明, 没加 extern, 意图告诉别人, 可以用不推荐用, low level api
void mem_heoo(void);
```
    二者表达意图有点不一样, 前者是外部可以调用. 后者是外部不推荐调用调用的时候需要小心, 了解细节.
    当然了也存在一个特殊情况, 如果定义的函数加了 inline内联行为, 那边声明的时候必须加上extern, 
    或者声明和定义一块写(常见cl编译器). 请同学们多实战演练. 函数的套路至关重要.
      对于 extern其实还有个相对的 关键字 static. 特别神奇它表示 文件或函数内私有的. 请看代码->
```C
static void _heoo(int arr[static 64]) {
  static int _cnt;
  ...
}
```
    上面精简代码中包含的套路和潜规则比较多.
    a. 第一个 static 表示当前函数是私有的, 被包含的文件所有, 作用域在具体文件中这个函数以下所有范围.
    b. 看 _heoo 第一个字符 _, 它是一种约束. 来表达这个函数是私有的. C很自由, 所以有点斑驳, 标准可以
      帮我们避免一些混乱. 当然你也可以有更好的策略, 那是后话哈哈. 同样对于上层语言的命名空间,函数重载,
      无外乎是在编译层面做的命名映射. 因而在C中通过命名规则可以构建你要要的语法糖的糖衣. 好的命名能够
      表达很多信息, 例如 _xxx -> 表示私有直接给咱们程序用. __xxx -> 底层库常见写法, 归系统底层用.
      $xxx -> 也表示私有, 同 _xxx 可应用于宏的局部变量中. 再或者全局变量声明 extern int g_cnt;
      通过 g_ 开头更加容易区分. 有一条准则, 哪怕用 goto也别用全局变量, 因为谁也不喜欢赤裸裸的开挂.
    c. [static 64]是 C99新加的语法, 告诉编译器这个"数组"至少有64个单元, 你可以放心优化.
    d. static int _cnt; 表示声明一个私有的全局变量 _cnt, 属于 _heoo函数域的. 生存周期是全局的.
    e. 另一个细节是, 生命周期是全局的变量, 默认内存初始化都是以0填充的. = 0 可以不写.

> 这里科普一点 C中 0, 00, 0x0, .0, '\0', '0', "0", NULL, false, EXIT_SUCCESS 是什么鬼!  

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
    对于C++部分, 争取不讲, 主要围绕C生态部分. 从上可以看出 NULL, false, EXIT_SUCCESS 都是宏.
    NULL 是个特殊的空指针 (void *)0, 后两个完全一样. 有时候函数退出使用 exit(EXIT_SUCCESS) 或者
    返回 return EXIT_SUCCESS 是个标准装波写法. 也是本书推荐的 (年少不装一波, 以后还能再雄起吗)
    前面几种 0, 00, 0x0, .0, '\0' 都是零, 区别在于 00是八进制, 0x是十六进制, '\0' 是字符常量的零.
    0值都是一样的, 写者想表达出来的语义不同. .0是 double双精度的0, 对于 "0" 是个字符串常量,  
    等同于 char [2] { '0', '\0' } 这里也可以看出来 '0' 和 '\0' 不同字符常量 '0' = (int)48;
    希望大家以后再遇到这些, 能了然于胸, 乘云破雾. 关于函数声明部分总的基调如下:
      a ). 给其它模块用的函数, 推荐 extern声明, 然后定义其实体
      b ). 只有自己模块用的, 推荐 static声明加定义
      c ). static 命令推荐 _xxx 开头, 方便一目了然
    
> 分享Over, 再赠送个打桩的小技巧. 在协同开发中. 先快速写好接口, 给对接方. 随后来个空实现.

#### 1.4 C中三大战神 - 指针 - 达姿·奥利哈刚
      C中一个很有争议的命题就是一切皆内存. 而指针就是内存的魔法杖. C is free and unsafe. 强调
**程序员是万能的**

    隐含的一层意思是你的程序你负责. 而这一切都因指针而起, 它无所不能在当前的系统架构的世界里.
    本小节里我们只表述华山剑法中C指针的写法. 首先看下面小段演示Demo.  
```C
// 第一种传统写法, * 靠右
void *piyo, *hoge;

// 函数定义
void *
free(void *block) {
  ...
}

// 第二种, * 靠左, 模仿上层语言的类型
void* piyo, *hoge;

void*
free(void* block) {
  ...
}
```
    发现上面两种写法很不自然, 由于C本身缺陷就不少, 上面两种写法都不完美. 第一种, 函数定义的时候 * 号
    就不知道改放在那里了. 这种用法比较广, 但也不是最好的选择. 第二种写法, 多数是有过面向对象的编程经验,
    重温一下C. 这种写法在定义多个变量指针的时候基本就废了.而且本身是变量指针声明, 却被强加指针类型. 
    这里采用下面写法, 追求自然和美脚本不要停歇!
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
    关于指针的范式基调就是如上. 简单扯一点关于C中变量声明. C一共才44个关键字, 推荐命名要短小精干.
    强烈不推荐, 驼峰规则, 因为在远古时期C是不区分大小写的. 而且C代码不好读懂, 推荐遵从内核源码古法. 
    命名语义区分用 _ 分隔. C也许不能让你工资高于同行, 但是可以让你在面对大佬的时候看见希望~

#### 1.5 C中三大战神 - 宏 - 封神记·天
      宏有点疯狂和好用. 但切记能用 inline内联, 就不要用宏! 如果说指针是自由, 那宏就是噩梦. 我们从
    struct.h 来认识一下这个偏执狂 ->
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
// flag_e - 函数返回值全局状态码
// >= 0 标识 Success状态, < 0 标识 Error状态
//
typedef enum {

	Error_Tout    = -5,	//超时错误
	Error_Fd      = -4,	//文件打开失败
	Error_Alloc   = -3,	//内存分配错误
	Error_Param   = -2,	//调用的参数错误
	Error_Base    = -1,	//错误基类型, 所有错误都可用它, 在不清楚的情况下

	Success_Base  = +0,	//结果正确的返回宏
	Success_Close = +1,	//文件描述符读取关闭, 读取完毕也会返回这个
	Success_Exist = +2,	//希望存在,设置之前已经存在了.

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
      这里我们主要讲解一下宏的命名准则. 以下对于宏态度的演示, 多感受其中范式
* 克制使用, 要用就要表述最清晰
```C
#define _H_SIMPLEC_SCHEAD
#define _STR_LOGDIR       "logs"
#define _UINT_LOG         (2048u)
#define _INT_MQ           (1 << 6)
#define _FLOAT_ZERO       (0.000001f)	
```
    上面是常量的宏命名流式, 统一大写. _ 开头表达'私有', 慎重使用. STR, H, UINT ...一眼看过去就知道
    其宏常量的类型. 第一个是头文件宏, 采用 _H_[项目名称]_[文件名称] 的命名规范, 防止重复. 还有
    字符串常量方便宏拼接, 不用加(). 数值常量防止意外拼接一定要加括号. 当你不清楚什么意外的时候, 
    加括号解决方案是最保险的. 
* 函数宏另起一行写, 局部使用宏直接在一行写
```C
// 置空操作, v必须是个变量
#define BZERO(v) \
	memset(&(v), 0, sizeof(v))

 // 添加双引号的宏 
#define CSTR(a)	_STR(a)
#define _STR(a) #a
```
    例如上面, BZERO(v) 可以独立存在, 另起一行加上 \. 对于 _CSTR(a)可以作为一个子语句, 直接一行写.
* 有些宏, 想被人认为是函数, 那就用写法
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

#include <Windows.h>
#define sh_msleep(m) \
    Sleep(m)	

#else
	#error "error : Currently only supports the Visual Studio and GCC!"
#endif    
```
    写的人意图希望 sh_sleep是个'函数', 使用了小写. 对于两个内置宏,  __GNUC__ 是 GCC 编译器自带的,
    表示当前是使用 GCC 来编译项目. _MSC_VER 表示使用 M$ 的 CL来编译项目. 做为开发人员, 推荐用最新
    的编译器. 因为活力和新鲜感很有趣, 一次新的尝试说不定就是一次机遇.
> 命名规范简单总结  

    a ). 变量命名, 函数命名
      -> 小写, 语义分隔使用 _ ; 
      -> 全局的使用 g_ 开头(强烈不建议使用全局量, 太污了);
      -> static 声明的, 开头加 _; 
      -> 宏中局部变量使用 $ 开头, 防止变量污染.
```C
// 简单的 time时间记录宏
#ifndef TIME_PRINT
#define _STR_TIME_PRINT "The current code block running time:%lf seconds\n"
#define TIME_PRINT(code) \
  do {\
    clock_t $st, $et;\
    $st = clock();\
    code\
    $et = clock();\
    printf(_STR_TIME_PRINT, (0.0 + $et - $st) / CLOCKS_PER_SEC);\
  } while(0)
#endif // !TIME_PRINT
```
    b ). 宏命名
      -> 大写, 语义分隔使用 _ ; 
      -> 宏常量采用 _[类型简写]_[名称]
      -> 文件头宏采用 _H_[项目名称]_[文件名称]
      -> 常用函数宏大写, 不希望常用函数宏加上前缀 _
      -> 跨平台相关工作, 且希望被常用, 可以用小写命名

    c ). 枚举声明, 类型声明
      C枚举同INT宏常量很同质化, 默认类型 int. 有时候很显得多余, 请用理会下面模式去仿照
```C
#ifndef _ENUM_FLAG

typedef enum {
  ...
	Success_Base	= +0,	//结果正确的返回宏
	Success_Close	= +1,	//文件描述符读取关闭, 读取完毕也会返回这个
	Success_Exist	= +2,	//希望存在,设置之前已经存在了.
  ...
} flag_e;

typedef flag_e	(* each_f )(void * node, void * arg);

// 定义的字符串类型
typedef struct tstr * tstr_t;

#define _ENUM_FLAG
#endif // !_ENUM_FLAG
```
    -> 类型声明 xxxx_[类型缩写], 很直观用于区分. 
    -> 为了区分枚举和INT宏常量, 枚举采用首字母大写, _ 分隔方式.

> 到这里基本把C的华山流派总纲讲的多多少少  
按照流派(定式)去写, 你会有更多的时间去学心法, 然后再出来构建你自己的独孤九剑.  
同是不年轻的穷屌丝, 要学会适应, 不流血那就流水. 孙子古法还是博大精深.  

#### 1.6 修炼剑法自然需要神兵

      万般皆自然, 一通百顺, 但不好真的通了ヾ(๑╹◡╹)ﾉ". 这里的神兵指的是你的编程环境. 
    硬件方面要是可以的话买最快, 最美, 最便宜的 computer套装. 软件方面, 分下面流派描述

**winds 武当流派**  

    a ). Install Best New Windows Profession ISO 
    b ). Install Best New Visual Studio Enterprise
    c ). Install Best New Visual Studio Code

    学习的时候推荐用最新的环境, 这样躺坑会多, 提升解决新问题的能力, 同样多数新的代表着'改进'. 
    这里我们多说一点, 没有一款IDE比VS猛. 那怕C++被M$打压了十多年, VC++仍然宇宙中最屌的编辑器
    (笔者此刻用的是 VS2017). winds的缺陷在于封闭和api的混乱. 导致后期提升艰难.  阻挡了开发
    人员的持续投入的兴趣. 同样其无敌的IDE, 为其引来无数上层商业应用. 总而言之 winds 比 linux
    难, 同级别工资比 linux工资低. 服务器开发领域就是是个鸡肋(虽强但不通用, 所以不用)! 

**linux 昆仑流派**  

    本人是在虚拟机上跑的, 推荐安装环境如下
    a). Install Best New VMware Workstation Pro
    b). Install Best New Ubuntu Desktop ISO
    c). Install Best New Xshell
    d). Install Best New GCC, GDB, VIM

    linux 真不错, 简单高效优美. vi gcc make 能够搞定一切. 无数的一手资料, 强大的 man手册.
    题外话 APPLE 的系统作为程序开发更加带感. 毕竟是 unix内核, 外观好看. 会 linux平移很轻松.
    就是作为盗版游戏机难点, 还有其作为服务器性能还是很低的. 有条件同学可以首选.

**你的剑就在前方**  

    市面上多数服务器书籍是 GCC弄的, 没啥意思, 同质化严重. 本文主打 Visual Studio CL 搞起. 
    先将 GCC的一些命令翻译到 VS 上面, 其它的用到时自行 Serach.  
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

    个人感悟, 跨平台是在浪费生命. 有些人单纯的用linux, 顺带配上 apple. 感觉很好, 毕竟越纯粹
    越强. linux是个回报率很高的平台. 其实想说, 岁月不尽然呀, 学了C/C++ 真是天高地厚不知路长~ 

#### 1.7 夜太黑正是练剑的好时光
      上面聊的有些多, 很多细节需要自己亲身建构. 回想起13年看<<自制编程语言>>那本书, 惊为天人.
    感觉作者实力好强. 因为看不明白, 强行撸, 狂看最好懂了点, 收益良多. 在编程的世界里, 不需要啥,
    只要wo are正在舞剑! 
      如果咱们一样只是为了, 更好的活下去. 那么学起来就更随意了, 君子当善假于物! 熟悉工具, 帮人
    实现梦想, 就成了~ 然后回家开个水果店挺好的.
    如果你没有对象, 那就使劲打代码
    如果你实在无聊, 那就跑步加看书
    如果你还不甘心, 那就呵呵闷闷哒
    最后推荐一首喜欢的诗人的歌
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