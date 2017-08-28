## 第4章-武技-常见轮子下三路

    这篇是关于系统中常见轮子的介绍, 是框架构建中最基础的组件. 是你参与战斗的第一层保障.
    本章定位是筑基期的顶阶功法, 融于了那些再妖魔大战中前辈们无数英魄构建的思想. 有章有
    法筑基功效厚积薄发, 此本武极能让你在战场上无往不利~

### 4.1 那些年写过的日志

    见过太多日志库, 也写过不少. 见证过漫天飞花, 也见过一个个地狱火撕裂天空, 最后选择了
    了50行的小小日志代码, 来表达所要的一切美好~ 好的代码理应越简单越优美越让人懂.
    一个高性能的日志库突破点无外乎
        1. 高效缓存机制
        2. 业务层无锁
        3. 生成业务和写入业务分离
    随后这个日志武技轮子, 会深入分析

#### 4.1.1 小小日志库, 详细设计

    先看接口设计部分

clog.h

```C
#ifndef _H_SIMPLEC_CLOG
#define _H_SIMPLEC_CLOG

#include "sctime.h"

//
// error info debug printf log  
//
#define CL_ERROR(fmt,	...)	CL_PRINTF("[ERROR]",	fmt, ##__VA_ARGS__)
#define CL_INFOS(fmt,	...)	CL_PRINTF("[INFOS]",	fmt, ##__VA_ARGS__)
#if defined(_DEBUG)
#define CL_DEBUG(fmt,	...)	CL_PRINTF("[DEBUG]",	fmt, ##__VA_ARGS__)
#else
#define CL_DEBUG(fmt,	...)	/*  (^_−)☆ */
#endif

//
// CLOG_PRINTF - 拼接构建输出的格式串,最后输出数据
// fstr		: 日志标识宏
// fmt		: 自己要打印的串,必须是双引号包裹. 
// return	: 返回待输出的串详细内容
//
#define CL_PRINTF(fstr, fmt, ...) \
	cl_printf(fstr "[%s:%s:%d]" fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

//
// cl_start - !单例! 开启单机日志库
// path		: 初始化日志系统文件名
// return	: void
//
extern void cl_start(const char * path);

//
// cl_printf - 具体输出日志内容
// fmt		: 必须双引号包裹起来的串
// ...		: 对映fmt参数
// return	: void
//
void cl_printf(const char * fmt, ...);

#endif // !_H_SIMPLEC_CLOG
```

    继承自 sctime.h, 我们唯一使用的就是 sctime.h 接口中的

```C
//
// stu_getmstrn - 得到毫秒的串, 每个中间分隔符都是fmt[idx]
// buf		: 保存最终结果的串
// len		: 当前buf串长度
// fmt		: 输出格式串例如 -> "simplec-%04d%02d%02d-%02d%02d%02d-%03ld.log"
// return	: 返回当前串长度
//
size_t 
stu_getmstrn(char buf[], size_t len, const char * const fmt) {
	time_t t;
	struct tm st;
	struct timespec tv;

	timespec_get(&tv, TIME_UTC);
	t = tv.tv_sec;
	localtime_r(&t, &st);
	return snprintf(buf, len, fmt,
					st.tm_year + _INT_YEAROFFSET, st.tm_mon + _INT_MONOFFSET, st.tm_mday,
					st.tm_hour, st.tm_min, st.tm_sec,
					tv.tv_nsec / _INT_MSTONS);
}
```

    得到带毫秒的时间串. 随后核心设计如下

```C
#include "clog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//
// 急速, 清洁, 可靠配合 logrorate的 c多线程单机日志库 clog.h
//					by simplec wz 2017年4月26日
//

static FILE * _log;

inline void 
cl_start(const char * path) {
	if (NULL == _log) {
		_log = fopen(path, "ab");
		if (NULL == _log) {
			fprintf(stderr, "fopen ab err path = %s!\n", path);
			exit(EXIT_FAILURE);
		}
	}
}

void 
cl_printf(const char * fmt, ...) {
	va_list ap;
	size_t len;
	// 每条日志的大小, 唯一值
	char str[2048];

	// 串:得到时间串并返回长度 [2016-07-10 22:38:34 999]
	len = stu_getmstrn(str, sizeof(str), "[" _STR_MTIME "]");

	// 开始数据填充
	va_start(ap, fmt);
	vsnprintf(str + len, sizeof(str) - len, fmt, ap);
	va_end(ap);
	
	// 下数据到文本中
	fputs(str, _log);
}
```

    是不是很恐怖, 一个日志库完了. fputs 是系统内部打印函数, 默认自带缓冲机制. 缓冲是什么
    说白了就是批量操作, 存在非及时性. vsnprintf 属于 printf 函数簇自带文件锁. 有兴趣的朋
    可以详细研究下 printf, C 基础入门最早的学的函数, 也是最复杂的函数之一. 那目前就差生成
    业务了! 这也是小小日志库的另一个高明之处 

### 4.1.2 小小日志库, VT 二连

    这里我们开始构建测试环境了. 模拟一个妖魔大战的场景~ 嗖 ~ 切换到 linux 平台

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _TXT_PATH   "simplec.log"

// 
// logrotate hello world
//
int main(int argc, char * argv[]) {
    int id = 0;
    FILE * log = fopen(_TXT_PATH, "ab");
    if (NULL == log) {
        fputs("fopen ab err path = " _TXT_PATH  "!\n", stderr);
        exit(EXIT_FAILURE);
    }   
    
    // Ctrl + C 中断结束
    for (;;) {
        printf(_TXT_PATH " id = %d\n", id);
        fprintf(log, _TXT_PATH " id = %d\n", id);
        fflush(log);
        ++id;
        sleep(1);
    }   

    
    fclose(log);
    return EXIT_SUCCESS;    
}
```

    顺带给个编译文件 Makefile

```C
.PHONY : all clean

all : simplec.exe

clean :
    -rm -rf *~
    -rm -rf simplec.exe
    -rm -rf simplec.log simplec.log.* simplec.log-*

simplec.exe : simplec.c
    gcc -g -Wall -O2 -o $@ $^
```

    这时候 make -> ./simplec.exe 开始持续输出日志了. 试炼场的环境已经搭建好了, 那么是
    时候 logrotate  出来了. 当前测试机是 centos, 那就以它举例, 看图:

![logrotate](./img/logrotate.png)

    通过上面安装好, 那就开始使用了. 推荐自己查相关手册, 我这里只是简单的 Demo. Ok开始那
    搞起了, 看下面所做的操作:

```Bash
su root

cd /etc/logrotate.d
vi simplec

#
# 添加 logrotate 规则
# daily : 日志文件将按天轮循
# rotate 7 : 存档7次, 时间最久的删除
# dateext : 日志添加日期后缀名
# copytruncate : 复制截断, (懒得写 SIGHUP 信号处理解决方案)
# create 644 root root : 截断日志文件权限
#
# size = 100 : 测试用的, 超过100B就生成日志备份文件, 单位 K, M 默认B
#

i
/home/ceshi/wangzhi/logrote/simplec.log {
        daily
        rotate 7
        dateext
        copytruncate
        create 644 root root
        size = 100
}

:wq!

logrotate -vf /etc/logrotate.d/simplec

```

    最终所要的结果如下

![logrotate log](./img/createlog.png)

    别人只会给你一条路, 随后就是自己的双脚, 如果没有那么是时候 -> 冲冲冲, 四驱兄弟在心中.
    当然了小小VT二连之后, 可以再A一下. 那就利用自带的定时器了, cron 等等. 
    以后的事情那就留给以后说吧~
    当然我教了最精简的日志库实战架构, 也是最优质的那一批. 对于普通选手, 还有的目的, 如何
    和人吹 NB呢. 因而这里我们再来分析一下其它几种日志库的套路, 知彼知己才能又说又吃~

* 多用户日志库  

    这类日志库在上层语言构建的业务框架中有所出现. 最大特点就是每条日志有个唯一标识码. 通
    过这个唯一标识码能够分析出一条请求所经过的所有流程, 时间消耗等. 实现的核心思路: 消息
    来到网关服务器的时候生成一个日志id, 保存在线程的私有变量中, 随后发给其它业务服务器.
    一套优秀的多用户日志库, 单纯通过日志就能分析玩家所进行的一切操作. 当然后备还有一系列
    自动化运维的脚本支持. 总结就是业务能力最强, 性能最弱~

* 异步消息队列日志库  

    这类日志库在游戏服务器中及其常见, 开个线程跑个日志消息队列. 它解决的问题是, 例如端游
    中大量日志打印, 运维备份的时候, 将业务机卡了. 所以消息队列就出来了, 作为缓存. 这类日
    志库可以秀一下代码功底吧, 毕竟线程轮询, 消息队列, 资源竞争, 对象池, 日志构建所有的业
    务都需要. 个人角度而言很俗, 屠龙刀太重, 哪有摘叶伤人来的快速呀. 它的缓冲层消息队列,
    还不一定比不进行 fflush 的系统层面输出接口来的直接. 而且启动了个单独线程处理日志, 线
    程其实很珍贵的能不起就不起. 万剑固然厉害~ 但我们没钱呀~

    到这里我们扯的差不多, 如果以后你和人交流的时候别人问你这个日志库为什么高效. 记住
        1. 无锁编程, 利用 fprintf IO锁
        2. fputs 最大限度的利用系统 IO缓冲层, 没必要 fflush, 从消息队列角度分析
        3. 万剑一, 小小日志库只负责写 IO, 其它交给系统层面最屌的工具搞. 专业
        ... 

### 4.2 提前来个阅读理解, 拔个随机数库

    为什么来个随机数库呢, 因为不同平台的随机数实现不一样, 导致期望结果不一样. 顺带给系统
    函数提提速度. 随机函数对于计算机行业真了不得, 奠定了人类模拟未知的一种可能. 顺带扯一点
    概率分析学上一种异常神奇的事情: "概率为0的事情, 也可能发生~". 数学有点有意思哈. 数学
    的本源不是为了解决具体遇到问题, 多数是人内部思维的升华 -> 自己爽就行了, 就同这个时代最
    伟大的数学家俄罗斯的[格里戈里·佩雷尔曼]渡劫真君, 爽了一发就影响来了整个人类思维的地震. 

    以下提供的随机算法, 是从 redis源码上拔下来的, redis是从 pysam源码上拔下来. 大家可以
    看见这就是薪火相传呀~ 首先看接口设计 

scrand.h

```C
#ifndef _H_SIMPLEC_SCRAND
#define _H_SIMPLEC_SCRAND

#include <stdint.h>

//
// sh_srand - 初始化随机数种子, (int32_t)time(NULL)
// seed		: 种子数
// return	: void
//
extern void sh_srand(int32_t seed);

//
// sh_rand  - 得到[0, INT32_MAX]随机数
// sh_rands - 得到[min, max] 范围内随机数
// sh_randk - 得到一个64位的key
//
extern int32_t sh_rand(void);
extern int32_t sh_rands(int32_t min, int32_t max);
extern int64_t sh_randk(void);

#endif//_H_SIMPLEC_SCRAND
```

    最核心是 sh_rand 阅读理解来了, 感受下计算数学魅力

scrand.c

```C
#include <scrand.h>
#include <assert.h>

#define N				(16)
#define MASK			((1 << N) - 1)
#define LOW(x)			((x) & MASK)
#define HIGH(x)			LOW((x) >> N)
#define CARRY(x, y)		((x + y) > MASK) // 二者相加是否进位, 基于16位
#define ADDEQU(x, y, z)	z = CARRY(x, y); x = LOW(x + y)

#define MUL(x, y, z)	l = (x) * (y); (z)[0] = LOW(l); (z)[1] = HIGH(l)

#define X0				(0x330E)
#define X1				(0xABCD)
#define X2				(0x1234)
#define A0				(0xE66D)
#define A1				(0xDEEC)
#define A2				(0x0005)
#define C				(0x000B)

static uint32_t _x[] = { X0, X1, X2 }, _a[] = { A0, A1, A2 }, _c = C;

static void _sh_next(void) {
	uint32_t p[2], q[2], r[2], c0, c1, l;

	MUL(_a[0], _x[0], p);
	ADDEQU(p[0], _c, c0);
	ADDEQU(p[1], c0, c1);
	MUL(_a[0], _x[1], q);
	ADDEQU(p[1], q[0], c0);
	MUL(_a[1], _x[0], r);
	
	l = c0 + c1 + CARRY(p[1], r[0]) + q[1] + r[1]
		+ _a[0] * _x[2] + _a[1] * _x[1] + _a[2] * _x[0];
	_x[2] = LOW(l);
	_x[1] = LOW(p[1] + r[0]);
	_x[0] = LOW(p[0]);
}

//
// sh_srand - 初始化随机数种子, (int32_t)time(NULL)
// seed		: 种子数
// return	: void
//
inline void 
sh_srand(int32_t seed) {
	_x[0] = X0; _x[1] = LOW(seed); _x[2] = HIGH(seed);
	_a[0] = A0; _a[1] = A1; _a[2] = A2;
	_c = C;
}

//
// sh_rand  - 得到[0, INT32_MAX]随机数
// sh_rands - 得到[min, max] 范围内随机数
// sh_randk - 得到一个64位的key
//
inline int32_t 
sh_rand(void) {
	_sh_next();
	return (_x[2] << (N - 1)) + (_x[1] >> 1);
}

inline int32_t 
sh_rands(int32_t min, int32_t max) {
	assert(max > min);
	return sh_rand() % (max - min + 1) + min;
}

inline int64_t 
sh_randk(void) {
	uint64_t x = ((sh_rand() << N) ^ sh_rand()) & INT32_MAX;
	uint64_t y = ((sh_rand() << N) ^ sh_rand()) & INT32_MAX;
	return ((x << 2 * N) | y) & INT64_MAX;
}
```

    代码都懂, 也就 _sh_next计算复杂点. 就看自己悟了, 毕竟世界也是咱们的. sh_rands, 
    sh_randk 思路很浅显了分别根据范围和位随机. 从上面也可以看出来 随机函数并不是线程
    安全的. 在多线程环境中就会出现未知行为了(至少咱们不清楚). 这样也很有意思, 毕竟不
    可控的随机才有点随机吗. 
    
    不怕折腾可以把上面代码直接刷到你的项目中, 解决平台无关性~
    目前 winds 和 linux 测试结果如下:

```C
/*
 describe:
	1亿的数据量, 测试随机生成函数
	front system rand, back sh_rand rand
 
 test code

 // 1亿的数据测试
 #define _INT_TEST	(100000000)
 
 static int _test_rand(int (* trand)(void)) {
 	 int rd = 0;
 	 for (int i = 0; i < _INT_TEST; ++i)
 	 	rd = trand();
 	 return rd;
 }

 winds test :
	cl version 14 Visual Studio 2015 旗舰版(Window 10 专业版)

	Debug
	The current code block running time:1.743000 seconds
	The current code block running time:4.408000 seconds

	Release
	The current code block running time:1.649000 seconds
	The current code block running time:0.753000 seconds

 linux test : 
	gcc version 6.3.0 20170406 (Ubuntu 6.3.0-12ubuntu2)
	-g -O2
	The current code block running time:0.775054 seconds
	The current code block running time:0.671887 seconds
 */
``` 

    到这里基本前戏做的够足了, 奥特曼要出现了~

### 4.3 奥特曼出现, 我们基础一个共有头文件

    大家看很多开源项目, 都会有个出现频率特别高的一个头文件, 项目中基本每个头文件都继承自
    它. 同样此刻出现的是筑基期至强奥义, 一切从头开始
    
schead.h

```C
#ifndef _H_SIMPLEC_SCHEAD
#define _H_SIMPLEC_SCHEAD

#include <clog.h>
#include <scrand.h>
#include <struct.h>
#include <pthread.h>

//
//  宏就是C的金字塔最底层, 所有丑陋的起源~
//  [ __clang__ -> clang | __GNUC__ -> gcc | __MSC_VER -> cl ]
//
#ifdef __GNUC__

#include <termio.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * 屏幕清除宏, 依赖系统脚本
 *  return	: void
 */
#define sh_cls() \
		printf("\ec")

//
// getch - 立即得到用户输入的一个字符, linux实现
// return	: 返回得到字符
//
extern int getch(void);

//
// sh_mkdir - 通用的单层目录创建宏 等同于 shell> mkdir path
// path		: 目录路径加名称
// return	: 0表示成功, -1表示失败, 失败原因都在 errno
// 
#define sh_mkdir(path) \
	mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

#elif _MSC_VER

#include <direct.h> 
#include <conio.h>

#define sh_cls() \
		system("cls")

#define sh_mkdir(path) \
	mkdir(path)

#else
#	error "error : Currently only supports the Best New CL and GCC!"
#endif

// 添加双引号的宏 
#define _STR(v) #v
#define CSTR(v)	_STR(v)

// 获取数组长度,只能是数组类型或""字符串常量,后者包含'\0'
#define LEN(a) (sizeof(a) / sizeof(*(a)))

// 置空操作, v必须是个变量
#define BZERO(v) \
	memset(&(v), 0, sizeof(v))

/*
 * 比较两个结构体栈上内容是否相等,相等返回true,不等返回false
 * a	: 第一个结构体值
 * b	: 第二个结构体值
 *		: 相等返回true, 否则false
 */
#define STRUCTCMP(a, b) (!memcmp(&a, &b, sizeof(a)))

//
// EXTERN_RUN - 简单的声明, 并立即使用的宏
// test		: 需要执行的函数名称
//
#define EXTERN_RUN(test, ...) \
	do { \
		extern void test(); \
		test (__VA_ARGS__); \
	} while(0)

// 简单的time时间记录宏
#define TIME_PRINT(code) \
	do { \
		clock_t $s, $e; \
		$s = clock(); \
		code \
		$e = clock(); \
		printf("Now code run time:%lfs.\n", ((double)$e - $s) / CLOCKS_PER_SEC); \
	} while (0)

//
// sh_pause - 等待的宏 是个单线程没有加锁 | "请按任意键继续. . ."
// return	: void
//
extern void sh_pause(void);

#ifndef SH_PAUSE

#	ifdef _DEBUG
#		define SH_PAUSE() atexit(sh_pause)
#	else
#		define SH_PAUSE() /* 别说了, 都重新开始吧 */
#	endif

#endif // !INIT_PAUSE

//
// sh_isbe - 判断是大端序还是小端序,大端序返回true
// sh_hton - 将本地四字节数据转成'小端'网络字节
// sh_ntoh - 将'小端'网络四字节数值转成本地数值
//
extern bool sh_isbe(void);
extern uint32_t sh_hton(uint32_t x);
extern uint32_t sh_ntoh(uint32_t x);

//
// async_run - 开启一个自销毁的线程 运行 run
// run		: 运行的主体
// arg		: run的参数
// return	: >= SufBase 表示成功
//
extern int async_run_(node_f run, void * arg);
#define async_run(run, arg) \
		async_run_((node_f)(run), arg)

#endif//_H_SIMPLEC_SCHEAD
```

    这里可以看出来, 头文件包含也是继承关系. 基本每一个包含了 schead.h 的文件, 都不用再
    包含其它的辅助文件, 开发起来会很方便. 额外说一下 getch 这个函数, 

```C
#if defined(__GNUC__)

inline int 
getch(void) {
	int cr;
	struct termios nts, ots;
	if (tcgetattr(0, &ots) < 0) // 得到当前终端(0表示标准输入)的设置
		return EOF;

	nts = ots;
	cfmakeraw(&nts); // 设置终端为Raw原始模式，该模式下所有的输入数据以字节为单位被处理
	if (tcsetattr(0, TCSANOW, &nts) < 0) // 设置上更改之后的设置
		return EOF;

	cr = getchar();
	if (tcsetattr(0, TCSANOW, &ots) < 0) // 设置还原成老的模式
		return EOF;
	return cr;
}

#endif
```

    很久以前一位化神期巨擘说过:由于 linux对于 getch支持的不友好, 导致了 linux错失了很多游
    戏开发人员. 我是挺喜欢 getch的, 写个小游戏太轻松了. 随后辅助函数:

```C
inline void 
sh_pause(void) {
	fflush(stderr);
	fflush(stdout);
	rewind(stdin);
	printf("Press any key to continue . . .");
	getch();
}

inline bool 
sh_isbe(void) {
	static union { uint16_t i; uint8_t c; } _u = { 1 };
	return 0 == _u.c;
}

inline uint32_t 
sh_hton(uint32_t x) {
	if (sh_isbe()) {
		uint8_t t;
		union { uint32_t i; uint8_t s[sizeof(uint32_t)]; } u = { x };
		t = u.s[0], u.s[0] = u.s[sizeof(u) - 1], u.s[sizeof(u) - 1] = t;
		t = u.s[1], u.s[1] = u.s[sizeof(u) - 1 - 1], u.s[sizeof(u) - 1 - 1] = t;
		return u.i;
	}
	return x;
}

inline uint32_t 
sh_ntoh(uint32_t x) {
	return sh_hton(x);
}
```

    这里说一下 sh_isbe 判断当前系统是否是大端系统, 是返回 true. 而自己多数接触的是小端机器.
    所以自己设计了个 sh_hton 如果是大端的结构会转成小端结构. sh_pause 相似功能在 winds上面
    是 system("pause"), 在 linux 是 pause(). 用起来不爽, 搞了个 SH_PAUSE(). 
    schead.h 接口文件定位目标就是所有业务层面的辅助头文件. 美好从此刻开始~
    
    新的风暴已经出现 怎么能够停滞不前 穿越时空竭尽全力 我会来到你身边

### 4.4 不妨用 c 写个 json 解释器

    在我刚做开发的时候, 那时候是维护系统, 所有配置走的是 xml和 csv类型. json在国内刚兴起, 
    所以自己一时兴起写了一个解释器. 过了1年接触到 cJSON库, 直接把自己当初写的那个删了. 用
    起了 cJSON, 后面觉得 cJSON 真的丑的不行不行, 写了个简单的 scjson. 
    这小节, 我来带大家写写这个 scjson 的解析引擎, 清洁高效, 小. 能够保证的就是比cJSON好.

    这里开始大概分析 scjson 的实现部分. 首先最关心的是 scjson的内存布局, 这里引入了 tstr
    设计结构图如下 :

![scjson内存布局](./img/scjson内存布局.png)

    str指向内存常变, tstr指向内存不怎么变. 所以采用两块内存保存. tstr 其实是个中转站. 因为
    读取到的文件内容, 或者中间 json 内容去注释, 压缩都需要一块内存. 这就是引入 tstr目的.

    我们开始从 scjson 结构设计上分析

```C
struct cjson {
	struct cjson * next;	// 采用链表结构处理, 放弃二叉树结构, 优化内存
	struct cjson * child;	// type == ( CJSON_ARRAY or CJSON_OBJECT ) 那么 child 就不为空

	unsigned char type;		// 数据类型 CJSON_XXXX, 一个美好的意愿
	char * key;				// json内容那块的 key名称 	
	union {
		char * vs;			// type == CJSON_STRING, 是一个字符串 	
		double vd;			// type == CJSON_NUMBER, 是一个num值, ((int)c->vd) 转成int 或 bool
	};
};

//定义cjson_t json类型
typedef struct cjson * cjson_t;
```

    使用了 c99的匿名结构体挺爽的, 详细点整个内存布局如下:

![scjson内存结构](./img/scjson内存结构.png)

    详细一点代码类型就是

```C
// json中几种数据结构和方式定义, 对于程序开发而言最难的还是理解思路(思想 or 业务)
#define CJSON_NULL		(0u << 0)
#define CJSON_FALSE		(1u << 0)
#define CJSON_TRUE		(1u << 1)
#define CJSON_NUMBER	(1u << 2)
#define CJSON_STRING	(1u << 3)
#define CJSON_ARRAY		(1u << 4)
#define CJSON_OBJECT	(1u << 5)

//
// cjson_getint - 这个宏, 协助我们得到 int 值
// item		: 待处理的目标cjson_t结点
// return	: int
//
#define cjson_getvi(item) ((int)((item)->vd))
```

    上面就是解析之后的具体结构类型. 下面简单分析一下文本解析规则.

![scjson递归下降分析](./img/scjson递归下降分析.png)

    主要思路就是递归下降分析. 到这里基本关于scjson详细设计图介绍完毕了.
    后面会看见这只麻雀嗲吗极少 ヽ(✿ﾟ▽ﾟ)ノ

#### 4.4.1 scjson 详细设计

    当初写这类东西, 最好的方式就是对着协议文档开撸~
    这类代码是协议文档和作者思路的杂糅体, 推荐读者最好手敲一遍, 自行加注释, 琢磨就吸收了.
    



