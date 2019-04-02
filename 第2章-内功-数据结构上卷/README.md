# 第2章-内功-数据结构上卷

        对于 C 而言, 数据结构不硬, 都将成为美丽的泡沫. 其他语言好点, 标准对结构算法支持
    的很好用(中庸). 重复说, 在 C 的世界里, 数据结构和操作系统是硬通货. 而数据结构就是核
    心内功, 一招一式全得实锤. 修炼数据结构本质是为了掌控全局, 细节规划, 捋顺输入输出. 
    而内功没有几个月苦练, 很难实现外放. 上卷我们只讲简单一点 list, string, array, 
    hash 等数据结构.

## 2.1 list

        基于数据结构是 C 内功说法, 毫无疑问那链市结构就是数据结构的两大基础内丹之一 (链
    式结构和顺序结构). 而链式结构最原始抽象模型就是链表 list, 极其普通和实用.

![list](img/list.jpg)

    上图是最简单的一种 list 结构布局, next 指向下一个结点的指针. 自己对于 list 结构的
    理解比较简单, list 是个实体, 并且这个实体还能找到他保存的下一个实体. 随后会为 
    list 构建部分接口. 学习一个陌生的东西有很多套路, 一条烂大街的大众路线图是:
	write demo -> see interface -> copy implement -> test

### 2.1.1 list interface

```C
#ifndef _LIST_H
#define _LIST_H

#include "struct.h"

//
// list.h 通用单链表库, 魔鬼的步伐
// $LIST 必须嵌入在 struct 中第一行
// void * list = NULL 创建 list 对象
// [可选] list_delete(list, fide) 删除操作
//
struct $list {
    struct $list * next;
};

#define $LIST struct $list $node;

//
// list_next - 获取结点 n 的下一个结点
// n        : 当前结点
#define list_next(n) ((void *)((struct $list *)(n))->next)

//
// list_each - 链表循环处理, feach(x)
// list     : 链表对象
// feach    : node_f 结点遍历行为
// return   : void
//
extern void list_each(void * list, void * feach);

//
// list_delete - 链表数据销毁操作, fdie(x)
// pist     : void ** 指向链表对象指针
// fdie     : node_f 链表中删除数据行为
// return   : void
//
extern void list_delete(void * pist, void * fdie);

//
// list_add - 链表中添加数据, 升序 fadd(left, x) <= 0
// pist     : void ** 指向链表对象指针
// fadd     : cmp_f 链表中插入数据方法
// left     : 待插入的链表结点
// return   : void
//
extern void list_add(void * pist, void * fadd, void * left);

//
// list_get - 查找到链表中指定结点值, fget(left, x) == 0
// list     : 链表对象
// fget     : cmp_f 链表中查找数据行为
// left     : 待查找的辅助数据 
// return   : 查找到的结点, NULL 表示没有查到
//
extern void * list_get(void * list, void * fget, const void * left);

//
// list_pop - 弹出链表中指定结点值, fget(left, x) == 0
// pist     : void ** 指向链表对象指针
// fget     : cmp_f 链表中查找数据行为
// left     : 待查找的辅助数据 
// return   : 查找到的结点, NULL 表示没有查到 
//
extern void * list_pop(void * pist, void * fget, const void * left);

#endif//_LIST_H
```

    其中很多 void * 用于消除编译器警告. 在很久之前也使用过宏来消除警告.

```C
//
// list_add - 链表中添加数据, 从小到大 fadd(left, ) <= 0
// list     : 链表对象
// pist     : void ** 指向链表对象指针
// fadd     : 插入数据方法
// left     : 待插入的链表结点
// return   : void
//
#define list_add(list, fadd, left)                                      \
list_add_((void **)&(list), (cmp_f)(fadd), (void *)(intptr_t)(left))
extern void list_add_(void ** pist, cmp_f fadd, void * left);
```

    最后选择了 void * 全面替代 define. 取舍在于性能不变基础上, 为了调试更方便. 而对于
    继承的头文件 struct.h 可以参看第1章设计部分, 这里轻微回顾一下.

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

    对于 struct $list { struct $list * next; }; 链式结构的设计模式, 可以稍微思考一下
    , 等同于内存级别的继承. $ 符号希望标识当前结构是私有的, 具备特定用途, 使用要谨慎, 
    要知道全貌. 下面我们用上面 list 提供的接口原型, 构建 people list 演示例子

```C
struct people {
$LIST               // $LIST 必须在开头位置
    int    free;    // 有理想
    char * ideal;   // 有文化
    double future;  // 有秩序
};

static inline int people_add_cmp(const struct people * n, 
                                 const struct people * r) {
    return n->free - r->free;
}

static inline int people_pop_cmp(double * future, 
                                 const struct people * r) {
    return *future != r->future;
}

// 创建
struct people * list = NULL;
struct people p = { .free = 100, .ideal = "59", .future = 0.0 };

// 添加
list_add(&list, people_add_cmp, &p);

// 删除和返回结点自己负责善后
double future = 6.6;
struct people * e = list_pop(&list, people_pop_cmp, &future);

// 销毁 - 栈上数据无需额外删除, list_delete 缺省
```

    来看下 struct people { $LIST ... }; 结构内在实现的全部

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

// 下面的关系将会成立 &people == &people::$node
(void *)&p == (void *)&(p.$node)

// 因而由 &p 地址可以确定 $node 地址, 因而也获得了 $node 内部的 $next
((struct $list *)&p)->$next
```

    读者可以画画写写感受哈, list 过于基础, 解释太多没有自己抄写 10 几类链表源码来的实在
    . 用 C 写业务, 几乎都是围绕 list 相关结构的增删改查. 而对于封装代码库基本套路是三思
    而后行, 想出大致思路, 定好基本接口, 再堆实现. 设计出优雅好用的接口, 是第一位. 在 C 
    中思路落地等同于数据结构定型. 后续实现相关代码就已经妥了! 最后就是 Debug 和 Unit 
    test 来回倒腾一段时间.

### 2.1.2 list implements

        这里将分析设计思路. 在设计一个库的时候, 首要考虑的是创建和删除(销毁), 事关生存
    周期的问题. list 创建比较简单采用了一个潜规则 void * list = NULL; 代表创建一个空
    链表. 链表头创建设计常见有两个套路, 其一是自带实体头结点, 这种思路在处理头结点的时
    候特别方便. 其二就是我们这种没有头结点一开始为从 NULL 开始, 优势在于节省空间. 对于
    链表的销毁删除操作, 使用 list_delete

```C
//
// list_delete - 链表数据销毁操作, fdie(x)
// pist     : void ** 指向链表对象指针
// fdie     : node_f 链表中删除数据行为
// return   : void
//
void 
list_delete(void * pist, void * fdie) {
    if (pist && fdie) {
        // 详细处理链表数据变化
        struct $list * head = *(void **)pist;
        while (head) {
            struct $list * next = head->next;
            ((node_f)fdie)(head);
            head = next;
        }
        *(void **)pist = NULL;
    }
}
```

    list_delete 做了 3 件事情
        1' 检查 pist 和 fdie 是否都不为 NULL
        2' 为 list 每个结点执行 fdie 注入的行为
        3' *(void **)pist = NULL 图个心安

    相似的继续看 list_add 实现, 直接通过注入函数决定插入的位置

```C
//
// list_next - 获取结点 n 的下一个结点
// n        : 当前结点
#undef  list_next
#define list_next(n) ((struct $list *)(n))->next

//
// list_add - 链表中添加数据, 升序 fadd(left, x) <= 0
// pist     : void ** 指向链表对象指针
// fadd     : cmp_f 链表中插入数据方法
// left     : 待插入的链表结点
// return   : void
//
void 
list_add(void * pist, void * fadd, void * left) {
    if (!pist || !fadd || !left)
        return;
    
    // 看是否是头结点
    struct $list * head = *(void **)pist;
    if (!head || ((cmp_f)fadd)(left, head) <= 0) {
        list_next(left) = head;
        *(void **)pist = left;
        return;
    }

    // 不是头结点, 挨个比对
    while (head->next) {
        if (((cmp_f)fadd)(left, head->next) <= 0)
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
// list_next - 获取结点 n 的下一个结点
// n        : 当前结点
#define list_next(n) ((void *)((struct $list *)(n))->next)

#undef  list_next
#define list_next(n) ((struct $list *)(n))->next
```

    可以看出他在外部用的时候, 相当于无类型指针, 只能获取值难于设值. 内部用的时候已经转为类
    型指针, 就可以操作了. 算是宏控制代码使用权限的一个小技巧. 继续抛砖引玉用宏带大家构造 C
    的常量技巧!

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

    通过宏和声明定义 static 函数, 构造 const_version const struct version 真常量. 
    是不是挺有意思, 希望读者有所消遣 ~ 
    
    随后看下 list_pop, list_get, list_each 操作, 都很直白.

```C
//
// list_pop - 弹出链表中指定结点值, fget(left, x) == 0
// pist     : void ** 指向链表对象指针
// fget     : cmp_f 链表中查找数据行为
// left     : 待查找的辅助数据 
// return   : 查找到的结点, NULL 表示没有查到 
//
void * 
list_pop(void * pist, void * fget, const void * left) {
    if (!pist || !*(void **)pist || !fget) 
        return NULL;

    // 看是否是头结点
    struct $list * head = *(void **)pist;
    if (((cmp_f)fget)(left, head) == 0) {
        *(void **)pist = head->next;
        return head;
    }

    // 不是头结点挨个处理
    for (struct $list * next; (next = head->next); head = next) {
        if (((cmp_f)fget)(left, next) == 0) {
            head->next = next->next;
            return next;
        }
    }

    return NULL;
}
```

    同样有 3 部曲, 1 检查, 2 头结点处理, 3 非头结点处理. 需要注意的是 list_pop 只是在 
    list 中通过 fget(left, x) 弹出结点. 后续 free or delete 操作还得依赖使用方自行控
    制.

```C
//
// list_get - 查找到链表中指定结点值, fget(left, x) == 0
// list     : 链表对象
// fget     : cmp_f 链表中查找数据行为
// left     : 待查找的辅助数据 
// return   : 查找到的结点, NULL 表示没有查到
//
void * 
list_get(void * list, void * fget, const void * left) {
    if (fget) {
        struct $list * head = list;
        while (head) {
            if (((cmp_f)fget)(left, head) == 0)
                return head;
            head = head->next;
        }
    }
    return NULL;
}

//
// list_each - 链表循环处理, feach(x)
// list     : 链表对象
// feach    : node_f 结点遍历行为
// return   : void
//
void 
list_each(void * list, void * feach) {    
    if (list && feach) {
        struct $list * head = list;
        while (head) {
            struct $list * next = head->next;
            ((node_f)feach)(head);
            head = next;
        }
    }
}
```

    list_get 和 list_each 代码是质朴中的质朴啊. 其中 list_each 还有一种设计思路

```C
//
// each_f - 遍历行为, node 是内部结点, arg 是外部参数
// : int echo(void * node, void * arg) { return 0; }
//
typedef int (* each_f)(void * node, void * arg);

int 
list_each(void * list, void * feach, void * arg) {
    if (list && feach) {
        struct $list * head = list;
        while (head) {
            struct $list * next = head->next;
            int ret = ((each_f)feach)(head, arg);
            if (ret < 0)
                return ret;
            head = next;
        }
    }
    return 0;
}
```

    注入 each_f 函数指针, 通过返回值来精细化控制 list_each 执行行为. 但最后还是选择了 
    node_f. 最终觉得, 不想再作了. 不好意思到这 list 设计套路解释完了. 喜欢的朋友可以多写
    几遍代码去体会和分享 ~ 

## 2.2 string

        有句话不知道当讲不当讲, C 中 char * 其实够用了! 写过几个 string 模型, 自我觉得
    都有些过度封装, 不怎么爽且应用领域很鸡肋. 但开发中也有场景需要扩展 char *. 例如说, 如
    果 char * 字符串长度不确定, 随时可能变化. 那我们怎么处理呢? 这会就需要为其封装个动态
    字符集的库去管理这种行为. 为了让大家对 string 体会更多, 先带大家为 string.h 扩展几
    个小接口. 磨刀不费砍柴功.

### 2.2.1 引入 strext.h

        strext.h 是基于 string.h 扩展而来, 先引入 strext.h 目的是方便后续相关 char * 
    操作. strext.h 功能设计分成 string 相关操作和 file 相关操作. 

```C
#ifndef _STREXT_H
#define _STREXT_H

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

#endif//_STREXT_H
```

    这个库比较简单, 我就多说点. 大家都懂看得也热闹 ~ 

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

    str_hash 延续了 C 语言之父展示一种极其简便快速 hash 算法实现. 哈希(hash)映射相当于
    定义数学上一个函数, f (char *) 映射为 unsigned 数值. 意图通过数值一定程度上反向确定
    这个字符串. 思路特别巧妙. 同样也隐含了一个问题, 如果两个串映射一样的值, 那怎么搞. 常用
    术语叫碰撞, 解决碰撞也好搞. 套路不少有桶式 hash, 链式 hash, 混合 hash(后面会看见相
    关例子). 回到问题, 即如果发生碰撞了后续怎么办? 假设把保存 hash 值集合的地方叫海藻池子
    . 一种思路是当池子中海藻挤在一起(碰撞)了, 就加大池子, 让海藻分开, 原理是池子越大碰撞机
    会越小. 另一种思路当池子中海藻挤在一块吹泡泡的时候, 那我们单独开小水沟把这些吹泡泡的海
    藻全引流到小水沟中, 思路是碰撞的单独放一起. 而对于 hash 最重要特性是"两个模型映射的哈
    希值不一样, 那么二者一定不一样!". 通过这个特性在数据查找时能够快速刷掉一批! 推荐也多查
    查其他资料, 把 hash 设计和编码仔细分析明白!!

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

    函数写的很普通. 不算"高效"(没采用编译器特定函数优化, 例如按照字长比较函数). 胜在有异常
    参数处理, 这也是要写这些函数原因, 不希望传入 NULL 就崩溃. 再展示 trim 函数.

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

    主要思路是在字符串头尾都查找 space 字符并缩进, 最后 e - s + 2 = (e + 1 - s) + 1 
    (最后 1 是 '\0' 字符). str_trim(char []) 声明设计希望调用方传入参数是数组, 且允许
    修改行为. 开始讲 str_printf 之前, 先回忆下系统 api

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
    后面看标准说, 不希望过多揉进动态内存分配平台相关的代码, 内存申请和释放都应该交给使用方.
    我们这里用了个很低效的傻技巧来动态生成 char *

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

    str_freads 和 str_printf 思路一样, 都在进行内存是否合适的尝试. str_fwrite 设计仅
    仅对系统的文件输出函数包装一下. 以上关于 string.h 接口扩展部分不华丽, 但是又不可或缺.
    适合传授新手, 带其上手 ~

### 2.2.2 tstr interface

        经过 strext.h 接口演练, 已经可以回忆起 C string.h 基础库的部分功能. 趁热打铁开
    始封装一类自带扩容缓冲的字符串模型, 比较好懂. 首先看总的接口声明, 有个感性认知. 
    tstr.h 支持堆上和栈上声明使用

```C
#ifndef _TSTR_H
#define _TSTR_H

#include "strext.h"

#ifndef TSTR_CREATE

struct tstr {
    char * str;   // 字符池
    size_t cap;   // 容量
    size_t len;   // 长度
};

// TSTR_INT 字符串构建的初始大小
#define TSTR_INT  (1<<8)

typedef struct tstr * tstr_t;

//
// TSTR_CREATE - 栈上创建 tstr_t 结构
// TSTR_DELETE - 释放栈上 tstr_t 结构
// var  : 变量名
//
#define TSTR_CREATE(var)                \
struct tstr var[1] = { {                \
    .str = malloc(TSTR_INT),            \
    .cap = TSTR_INT,                    \
} }

#define TSTR_DELETE(var)                \
free((var)->str)

#endif//TSTR_CREATE

#endif//_TSTR_H
```

    通过 struct tstr 就能猜出作者思路, str 存放内容, len 记录当前字符长度, cap 表示字
    符池容量. 声明字符串类型 tstr_t 用于堆上声明. 如果想在栈上声明, 可以用提供的操作宏. 
    其实很多编译器支持运行期结束自动析构操作, 只是编译器的语法糖, 内嵌析构操作. 类比下面套
    路(编译器协助开发者插入 free or delete 代码), 模拟自动退栈销毁栈上字符串 var 变量

```C
#define TSTR_USING(var, code)           \
do {                                    \
	TSTR_CREATE(var);                   \
	code                                \
	TSTR_DELETE(var);                   \
} while(0)
```

    C 修炼入门, 有的会说, 虽早已看穿, 却入戏太深. 有了上面数据结构, 关于行为的部分代码那就
    好理解多了. 多扯一点, C 中没有'继承'(当然也可以搞)但是有文件依赖, 也像是文件继承. 例
    如上面 #include "strext.h" 表达的意思是 tstr.h 接口文件继承 strext.h 接口文件. 
    强加文件继承关系, 能够明朗文件包含关系拊顺脉络. 继续看后续设计

```C
//
// tstr_delete - tstr_t 释放函数
// tsr      : 待释放的串结构
// return   : void
//
extern void tstr_delete(tstr_t tsr);

//
// tstr_expand - 字符串扩容, low level api
// tsr      : 可变字符串
// len      : 扩容的长度
// return   : tsr->str + tsr->len 位置的串
//
char * tstr_expand(tstr_t tsr, size_t len);

//
// tstr_t 创建函数, 根据 C 串创建 tstr_t 字符串
// str      : 待创建的字符串
// len      : 创建串的长度
// return   : 返回创建的字符串
//
extern tstr_t tstr_create(const char * str, size_t len);
extern tstr_t tstr_creates(const char * str);

//
// tstr_t 串结构中添加字符等
// tsr      : tstr_t 串
// c        : 添加 char
// str      : 添加 char *
// sz       : 添加串的长度
// return   : void
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
    在自家泥河里. 对于 tstr_cstr 封装很直白, 就是看结尾是否有 C 的 '\0'.

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

### 2.2.3 tstr implement

        详细谈一下 tstr 的实现, 首先看最重要的一个接口 tstr_expand 操作内存. C 中只
    要内存有了, 完全可以为所欲为.

```C
//
// tstr_expand - 字符串扩容, low level api
// tsr      : 可变字符串
// len      : 扩容的长度
// return   : tsr->str + tsr->len 位置的串
//
char * 
tstr_expand(tstr_t tsr, size_t len) {
    size_t cap = tsr->cap;
    if ((len += tsr->len) > cap) {
        // 走 1.5 倍内存分配, '合理'降低内存占用
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
   
	上面 cap = cap * 3 / 2 的做法, 在内存分配的时候很常见, 初始值加幂级别增长. 也有
    按照分阶段增长, 不同解决增长系数不一样. 最初版本增长用的是 cap <<= 1, 并用了位操作
    , 一个失传已久的装波升级技巧.

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

	原理是 2 ^ n - 1 位数全是1, 1 -> 11 -> 1111 -> 11111111 -> ... (不太通用) 

```C
//
// tstr_t 串结构中添加字符等
// tsr      : tstr_t 串
// c        : 添加 char
// str      : 添加 char *
// sz       : 添加串的长度
// return   : void
//
inline void 
tstr_appendc(tstr_t tsr, int c) {
    // 这类函数不做安全检查, 为了性能
    tstr_expand(tsr, 1);
    tsr->str[tsr->len++] = c;
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

inline void 
tstr_appendn(tstr_t tsr, const char * str, size_t sz) {
    tstr_expand(tsr, sz);
    memcpy(tsr->str + tsr->len, str, sz);
    tsr->len += sz;
}
```

	思路完全是大白话, [ 还能撑住吗 -> 不能, 请求支援 -> 援兵赶到 -> 继续.... ]
	随后看看创建模块

```C
//
// tstr_t 创建函数, 根据 C 串创建 tstr_t 字符串
// str      : 待创建的字符串
// len      : 创建串的长度
// return   : 返回创建的字符串
//
inline tstr_t 
tstr_create(const char * str, size_t len) {
    tstr_t tsr = malloc(sizeof(struct tstr));
    tsr->str = malloc(TSTR_INT);
    tsr->cap = TSTR_INT;
    tsr->len = 0;
    if (str && len) tstr_appendn(tsr, str, len);
    return tsr;
}

inline tstr_t 
tstr_creates(const char * str) {
    tstr_t tsr = malloc(sizeof(struct tstr));
    tsr->str = malloc(TSTR_INT);
    tsr->cap = TSTR_INT;
    tsr->len = 0;
    if (str) tstr_appends(tsr, str);
    return tsr;
}
```

	存在潜规则 1' struct tstr 初始属性要求全为 0. 2' calloc 返回成功的内存会填充 0.
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

	tstr_dupstr 用于 tstr_t 到 char * 转换. tstr_popup 操作会在 str 头部弹出特定
    长度字符, 可用于协议解析模块. 再赠送大家个 tstr_printf 用于 tstr sprintf 操作

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
// tstr_printf - 参照 sprintf 方式填充内容
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

	到这 C 字符串辅助模块也大致搞定. string 不是 C 必须的, 有时候在特定场景会很舒服. 
	这么久, 也可以看出 C 写代码方式是 [数据结构设计 -> 内存处理设计 -> 业务设计]. 而
    大多数现代语言写代码方式只需要关心 [业务设计]. 硬要对比的话, 存在性能和生产力成反比
    相关性规律. 

## 2.3 array

	    array 指的是动态数组. C 中同样固定数组就够用了! 顺带说一点 C99 中新加变长
    (变量)数组. 如下声明, 本质是编译器运行时帮我们在栈上分配和释放, 但不能改变长度. 

```C
int n = 64;
int array[n];
``` 

	这里要说的 array, 支持运行时容量扩容. 设计原理与上面封装 tstr 很相似, 只是 char 
	独立单元变成了 void * 独立单元.

```C
#ifndef _ARRAY_H
#define _ARRAY_H

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
// ARRAY_UINT   - 数组初始化默认大小
#define ARRAY_UINT      (1u<<5)
#define ARRAY_CREATE(type, var)             \
struct array var[1] = { {                   \
    sizeof(type),                           \
    ARRAY_UINT,                             \
    0,                                      \
    malloc(sizeof(type) * ARRAY_UINT)       \
} }

#define ARRAY_DELETE(var)                   \
free((var)->data)

#endif//_ARRAY_H
```

	在 C 中数组 [1] 这个技巧主要为了追求指针对象写法的统一, 全用 -> 去操作. 是不是有点
    意思. 其中 struct array 通过注册的 size 确定数组中每个对象模型内存大小, 是一种很
    原始的反射套路. 高级语言做的很多工作就是把原本编译时做的事情转到了运行时. 更现代化的
    魔法直接跳过编译时吟唱阶段而瞬发.

### 2.3.1 array interface

	    array 接口设计分为两部分, 第一部分是核心围绕创建, 删除, 压入, 弹出. 第二部分
    是应用围绕 array 结构做一些辅助操作. 

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

	上面是第一部分核心接口设计, 下面第二部分外围接口设计

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
// array_idx - 通过结点返回索引
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
 
	结构是设计的内在, 内功循环的丹田. 接口是设计的外在, 发招后内力外放波动. 而实现将是
    无数次重复锤炼 ~

### 2.3.2 array implement

	    情不知所起，一往而深. 这里继续扯编程实现, 同样 9 成多库打头阵还是内存管理这块. 

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
    a->cap = ARRAY_UINT;
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

//
// array_pop - 数组中弹出一个数据
// a        : 动态数组对象
// return   : void * 返回数据首地址
//
inline void * 
array_pop(array_t a) {
    assert(NULL != a && a->len > 0);
    --a->len;
    return (char *)a->data + a->size * a->len;
}
```

	代码写来写去, 也就那点东西了. 

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
// array_idx - 通过结点返回索引
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

	顺带补充点, 对于编程而言, 尽量少 typedef, 多 struct 写全称. 谎言需要另一个谎言来
    弥补. 并且多用标准中推出的解决方案. 例如标准提供的 stdint.h 和 stddef.h 定义全平
    台类型. 不妨传大家我这么多年习得的无上秘法, 开 血之限界 -> 血轮眼 -> 不懂装懂, 抄
    抄抄. 一切如梦如幻! 回到正题. 再带大家写个很傻的单元测试, 供参考. 有篇幅的话会带大
    家写个简单的单元测试功能设计.

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

## 2.4 hash

	    这里讲述的 hash 针对性很强, 对数值进行哈希映射(想起一个分析名词收敛). 应用场景
    也多, 例如内核层给应用层的句柄 id. 无法对其规律进行假设, 那我们把他映射到特定的范围
    内, 通过映射值去控制. 封装系统 io 复用层的时候很常见. 下面展示一个 hash id 相关库
    封装. 原始思路来自云风大佬的 skynet c gate server 上设计

```C
#ifndef _HAID_H
#define _HAID_H

#include <assert.h>
#include <stdlib.h>

struct noid {
    struct noid * next;
    int id;
};

struct haid {
    struct noid ** hash;
    struct noid * set;
    int mod;
    int cap;
    int len;
};

static void haid_init(struct haid * h, int max) {
    int cap = sizeof(struct haid);
    assert(h && max >= 0);
    while (cap < max)
        cap <<= 1;

    for (int i = 0; i < max; ++i)
        h->set[i] = (struct noid) { NULL, -1 };
    h->hash = calloc(cap, sizeof(struct haid *));
    assert(h->hash && cap);

    h->set = malloc(max * sizeof(struct noid));
    assert(h->set && max);

    h->mod = cap - 1;
    h->cap = max;
    h->len = 0;
}

static inline void haid_clear(struct haid * h) {
    free(h->hash); h->hash = NULL;
    free(h->set); h->set = NULL;
    h->mod = 1;
    h->len = h->cap = 0;
}

static int haid_lookup(struct haid * h, int id) {
    struct noid * c = h->hash[id & h->mod];
    while (c) {
        if (c->id == id)
            return c - h->set;
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
        goto ret_clr;
    }

    while (c->next) {
        if (c->next->id == id) {
            struct noid * tmp = c->next;
            c->next = tmp->next;
            c = tmp;
            goto ret_clr;
        }
        c = c->next;
    }

ret_clr:
    c->id = -1;
    c->next = NULL;
    --h->len;
    return c - h->set;
}

static int haid_insert(struct haid * h, int id) {
    int i;
    struct noid * c = NULL;
    for (i = 0; i < h->cap; ++i) {
        int j = (i + id) % h->cap;
        if (h->set[j].id == -1) {
            c = h->set + j;
            break;
        }
    }
    assert(c && c->next == NULL);

    ++h->len;
    c->id = id;
    i = id & h->mod;
    if (h->hash[i])
        c->next = h->hash[i];
    h->hash[i] = c;
    return c - h->set;
}

static inline int haid_full(struct haid * h) {
    return h->len >= h->cap;
}

#endif//_HAID_H
```

	代码比注释值钱. 一般书中也许会有习题, 我们这里独创"阅读理解". 哈哈. 来一同感受设计
    的细节. hsid 库设计就是这次的阅读理解, 有些飘逸, 有些巧妙. 阅读完后讲解一点潜规则,
    先入为主

    0' return -1;

		没有找见就返回索引 -1, 作为默认错误和 POSIX 默认错误码相同. POSIX 错误码引入
        了 errno 机制, 不太好, 封装过度. 上层需要二次判断, 开发起来难受. 我们后续设计
        思路也是承接这种 POSIX 思路, 但愿上错花轿嫁对郎.

    1' cap <<= 1;

		这个 cap 初始值必须是 2 的幂数, 方便得到 h->mod = 2 ^ x - 1 = cap - 1

    2' h->hash = calloc(cap, sizeof(struct haid *));

		这里表示, 当前 hash 实体已经全部申请好了, 只能用这些了. 所以有了 haid_full 
        接口.

    3' assert(c && c->next == NULL);  

		代码在 haid_insert 中出现, 表明插入一定会成功. 那么这个接口必须在 haid_full 
		之后执行. 

    4' int j = (i + id) % h->cap;

		一种查找策略, 可以有也可以无. 和 O(n) 纯 for 查找没啥区别. 看数据随机性.

    5' 小总结
		
        有了这些阅读理解会容易点. 上面构建的 hash id api, 完成的工作就是方便 int id
        的映射工作. 查找急速, 实现上采用的是桶算法. 映射到固定空间上索引. 写一遍想一遍
        就能感受到那些游动于指尖的美好 ~

## 2.5 内功心法上卷

***

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

***

![云飘宁](./img/我猜中了前头.jpg)
