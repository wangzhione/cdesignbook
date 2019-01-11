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

    通过宏和定义 static 函数构造 const_version const struct version 真常量. 是不是
	挺有意思, 希望读者有所消遣 ~ 
    
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
... .. .
	int ret = feach(head, arg);
	if (ret < 0)
		return ret;
... .. .
```

    注入 each_f 函数指针, 通过返回值来精细化控制 list_each 执行行为. 但最后还是选择
    了 node_f. 可能最终觉得, 不作才是最好. 不好意思到这 list 设计套路解释完了. 喜欢
	的朋友可以多写几遍代码去体会和分享 ~ 

## 2.2 string

        有句话不知道当讲不当讲, C 中 char * 其实够用了! 写过几个 string 模型, 自我感
	觉都有些过度封装, 不怎么爽且应用领域很鸡肋. 但也有场景需要扩展 char *. 例如说, 如果 
	char * 字符串长度不确定, 随时可能变化. 那我们怎么处理呢? 此刻就需要为其封装个动态字
	符串库去管理这种行为. 为了让大家对 string 体会更多, 先带大家为 string.h 扩展几个小
    接口. 磨刀不费砍柴功.

### 2.2.1 引入 strext.h

        strex.h 是从 string.h 扩展而来, 为什么先引入 strext.h 主要方便后续相关 char *
    操作更加顺利. strext.h 接口设计分成 string 相关操作和 file 交互 string 相关操作. 

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
// format   : 构建格式参照 printf
// ...      : 参数集
// return   : char * 堆上内存
//
extern char * str_printf(const char * format, ...);

//
// str_freads - 读取整个文件内容返回, 需要事后 free
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

    上面是 C 语言之父展示一种极其简便快速 hash 算法. 哈希(hash) 映射相当于定义一个数学上函
	数, f (char *) 映射为 unsigned 数值. 意图是通过数值反向确定这个字符串一定程度的相似性.
    思路特别巧妙. 同样也隐含了一个问题, 如果两个串映射一样的值, 那怎么搞. 常用术语叫碰撞, 解
    决碰撞也好搞. 套路不少如链式 hash, 桶式 hash, 混合 hash, 后面会看见相关例子. 即如果发
    生碰撞了后续怎么办? 假设把保存 hash 值集合的地方叫海藻池子. 一种思路是池子中海藻挤在一起
	(碰撞)了, 就加大池子, 让海藻分开. 原理是池子越大碰撞机会越小. 另一种思路当池子中海藻挤在
	一块吹泡泡的时候, 那我们单独开小水沟把这些吹泡泡的值全引流到小水沟中. 思路是碰撞的单独放一
	起. 对于 hash 最重要特性是"两个模型映射的哈希值不一样, 那么二者一定不一样!". 通过这个特性
	在数据查找时能够非常快速刷掉一批! 推荐也多查查其它资料, 把 hash 设计和编码仔细搞明白!!

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
    if (!src || !tar || !n) return EParam;
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

    函数写的很普通. 不算"高效"(没有用编译器特定函数, 例如按照字长比较函数). 胜在异常参数
	处理和代码朴实, 这也是要写这些函数原因, 不希望传入 NULL 就崩溃. 再展示 trim 函数

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

    主要思路是头尾都查找 space 字符并缩进, 最后 e - s + 2 = (e + 1 - s) + 1 最后 1
    是 '\0' 字符. str_trim(char []) 声明设计希望调用方传入参数是数组, 且允许修改行为.
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

    当初刚学习 snprintf 时候, 总觉得第一个参数好不爽. 凭什么让我来预先构造 char * 大小. 
	后面看标准说不希望过多揉进动态内存分配平台相关的代码, 内存申请和释放都应该交给使用方. 目
	前用了个很傻技巧来克服 char * 大小不确定

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
// str_freads - 读取整个文件内容返回, 需要事后 free
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
        return EParam;
    // 打开文件, 写入消息, 关闭文件
    if (!(txt = fopen(p, m)))
        return EFd;

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
 
    str_freads 和 str_printf 思路一样, 都在进行内存是否合适的尝试. str_fwrite 设计
    就很普通. 以上关于 string.h 接口扩展部分不华丽, 但是又不可或缺. 适合传授新手演练 ~

### 2.2.2 interface

        经过 strext.h 接口设计, 已经可以回忆起 C string.h 基础库的部分功能. 趁热打铁
    开始封装一种自带扩容缓冲的字符串模型, 比较好懂. 首先看下面总的接口声明, 有个感性认知. 
    tstr.h 支持堆上和栈上声明使用

```C
#ifndef _H_TSTR
#define _H_TSTR

#include "strext.h"

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

#endif//_H_TSTR
```

    通过 struct tstr 就可以理解作者思路, str 存放内容, len 记录当前字符长度, cap 表示字符池
	容量. 声明了字符串类型 tstr_t 用于堆上声明. 如果想在栈上声明, 可以用提供的宏. 其实很多编译
	器支持运行期结束自动析构操作, 其实只是编译器的语法糖, 内嵌析构操作. 例如下面套路

```C
#define TSTR_USING(var, code)								\
do { 														\
	TSTR_CREATE(var); 										\
	code 													\
	TSTR_DELETE(var); 										\
} while(0)
```

    模拟函数退栈自动销毁栈上字符串 var 变量. C 修炼入门, 多数会说, 早已看穿, 却入戏太深.
    有了上面数据结构结构, 关于行为的部分代码那就好理解多了. 多扯一点, C 中没有'继承'(当然也可
	以搞)但是有文件依赖, 本质是文件继承. 例如上面 #include "strext.h" 表达的意思是 tstr.h 
	接口文件继承 strext.h 接口文件. 强加文件继承关系, 能够明朗化文件包含关系. 那么看后续设计

```C
//
// tstr_delete - tstr_t 释放函数
// tsr      : 待释放的串结构
// return   : void
//
extern void tstr_delete(tstr_t tsr);

//
// tstr_expand - 为当前字符串扩容, 属于低级api
// tsr      : 可变字符串
// len      : 扩容的长度
// return   : tsr->str + tsr->len 位置的串
//
char * tstr_expand(tstr_t tsr, size_t len);

//
// tstr_t 创建函数, 根据 C 的 str 串创建 tstr_t 字符串
// str      : 待创建的字符串
// len      : 创建串的长度
// return   : 返回创建好的字符串,内存不足会打印日志退出程序
//
extern tstr_t tstr_create(const char * str, size_t len);
extern tstr_t tstr_creates(const char * str);

//
// 向 tstr_t 串结构中添加字符等, 内存分配失败内部会自己处理
// c        : 单个添加的char
// str      : 添加的 C 串
// sz       : 添加串的长度
//
extern void tstr_appendc(tstr_t tsr, int c);
extern void tstr_appends(tstr_t tsr, const char * str);
extern void tstr_appendn(tstr_t tsr, const char * str, size_t sz);

//
// tstr_cstr - 通过 str_t 串得到一个 C 串以'\0'结尾
// tsr      : tstr_t 串
// return   : 返回构建 C 串, 内存地址 tsr->str
//
extern char * tstr_cstr(tstr_t tsr);
```

    还是无外乎创建销毁, 其中 tstr_expand 表示为 tstr 扩容操作. 没加 extern 表达的意图
	是使用这个低等级接口要小心. tstr_cstr 安全的得到 C 类型 char * 串. 当然如果足够自信
	, 也可以直接 tstr->str 走起, 安全因人而异. 这个是 C 的'自由', 大神在缥缈峰上, 菜鸡
	在自家坑中. 对于 tstr_cstr 封装也很直白, 就是看结尾是否有 C 的 '\0'

```C
//
// tstr_cstr - 通过 str_t 串得到一个 C 串以'\0'结尾
// tsr      : tstr_t 串
// return   : 返回构建 C 串, 内存地址 tsr->str
//
inline char * 
tstr_cstr(tstr_t tsr) {
    if (tsr->len < 1u || tsr->str[tsr->len - 1]) {
        tstr_expand(tsr, 1u);
        tsr->str[tsr->len] = '\0';
    }
    return tsr->str;
}
```

    一切封装从简, 最好的自然 ~ 大道于无为 ~

#### 2.2.2 tstr implement

    这里详细谈一下 tstr 的实现, 首先看最重要的一个接口 tstr_expand 操作内存. C 中只要
	内存有了, 完全可以为所欲为.

```C
// INT_TSTR - 字符串构建的初始化大小
#define INT_TSTR  (1<<8)

//
// tstr_expand - 为当前字符串扩容, 属于低级api
// tsr      : 可变字符串
// len      : 扩容的长度
// return   : tsr->str + tsr->len 位置的串
//
char * 
tstr_expand(tstr_t tsr, size_t len) {
    size_t cap = tsr->cap;
    if ((len += tsr->len) > cap) {
        // 走 1.5 倍内存分配, '合理'降低内存占用
        cap = cap < INT_TSTR ? INT_TSTR : cap;
        while (cap < len) cap = cap * 3 / 2;
        tsr->str = realloc(tsr->str, cap);
        tsr->cap = cap;
    }
    return tsr->str + tsr->len;
}

//
// tstr_delete - tstr_t 释放函数
// tsr      : 待释放的串结构
// return   : void
//
inline void 
tstr_delete(tstr_t tsr) {
    free(tsr->str);
    free(tsr);
}
```
    
	上面关于 cap = cap * 3 / 2 的做法在内存分配的时候很常见, 初始值加幂级别增长. 最初
	版本用的是 cap <<= 1, 并用了位操作失传已久的装波升级技巧

```C
// pow2gt - 2 ^ n >= x , 返回 [2 ^ n] 
static inline int pow2gt(int x) {
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
```

	原理是 2 ^ n - 1 位数全是1, 1 -> 11 -> 1111 -> 11111111 -> ... 但不太通用. 

```C
//
// 向 tstr_t 串结构中添加字符等, 内存分配失败内部会自己处理
// c        : 单个添加的char
// str      : 添加的 C 串
// sz       : 添加串的长度
//
inline void 
tstr_appendc(tstr_t tsr, int c) {
    // 这类函数不做安全检查, 为了性能
    tstr_expand(tsr, 1);
    tsr->str[tsr->len++] = c;
}

inline void 
tstr_appendn(tstr_t tsr, const char * str, size_t sz) {
    tstr_expand(tsr, sz);
    memcpy(tsr->str + tsr->len, str, sz);
    tsr->len += sz;
}

inline void 
tstr_appends(tstr_t tsr, const char * str) {
    if (tsr && str) {
        unsigned sz = (unsigned)strlen(str);
        if (sz > 0)
            tstr_appendn(tsr, str, sz);
        tstr_cstr(tsr);
    }
}
```

	思路完全是大白话, [ 还能撑住吗 -> 不能, 请求支援 -> 能撑住了 -> 继续撑.... ]
	随后看看创建模块

```C
//
// tstr_t 创建函数, 根据 C 的 str 串创建 tstr_t 字符串
// str      : 待创建的字符串
// len      : 创建串的长度
// return   : 返回创建好的字符串,内存不足会打印日志退出程序
//
inline tstr_t 
tstr_create(const char * str, size_t len) {
    tstr_t tsr = calloc(1, sizeof(struct tstr));
    if (str && len) tstr_appendn(tsr, str, len);
    return tsr;
}

inline tstr_t 
tstr_creates(const char * str) {
    tstr_t tsr = calloc(1, sizeof(struct tstr));
    if (str) tstr_appends(tsr, str);
    return tsr;
}
```

	存在潜规则 1' struct tstr 属性全部要求为空(0); 2' calloc 返回成功的内存会填充 0
	到这简单完成了一个 C 字符串, 准确的说是字符池模块. 顺手展示几个应用

```C
//
// tstr_dupstr - 得到 C 的串, 需要自行 free
// tsr      : tstr_t 串
// return   : 返回创建好的 C 串
//
inline char * 
tstr_dupstr(tstr_t tsr) {
    if (tsr && tsr->len >= 1) {
        // 构造内存, 返回最终结果
        size_t len = tsr->len + !!tsr->str[tsr->len - 1];
        char * str = malloc(len * sizeof(char));
        memcpy(str, tsr->str, len - 1);
        str[len - 1] = '\0';
        return str;
    }
    return NULL;
}

//
// tstr_popup - 字符串头弹出 len 长度字符
// tsr      : 可变字符串
// len      : 弹出的长度
// return   : void
//
inline void 
tstr_popup(tstr_t tsr, size_t len) {
    if (len >= tsr->len)
        tsr->len = 0;
    else {
        tsr->len -= len;
        memmove(tsr->str, tsr->str + len, tsr->len);
    }
}
```

	tstr_dupstr 用于 tstr_t 到 char * 转换. tstr_popup 头部弹出特定长度字符操作, 可用于
	协议解析模块. 随后带大家写个 tstr_printf 用于 tstr sprintf 操作

```C
// tstr_vprintf - BUFSIZ 以下内存处理
static int tstr_vprintf(tstr_t tsr, const char * fmt, va_list arg) {
    char buf[BUFSIZ];
    int len = vsnprintf(buf, sizeof buf, fmt, arg);
    if (len < sizeof buf) {
        // 合法直接构建内存返回
        if (len > 0)
            tstr_appendn(tsr, buf, len);
        tstr_cstr(tsr);
        va_end(arg);
    }
    return len;
}

//
// tstr_printf - 参照 sprintf 填充方式写入内容
// tsr      : tstr_t 串
// fmt      : 待格式化的串
// ...      : 等待进入的变量
// return   : 返回创建的 C 字符串内容
//
char * 
tstr_printf(tstr_t tsr, const char * fmt, ...) {
    int n, cap;
    va_list arg;
    va_start(arg, fmt);

    // 初步构建失败直接返回
    cap = tstr_vprintf(tsr, fmt, arg);
    if (cap < BUFSIZ)
        return tsr->str;
    
    // 开始详细构建内存
    do {
        char * ret = malloc(cap <<= 1);
        n = vsnprintf(ret, cap, fmt, arg);
        // 内存足够就开始填充, 以备结束
        if (n < cap && n > 0)
            tstr_appendn(tsr, ret, n);
        free(ret);
    } while (n < cap);

    va_end(arg);
    return tstr_cstr(tsr);
}
```

	到这 C 字符串辅助模块基本搞定了. string 不是 C 必须的, 有的话在特定场景会很舒服.
	从中可以看出 C 写代码方式是 数据结构设计 -> 内存处理设计 -> 业务设计. 而多数现代
	语言在多数只需要处理 业务设计. 总结就是性能和生产力成反比相关性. 

### 2.3 array

	array 指的是可变数组模块. 但在 C 中其实固定数组就够用了! 顺带说一点 C99 中支持变
	量数数组, 如下声明本质是编译器帮我们在栈上分配和释放, 但是运行时不可改变大小. 

```C
int n = 64;
int array[n];
``` 

	我们这里的 array, 支持运行时大小的扩容. 设计原理和上面封装 tstr 很相似, 只是 char 
	独立单元变成了 void * 独立单元.

```C
#ifndef _H_ARRAY
#define _H_ARRAY

#include "struct.h"

struct array {
    unsigned size;      // 元素大小
    unsigned cap;       // 数组容量
    unsigned len;       // 数组长度
    void *  data;       // 数组存储
};

// array_t - 动态数组类型
typedef struct array * array_t;

//
// ARRAY_CREATE - 栈上创建动态数组对象
// ARRAY_DELETE - 销毁栈上动态数组对象
// var          : 创建动态数组对象名字
// UINT_AINIT   - 数组初始化默认大小
#define UINT_AINIT      (1u<<5)
#define ARRAY_CREATE(type, var)             \
struct array var[1] = { {                   \
    sizeof(type),                           \
    UINT_AINIT,                             \
    0,                                      \
    malloc(sizeof(type) * UINT_AINIT)       \
} }

#define ARRAY_DELETE(var)                   \
free((var)->data)

#endif//_H_ARRAY
```

	在 C 中数组 [1] 这个技巧本质是为了追求指针对象写法的统一, 全是 -> 去调用. 是不是有点
	意思. 其中 struct array 通过注册的 size 确定数组中每个对象模型内存大小, 是一种很原
	始的反射思路. 高级语言做的很多工作就是把原本编译时做的事情转到了运行时. 更现代化的魔法
	直接跳过编译时吟唱阶段而瞬发.

#### 2.3.1 array interface

	array 接口设计分为两部分, 第一部分是核心围绕创建, 删除, 入栈, 出栈. 第二部分是外围
	围绕 array 做一些事情. 

```C
//
// array_create - 返回创建动态数组对象
// size     : 元素大小
// return   : 返回创建的动态数组对象
//
extern array_t array_create(unsigned size);

//
// array_delete - 销毁动态数组对象
// a        : 动态数组对象 
// return   : void
//
extern void array_delete(array_t a);

//
// array_push - 数组中插入一个数据
// a        : 动态数组
// return   : void * 压入数据首地址
//
extern void * array_push(array_t a);

//
// array_pop - 数组中弹出一个数据
// a        : 动态数组
// return   : void * 返回数据首地址
//
extern void * array_pop(array_t a);
```

	上面是第一部分核心接口设计, 其中第二部分外围接口设计如下

```C
//
// array_top - 得到动态数组顶元素
// a        : 动态数组
// return   : 得到返回动态数组顶部元素
//
extern void * array_top(array_t a);

//
// array_get - 索引映射数组元素
// a        : 动态数组
// idx      : 索引位置
// return   : NULL is not found
//
extern void * array_get(array_t a, unsigned idx);

//
// array_idx - 通过节点返回索引
// a        : 动态数组
// elem     : 查询元素
// return   : 索引
//
extern unsigned array_idx(array_t a, void * elem);

//
// array_swap - 动态数组交换
// a        : 动态数组
// b        : 动态数组
// return   : void
//
extern void array_swap(array_t a, array_t b);

//
// array_sort - 动态数组排序
// a        : 动态数组
// fcmp     : 数组元素的比较函数
// return   : void
//
extern void array_sort(array_t a, cmp_f fcmp);

//
// array_each - 动态数组遍历
// a        : 动态数组
// func     : 遍历行为
// return   : >= 0 表示成功, < 0 表示失败
//
extern int array_each(array_t a, each_f func, void * arg);
```

	结构是设计的内在, 内功循环的丹田. 接口是设计的外在, 发招后内力外在波动. 
	而实现就是无数次重复锤炼 ~

#### 2.3.2 array implement

	情不知所起，一往而深.
	这里还是继续扯编程实现, 同样的 9 成库打头阵的都是内存管理这块. 

```C
//
// array_create - 返回创建动态数组对象
// size     : 元素大小
// return   : 返回创建的动态数组对象
//
inline array_t 
array_create(unsigned size) {
    struct array * a = malloc(sizeof(struct array));
    assert(NULL != a && size > 0);
    // set default cap size
    a->cap = UINT_AINIT;
    a->data = malloc(size * a->cap);
    a->size = size;
    a->len = 0;
    return a;
}

//
// array_delete - 销毁动态数组对象
// a        : 动态数组对象 
// return   : void
//
inline void 
array_delete(array_t a) {
    if (a) {
        free(a->data);
        free(a);
    }
}
```

	不忍直视, 那里要内存就那里向服务器申请要内存. 随后展示 push 和 pop 设计

```C
//
// array_delete - 销毁动态数组对象
// a        : 动态数组对象 
// return   : void
//
inline void 
array_delete(array_t a) {
    if (a) {
        free(a->data);
        free(a);
    }
}

//
// array_push - 数组中插入一个数据
// a        : 动态数组对象
// return   : void * 压入数据首地址
//
inline void * 
array_push(array_t a) {
    if (a->len >= a->cap) {
        /* the array is full; allocate new array */
        a->cap <<= 1;
        a->data = realloc(a->data, a->cap * a->size);
    }

    return (char *)a->data + a->size * a->len++;
}
```

	代码写来写去, 也就有点朴实无华了. 

```C
//
// array_top - 得到动态数组顶元素
// a        : 动态数组
// return   : 得到返回动态数组顶部元素
//
inline void * 
array_top(array_t a) {
    assert(NULL != a && a->len > 0);
    return (char *)a->data + a->size * (a->len - 1);
}

//
// array_get - 索引映射数组元素
// a        : 动态数组
// idx      : 索引位置
// return   : NULL is not found
//
inline void * 
array_get(array_t a, unsigned idx) {
    assert(NULL != a && idx < a->len);
    return (char *)a->data + a->size * idx;
}

//
// array_idx - 通过节点返回索引
// a        : 动态数组
// elem     : 查询元素
// return   : 索引
//
inline unsigned 
array_idx(array_t a, void * elem) {
    unsigned off = (unsigned)((char *)elem - (char *)a->data);
    assert(a && elem >= a->data && off % a->size == 0);
    return off / a->size;
}

//
// array_swap - 动态数组交换
// a        : 动态数组
// b        : 动态数组
// return   : void
//
inline void 
array_swap(array_t a, array_t b) {
    struct array t = *a;
    *a = *b; 
    *b = t;
}
```

	当然了, 越是经过筛选的好东西, 理应很顺很清晰. 最后一弹, 抽象行为, 自定义动作

```C
//
// array_sort - 动态数组排序
// a        : 动态数组
// fcmp     : 数组元素的比较函数
// return   : void
//
inline void 
array_sort(array_t a, cmp_f fcmp) {
    assert(NULL != a && a->len && fcmp != NULL);
    qsort(a->data, a->len, a->size, 
        (int (*)(const void *, const void *))fcmp);
}

//
// array_each - 动态数组遍历
// a        : 动态数组
// func     : 遍历行为
// return   : >= 0 表示成功, < 0 表示失败
//
int 
array_each(array_t a, each_f func, void * arg) {
    assert(NULL != a && func != NULL);
    char * s = a->data;
    char * e = s + a->size * a->len;
    while (s < e) {
        int ret = func(s, arg);
        if (ret < 0)
            return ret;
        s += a->size;
    }

    return 0;
}
```

	顺带扯一点, 对于编程而言, 尽量少 typedef, 多 struct 写全称. 谎言需要另一个谎言来弥补.
	并且多用标准中推出的解决方案. 例如标准提供的跨平台类型, stdint.h 和 stddef.h 定义全平
	台类型. 不妨传大家我这么多年习得的无上秘法, 开 血之限界 -> 血轮眼 -> 不懂装懂, 抄抄抄. 
	一切如梦如幻! 回到正题. 再带大家写个很傻的单元测试, 供参考. 有篇幅的话会带大家写个单元
	测试框架.

```C
#include <array.h>

// array_test - array test
void array_test(void) {
    // 构建一个在栈上的动态数组
    ARRAY_CREATE(double, a);

    // 开始处理数据
    *(double *)array_push(a) = 1.1234;
    *(double *)array_push(a) = 2.2345;
    *(double *)array_push(a) = 4.9876;

    // 输出数据
    printf("v = %lf\n", *(double *)array_pop(a));
    printf("v = %lf\n", *(double *)array_pop(a));
    printf("v = %lf\n", *(double *)array_pop(a));

    ARRAY_DELETE(a);
}
```

### 2.4 hash

	这里讲述的 hash 比较针对, 对数值进行哈希映射(想起一个分析名词收敛). 应用场景也多, 
	例如内核层给应用层的句柄 id. 无法对其规律进行假设, 那我们把它映射到特定的范围内. 
	就很控制了. 封装系统 io 复用的时候很常见. 下面展示一个 hash id 相关库封装. 原始
	思路来自云风大佬的 skynet c gate server 上设计

```C
#ifndef _H_HAID
#define _H_HAID

#include <assert.h>
#include <stdlib.h>

struct noid {
    struct noid * next;
    int id;
};

struct haid {
    struct noid ** hash;
    struct noid * ids;
    int mod;
    int cap;
    int cnt;
};

static void haid_init(struct haid * h, int max) {
    int cap = sizeof(struct haid);
    assert(h && max >= 0);
    while (cap < max)
        cap <<= 1;

    for (int i = 0; i < max; ++i)
        h->ids[i] = (struct noid) { NULL, -1 };
    h->hash = calloc(cap, sizeof(struct haid *));
    assert(h->hash && cap);

    h->ids = malloc(max * sizeof(struct noid));
    assert(h->ids && max);

    h->mod = cap - 1;
    h->cap = max;
    h->cnt = 0;
}

static inline void haid_clear(struct haid * h) {
    free(h->hash); h->hash = NULL;
    free(h->ids); h->ids = NULL;
    h->mod = 1;
    h->cnt = h->cap = 0;
}

static int haid_lookup(struct haid * h, int id) {
    struct noid * c = h->hash[id & h->mod];
    while (c) {
        if (c->id == id)
            return c - h->ids;
        c = c->next;
    }
    return -1;
}

static int haid_remove(struct haid * h, int id) {
    int i = id & h->mod;
    struct noid * c = h->hash[i];
    if (NULL == c)
        return -1;

    if (c->id == id) {
        h->hash[i] = c->next;
        goto out_clr;
    }

    while (c->next) {
        if (c->next->id == id) {
            struct noid * tmp = c->next;
            c->next = tmp->next;
            c = tmp;
            goto out_clr;
        }
        c = c->next;
    }

out_clr:
    c->id = -1;
    c->next = NULL;
    --h->cnt;
    return c - h->ids;
}

static int haid_insert(struct haid * h, int id) {
    int i;
    struct noid * c = NULL;
    for (i = 0; i < h->cap; ++i) {
        int j = (i + id) % h->cap;
        if (h->ids[j].id == -1) {
            c = h->ids + j;
            break;
        }
    }
    assert(c && c->next == NULL);

    ++h->cnt;
    c->id = id;
    i = id & h->mod;
    if (h->hash[i])
        c->next = h->hash[i];
    h->hash[i] = c;
    return c - h->ids;
}

static inline int haid_full(struct haid * h) {
    return h->cnt >= h->cap;
}

#endif//_H_HAID
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
