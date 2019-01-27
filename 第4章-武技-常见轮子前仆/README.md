# 第4章-武技-常见轮子下前仆

    	本章是关于系统中常见轮子的介绍. 也是构建框架中最基础的组件. 可以说是咱们参与战斗
	的生命线. 当前定位是筑基期的顶阶武技, 融合了那些在妖魔大战中无数前辈们的英魄构建的套
	路. 最大程度的发挥筑基的实力, 一招飞龙在天, 同阶无敌. 此武技的宗旨就是让你成为战场上
	苟延残喘的小强 ┗|｀O′|┛ . 嗷那开始出招吧 ~

## 4.1 那些年写过的日志库

    	用过太多日志库轮子, 也写过不少. 见过漫天飞花, 也遇到过一个个地狱火撕裂天空, 最
	后展示了 50 行的极小的日志库, 来表达所要的一切美好 ~ 越简单越优美越让人懂的代码总会
	出彩, 不是吗? 一个高性能的日志库突破点无外乎
        1. 缓存
        2. 无锁
        3. 定位
    随后会对这个日志武技轮子, 深入剖析

### 4.1.1 日志库小小

    先看接口 clog.h 设计部分, 感受下三个宏解决一切幺蛾子.

```C
#ifndef _LOG_H
#define _LOG_H

#include "times.h"
#include <stdlib.h>
#include <stdarg.h>

//
// LOG_PRINTF - 拼接构建输出的格式串
// pre      : 日志前缀串必须 "" 包裹
// fmt      : 自己要打印的串, 必须 "" 包裹
// return   : void
//
#define LOG_PRINTF(pre, fmt, ...)   \
log_printf(pre"[%s:%s:%d]"fmt"\n", __FILE__, __func__, __LINE__, __VA_ARGS__)

//
// log 有些朴实, 也许很快很安全 ~
//
#define LOG_ERROR(fmt, ...) LOG_PRINTF("[ERROR]", fmt, __VA_ARGS__)
#define LOG_INFOS(fmt, ...) LOG_PRINTF("[INFOS]", fmt, __VA_ARGS__)
#ifdef _DEBUG
#define LOG_DEBUG(fmt, ...) LOG_PRINTF("[DEBUG]", fmt, __VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) /*  (^_−)☆ */
#endif

//
// log_printf - 具体输出的日志内容
// fmt      : 必须 "" 包裹的串
// ...      : 对映 fmt 参数
// return   : void
//
void log_printf(const char * fmt, ...);

#endif//_LOG_H
```

    clog.h 继承自 times.h 唯一使用的就是 times_fmt 接口得到特定时间格式串.

```C
// TIMES_STR - "{年}.{月}.{日}.{时}.{分}.{秒}.{毫秒}"
#define TIMES_STR "%04d-%02d-%02d %02d:%02d:%02d %03d"

//
// times_fmt - 通过 fmt 格式最终拼接一个字符串
// fmt          : 必须包含 %04d %02d %02d %02d %02d %02d %03d
// out          : 最终保存的内容
// sz           : buf 长度
// return       : 返回生成串长度
//
int
times_fmt(const char * fmt, char out[], size_t sz) {
    struct tm m;
    struct timespec s;

    timespec_get(&s, TIME_UTC);
    localtime_r(&s.tv_sec, &m);

    return snprintf(out, sz, fmt,
                    m.tm_year + 1900, m.tm_mon + 1, m.tm_mday,
                    m.tm_hour, m.tm_min, m.tm_sec,
                    (int)s.tv_nsec / 1000000);
}
```

    填充到日志的头部进行标识. 小小核心构造原理展开如下:

```C
#include "log.h"

static FILE * log;

//
// log_init - !单例! 日志库初始化
// path     : 初始化日志系统文件名
// return   : void
//
void log_init(const char * path) {
    if (!(log = fopen(path, "ab"))) {
        fprintf(stderr, "fopen ab path err %s\n", path);
        exit(EXIT_FAILURE);
    }
}

//
// log_printf - 具体输出的日志内容
// fmt      : 必须 "" 包裹的串
// ...      : 对映 fmt 参数
// return   : void
//
void 
log_printf(const char * fmt, ...) {
    va_list ap;
    // 每条日志大小, 按照系统缓冲区走
    char str[BUFSIZ];
    int len = times_fmt("["TIMES_STR"]", str, sizeof str);

    // 填入日志内容
    va_start(ap, fmt);
    vsnprintf(str + len, sizeof str - len, fmt, ap);
    va_end(ap);

    // 数据刷入文件缓存
    fputs(str, log);
}
```

	其中 log_init 可以通过 EXTERN_RUN 在 main 中初始化注册.

```C
//
// EXTERN_RUN - 简单的声明, 并立即使用的宏
// ftest    : 需要执行的函数名称
// ...      : 可变参数, 保留
//
#define EXTERN_RUN(ftest, ...)                                    \
do {                                                              \
    extern void ftest();                                          \
    ftest (__VA_ARGS__);                                          \
} while(0)

EXTERN_RUN(log_init, LOG_PATH_STR);
```

    是不是很恐怖, 一个日志库完了. fputs 是系统内部打印函数, 默认自带缓冲机制. 缓冲说白
    了就是批量处理, 存在非及时性. vsnprintf 属于 printf 函数簇自带文件锁. 有兴趣的可以
    详细研究 printf, C 入门最早的学的函数, 也是最复杂的函数之一. 那目前就差生成业务了!
    也就是第三点定位, 这也是 小小日志库的另一个高明之处, 借天罚来惩戒妖魔鬼怪. 

### 4.1.2 小小日志库 VT 二连

    先构建一下测试环境. 模拟一个妖魔大战的场景~ 嗖 ~ 切换到 linux 平台. 依次看下去

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH_STR   "simplec.log"

// 
// logrotate hello world
//
int main(int argc, char * argv[]) {
    FILE * log = fopen(PATH_STR, "ab");
    if (NULL == log) {
        fputs("fopen ab err path = "PATH_STR "!\n", stderr);
        exit(EXIT_FAILURE);
    }   
    
    // Ctrl + C 中断结束
    for (int id = 0; ; ++id) {
        printf(PATH_STR" id = %d\n", id);
        fprintf(log, PATH_STR" id = %d\n", id);
        fflush(log);
        sleep(1);
    }

    return fclose(log);
}
```

    顺带给个编译文件 Makefile

```C
.PHONY : all clean

all : simplec.exe

clean :
    -rm -rf *~
    -rm -rf simplec.exe
    -rm -rf simplec.log simplec.log-*

simplec.exe : simplec.c
    gcc -g -Wall -O2 -o $@ $^
```

    通过 make 得到 simplec.exe 运行起来, 就开始在日志文件中持续输出. 有关试炼场的环境
	已经搭建成功. 那么是时候主角 T logrotate 出场了. 很久前在 centos 测试构建过看图:

![logrotate](./img/logrotate.png)

    安装好 logrotate 和 crontabs 工具, 那么日志轮询器就能够开始使用了. 推荐自己查相关
	手册, 我这里只带大家简单弄个 Demo. Ok 开始搞起来, 看下面所做的 shell 批处理:

```Bash
su root

cd /etc/logrotate.d
vi simplec
i

#
# 添加 logrotate 规则
# daily : 日志文件将按天轮循
# rotate 7 : 存档7次, 时间最久的删除, LRU
# dateext : 日志添加日期后缀名
# copytruncate : 复制截断, (懒得写 SIGHUP 信号处理解决方案)
# create 644 root root : 创建的日志文件权限
#
# size = 100 : 测试用的, 超过 100B 就生成日志备份文件, 单位 K, M 默认 B
#
/home/ceshi/wangzhi/logrote/simplec.log {
        daily
        rotate 7
        dateext
        copytruncate
        create 644 root root
        size = 100
}

Esc
:wq!

logrotate -vf /etc/logrotate.d/simplec
```

	copytruncate 复制截断存在一个隐患是 logrotate 在 copy 后 truncate 时候会丢失那一
	瞬间新加的日志. 如果不想日志发生丢失, 可以自行实现, 最终取舍在于你对于业务的认识. 最
	终所搭建的环境:

![logrotate log](./img/createlog.png)

    如果你有幸遇到贵人, 也只会给你一条路, 随后就是自己双脚的主场. 如果没有那么是时候 -> 
	冲冲冲, 四驱兄弟在心中 ~ 当然了小小 VT 二连之后, 可以再 A 一下. 那就利用自带的定时
	器了, crontabs 等等以后的事情那就留给以后自己做吧 ~ 以上就是最精简的优质日志库实战
	架构. 对于普通选手可能难以吹 NB(说服别人), 因而这里会再来分析一波所见过日志库的套路
	, 知彼知己才能又吃又喝 ~ 日志库大体实现还存在一种套路, 开个线程跑日志消息队列. 这类
	日志库在游戏服务器中极其常见, 例如端游中大量日志打印, 运维备份的时候, 同步日志会将业
	务机卡死(日志无法写入, 玩家业务挂起). 所以构造出消息队列来缓存日志. 此类日志库可以秀
	一下代码功底, 毕竟线程轮询, 消息队列, 资源竞争, 对象池, 日志构建这些都需要有. 个人看
	法它很重. 哪有摘叶伤人来的迅捷呀. 其缓冲层消息队列, 还不一定比不进行 fflush 的系统层
	面输出接口来的直接. 而且启动一个单独线程处理日志, 那么就一定重度依赖对象池. 一环套一
	环, 收益普通 ~ 业务设计的时候能不用线程就别用. 因为线程脾气可大了, 还容易琢磨不透. 
	到这也扯的差不多了, 如果以后和人交流的时候, 被问到这个日志库为什么高效. 记住
		1. 无锁编程, 利用 fprintf IO 锁
		2. fputs 最大限度利用系统 IO 缓冲层, 没必要 fflush, 从消息队列角度分析
		3. 各司其职, 小小日志库只负责写, 其它交给系统层面最合适的工具搞. 定位单一

### 4.2 开胃点心, 高效随机数库

    	为什么来个随机数库呢? 因为不同平台的随机数实现不一样, 导致期望结果不一样. 顺便嫌
	弃系统 rand 函数不够快和安全. 随机函数算法诞生对于计算机行业的发展真不得了, 奠定了人
	类模拟未知的一种可能. 顺带扯一点概率分析学上一种神奇的事情是: "概率为 0 的事情, 也可
	能发生 ~". 还是有点呵呵. 数学的本源不是为了解决具体遇到问题, 多数是人内部思维的升华 
	-> 自己爽就好了. 就如同这个时代最强数学家俄罗斯[格里戈里·佩雷尔曼]渡劫真君, 嗨了一发
	就影响了整个人类思维的跳跃. 我们的随机函数算法是从 redis 源码上拔下来的, redis 是从 
	pysam 源码上拔下来. 也只能说是薪火相传, 生生不息. 哭 ~ 首先看接口设计

```C
#ifndef _RAND_H
#define _RAND_H

//
// 线程安全的 rand 库, by redis
//

#include <time.h>
#include <stdint.h>
#include <assert.h>

struct rand {
    uint32_t x[3];
    uint32_t a[3];
    uint32_t c;
};

typedef struct rand rand_t[1];

#define X0              (0x330E)
#define X1              (0xABCD)
#define X2              (0x1234)
#define A0              (0xE66D)
#define A1              (0xDEEC)
#define A2              (0x0005)
#define C               (0x000B)

#define N               (16)
#define MASK            ((1 << N) - 1)
#define LOW(x)          ((unsigned)(x) & MASK)
#define HIGH(x)         LOW((x) >> N)

//
// rand_init - 随机函数初始化种子方法
// r        : 随机函数对象
// seed     : 种子数
// return   : void
//
inline void rand_init(rand_t r, int64_t seed) {
    r->x[0] = X0; r->x[1] = LOW(seed); r->x[2] = HIGH(seed);
    r->a[0] = A0; r->a[1] = A1; r->a[2] = A2;
    r->c = C;
}

//
// rand_rand - 获取一个随机值
// r        : 随机函数对象
// return   : 返回 [0, INT32_MAX] 随机数
//
extern int32_t rand_rand(rand_t r);

//
// r_rand  - 得到 [0, INT32_MAX] 随机数
// r_randk - 得到一个 64 位的 key
// r_rands - 得到 [min, max] 范围内随机数
//
extern int32_t r_rand(void);

inline int64_t r_randk(void) {
    uint64_t x = ((r_rand() << N) ^ r_rand()) & INT32_MAX;
    uint64_t y = ((r_rand() << N) ^ r_rand()) & INT32_MAX;
    return ((x << 2 * N) | y) & INT64_MAX;
}

inline int32_t r_rands(int32_t min, int32_t max) {
    assert(max > min);
    return r_rand() % (max - min + 1) + min;
}

#endif//_RAND_H
```

    最核心是 rand_rand 函数实现, 阅读理解来了, 感受下离散数学的魅力

scrand.c

```C
#include "rand.h"

#define CARRY(x, y)     ((x + y) > MASK) // 基于 16位判断二者和是否进位
#define ADDRQ(x, y, z)  (z = CARRY(x, y), x = LOW(x + y))

#define MUL(l, x, y, z) l = (x) * (y); z[0] = LOW(l); z[1] = HIGH(l)

inline void rand_next(rand_t r) {
    uint32_t l, p[2], q[2], s[2], c[2];

    MUL(l, r->a[0], r->x[0], p);
    ADDRQ(p[0], r->c, c[0]);
    ADDRQ(p[1], c[0], c[1]);
    MUL(l, r->a[0], r->x[1], q);
    ADDRQ(p[1], q[0], c[0]);
    MUL(l, r->a[1], r->x[0], s);

    l = c[0] + c[1] + CARRY(p[1], s[0]) + q[1] + s[1] + 
        r->a[0] * r->x[2] + r->a[1] * r->x[1] + r->a[2] * r->x[0];
    r->x[2] = LOW(l);
    r->x[1] = LOW(p[1] + s[0]);
    r->x[0] = LOW(p[0]);
}

//
// rand_rand - 获取一个随机值
// r        : 随机函数对象
// return   : 返回 [0, INT32_MAX] 随机数
//
inline int32_t 
rand_rand(rand_t r) {
    rand_next(r);
    return (r->x[2] << (N - 1)) + (r->x[1] >> 1);
}

//
// 我 - 想云, 因为不甘心 :0
//
static rand_t r_r = { { { X0, X1, X2 }, { A0, A1, A2 }, C } };

// EXTERN_RUN(r_init) 启动初始化
extern inline void r_init(void) {
    rand_init(r_r, time(NULL));
}

//
// r_rand  - 得到 [0, INT32_MAX] 随机数
// r_randk - 得到一个 64 位的 key
// r_rands - 得到 [min, max] 范围内随机数
//
inline int32_t 
r_rand(void) {
    return rand_rand(r_r);
}
```

	弃系统 rand 函数不够快和安全. 随机函数算法诞生对于计算机行业的发展真不得了, 奠定了人
    (为什么成篇的刷代码, 方便你一个个对着敲到你的电脑中, 也方便你找出作者错误 ~) 代码都
	懂, rand_next 计算复杂点. 之后就看自己悟了, 毕竟世界也是咱们的. r_randk, r_rands
    思路很浅显分别根据位随机和区间范围. 从上面 r_r 可以看出来随机函数并不是线程安全的. 
	在多线程环境中就会出现未知行为了(至少咱们不清楚). 这样也很有意思, 毕竟不可控的随机才
	会有点随机吗? 同样我们也提供了 rand_rand 这种线程安全的伪随机函数. 不怕折腾可以把上
	面代码直接刷到你的项目中, 解决随机数的平台无关性 ~ 目前 winds 和 linux 测试良好.

```C
/*
 describe:
	1亿的数据量, 测试随机生成函数
	front system rand, back r_rand rand
 
 test code

 // 1 亿的数据测试
 #define TEST_INT	(100000000)
 
 static int test_rand(int (* trand)(void)) {
 	 int rd = 0;
 	 for (int i = 0; i < TEST_INT; ++i)
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

	弃系统 rand 函数不够快和安全. 随机函数算法诞生对于计算机行业的发展真不得了, 奠定了人	
    到这基本前戏做的够足了. 这里不妨带大家去武当山抓个宝宝.

```C
#include <stdio.h>
#include <stdlib.h>

#define R_INT         (128)
#define F_INT         (100000000)

// getr - 得到 rand() 返回值, 并写入到文件中
static int getr(long long *pnt) {
    static int cnt;

    int r = rand();
    long long t = *pnt + 1;
    
    // 每次到万再提醒一下
    if(t % F_INT == 0)
        fprintf(stdout, "%d 个数据跑完了[%d, %lld]\n", F_INT, cnt, t);

    if(t < 0) { // 数据超标了
        ++cnt;

        fprintf(stderr, "Now %d T > %lld\n", cnt, t - 1);
        *pnt = 0; // 重新开始一轮
    }
    *pnt = t;
    return r;
}

// main - 验证 rand 函数的周期
int main(int argc, char* argv[]) {
    int base[R_INT];
    int r, i = -1;
    long long cnt = 0;


    // 先产生随机函数
    while(++i < R_INT)
        base[i] = getr(&cnt);

    // 这里开始随机了
    for(;;) {
        r = getr(&cnt);
        if (r != base[0])
            continue;

        // 继续匹配查找
        for(i = 1; i < R_INT; ++i) {
            r = getr(&cnt);
            if(r != base[i]) 
                break;
        }

        // 找见了数据
        if(i == R_INT) {
            printf("Now T = %lld\n", cnt);
            break;
        }
    }

    return EXIT_SUCCESS;
}
```

	可以将 R_INT 修改为 (1024) 最终得到结果也是一样. 因为抓到了 window 平台上面 rand()
	伪随机函数的周期 G 点. 希望大家玩的开心.

![rand T](./img/rand.png)

## 4.3 file 文件库封装

		文件相关操作无外乎删除创建获取文件属性. 更加具体点的需求有, 想获取程序的运行目录,
	需要多级删除目录, 需要多级创建目录... 这里就是为了解决这个问题. 先展示部分设计, 再逐
	个击破.

```C
#ifndef _FILE_H
#define _FILE_H

#include "atom.h"
#include "struct.h"
#include "strext.h"

#ifdef __GNUC__

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//
// mkdir - 单层目录创建函数宏, 类比 mkdir path
// path     : 目录路径
// return   : 0 表示成功, -1 表示失败, 失败原因见 errno
// 
#undef  mkdir
#define mkdir(path)                                 \
mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

//
// mtime - 得到文件最后修改时间
// path     : 文件路径
// return   : 返回时间戳, -1 表示失败
//
inline time_t mtime(const char * path) {
    struct stat st;
    // 数据最后的修改时间
    return stat(path, &st) ? -1 : st.st_mtime;
}

#endif

#ifdef _MSC_VER

#include <io.h>
#include <direct.h>
#include <windows.h>

// int access(const char * path, int mode /* 四个检测宏 */);
#ifndef F_OK
#   define  F_OK    (0)
#endif       
#ifndef X_OK 
#   define  X_OK    (1)
#endif       
#ifndef W_OK 
#   define  W_OK    (2)
#endif       
#ifndef R_OK 
#   define  R_OK    (4)
#endif

inline time_t mtime(const char * path) {
    WIN32_FILE_ATTRIBUTE_DATA st;
    if (!GetFileAttributesEx(path, GetFileExInfoStandard, &st))
        return -1;
    // 基于 winds x64 sizeof(long) = 4
    return *(time_t *)&st.ftLastWriteTime;
}

#endif

//
// removes - 删除非空目录 or 文件
// path     : 文件路径
// return   : < 0 is error, >=0 is success
//
extern int removes(const char * path);

//
// mkdirs - 创建多级目录
// path     : 目录路径
// return   : < 0 is error, 0 is success
//
extern int mkdirs(const char * path);

//
// mkfdir - 通过文件路径创建目录
// path     : 文件路径
// return   : < 0 is error, 0 is success
//
extern int mkfdir(const char * path);

//
// getawd - 得到程序运行目录, \\ or / 结尾
// buf      : 存储地址
// size     : 存储大小
// return   : 返回长度, -1 is error 
//
extern int getawd(char * buf, size_t size);

#endif//_FILE_H
```

	removes, mkdirs, mkfdir, getawd 是不是有了这些接口, 以后程序操作目录方便了很多. 其
	中 removes 省力的通过系统 shell 的能力来实现的.

```C
//
// removes - 删除非空目录 or 文件
// path     : 文件路径
// return   : < 0 is error, >=0 is success
//
inline int removes(const char * path) {
    char s[BUFSIZ];

#ifndef RMRF_STR
#   ifdef _MSC_VER
#       define RMRF_STR    "rmdir /s /q \"%s\""
#   else
#       define RMRF_STR    "rm -rf '%s'"
#   endif
#endif

    // path 超过缓冲区长度, 返回异常
    if (snprintf(s, sizeof s, RMRF_STR, path) == sizeof s) 
        return -1;
    return access(path, F_OK) ? 0 : -system(s);
}
```

	access 检查 path 是否存在, 存在返回 0. 不存在返回 -1, 并且执行 system RMRF_STR 相关
	操作. 而 mkdirs 和 mkfdir 核心在于 access 和 mkdir 来回瞎搞. 

```C
//
// mkdirs - 创建多级目录
// path     : 目录路径
// return   : < 0 is error, 0 is success
//
int 
mkdirs(const char * path) {
    char c, * p, * s;

    // 参数错误直接返回
    if (!path || !*path) return -2;
    // 文件存在 or 文件一次创建成功 直接返回
    if (!access(path, F_OK) || !mkdir(path))
        return 0;

    // 跳过第一个 ['/'|'\\'] 检查是否是多级目录
    p = (char *)path;
    while ((c = *++p) != '\0')
        if (c == '/' || c == '\\')
            break;
    if (c == '\0') return -1;

    // 开始循环构建多级目录
    s = p = strdup(path);
    while ((c = *++p) != '\0') {
        if (c == '/' || c == '\\') {
            *p = '\0';

            if (access(s, F_OK)) {
                // 文件不存在, 开始创建, 创建失败直接返回错误
                if (mkdir(s)) {
                    free(s);
                    return -1;
                }
            }

            *p = c;
        }
    }

    // 最后善尾
    c = p[-1]; free(s);
    if (c == '/' || c == '\\')
        return 0;

    // 剩下最后文件路径, 开始构建
    return mkdir(path) ? -1 : 0;
}

//
// mkfdir - 通过文件路径创建目录
// path     : 文件路径
// return   : < 0 is error, 0 is success
//
int 
mkfdir(const char * path) {
    const char * r;
    char c, * p, * s;
    if (!path) return -2;

    for (r = path + strlen(path); r >= path; --r)
        if ((c = *r) == '/' || c == '\\')
            break;
    if (r < path) return -1;

    // 复制地址地址并构建
    s = p = strdup(path);
    p[r - path] = '\0';

    while ((c = *++p) != '\0') {
        if (c == '/' || c == '\\') {
            *p = '\0';

            if (access(s, F_OK)) {
                // 文件不存在, 开始创建, 创建失败直接返回错误
                if (mkdir(s)) {
                    free(s);
                    return -1;
                }
            }

            *p = c;
        }
    }

    // 一定不是 / or \\ 结尾直接, 构建返回
    if (access(s, F_OK)) {
        if (mkdir(s)) {
            free(s);
            return -1;
        }
    }
    free(s);
    return 0;
}
```

	最后 getawd 获取程序运行目录

```C
//
// getawd - 得到程序运行目录, \\ or / 结尾
// buf      : 存储地址
// size     : 存储大小
// return   : 返回长度, -1 is error 
//
int 
getawd(char * buf, size_t size) {
    char * tail;

#ifndef getawe
#   ifdef _MSC_VER
#       define getawe(b, s)    (int)GetModuleFileName(NULL, b, (DWORD)s);
#   else
#       define getawe(b, s)    (int)readlink("/proc/self/exe", b, s);
#   endif
#endif

    int r = getawe(buf, size);
    if (r <= 0 || r >= size)
        return -1;

    for (tail = buf + r - 1; tail > buf; --tail)
        if ((r = *tail) == '/' || r == '\\')
            break;
    // believe getawe return
    *++tail = '\0';
    return (int)(tail - buf);
}
```

	主要使用场景如下, 通过 getawd 得到程序运行目录, 随后拼接出各种文件的绝对路径. 再去嗨.

```C
#define LOG_PATH_STR        "logs/structc.log"

int n;
char r[BUFSIZ];
// 配置模块初始化
//
n = getawd(r, sizeof r);
assert(0 < n && n < sizeof r);

memcpy(r+n, LOG_PATH_STR, LEN(LOG_PATH_STR));
mkfdir(r);
EXTERN_RUN(log_init, r);
```

### 4.3.1 file 监控

    很多时候有这样一个需求, 某个配置是可刷新的. 完成这个功能也很简单, 无外乎外部触发或者内部
    监控. 两种方式内部触发是最省力, 我们也想这种能力包含到 file.h 接口设计中.

```C
#ifndef _FILE_H
#define _FILE_H

...

//
// file_f - 文件更新行为
//
typedef void (* file_f)(FILE * c, void * arg);

//
// file_set - 文件注册更新行为
// path     : 文件路径
// func     : file update -> func(path -> FILE, arg), func is NULL 标记清除
// arg      : func 额外参数
// return   : void
//
extern void file_set(const char * path, file_f func, void * arg);

//
// file_update - 配置文件刷新操作
// return   : void
//
extern void file_update(void);

#endif//_FILE_H
```

    file_set 注册需要监控的文件, file_f 是监控到变化后触发的行为. file_update 是全局的更新行
    为, 用于监控是否有文件发生了变化. 它的本质是依赖 mtime 获取最后一次文件变化的时间. 用于确
    定此文件当前是否发生了变化. 其中核心的数据结构如下

```C
struct file {
    time_t last;            // 文件最后修改时间点
    char * path;            // 文件全路径
    unsigned hash;          // 文件路径 hash 值

    file_f func;            // 执行行为
    void * arg;             // 行为参数

    struct file * next;     // 文件下一个节点
};

static struct files {
    atom_t lock;            // 当前对象原子锁
    struct file * head;     // 当前文件对象集
} f_s;

// files add 
static void f_s_add(const char * p, unsigned h, file_f func, void * arg) {
    struct file * fu;
    if (mtime(p) == -1) {
        RETNIL("mtime error p = %s", p);
    }

    fu = malloc(sizeof(struct file));
    fu->last = -1;
    fu->path = strdup(p);
    fu->hash = h;
    fu->func = func;
    fu->arg = arg;

    // 直接插入到头节点部分
    atom_lock(f_s.lock);
    fu->next = f_s.head;
    f_s.head = fu;
    atom_unlock(f_s.lock);
}

// files get 
static struct file * f_s_get(const char * p, unsigned * r) {
    struct file * fu = f_s.head;
    unsigned h = *r = str_hash(p);

    while (fu) {
        if (fu->hash == h && strcmp(fu->path, p) == 0)
            break;
        fu = fu->next;
    }

    return fu;
}
```

    file_set 注册需要监控的文件, file_f 是监控到变化后触发的行为. file_update 是全局的更新行
    对于每个要监控的文件, 我们记录了最后一次修改时间 last, 文件全路径 path, 执行体 func 和 
    arg. 有了这些基本上就查码代码了. 其中 file_set 设计包含了 del 操作, 即当 file_f 设置为空
    NULL 就认为是 file_del(path) 操作. 

```C
//
// file_set - 文件注册更新行为
// path     : 文件路径
// func     : file update -> func(path -> FILE, arg), func is NULL 标记清除
// arg      : func 额外参数
// return   : void
//
void 
file_set(const char * path, file_f func, void * arg) {
    unsigned h;
    assert(path && *path);
    struct file * fu = f_s_get(path, &h);
    if (NULL == fu)
        f_s_add(path, h, func, arg);
    else {
        atom_lock(f_s.lock);
        fu->last = -1;
        fu->func = func;
        fu->arg = arg;
        atom_unlock(f_s.lock);
    }
}
```

    随后就是 file_update 全局监控和更新

```C
//
// file_update - 配置文件刷新操作
// return   : void
//
void 
file_update(void) {
    atom_lock(f_s.lock);
    struct file * fu = f_s.head;
    while (fu) {
        struct file * next = fu->next;

        if (NULL == fu->func) {
            // 删除的是头节点
            if (f_s.head == fu)
                f_s.head = next;

            free(fu->path);
            free(fu);
        } else {
            time_t last = mtime(fu->path);
            if (fu->last != last && last != -1) {
                FILE * c = fopen(fu->path, "rb+");
                if (NULL == c) {
                    CERR("fopen rb+ error = %s.", fu->path);
                    continue;
                }
                fu->last = last;
                fu->func(c, fu->arg);
                fclose(c);
            }
        }

        fu = next;
    }
    atom_unlock(f_s.lock);
}
```

    file_update 做的工作就是循环遍历 struct files::head 链表, 挨个检查文件最后一次修改时间
    mtime 是否发生变化. 如果发生了就触发 file_f 注册行为. 当然也会清除待删除的注册行为. 到这
    里我们的文件操作就讲完了. 很枯燥, 但是是你的鲤鱼跃龙门的阶梯.

## 4.4 C 来个 json 轮子

        在我刚做开发的时候, 那时候维护的系统, 所有配置走的是 xml 和 csv. 刚好 json 在国内刚
    兴起, 所以一时兴起为其写了个解释器. 过了 1 年接触到 cJSON 库, 直接把自己当初写的那个删了.
    用起了 cJSON, 后面觉得 cJSON 真的丑的不行不行, 就琢磨写了个简单的 c json. 这小节, 就带大
    家写写这个 c json 的解析引擎, 清洁高效小. 能够保证的就是比 cJSON 好学习.

### 4.4.1 c json 设计布局

    首先大概分析 scjson 的实现部分. 最关心的是 scjson的内存布局, 这里引入了 tstr 布局.
    设计结构图如下 :

![scjson内存布局](./img/scjson内存布局.png)

    str 指向内存常变, tstr指向内存不怎么变. 所以采用两块内存保存. tstr 存在目的是个中转
    站. 因为读取文件内容, 中间 json 内容去注释, 压缩需要一块内存. 这就是引入 tstr目的.

    再看看 scjson 结构代码设计:

```C
struct cjson {
    struct cjson * next;    // 采用链表结构处理, 放弃二叉树结构, 优化内存
    struct cjson * child;   // type == ( CJSON_ARRAY or CJSON_OBJECT ) 那么 child 就不为空

    unsigned char type;     // 数据类型 CJSON_XXXX, 一个美好的意愿
    char * key;             // json内容那块的 key名称 	
    union {
        char * vs;          // type == CJSON_STRING, 是一个字符串 	
        double vd;          // type == CJSON_NUMBER, 是一个num值, ((int)c->vd) 转成int 或 bool
    };
};

//定义cjson_t json类型
typedef struct cjson * cjson_t;
```

    使用 c99的匿名结构体挺爽的, 整个内存详细布局如下:

![scjson内存结构](./img/scjson内存结构.png)

    scjson 中处理的类型类型无外乎:

```C
// json中几种数据结构和方式定义, 对于程序开发而言最难的还是理解思路(思想 or 业务)
#define CJSON_NULL      (0u << 0)
#define CJSON_FALSE     (1u << 0)
#define CJSON_TRUE      (1u << 1)
#define CJSON_NUMBER    (1u << 2)
#define CJSON_STRING    (1u << 3)
#define CJSON_ARRAY     (1u << 4)
#define CJSON_OBJECT    (1u << 5)

//
// cjson_getint - 这个宏, 协助我们得到 int 值
// item		: 待处理的目标cjson_t结点
// return	: int
//
#define cjson_getvi(item) ((int)((item)->vd))
```

    以上就是解析之后的具体结构类型. 下面简单分析一下文本解析规则.

![scjson递归下降分析](./img/scjson递归下降分析.png)

    思路是递归下降分析. 到这里基本关于 scjson详细设计图介绍完毕了.
    后面会看见这只麻雀代码极少 ヽ(✿ﾟ▽ﾟ)ノ

#### 4.4.1 scjson 详细设计

    当初写这类东西, 就是对着协议文档开撸 ~
    这类代码是协议文档和作者思路的杂糅体, 推荐最好手敲一遍, 自行加注释, 琢磨后吸收.
    来看看删除函数

```C
//
// cjson_delete - 删除json串内容  
// c		: 待释放json_t串内容
// return	: void
//
void 
cjson_delete(cjson_t c) {
	while (c) {
		cjson_t next = c->next;
		// 放弃引用和常量的优化选项
		free(c->key);
		if (c->type & CJSON_STRING)
			free(c->vs);
		// 递归删除子节点
		if (c->child)
			cjson_delete(c->child);
		free(c);
		c = next;
	}
}
```

    一直到此刻我们一直直接使用 malloc 和 free. 没有提供自定义分配器接口. 其实这个很好
    扩展, 后期自己可以内嵌个 jemalloc 传说中最屌的内存分配器. 全局宏替换, 或者直接包
    装层再全局搞. 
    上面操作无外乎就是递归找到最下面的儿子结点, 期间删除自己挂载的结点. 然后依次按照 
    next 链表顺序循环执行. 随后通过代码逐个分析思维过程, 例如我们得到一个 json串, 
    这个串中可能存在多余的空格, 多余的注释等. 就需要做洗词的操作, 只留下最有用的
    json 字符串 :

```C
//  将 jstr中 不需要解析的字符串都去掉, 返回压缩后串的长度. 并且纪念mini 比男的还平
static size_t _cjson_mini(char * jstr) {
	char c, * in = jstr, * to = jstr;

	while ((c = *to)) {
		// step 1 : 处理字符串
		if (c == '"') {
			*in++ = c;
			while ((c = *++to) && (c != '"' || to[-1] == '\\'))
				*in++ = c;
			if (c) {
				*in++ = c;
				++to;
			}
			continue;
		}
		// step 2 : 处理不可见特殊字符
		if (c < '!') {
			++to;
			continue;
		}
		if (c == '/') {
			// step 3 : 处理 // 解析到行末尾
			if (to[1] == '/') {
				while ((c = *++to) && c != '\n')
					;
				continue;
			}

			// step 4 : 处理 /*
			if (to[1] == '*') {
				while ((c = *++to) && (c != '*' || to[1] != '/'))
					;
				if (c)
					to += 2;
				continue;
			}
		}
		// step 5 : 合法数据直接保存
		*in++ = *to++;
	}

	*in = '\0';
	return in - jstr;
}
```

    上面操作主要目的是让解析器能够处理 json串中 // 和 /**/, 并删除些不可见字符.
    开始上真正的解析器入口函数 :

```C
// 递归下降分析 需要声明这些函数
static const char * _parse_value(cjson_t item, const char * str);
static const char * _parse_array(cjson_t item, const char * str);
static const char * _parse_object(cjson_t item, const char * str);

//构造一个空 cjson 对象
static inline cjson_t _cjson_new(void) {
	cjson_t node = malloc(sizeof(struct cjson));
	if (NULL == node)
		CERR_EXIT("malloc struct cjson is null!");
	return memset(node, 0, sizeof(struct cjson));
}

// jstr 必须是 _cjson_mini 解析好的串
static cjson_t _cjson_parse(const char * jstr) {
	const char * end;
	cjson_t json = _cjson_new();

	if (!(end = _parse_value(json, jstr))) {
		cjson_delete(json);
		RETURN(NULL, "_parse_value params end = %s!", end);
	}

	return json;
}

//
// cjson_newxxx - 通过特定源, 得到内存中json对象
// str		: 普通格式的串
// tstr		: tstr_t 字符串, 成功后会压缩 tstr_t
// path		: json 文件路径
// return	: 解析好的 json_t对象, 失败为NULL
//
inline cjson_t 
cjson_newstr(const char * str) {
	cjson_t json;
	TSTR_CREATE(tstr);
	tstr_appends(tstr, str);

	_cjson_mini(tstr->str);
	json = _cjson_parse(tstr->str);

	TSTR_DELETE(tstr);
	return json;
}
```

    从 cjson_newstr看起, 声明了栈上字符串 tstr填充 str, 随后进行 _cjson_mini洗词, 然后
    通过 _cjson_parse 解析最终结果返回. 随后可以看哈 _cjson_parse 实现, 非常好理解, 本
    质就是走分支. 不同分支走不同的解析函数:

```C
// 将 value 转换塞入 item json 值中一部分
static const char * 
_parse_value(cjson_t item, const char * str) {
	char c = '\0'; 
	if ((str) && (c = *str)) {
		switch (c) {
		// n = null, f = false, t = true ... 
		case 'n' : return item->type = CJSON_NULL, str + 4;
		case 'f' : return item->type = CJSON_FALSE, str + 5;
		case 't' : return item->type = CJSON_TRUE, item->vd = 1.0, str + 4;
		case '\"': return _parse_string(item, str);
		case '0' : case '1' : case '2' : case '3' : case '4' : case '5' :
		case '6' : case '7' : case '8' : case '9' :
		case '+' : case '-' : case '.' : return _parse_number(item, str);
		case '[' : return _parse_array(item, str);
		case '{' : return _parse_object(item, str);
		}
	}
	// 循环到这里是意外 数据
	RETURN(NULL, "params value = %c, %s!", c, str);
}
```

    肉眼层面的协议处理了, 像 MSGPACK 就是对上面 n f ... { 扩展成1字节内数值. 核心原理
    还是一样. 看下 _parse_string 处理, 内嵌了 utf8 字符处理会感觉套路长一点点

```C
// parse 4 digit hexadecimal number
static unsigned _parse_hex4(const char str[]) {
	unsigned c, h = 0, i = 0;
	// 开始转换16进制
	for(;;) {
		c = *str;
		if (c >= '0' && c <= '9')
			h += c - '0';
		else if (c >= 'A' && c <= 'F')
			h += 10 + c - 'A';
		else if (c >= 'a' && c <= 'z')
			h += 10 + c - 'a';
		else
			return 0;
		// shift left to make place for the next nibble
		if (4 == ++i)
			break;
		h <<= 4;
		++str;
	}

	return h;
}

// 分析字符串的子函数,
static const char * _parse_string(cjson_t item, const char * str) {
	static unsigned char _marks[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
	const char * ptr;
	char c, * nptr, * out;
	unsigned len = 1, uc, nuc;

	// 检查是否是字符串内容, 并记录字符串大小
	if (*str != '\"')
		RETURN(NULL, "need \\\" str => %s error!", str);
	for (ptr = str + 1; (c = *ptr++) != '\"' && c; ++len)
		if (c == '\\') {
			//跳过转义字符
			if (*ptr == '\0')
				RETURN(NULL, "ptr is end len = %d.", len);
			++ptr;
		}
	if (c != '\"')
		RETURN(NULL, "need string \\\" end there c = %d, %c!", c, c);

	// 这里开始复制拷贝内容
	if (!(nptr = out = malloc(len)))
		CERR_EXIT("calloc size = %d is error!", len);
	for (ptr = str + 1; (c = *ptr) != '\"' && c; ++ptr) {
		if (c != '\\') {
			*nptr++ = c;
			continue;
		}
		// 处理转义字符
		switch ((c = *++ptr)) {
		case 'b': *nptr++ = '\b'; break;
		case 'f': *nptr++ = '\f'; break;
		case 'n': *nptr++ = '\n'; break;
		case 'r': *nptr++ = '\r'; break;
		case 't': *nptr++ = '\t'; break;
		case 'u': // 将utf16 => utf8, 专门的utf处理代码
			uc = _parse_hex4(ptr + 1);
			ptr += 4; //跳过后面四个字符, unicode
			if (0 == uc || (uc >= 0xDC00 && uc <= 0xDFFF))
				break;	/* check for invalid. */

			if (uc >= 0xD800 && uc <= 0xDBFF) { /* UTF16 surrogate pairs. */
				if (ptr[1] != '\\' || ptr[2] != 'u')	
					break;	/* missing second-half of surrogate. */
				nuc = _parse_hex4(ptr + 3);
				ptr += 6;
				if (nuc < 0xDC00 || nuc>0xDFFF)		
					break;	/* invalid second-half of surrogate.	*/
				uc = 0x10000 + (((uc & 0x3FF) << 10) | (nuc & 0x3FF));
			}

			if (uc < 0x80)
				len = 1;
			else if (uc < 0x800)
				len = 2;
			else if (uc < 0x10000)
				len = 3;
			else
				len = 4;
			nptr += len;

			switch (len) {
			case 4: *--nptr = ((uc | 0x80) & 0xBF); uc >>= 6;
			case 3: *--nptr = ((uc | 0x80) & 0xBF); uc >>= 6;
			case 2: *--nptr = ((uc | 0x80) & 0xBF); uc >>= 6;
			case 1: *--nptr = (uc | _marks[len]);
			}
			nptr += len;
			break;
		default: *nptr++ = c;
		}
	}
	*nptr = '\0';
	item->vs = out;
	item->type = CJSON_STRING;
	return ++ptr;
}
```

    编码转换非内幕人员多数只能看看. 扯一点, 很久以前对于编码解决方案. 采用的是 libiconv 
    方案, 将其移植到 winds上. 后面学到一招, 因为国内开发最多的需求就是 gbk 和 utf-8 国际
    标准的来回切. 那就直接把这个编码转换的算法拔下来, 岂不最好~

> 引述一丁点维基百科 UTF-8 编码字节含义:  
>  
> 对于UTF-8编码中的任意字节B，如果B的第一位为0，则B独立的表示一个字符(ASCII码)；  
> 如果B的第一位为1，第二位为0，则B为一个多字节字符中的一个字节(非ASCII字符)；  
> 如果B的前两位为1，第三位为0，则B为两个字节表示的字符中的第一个字节；  
> 如果B的前三位为1，第四位为0，则B为三个字节表示的字符中的第一个字节；  
> 如果B的前四位为1，第五位为0，则B为四个字节表示的字符中的第一个字节；  
>  
> 因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符；根据前二位，  
> 可判断该字节是否为一个字符编码的第一个字节；根据前四位（如果前两位均为1），  
> 可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示；  
> 根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误。 

    最后一个前戏呼之欲出了, 字符串转 number

```C
// 分析数值的子函数,写的可以
static const char * _parse_number(cjson_t item, const char * str) {
	double n = .0, ns = 1.0, nd = .0; // ns表示开始正负, 负为-1, nd表示小数后面位数
	int e = 0, es = 1; // e表示后面指数, es表示 指数的正负, 负为-1
	char c;

	if ((c = *str) == '-' || c == '+') {
		ns = c == '-' ? -1.0 : 1.0; // 正负号检测, 1表示负数
		++str;
	}
	// 处理整数部分
	for (c = *str; c >= '0' && c <= '9'; c = *++str)
		n = n * 10 + c - '0';
	if (c == '.')
		for (; (c = *++str) >= '0' && c <= '9'; --nd)
			n = n * 10 + c - '0';

	// 处理科学计数法
	if (c == 'e' || c == 'E') {
		if ((c = *++str) == '+') //处理指数部分
			++str;
		else if (c == '-')
			es = -1, ++str;
		for (; (c = *str) >= '0' && c <= '9'; ++str)
			e = e * 10 + c - '0';
	}

	//返回最终结果 number = +/- number.fraction * 10^+/- exponent
	item->vd = ns * n * pow(10.0, nd + es * e);
	item->type = CJSON_NUMBER;
	return str;
}
```

    最后就到了重头戏, 递归下降分析的两位主角了 _parse_array 和 _parse_object 

```C
// 分析数组的子函数, 采用递归下降分析
static const char * 
_parse_array(cjson_t item, const char * str) {
	cjson_t child;

	if (*str != '[') {
		RETURN(NULL, "array str error start: %s.", str);
	}

	item->type = CJSON_ARRAY;
	if (*++str == ']') // 低估提前结束, 跳过']'
		return str + 1;

	item->child = child = _cjson_new();
	str = _parse_value(child, str);
	if (NULL == str) {
		RETURN(NULL, "array str error e n d one: %s.", str);
	}

	while (*str == ',') {
		// 支持行尾处理多余 ','
		if (str[1] == ']')
			return str + 1;

		// 写代码是一件很爽的事
		child->next = _cjson_new();
		child = child->next;
		str = _parse_value(child, str + 1);
		if (NULL == str) {
			RETURN(NULL, "array str error e n d two: %s.", str);
		}
	}

	if (*str != ']') {
		RETURN(NULL, "array str error e n d: %s.", str);
	}
	return str + 1;
}
```

    处理的格式 '[ ... , ... , ... ]'. 
    同样处理 object 格式如下 ' { "str":..., "str":..., ... } '

```C
// 分析对象的子函数
static const char * 
_parse_object(cjson_t item, const char * str) {
	cjson_t child;

	if (*str != '{') {
		RETURN(NULL, "object str error start: %s.", str);
	}

	item->type = CJSON_OBJECT;
	if (*++str == '}')
		return str + 1;

	//处理结点, 开始读取一个 key
	item->child = child = _cjson_new();
	str = _parse_string(child, str);
	if (!str || *str != ':') {
		RETURN(NULL, "_parse_string is error : %s!", str);
	}
	child->key = child->vs;

	child->vs = NULL;
	str = _parse_value(child, str + 1);
	if (!str) {
		RETURN(NULL, "_parse_value is error 2!");
	}

	// 递归解析
	while (*str == ',') {
		// 支持行尾处理多余 ','
		if (str[1] == '}')
			return str + 1;

		child->next = _cjson_new();
		child = child->next;
		str = _parse_string(child, str + 1);
		if (!str || *str != ':'){
			RETURN(NULL, "_parse_string need name or no equal ':' %s.", str);
		}
		child->key = child->vs;

		child->vs = NULL;
		str = _parse_value(child, str+1);
		if (!str) {
			RETURN(NULL, "_parse_value need item two ':' %s.", str);
		}
	}

	if (*str != '}') {
		RETURN(NULL, "object str error e n d: %s.", str);
	}
	return str + 1;
}
```

    关于 json 串的解析部分就完工了. 核心是学习递归下降分析的套路, 一个间接递归套路.
    通过上面的思路, 花些心思也可以构建出 json对象转 json串的思路. 或者写个小型计算
    器等等. 有了 json 的处理库, 有没有感觉基础的业务都能轻松胜任了哈哈.

## 4.5 奥特曼, 通用头文件构建

    在实战项目中, 都会有个出现频率特别高的一个头文件, 项目中基本每个头文件都继承自
	它. 同样此刻出现的就是筑基期至强奥义, 一切从头开始
    
schead.h

```C
#ifndef _H_SIMPLEC_SCHEAD
#define _H_SIMPLEC_SCHEAD

#include "clog.h"
#include "scrand.h"
#include "struct.h"
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
        async_run_((node_f)(run), (void *)(intptr_t)arg)

#endif//_H_SIMPLEC_SCHEAD
```

    可以看出来, 头文件包含是一种继承关系. 基本每一个包含了 schead.h 的文件, 都不用
    再包含其它的辅助文件, 开发起来会很方便. 额外说一下 getch 这个函数: 

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

    很久以前一位化神期巨擘说过: 由于 linux对于 getch支持不友好, 导致了 linux错失了很多
    游戏开发人员. 我是挺喜欢 getch的, 写个小游戏太轻松了. 就顺手补上了.
    其它的辅助函数:

```C
inline void 
sh_pause(void) {
    rewind(stdin);
	fflush(stderr); fflush(stdout);
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

    说一下 sh_isbe, 它是判断当前系统是否是大端系统, 是返回 true. 而多数接触的是小端
    机器, 所以自己设计了个 sh_hton 如果是大端结构会转成小端结构. 这样减少转换频率.
    sh_pause 相似功能在 winds上面是 system("pause"), 在 linux 是 pause(). 用起来
    不爽, 顺带为了方便 DEBUG 测试, 搞了个 SH_PAUSE(). schead.h 接口文件定位目标是
    所有业务层面的辅助头文件. 美好从此刻开始 ~
    
    新的风暴已经出现 怎么能够停滞不前 穿越时空竭尽全力 我会来到你身边

#### 4.5 阅读理解环节, csv解析库

    很久以前桌面项目配置文件基本都走 csv文件配置. 采用 ',' 分隔. 同 excel表格形式.
    维护人员通过 notepad++ or excel 编辑操作. 程序人员直接读取开撸. 展示个自己写的
    解决方案, 灰常节约内存. 首先展示 interface:

scscv.h

```C
#ifndef _H_SIMPLEC_SCCSV
#define _H_SIMPLEC_SCCSV

//
// 这里是一个解析 csv 文件的 简单解析器.
// 它能够帮助我们切分文件内容, 保存在字符串数组中.
//
typedef struct sccsv {      //内存只能在堆上
    int rlen;               //数据行数,索引[0, rlen)
    int clen;               //数据列数,索引[0, clen)
    const char * data[];    //保存数据一维数组,希望他是二维的 rlen*clen
} * sccsv_t;

//
// 从文件中构建csv对象, 最后需要调用 sccsv_delete 释放
// path		: csv文件内容
// return	: 返回构建好的 sccsv_t 对象
//
extern sccsv_t sccsv_create(const char * path);

//
// 释放由sccsv_create构建的对象
// csv		: sccsv_create 返回对象
//
extern void sccsv_delete(sccsv_t csv);

//
// 获取某个位置的对象内容
// csv		: sccsv_t 对象, new返回的
// ri		: 查找的行索引 [0, csv->rlen)
// ci		: 查找的列索引 [0, csv->clen)
// return	: 返回这一项中内容,后面可以用 atoi, atof, tstr_dup 等处理了...
//
extern const char * sccsv_get(sccsv_t csv, int ri, int ci);

#endif // !_H_SIMPLEC_SCCSV
```

    这里我们只提供了读接口, 比较有特色的思路是. sccsv_t 我们采用一整块内存构建. 非常
    爽. 

```C
#include "sccsv.h"
#include "tstr.h"

//从文件中读取 csv文件内容, 构建一个合法串
static bool _csv_parse(tstr_t tstr, int * prl, int * pcl) {
	int c = -1, n = -1;
	int cl = 0, rl = 0;
	char * sur = tstr->str, * tar = tstr->str;

	while (!!(c = *tar++)) {
		// 小型状态机切换, 相对于csv文件内容解析
		switch (c) {
		case '"': // 双引号包裹的特殊字符处理
			while (!!(c = *tar++)) {
				if ('"' == c) {
					if ((n = *tar) == '\0') // 判断下一个字符
						goto _faild;
					if (n != '"') // 有效字符再次压入栈, 顺带去掉多余 " 字符
						break;
					++tar;
				}

				// 添加得到的字符
				*sur++ = c;
			}
			// 继续判断,只有是c == '"' 才会下来,否则都是错的
			if ('"' != c)
				goto _faild;
			break;
		case ',':
			*sur++ = '\0';
			++cl;
			break;
		case '\r':
			break;
		case '\n':
			*sur++ = '\0';
			++cl;
			++rl;
			break;
		default: // 其它所有情况只添加数据就可以了
			*sur++ = c;
		}
	}
	
	if (cl % rl) { // 检测 , 号是个数是否正常
	_faild:
		RETURN(false, "now csv error c = %d, n = %d, cl = %d, rl = %d.", c, n, cl, rl);
	}
	
	// 返回最终内容
	*prl = rl;
	*pcl = cl;
	// 构建最终处理的串内容
	tstr->len = sur - tstr->str + 1;
	tstr->str[tstr->len - 1] = '\0';
	return true;
}

// 将 _csv_get 得到的数据重新构建返回, 执行这个函数认为语法检测都正确了
static sccsv_t _csv_create(tstr_t tstr) {
	sccsv_t csv;
	size_t pdff;
	char * cstr;
	int rl, cl, i;
	if (!_csv_parse(tstr, &rl, &cl))
		return NULL;

	// 分配最终内存
	pdff = sizeof(struct sccsv) + sizeof(char *) * cl;
	csv = malloc(pdff + tstr->len);
	if (NULL == csv) {
		RETURN(NULL, "malloc error cstr->len = %zu, rl = %d, cl = %d.", tstr->len, rl, cl);
	}

	// 这里开始拷贝内存, 构建内容了
	cstr = (char *)csv + pdff;
	memcpy(cstr, tstr->str, tstr->len);
	csv->rlen = rl;
	csv->clen = cl / rl;
	i = 0;
	do {
		csv->data[i] = cstr;
		while(*cstr++) // 找到下一个位置处
			;
	} while(++i < cl);
	
	return csv;
}

//
// 从文件中构建csv对象, 最后需要调用 sccsv_delete 释放
// path		: csv文件内容
// return	: 返回构建好的 sccsv_t 对象
//
sccsv_t
sccsv_create(const char * path) {
	sccsv_t csv;
	tstr_t tstr = tstr_freadend(path);
	if (NULL == tstr) {
		RETURN(NULL, "tstr_freadend path = %s is error!", path);
	}

	// 如果解析 csv 文件内容失败直接返回
	csv = _csv_create(tstr);

	tstr_delete(tstr);
	// 返回最终结果
	return csv;
}

//
// 释放由sccsv_create构建的对象
// csv		: sccsv_new 返回对象
//
inline void 
sccsv_delete(sccsv_t csv) {
	free(csv);
}

//
// 获取某个位置的对象内容
// csv		: sccsv_t 对象, new返回的
// ri		: 查找的行索引 [0, csv->rlen)
// ci		: 查找的列索引 [0, csv->clen)
// return	: 返回这一项中内容,后面可以用 atoi, atof, tstr_dup 等处理了...
//
inline const char * 
sccsv_get(sccsv_t csv, int ri, int ci) {
	DEBUG_CODE({
		if (!csv || ri < 0 || ri >= csv->rlen || ci < 0 || ci >= csv->clen) {
			RETURN(NULL, "params is csv:%p, ri:%d, ci:%d.", csv, ri, ci);
		}
	});

	// 返回最终结果
	return csv->data[ri * csv->clen + ci];
}
```

    核心重点在 _csv_parse 和 _csv_create 上面. 前者负责预建内存布局, 后者负责构建内
    存. 代码很短, 但却很有效不是吗~ 
    希望上面的阅读理解你能喜欢~

### 4.6 筑基展望

    妖魔战场逐渐急促起来, 我们筑基期顶天功法也就介绍到此. 数据结构算法可能要勤学苦练,
    这些轮子基本都是3遍过, 战无不利, 终身会用. 本章多数在抠细节, 协助熟悉常用基础轮子
    开发套路. 从 clog -> scrand -> schead -> scjson -> sccsv 遇到的妖魔鬼怪也不过
    如此. 真实开发中这类基础库, 要么是行业前辈遗留下来的馈赠. 要么就是远古大能的传世组
    件. 但总的而言, 如果你想把他们的精华用的更自然, 显然你也得懂行(自己会写). 
    凡事总要瞎搞搞才能有所突破 <--:-o

***

    西门吹雪忽然道：“你学剑?”

    叶孤城道：“我就是剑。”

    西门吹雪道：“你知不知道剑的精义何在?”

    叶孤城道：“你说。”

    西门吹雪道：“在于诚。”

    叶孤城道：“诚?”

    西门吹雪道：“唯有诚心正义，才能到达剑术的颠峰，不诚的人，根本不足论剑。”

    叶孤城的瞳孔突又收缩。

    西门吹雪盯着他，道：“你不诚。”

    叶孤城沉默了很久，忽然也问道：“你学剑?”

    西门吹雪道：“学无止境，剑更无止境。”

    叶孤城道：“你既学剑，就该知道学剑的人只在诚于剑，并不必诚于人。”  

***

    思绪有些乱, 突然想起飞升真仙 ~ 我们仨 ~ 或许是她(他)们撑起种族底蕴 ~

![我们仨](./img/我们仨.png)