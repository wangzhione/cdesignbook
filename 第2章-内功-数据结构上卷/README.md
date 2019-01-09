# 第2章-内功-数据结构上卷

        对于 C 而言, 数据结构不举, 一切都是虚妄. 其它语言好点, 结构算法标准都支持的很
    好用(中庸). 重复说, 在 C 的世界里, 数据结构和操作系统是硬通货. 而数据结构就是核心
    内功, 一招一式全是内劲. 修炼数据结构本质是为了掌控全局, 规划整体, 捋顺输入输出.
    内功没有几个月苦练, 很难实现外放. 内功上卷我们只讲简单一点 list, string, array,
    hash.

## 2.1 list

        如果说数据结构是 C 的内功, 那毫无疑问链表 list 就是数据结构的内丹. 链式结构最
    原始抽象模型就是 list. 同样也最实用.

![list](img/list.jpg)

    上图是一种最原始的 list 结构, next 是指向下一个节点的指针. 自己对于 list 结构的体
    会比较简单, list 只是个实体, 并且这个实体还能找到它保存的下一个实体. 随后会展示 list
    构建部分接口. 学习一个陌生的东西有很多套路, 一条迅速的路线图是:
	write demo -> see interface -> copy implement -> ...

### 2.1.1 list.h interface

```C
#ifndef _H_LIST
#define _H_LIST

#include "struct.h"

//
// list.h 通用的单链表库
// void * list = NULL;
// 
struct $list {
    struct $list * next;
};

#define $LIST struct $list $node;

//
// list_next - 获取节点 n 的下一个节点.
// n        : 当前节点
//
#define list_next(n) ((void *)((struct $list *)(n))->next)

//
// list_delete - 链表数据销毁操作
// list     : 基础的链表结构
// pist     : 指向基础的链表结构
// fdie     : 链表中删除数据执行的方法
// return   : void
//
#define list_delete(list, fdie)                                         \
list_delete_((void **)&(list), (node_f)(fdie))
extern void list_delete_(void ** pist, node_f fdie);

//
// list_get - 匹配得到链表中指定值
// list     : 基础的链表结构
// fget     : 链表中查找数据执行的方法
// left     : 待查找的节点内容 
// return   : 查找到的节点, NULL 表示没有查到
//
#define list_get(list, fget, left)                                      \
list_get_((list), (cmp_f)(fget), (const void *)(intptr_t)(left))
extern void * list_get_(void * list, cmp_f fget, const void * left);

//
// list_pop - 匹配弹出链表中指定值
// list     : 基础的链表结构
// pist     : 指向基础的链表结构
// fget     : 链表中查找数据执行的方法
// left     : 待查找的节点内容 
// return   : 查找到的节点, NULL 表示没有查到 
//
#define list_pop(list, fget, left)                                      \
list_pop_((void **)&(list), (cmp_f)(fget), (const void *)(intptr_t)(left))
extern void * list_pop_(void ** pist, cmp_f fget, const void * left);

//
// list_add - 链表中添加数据, 从小到大 fadd(left, ) <= 0
// list     : 基础的链表结构
// pist     : 指向基础的链表结构
// fadd     : 插入数据方法
// left     : 待插入的链表节点
// return   : void
//
#define list_add(list, fadd, left)                                      \
list_add_((void **)&(list), (cmp_f)(fadd), (void *)(intptr_t)(left))
extern void list_add_(void ** pist, cmp_f fadd, void * left);

//
// list_each - 链表循环处理函数, 仅仅测试而已
// list     : 基础的链表结构
// feach    : 处理每个节点行为函数
// return   : void
//
#define list_each(list, feach)                                          \
list_each_((list), (node_f)(feach))
extern void list_each_(void * list, node_f feach);

#endif//_H_LIST
```

    其中 (const void *)(intptr_t)left 是一个去除警告的函数宏技巧, 便于传入指针和整型变量. 
    对于 struct.h 可以参看第1章设计部分, 这里轻微回顾一下

```C
//
// cmp_f - 比较行为 > 0 or = 0  or < 0
// : int add_cmp(const void * now, const void * node)
//
typedef int (* cmp_f)();

//
// node_f - 销毁行为
// : void list_die(void * node)
//
typedef void (* node_f)(void * node);
```

    对于 struct $list { struct $list * next; }; 结构的设计模式, 推荐详细思考. 其设计模式
	等同于最原始的继承. $ 符号希望标识当前结构是私有的, 具备特定用途, 不推荐随便使用. 下面我
	们用 list 提供的接口原型, 构建 list people 演示例子

```C
struct people {
    $LIST           // $LIST 必须在开头位置

    int    free;    // 有理想
    char * ideal;   // 有文化
    double future;  // 有秩序
};

static inline int people_add_cmp(const struct people * n, const struct people * r) {
    return n->free - r->free;
}

static inline int people_find_cmp(double * future, const struct people * r) {
    return *future != r->future;
}

// 构建
list_t list = NULL;
struct people p = { .free = 100, .ideal = "59", .future = 0.0 };

// 添加
list_add(list, people_add_cmp, &p);

// 删除和返回节点自己负责善后
double future = 6.6;
struct people * e = list_pop(list, people_find_cmp, &future);

// 销毁 - 节点中没有待释放东西, 所以 list_delete 缺省
list_delete(list, NULL);
```

    来看下 struct people { $LIST; ... }; 结构实现的原理

```C
// struct people 结构全展开
struct people {
    struct $list {
        struct $list * next;
    } $node;		// 真面目

    int    free;    // 有理想
    char * ideal;   // 有文化
    double future;  // 有秩序
};

// p 为 struct people 结构 
struct people p = { .free = 100, .ideal = "59", .future = 0.0 };

// 存在下面成立的关系(true)
(void *)&(p.$node) == (void *)&p

// 因而由 &p 地址可以确定 $node 地址, 通过 $next 自然得到指向下个业务节点.
((struct $list *)&p)->$next
```

    读者可以画画写写感受哈, list 过于基础, 解释太多没有自己抄写 10 几类链表源码来的实在. 做
	C 相关开发, 几乎是围绕 list 结构增删改查, 高级些加个缓存层, 伪删除. 对于封装库基本套路
    是三思而后行, 想好思路, 定好基本接口, 再堆实现. 设计出优雅的接口, 是第1位. 在 C 中思路
    落地等同于数据结构定型. 后续相关代码实现就已经妥了! 最后就是 Debug <-> Unit testing 
    来回倒腾一段时间.

### 2.1.2 list implements

    这里会展示并分析作者设计思路. 在设计一个库的时候, 首要考虑的是创建和删除(销毁), 事关生存
    周期的问题. 这里 list 创建比较简单采用了一个潜规则 list_t list = NULL; 代表创建一个
    空链表. 链表头创建常见有两个套路, 其一是自带实体头节点, 这种思路在处理头节点的时候特别方
    便. 其二就是我们这里没有头节点一开始为从 NULL 开始, 优势在于节省空间. 对于链表的销毁操作
    , 使用 list_delete 用于链表节点资源删除

```C
//
// list_delete - 链表数据销毁操作
// pist     : 指向基础的链表结构
// fdie     : 链表中删除数据执行的方法
// return   : void
//
void 
list_delete_(void ** pist, node_f fdie) {
    if (pist && fdie) {
        // 详细处理链表数据变化
        struct $list * head = *pist;
        while (head) {
            struct $list * next = head->next;
            fdie(head);
            head = next;
        }
        *pist = NULL;
    }
}
```

    list_delete -> list_delete_ 做了 3 件事情
        1' 检查 pist 和 fdie 是否都不为 NULL
        2' 为 list 每个节点执行 fdie 注入函数
        3' *pist = NULL 图个心安

    相似的继续看看 list_add 实现, 直接根据注入函数决定插入的位置

```C
//
// list_next - 获取节点 n 的下一个节点.
// n        : 当前节点
//
#undef  list_next
#define list_next(n) ((struct $list *)(n))->next

//
// list_add - 链表中添加数据, 从小到大 fadd(left, ) <= 0
// pist     : 指向基础的链表结构
// fadd     : 插入数据方法
// left     : 待插入的链表节点
// return   : void
//
void 
list_add_(void ** pist, cmp_f fadd, void * left) {
    struct $list * head;
    if (!pist || !fadd || !left)
        return;
    
    // 看是否是头节点
    head = *pist;
    if (!head || fadd(left, head) <= 0) {
        list_next(left) = head;
        *pist = left;
        return;
    }

    // 不是头节点, 挨个比对
    while (head->next) {
        if (fadd(left, head->next) <= 0)
            break;
        head = head->next;
    }

    // 添加最终的连接关系
    list_next(left) = head->next;
    head->next = left;
}
```

    代码和注释很详细. 额外的 list_next 函数宏很有意思, 不妨多讲点.

```C
//
// list_next - 获取节点 n 的下一个节点.
// n        : 当前节点
//
#define list_next(n) ((void *)((struct $list *)(n))->next)

#undef  list_next
#define list_next(n) ((struct $list *)(n))->next
```

    可以看出它在外部用的时候, 相当于无类型指针, 只能获取值却不能操作. 内部用的时候已经转成了
    类型指针, 就能够操作起来. 算宏控制代码使用权限的一个小技巧. 继续抛砖引玉用宏带大家写 C 
    常量的技巧!

```C
#include <stdio.h>

struct version {
    int major; // 主版本号
    int minor; // 副版本号
    int micro; // 子版本号
};

#ifndef const_version
inline static const struct version const_version(void) {
    return (struct version){ .major = 1, .minor = 2, .micro = 3 };
}
#define const_version const_version()
#endif

int main(void) {
    printf("version = { major = %d, minor = %d, micro = %d }\n", 
        const_version.major, const_version.minor, const_version.micro);
    return 0;
}
```

```bash
$ gcc -g -Wall const_version.c ; ./a.out
version = { major = 1, minor = 2, micro = 3 }
```

    通过宏和 static 函数构造个 const_version const struct version 真常量. 是不是挺有
    意思, 希望读者有所消遣 ~ 
    
    随后看下 list_pop, list_get, list_each 操作, 都很直白.

```C
//
// list_pop - 匹配弹出链表中指定值
// pist     : 指向基础的链表结构
// fget     : 链表中查找数据执行的方法
// left     : 待查找的节点内容 
// return   : 查找到的节点, NULL 表示没有查到 
//
void * 
list_pop_(void ** pist, cmp_f fget, const void * left) {
    struct $list * head, * next;
    if (!pist || fget)
        return NULL;

    // 看是否是头节点
    head = *pist;
    if (fget(left, head) == 0) {
        *pist = head->next;
        return head;
    }

    // 不是头节点挨个处理
    while (!!(next = head->next)) {
        if (fget(left, next) == 0) {
            head->next = next->next;
            return next;
        }
        head = next;
    }

    return NULL;
}
```

    同样分为3部曲, 1检查, 2头节点处理, 3非头节点处理. 需要注意的是 list_pop 只是在 list
    中通过 fget(left, x) 弹出节点. 后续 free or delete 操作还得依赖使用方自行控制.

```C
//
// list_get - 匹配得到链表中指定值
// list     : 基础的链表结构
// fget     : 链表中查找数据执行的方法
// left     : 待查找的节点内容 
// return   : 查找到的节点, NULL 表示没有查到
//
void * 
list_get_(void * list, cmp_f fget, const void * left) {
    if (fget) {
        struct $list * head = list;
        while (head) {
            if (fget(left, head) == 0)
                return head;
            head = head->next;
        }
    }
    return NULL;
}

//
// list_each - 链表循环处理函数, 仅仅测试而已
// list     : 基础的链表结构
// feach    : 处理每个节点行为函数
// return   : void
//
void 
list_each_(void * list, node_f feach) {    
    if (list && feach) {
        struct $list * head = list;
        while (head) {
            struct $list * next = head->next;
            feach(head);
            head = next;
        }
    }
}
```

    list_get 和 list_each 代码是质朴中的质朴. 对于 list_each 还有一种设计思路

```C
//
// each_f - each 循环操作, arg 外部参数, node 是内部节点
// : int echo(void * node, void * arg) { return 0; }
//
typedef int (* each_f)(void * node, void * arg);

extern int list_each(void * list, each_f feach, void * arg);
... ...
	int ret = feach(head, arg);
	if (ret < 0)
		return ret;
... ...
```

    注入 each_f 函数指针, 通过返回值来精细化控制 list_each 执行行为. 但最后还是选择
    了 node_f. 可能最终觉得, 不作才是最好. 不好意思到这 list 设计套路解释完了. 喜欢
	的朋友可以多写几遍代码或博文去体会和分享 ~ 

## 2.2 string

        有句话不知道当讲不当讲, C 中 char * 其实够用了! 也写过几个 string 模型, 自我感觉
    都有些过度封装, 不怎么爽且应用领域很模糊. 但有个方面需要扩展 char []. 举个简单场景说, 
    如果 char * 字符串长度不确定, 随时可能变化. 那我们怎么处理呢? 此刻就需要为其封装个动态
    字符串库去管理这种行为. 为了让大家对 string 体会更多, 先带大家给 string.h 扩展几个小
    接口. 磨刀不费砍柴功.

### 2.2.1 引入 strext.h

        strex.h 是从 string.h 扩展而来, 为什么先引入 strext.h 主要方便后续相关 char *
    操作更加顺利. strext.h 接口设计分成 string 相关操作和 file 相关操作两部分. 

```C
#ifndef _H_STREXT
#define _H_STREXT

/*
    继承 : string.h
    功能 : 扩展 string.h 中部分功能, 方便业务层调用
 */

#include "alloc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//
// str_hash - Brian Kernighan 与 Dennis Ritchie hash 算法
// str      : 字符串内容
// return   : 返回计算后的 hash 值
//
extern unsigned str_hash(const char * str);

//
// str_cpyn - tar 复制内容到 src 中
// src      : 返回保存内容
// tar      : 目标内容
// n        : 最大容量
// return   : 返回字符串长度
//
extern int str_cpyn(char * src, const char * tar, size_t n);

//
// str_cmpi - 字符串不区分大小写比较函数
// ls       : 左串
// rs       : 右串
// return   : ls > rs 返回 > 0; ... < 0; ... =0
//
extern int str_cmpi(const char * ls, const char * rs);

//
// str_cmpin - 字符串不区分小写的限定字符比较函数
// ls       : 左串
// rs       : 右串
// n        : 长度
// return   : ls > rs 返回 > 0; ... < 0; ... =0
//
extern int str_cmpin(const char * ls, const char * rs, size_t n);

//
// str_trim - 去除字符数组前后控制字符
// str      : 待操作的字符数组 \0 结尾
// return   : 返回构建好字符数组首地址
//
extern char * str_trim(char str[]);

//
// str_printf - 字符串构建函数
// format   : 构建格式参照 pritnf
// ...      : 参数集
// return   : char * 堆上内存
//
extern char * str_printf(const char * format, ...);

//
// str_freads - 读取完整个文件内容返回, 需要事后 free
// path     : 文件路径
// return   : 文件内容字符串, NULL 表示读取失败
//
extern char * str_freads(const char * path);

//
// str_fwrites - 将 C 串 str 覆盖写到 path 文件中
// path     : 文件路径
// str      : C 串内容
// return   : >=0 is success, < 0 is error
//
extern int str_fwrites(const char * path, const char * str);

//
// str_fappends - 将 C 串 str 追加写到 path 文件末尾
// path     : 文件路径
// str      : C 串内容
// return   : >=0 is success, < 0 is error
//
extern int str_fappends(const char * path, const char * str);

#endif//_H_STREXT
```

    这个库比较简单, 我就多说点. 大家都懂看得热闹 ~ 

```C
//
// str_hash - Brian Kernighan 与 Dennis Ritchie hash 算法
// str      : 字符串内容
// return   : 返回计算后的 hash 值
//
unsigned 
str_hash(const char * str) {
    register unsigned h = 0u;
    if (str) {
        register unsigned c;
        while ((c = *str++))
            h = h * 131u + c;
    }
    return h;
}
```

    上面是 C 语言之父展示一种 hash 算法, 极其简便快速. 哈希(hash) 相当于一个数学定义上函数,
    f (char *) 映射为 unsigned 数值. 意图是能够通过数值反向确定这个字符串一定程度的相似性.
    思路特别巧妙, 同样也隐含了一个问题, 如果两个串映射一样的值, 那怎么搞. 常用术语叫碰撞, 解
    决碰撞也好搞. 套路不少如链式 hash, 桶式 hash, 混合 hash, 后面会看见相关例子. 那如果发
    生碰撞了怎么办? 假定把保存 hash 值集合的地方叫海藻池子. 一种思路是池子中海藻挤在一起(碰撞
    )了, 就加大池子, 让海藻分开. 原理是池子越大碰撞机会越小. 另一种思路当池子中海藻挤在一块吹
    泡泡的时候, 那我们单独开小水沟把这些吹泡泡的值全引到小水沟中. 思路是碰撞的单独放一起. 对于
    hash 最重要特性是"两个模型映射的哈希值不一样, 那么二者一定不一样!". 通过这个特性在数据查
    找时候一下就能够刷掉一批(非常快速)! 推荐也多查查其它资料, 把 hash 设计和编码都搞明白!!!

```C
//
// str_cpyn - tar 复制内容到 src 中
// src      : 返回保存内容
// tar      : 目标内容
// n        : 最大容量
// return   : 返回字符串长度
//
int 
str_cpyn(char * src, const char * tar, size_t n) {
    size_t i;
    if (!src || !tar || !n) return -2;
    for (i = 1; 
        (i < n) && (*src++ = *tar++); ++i)
        ;
    if (i == n) *src = '\0';
    return (int)i - 1;
}

//
// str_cmpi - 字符串不区分大小写比较函数
// ls       : 左串
// rs       : 右串
// return   : ls > rs 返回 > 0; ... < 0; ... =0
//
int 
str_cmpi(const char * ls, const char * rs) {
    int l, r;
    if (!ls || !rs) return (int)(ls - rs);
    
    do {
        if ((l = *ls++) >= 'A' && l <= 'Z')
            l += 'a' - 'A';
        if ((r = *rs++) >= 'A' && r <= 'Z')
            r += 'a' - 'A';
    } while (l == r && l);
    return l - r;
}

//
// str_cmpin - 字符串不区分小写的限定字符比较函数
// ls       : 左串
// rs       : 右串
// n        : 长度
// return   : ls > rs 返回 > 0; ... < 0; ... =0
//
int 
str_cmpin(const char * ls, const char * rs, size_t n) {
    int l, r;
    if (!ls || !rs || !n) return (int)(ls - rs);

    do {
        if ((l = *ls++) >= 'A' && l <= 'Z')
            l += 'a' - 'A';
        if ((r = *rs++) >= 'A' && r <= 'Z')
            r += 'a' - 'A';
    } while (--n > 0 && l == r && l);
    return l - r;
}
```

    函数写的很直白. 不算"高效"(没有用编译器特定函数, 例如按照字长比较函数). 胜在异常参数处
    理和代码通用, 这也是要写这些函数原因, 不希望传入 NULL 函数就崩溃. 再展示个 trim 功能

```C
//
// str_trim - 去除字符数组前后控制字符
// str      : 待操作的字符数组 \0 结尾
// return   : 返回构建好字符数组首地址
//
char * 
str_trim(char str[]) {
    char * s, * e;
    if (!str || !*str)
        return str;

    // 找到第一个不是空格字符的地址
    for (s = str; isspace(*s); ++s)
        ;

    // 找到最后一个不是空格字符的地址
    e = s + strlen(s) - 1;
    if (isspace(*e)) {
        do --e; while (isspace(*e));
        e[1] = '\0';
    }

    // 开始返回移动后的首地址
    return s == str ? str : memmove(str, s, e - s + 2);
}
```

    主要思路是头尾查找 space 空格字符缩进, 最后 e - s + 2 = (e - s + 1) + 1 最后 1
    是 '\0' 字符. str_trim(char []) 声明设计希望调用方传入个数组参数, 且存在修改行为.
    对于 str_printf 由来先回忆下系统 api

```C
#if defined __USE_ISOC99 || defined __USE_UNIX98
/* Maximum chars of output to write in MAXLEN.  */
extern int snprintf (char *__restrict __s, size_t __maxlen,
		     const char *__restrict __format, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
		      const char *__restrict __format, _G_va_list __arg)
     __THROWNL __attribute__ ((__format__ (__printf__, 3, 0)));
#endif
```

    当初刚学习 snprintf 时候, 总觉得第一个参数好不爽. 凭什么让我来构造 char * 大小. 后面
    看标准说不希望过多揉进动态内存分配平台相关的代码, 内存申请和释放都应该交给使用方. 目前用
    了个很傻技巧来克服 char * 大小不确定

```C
// str_vprintf - 成功直接返回
static char * str_vprintf(const char * format, va_list arg) {
    char buf[BUFSIZ];
    int n = vsnprintf(buf, sizeof buf, format, arg);
    if (n < sizeof buf) {
        char * ret = malloc(n + 1);
        return memcpy(ret, buf, n + 1);
    }
    return NULL;
}

//
// str_printf - 字符串构建函数
// format   : 构建格式参照 printf
// ...      : 参数集
// return   : char * 堆上内存
//
char * 
str_printf(const char * format, ...) {
    char * ret;
    int n, cap;
    va_list arg;
    va_start(arg, format);

    // BUFSIZ 以下内存直接分配
    ret = str_vprintf(format, arg);
    if (ret != NULL)
        return ret;

    cap = BUFSIZ << 1;
    for (;;) {
        ret = malloc(cap);
        n = vsnprintf(ret, cap, format, arg);
        // 失败的情况
        if (n < 0) {
            free(ret);
            return NULL;
        }

        // 成功情况
        if (n < cap)
            break;

        // 内存不足的情况
        free(ret);
        cap <<= 1;
    }

    return realloc(ret, n + 1);
}
```

    核心思路是利用 vsnprintf 返回值判断当前内存是否够用. 愚笨但实在. 

```C
//
// str_freads - 读取完整个文件内容返回, 需要事后 free
// path     : 文件路径
// return   : 文件内容字符串, NULL 表示读取失败
//
char * 
str_freads(const char * path) {
    size_t n, cap, len;
    char * str, buf[BUFSIZ];
    FILE * txt = fopen(path, "rb");
    if (NULL == txt) return NULL;

    // 读取数据
    n = fread(buf, sizeof(char), BUFSIZ, txt);
    if (n == 0 || ferror(txt)) {
        fclose(txt);
        return NULL;
    }

    // 直接分配内存足够直接返回内容
    if (n < BUFSIZ) {
        fclose(txt);
        str = malloc(n + 1);
        memcpy(str, buf, n);
        str[n] = '\0';
        return str;
    }

    str = malloc((cap = n << 1));
    memcpy(str, buf, len = n);
    do {
        n = fread(buf, sizeof(char), BUFSIZ, txt);
        if (ferror(txt)) {
            fclose(txt);
            free(str);
            return NULL;
        }

        // 填充数据
        if (len + n >= cap)
            str = realloc(str, cap <<= 1);
        memcpy(str + len, buf, n);
        len += n;
    } while (n == BUFSIZ);

    // 设置结尾, 并返回结果
    fclose(txt);
    str[len] = '\0';
    return realloc(str, len + 1);
}

// str_fwrite - 按照约定输出数据到文件中
static int str_fwrite(const char * p, const char * s, const char * m) {
    int len;
    FILE * txt;
    if (!p || !*p || !s || !m)
        return -2;
    // 打开文件, 写入消息, 关闭文件
    if (!(txt = fopen(p, m)))
        return -3;

    len = fputs(s, txt);
    fclose(txt);
    // 输出文件长度
    return len;
}

//
// str_fwrites - 将 C 串 str 覆盖写到 path 文件中
// path     : 文件路径
// str      : C 串内容
// return   : >=0 is success, < 0 is error
//
inline int 
str_fwrites(const char * path, const char * str) {
    return str_fwrite(path, str, "wb");
}

//
// str_fappends - 将 C 串 str 追加写到 path 文件末尾
// path     : 文件路径
// str      : C 串内容
// return   : >=0 is success, < 0 is error
//
inline int 
str_fappends(const char * path, const char * str) {
    return str_fwrite(path, str, "ab");
}
```
 
    str_freads 和 str_printf 思路是一样的都在进行内存是否合适的尝试. str_fwrite 结果
    就很普通. 以上关于 string.h 接口扩展部分不华丽, 但是又不可或缺. 适合传授新手演练 ~

### 2.2.2 interface

        经过 strext.h 接口设计, 我们已经回忆起 C string.h 最基础库的部分功能. 趁热打铁
    开始封装一种自带扩容缓冲的字符串模型, 比较好懂. 首先看下面总的接口声明, 有个感性认知. 
    支持堆上和栈上声明使用

```C
#ifndef _H_SIMPLEC_TSTR
#define _H_SIMPLEC_TSTR

#include "struct.h"

#ifndef _STRUCT_TSTR

struct tstr {
	char * str;			// 字符串实际保存的内容
	size_t len;			// 当前字符串长度
	size_t cap;			// 字符池大小
};

// 定义的字符串类型
typedef struct tstr * tstr_t;

#define _STRUCT_TSTR
#endif

//文本串栈上创建内容,不想用那些技巧了,就这样吧
#define TSTR_CREATE(var) \
	struct tstr var[1] = { { NULL } }
#define TSTR_DELETE(var) \
	free((var)->str)

#endif // !_H_SIMPLEC_TSTR
```

    通过 struct tstr 就可以理解作者思路, str存放内容, len记录当前字符个数, cap 表示str能够保存
    的容量. 声明了字符串类型 tstr_t 用于堆上声明, 如果想在栈上声明, 可以用提供的宏. 其实很多编译
	器支持运行期结束自动执行析构操作, 其实只是编译器的语法糖, 编译内嵌析构操作. 例如下面套路

```C
#define TSTR_USING(var, code) \
    do { \
        TSTR_CREATE(var); \
        code \
        TSTR_DELETE(var); \
    } while(0)
```

    模拟函数退栈 pop自动销毁栈上字符串 var变量. C 修炼入门, 至少也会说, 早已看穿, 却入戏太深.
    通过上面结构的构造, 关于行为的部分那就好理解多了. 多扯一点, C中没有'继承'(当然也可以搞)但
	是有文件依赖, 本质是文件继承. 例如上面 #include "struct.h" 表达的意思是 tstr.h 接口文件
	继承 struct.h 接口文件. 强加文件继承关系, 能够明朗化文件包含关系. 那么看后续设计

```C
//
// tstr_t 创建函数, 会根据c的tstr串创建一个 tstr_t结构的字符串
// str		: 待创建的字符串
// len		: 创建串的长度
// return	: 返回创建好的字符串,内存不足会打印日志退出程序
//
extern tstr_t tstr_create(const char * str, size_t len);
extern tstr_t tstr_creates(const char * str);

//
// tstr_t 释放函数
// tstr		: 待释放的串结构
//
extern void tstr_delete(tstr_t tstr);

//
// tstr_expand - 为当前字符串扩容, 属于低级api
// tstr		: 可变字符串
// len		: 扩容的长度
// return	: tstr->str + tstr->len 位置的串
//
char * tstr_expand(tstr_t tstr, size_t len);

//
// 向tstr_t串结构中添加字符等, 内存分配失败内部会自己处理
// c		: 单个添加的char
// str		: 添加的c串
// sz		: 添加串的长度
//
extern void tstr_appendc(tstr_t tstr, int c);
extern void tstr_appends(tstr_t tstr, const char * str);
extern void tstr_appendn(tstr_t tstr, const char * str, size_t sz);

//
// 通过cstr_t串得到一个c的串以'\0'结尾
// tstr		: tstr_t 串
// return	: 返回构建好的c的串, 内存地址tstr->str
//
extern char * tstr_cstr(tstr_t tstr);
```

    还是无外乎创建销毁, 其中 tstr_expand 表示为 tstr扩容操作. 没加 extern 表达的意图是使用
    这个接口要小心, 低等级接口. tstr_cstr 安全的得到 C 类型 char * 串. 当然如果足够自信, 也
    可以直接 tstr->str走起, 安全因人而异. 这个是 C的'自由', 大神在缥缈峰上, 菜鸡在自家坑中.
    对于 tstr_cstr 封装也很直白, 就是看结尾是否有 C的 '\0'

```C
inline char * 
tstr_cstr(tstr_t tstr) {
	// 本质是检查最后一个字符是否为 '\0'
	if (tstr->len < 1 || tstr->str[tstr->len - 1]) {
		tstr_expand(tstr, 1);
		tstr->str[tstr->len] = '\0';
	}

	return tstr->str;
}
```

    一切封装从简, 最好的自然是大道于无为~

#### 2.2.2 tstr implement

    这里详细谈一下 tstr的实现, 首先看最重要的一个接口 tstr_expand 操作内存.
	C 中, 内存有了完全可以为所欲为

```C
inline void 
tstr_delete(tstr_t tstr) {
	free(tstr->str);
	free(tstr);
}

// 文本字符串创建的初始化大小
#define _UINT_TSTR		(32u)

char *
tstr_expand(tstr_t tstr, size_t len) {
	size_t cap = tstr->cap;
	if ((len += tstr->len) >= cap) {
		char * nstr;
		for (cap = cap < _UINT_TSTR ? _UINT_TSTR : cap; cap < len; cap <<= 1)
			;
		// 开始分配内存
		if ((nstr = realloc(tstr->str, cap)) == NULL) {
			tstr_delete(tstr);
			CERR_EXIT("realloc cap = %zu empty!!!", cap);
		}

		// 重新内联内存
		tstr->str = nstr;
		tstr->cap = cap;
	}

	return tstr->str + tstr->len;
}
```
    
	上面关于 cap <<= 1 的做法在内存分配的时候很常见, 一个最小初始值, 幂级别增长. 还有一种封装
	库的时候替代上面写法的操作, 如下(失传已久的装波升级)

```C
// 2 ^ n >= x , 返回 [2 ^ n] 
static inline int _pow2gt(int x) {
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
```

	原理是 2 ^ n - 1 位数全是1, 但不太通用. 对于 realloc返回 NULL的时候, 直接 exit操作也可以
	不需要再次销毁内存了. 以前一位巨擘说过, 进程的事情理应交给操作系统. 想过好久感觉特别有道理.
	后面有机会会统一构建内存管理模块. 再看看添加数据模块实现, 就轻松许多了 ->

```C
inline void 
tstr_appendc(tstr_t tstr, int c) {
	// 这类函数不做安全检查, 为了性能
	tstr_expand(tstr, 1);
	tstr->str[tstr->len++] = c;
}

void 
tstr_appends(tstr_t tstr, const char * str) {
	size_t len;
	if (!tstr || !str) {
		RETURN(NIL, "check '!tstr || !str' param is error!");
	}

	len = strlen(str);
	if(len > 0)
		tstr_appendn(tstr, str, len);
	tstr_cstr(tstr);
}

inline void 
tstr_appendn(tstr_t tstr, const char * str, size_t sz) {
	tstr_expand(tstr, sz);
	memcpy(tstr->str + tstr->len, str, sz);
	tstr->len += sz;
}
```

	思路完全是大白话, [ 还能撑住吗 -> 不能了, 请求支援 -> 能撑住了 -> 继续撑.... ]
	随后看看创建模块

```C
tstr_t 
tstr_create(const char * str, size_t len) {
	tstr_t tstr = calloc(1, sizeof(struct tstr));
	if (NULL == tstr)
		CERR_EXIT("malloc sizeof struct tstr is error!");
	if (str && len > 0)
		tstr_appendn(tstr, str, len);
	return tstr;
}

tstr_t
tstr_creates(const char * str) {
	tstr_t tstr = calloc(1, sizeof(struct tstr));
	if (NULL == tstr)
		CERR_EXIT("malloc sizeof struct tstr is error!");
	if(str)
		tstr_appends(tstr, str);
	return tstr;
}
```

	使用了潜规则 1' struct tstr 为空属性全部要为0; 2' calloc 返回成功后内存会设置为 '0'
	到这里简单的完成了一个C字符串, 准确的说是字符池模块. 顺手展示几个应用

```C
//
// 简单的文件读取类,会读取完毕这个文件内容返回,失败返回NULL.
// path		: 文件路径
// return	: 创建好的字符串内容, 返回NULL表示读取失败
//
tstr_t 
tstr_freadend(const char * path) {
	int err;
	size_t rn;
	tstr_t tstr;
	char buf[BUFSIZ];
	FILE * txt = fopen(path, "rb");
	if (NULL == txt) {
		RETURN(NULL, "fopen r %s is error!", path);
	}

	// 分配内存
	tstr = tstr_creates(NULL);

	// 读取文件内容
	do {
		rn = fread(buf, sizeof(char), BUFSIZ, txt);
		if ((err = ferror(txt))) {
			fclose(txt);
			tstr_delete(tstr);
			RETURN(NULL, "fread err path = %d, %s.", err, path);
		}
		// 保存构建好的数据
		tstr_appendn(tstr, buf, rn);
	} while (rn == BUFSIZ);

	fclose(txt);

	// 继续构建数据, 最后一行补充一个\0
	tstr_cstr(tstr);

	return tstr;
}
```

	上面是一个性能不错的读取文件全部内容的一个子功能. 有了读自然需要有写了, 不妨搞个

```C
static int _tstr_fwrite(const char * path, const char * str, const char * mode) {
	FILE * txt;
	if (!path || !*path || !str) {
		RETURN(ErrParam, "check !path || !*path || !str'!!!");
	}

	// 打开文件, 写入消息, 关闭文件
	if ((txt = fopen(path, mode)) == NULL) {
		RETURN(ErrFd, "fopen mode = '%s', path = '%s' error!", mode, path);
	}
	fputs(str, txt);
	fclose(txt);

	return SufBase;
}

//
// 将c串str覆盖写入到path路径的文件中
// path		: 文件路径
// str		: c的串内容
// return	: SufBase | ErrParam | ErrFd
//
inline int 
tstr_fwrites(const char * path, const char * str) {
	return _tstr_fwrite(path, str, "wb");
}

//
// 将c串str写入到path路径的文件中末尾
// path		: 文件路径
// str		: c的串内容
// return	: SufBase | ErrParam | ErrFd
//
inline int 
tstr_fappends(const char * path, const char * str) {
	return _tstr_fwrite(path, str, "ab");
}
```

	到这 C中字符串辅助模块基本搞定了. string 不是 C必须的, 目前 C的标准更新完全取决于内核层
	的需要. 单纯就目前而言对 C标准支持的最好的是 gcc , 最被期待是 clang, 最水的是 cl!


### 2.3 array

	这里的 array指的是可变数组模块. 但在 C中其实固定数组就够用了! 顺带说一点 C99中支持变量数
	数组, 如下声明本质是编译器帮我们 malloc + free, 但是运行时不可改变大小. 我们这里的 array

```C
int n = 64;
int a[n];
``` 

	支持运行时大小的扩容. 设计原理简单和上面封装 tstr很相似, 只是 char独立单元变成了 void *
	独立单元.

```C
#ifndef _H_SIMPLEC_ARRAY
#define _H_SIMPLEC_ARRAY

#include "struct.h"

struct array {
	void *		as;		/* 存储数组具体内容首地址 */
	unsigned	len;	/* 当前数组的长度 */
	unsigned	size;   /* 当前数组容量大小 */
	size_t		alloc;	/* 每个元素字节大小 */
};

// 定义可变数组类型 对象
typedef struct array * array_t;

#endif//_H_SIMPLEC_ARRAY
```

	array通过注册的 alloc确定数组中每个对象内存模型. 一种原始的反射思路. 上层语言做的
	很多工作就是把编译时转到了运行时. 更高级的魔法直接跳过编译时吟唱阶段瞬发.

#### 2.3.1 array interface

	array 接口设计分为两部分, 核心部分围绕构建删除, 入栈出栈. 

```C
/*
 * 返回创建数组对象
 * size		: 创建数组的总大小个数, 0表示走默认值创建
 * alloc	: 数组中每个元素的字节数, need > 1 否则行为未知
 *			: 返回创建的数组对象
 */
extern array_t array_new(unsigned size, size_t alloc);

/*
 * 销毁这个创建的数组对象
 * a		: 创建的数组对象
 */
extern void array_die(array_t a);

/*
 * 重新构建一个数组对象
 * a		: 可变数组对象
 * size		: 新可变数组总长度
 */
extern void array_init(array_t a, unsigned size);

/*
 * 为可变数组插入一个元素, 并返回这个元素的首地址
 * a		: 可变数组对象
 *			: 返回创建对象位置
 */
extern void * array_push(array_t a);

/*
 * 弹出一个数组元素
 * a		: 可变数组对象
 *			: 返回弹出数组元素节点
 */
extern void * array_pop(array_t a);
```

	上面接口构建的是堆上对象, 如果想构建栈上的对象, 可以采用下面设计

```C
/*
 * 在栈上创建对象var
 * var		: 创建对象名称
 * size		: 创建对象总长度
 * alloc	: 每个元素分配空间大小
 */
#define ARRAY_NEW(var, size, alloc) \
	struct array var[1] = { { NULL, 0, 0, alloc } }; \
	array_init(var, size)
#define ARRAY_DIE(var) \
	free(var->as)
```

	在 C中 数组 [1] 这个技巧本质是为了追求指针对象写法的统一, 全是 -> . 是不是很有意思.
	另外部分辅助接口设计如下

```C
/*
 * 按照索引得到数组元素
 * a		: 可变数组对象
 * idx		: 索引位置
 *			: 返回查询到数据
 */
extern void * array_get(array_t a, unsigned idx);

/*
 * 得到节点elem在数组中索引
 * a		: 可变数组对象
 * elem		: 查询元素
 *			: 返回查询到位置
 */
extern unsigned array_idx(array_t a, void * elem);

/*
 * 得到数组顶的元素
 * a		: 可变数组对象
 *			: 返回得到元素
 */
extern void * array_top(array_t a);

/*
 * 两个数组进行交换
 * a		: 数组a
 * b		: 数组b
 */
extern void array_swap(array_t a, array_t b);

/*
 * 数组进行排序
 * a		: 数组对象
 * compare	: 比对规则
 */
extern void array_sort(array_t a, icmp_f compare);

/*
 * 数组进行遍历
 * a		: 可变数组对象
 * func		: 执行每个节点函数, typedef int	(* each_f)(void * node, void * arg);
 * arg		: 附加参数
 *			: 返回操作结果状态码
 */
int array_each(array_t a, each_f func, void * arg);
```

	结构是设计的内在, 内功循环的套路. 接口是设计的外在, 发招后内力外在波动.
	而实现就是无数次重复磨炼~

#### 2.3.2 array implement

	情不知所起，一往而深.
	这里继续扯编程实现, 同样的, 9成库打头阵的都是内存管理这块. 

```C
#define _INT_ARRAY_SIZE		(16)		// 数组的默认大小

array_t 
array_new(unsigned size, size_t alloc) {
	struct array * a = malloc(sizeof(struct array));
	assert(NULL != a);
	// 指定默认size大小
	size = size ? size : _INT_ARRAY_SIZE;
	a->as = malloc(size * alloc);
	assert(NULL != a->as);
	a->len = 0;
	a->size = size;
	a->alloc = alloc;

	return a;
}

inline void 
array_die(array_t a) {
	if (a) {
		free(a->as);
		free(a);
	}
}

inline void
array_init(array_t a, unsigned size) {
	assert(NULL != a);
	a->as = realloc(a->as, size * a->alloc);
	assert(NULL != a->as);
	if (a->len > size)
		a->len = size;
	a->size = size;
}
```

	思路很朴实那里需要内存那里就向服务器申请内存. 随后展示 push 和 pop 设计

```C
inline void * 
array_push(array_t a) {
	assert(NULL != a);

	if (a->len == a->size) {
		/* the array is full; allocate new array */
		a->size <<= 1;
		a->as = realloc(a->as, a->size * a->alloc);
		assert(NULL != a->as);
	}

	return (unsigned char *)a->as + a->alloc * a->len++;
}

inline void * 
array_pop(array_t a) {
	assert(NULL != a && 0 != a->len);
	--a->len;
	return (unsigned char *)a->as + a->alloc * a->len;
}
```

	代码思索多了, 也就更朴实了. 
	毕竟是剖析, 上面有个争议的地方关于 realloc 用法, 写个装波的教科书用法:

```C
char * str = malloc(sizeof int);
if (NULL == str) {
	return NULL;
}

char * nstr = realloc(str, sizeof long long);
if (NULL == nstr) {
	free(nstr);
	return NULL;
}

str = nstr;
```

	核心围绕 realloc 存在返回 NULL, 泄漏内存的风险. 3年内程序猿喜欢这么搞. 除了定式
	范式, 其它部分就随意了. 最上面做法造成结果会依操作系统而定, 另一个名词统称
	`未定义行为'. 后面看一下辅助代码的设计就异常简单:

```C
inline void * 
array_get(array_t a, unsigned idx) {
	assert(NULL != a && idx < a->len);
	return (unsigned char *)a->as + a->alloc * idx;
}

inline unsigned 
array_idx(array_t a, void * elem) {
	unsigned char * p, * q;
	unsigned off;

	assert(NULL != a && elem >= a->as);

	p = a->as;
	q = elem;
	off = (unsigned)(q - p);

	assert(off % (unsigned)a->alloc == 0);

	return off / (unsigned)a->alloc;
}

inline void * 
array_top(array_t a) {
	assert(NULL != a && 0 != a->len);
	return (unsigned char *)a->as + a->alloc * (a->len - 1);
}
```

	当然了, 越是经过筛选的好东西, 理应很顺很清晰.
	最后一弹, 抽象行为, 自定义动作

```C
inline 
void array_swap(array_t a, array_t b) {
	struct array t = *a;
	*a = *b;
	*b = t;
}

inline void 
array_sort(array_t a, icmp_f compare) {
	assert(NULL != a && 0 != a->len && NULL != compare);
	qsort(a->as, a->len, a->alloc, (int (*)(const void *, const void *))compare);
}

int 
array_each(array_t a, each_f func, void * arg) {
	int rt;
	unsigned char * s, * e;

	assert(NULL != a && NULL != func);

	s = a->as;
	e = s + a->alloc * a->len;
	while (s < e) {
		rt = func(s, arg);
		if (SufBase != rt)
			return rt;
		s += a->alloc;
	}

	return SufBase;
}
```

	顺带扯一点, 对于编程而言, 尽量少 typedef, 多 struct 写全称. 谎言需要另一个谎言来弥补.
	多大量的用标准中推出的解决方案. 例如 多用标准提供的跨平台类型, stdint.h and stddef.h 
	定义全平台类型. 不妨将这么多年习得的无上秘法, 血之限界 - 血轮眼 - 不懂的代码抄几遍, 
	一切如梦如幻! 回到正题, 真没必要多说. 神来一笔就是 if (SufBase != rt) return rt; 
	关于 array的内功学会之后, 可以自己写写单元测试, 融会贯通一下. 
	有篇幅的话会写个单元测试框架, 供参考.

### 2.4 hash

	我们这里讲述的 hash比较针对, 对数值进行哈希映射(想起一个分析名词收敛). 应用场景也多, 
	例如内核层给应用层的句柄id. 无法对其规律进行假设, 那我们把它映射到特定的范围内. 就很
	控制了. 封装系统 io复用的时候很常见.
	下面展示一个 hashid的封装, 原始思路来自云风大佬的 skynet c gate server 上设计

```C
#ifndef _H_SIMPLEC_HASHID
#define _H_SIMPLEC_HASHID

#include <assert.h>
#include <stdlib.h>

struct nodeid {
	int id;
	struct nodeid * next;
};

struct hashid {
	int mod;
	int cap;
	int cnt;
	struct nodeid * ids;
	struct nodeid ** hash;
};

static void hashid_init(struct hashid * hi, int max) {
	int hcap = 16;
	assert(hi && max > 1);
	while (hcap < max)
		hcap *= 2;

	hi->mod = hcap - 1;
	hi->cap = max;
	hi->ids = malloc(max * sizeof(struct nodeid));
	hi->cnt = 0;
	for (int i = 0; i < max; ++i) 
		hi->ids[i] = (struct nodeid) { -1, NULL };
	
	hi->hash = calloc(hcap, sizeof(struct hashid *));
}

static void inline hashid_clear(struct hashid * hi) {
	free(hi->ids); 	hi->ids = NULL;
	free(hi->hash); hi->hash = NULL;
	hi->mod = 1;
	hi->cap = 0;
	hi->cnt = 0;
}

static int hashid_lookup(struct hashid * hi, int id) {
	int h = id & hi->mod;
	struct nodeid * c = hi->hash[h];
	while (c) {
		if (c->id == id)
			return c - hi->ids;
		c = c->next;
	}
	return -1;
}

static int hashid_remove(struct hashid * hi, int id) {
	int h = id & hi->mod;
	struct nodeid * c = hi->hash[h];
	if (c == NULL)
		return -1;

	if (c->id == id) {
		hi->hash[h] = c->next;
		goto _clear;
	}
	while (c->next) {
		if (c->next->id == id) {
			struct nodeid * temp = c->next;
			c->next = temp->next;
			c = temp;
			goto _clear;
		}
		c = c->next;
	}

_clear:
	c->id = -1;
	c->next = NULL;
	--hi->cnt;
	return c - hi->ids;
}

static int hashid_insert(struct hashid * hi, int id) {
	struct nodeid * c = NULL;
	for (int i = 0; i < hi->cap; ++i) {
		int idx = (i + id) % hi->cap;
		if (hi->ids[idx].id == -1) {
			c = hi->ids + idx;
			break;
		}
	}
	assert(c && c->next == NULL);

	++hi->cnt;
	c->id = id;
	int h = id & hi->mod;
	if (hi->hash[h])
		c->next = hi->hash[h];
	hi->hash[h] = c;
	return c - hi->ids;
}

static inline int hashid_full(struct hashid * hi) {
	return hi->cnt >= hi->cap;
}

#endif//_H_SIMPLEC_HASHID
```

	代码比注释值钱.
	一般书中也许会有习题, 我们这里当成一篇阅读理解. 哈哈. 来一同感受设计的细节. 有些朴实, 
	有些飘逸, 有些巧妙. 下面先阅读完讲解一点潜规则, 先入为主

* 0' -> 	return -1;

		没有找见就返回索引 -1, 作为默认错误和 POSIX 默认错误码相同. POSIX 错误码引入了
		errno机制, 不太好, 封装过度. 上层需要二次判断, 开发起来要命. 可能我们后续设计
		思路也是采用这种 POSIX思路, 但愿是 上错花轿嫁对郎.

* 1' -> int hcap = 16;  

		这个 hcap 初始值必须是2的幂数, 方便得到 hi->mod = 2 ^ x - 1 = hcap - 1

* 2' -> hi->hash = calloc(hcap, sizeof(struct hashid *));  

		这里表明当前 hash实体已经全部申请好了, 只能用这些了. 所以有了 hashid_full 接口.

* 3' -> assert(c && c->next == NULL);  

		这个在 hashid_insert中, 表明意思是插入一定成功. 那么这个接口必须在 hashid_full
		之后执行. 

* 4' -> int idx = (i + id) % hi->cap;  

		一种查找策略, 可以有也可以无. 和 O(n) 纯 for 查找没啥区别. 看数据随机性.

* 5' -> 最后小总结
		
		通过以上就好理解了. 上面我们构建的一种 hashid api, 完成的工作就是方便 
		int id的映射工作. 查找急速, 实现上采用的是桶算法. 映射到固定空间上索引. 
		写一遍想一遍就能感受到那些游动于指尖的美好~

### 2.5 内功心法上卷结构

	渔家傲·平岸小桥千嶂抱
	王安石·宋

	平岸小桥千嶂抱，
	柔蓝一水萦花草。
	茅屋数间窗窈窕，
	尘不到，
	时时自有春风扫。

	午枕觉来闻语鸟，
	欹眠似听朝鸡早。
	忽忆故人今总老，
	贪梦好，
	茫然忘了邯郸道。

![云飘宁](./img/七彩祥云.jpg)