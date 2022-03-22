<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [第2章-内功-数据结构上卷](#第2章-内功-数据结构上卷)
  - [2.1 list](#21-list)
    - [2.1.1 list interface](#211-list-interface)
    - [2.1.2 list implements](#212-list-implements)
  - [2.2 string](#22-string)
    - [2.2.1 包装 string.h => strext.h](#221-包装-stringh-strexth)
    - [2.2.2 tstr interface](#222-tstr-interface)
    - [2.2.3 cstr implement](#223-cstr-implement)
  - [2.3 array](#23-array)
  - [2.4 两篇阅读理解](#24-两篇阅读理解)
    - [2.4.1 stack 设计](#241-stack-设计)
    - [2.4.2 id hash 设计](#242-id-hash-设计)
  - [2.5 拓展阅读](#25-拓展阅读)
  - [2.6 展望](#26-展望)

<!-- /code_chunk_output -->
# 第2章-内功-数据结构上卷

对于 C 而言, 数据结构不熟练, 很难不是美丽的泡沫. 其他语言好点, 标准或者框架中对结构算法有很好用(中庸)的支持. 重复说, 在 C 的世界里, 数据结构和操作系统是硬通货. 其中数据结构就是核心内功, 一招一式全得实锤. 修炼数据结构本质是为了掌握业务世界和编程世界沟通单元, 规划细节, 捋顺输入输出. 而关于数据结构内功没有几个月苦练, 很难实现外放得心应手. 上卷我们只讲简单一点 list, string, array, stack, hash 等类型的数据结构.

## 2.1 list

基于数据结构是 C 内功说法, 毫无疑问那链表结构就是数据结构的两大基础基石之一 (链式结构和顺序结构). 而链式结构最原始抽象模型就是链表 list, 极其普通和实用.

![list](img/list.jpg)

上图是最简单的一种 list 结构布局, next 指向下一个结点的指针. 对于 list 结构的理解比较简单, list 是个实体, 并且这个实体还能找到他保存的下一个实体. 随后会为 list 构建部分接口. 学习一个陌生的东西有很多套路, 一条烂大街的大众路线图是:

> write demo -> see interface -> copy implement -> test -> source code

### 2.1.1 list interface

**list.h**

```C
#pragma once

#include "struct.h"

//
// list.h 似魔鬼的步伐, 单链表库
// $LIST 需要嵌入 struct 的第一行
// void * list = NULL;      //        create list
// list_delete(list, fide); // [可选] delete list
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

```

其中很多 void * 用于消除编译器警告. 也可以用宏操作来消除警告, 不过宏最好能不用就不要用宏. 毕竟宏调试不是很方便. 而对于继承的头文件 struct.h 可以参看第1章设计部分, 这里轻微回顾一下.

```C
#ifndef CMP_F
#define CMP_F

//
// cmp_f - 比较行为 > 0 or = 0  or < 0
// : int add_cmp(const void * now, const void * node)
//
typedef int (* cmp_f)();

#endif//CMP_F

#ifndef NODE_F
#define NODE_F

//
// node_f - 销毁行为
// : void list_die(void * node)
//
typedef void (* node_f)();

#endif//NODE_F
```

对于 **struct $list { struct $list * next; };** 链式结构的设计方式, 可以稍微思考一下, 等同于内存级别的继承. $ 符号希望标识当前结构是私有的, 使用要谨慎, 需要知道其内存的全貌. 下我们用上面 list 提供的接口原型, 构建 people list 演示例子

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

来看下 **struct people { $LIST ... };** 结构内在实现的全部

```C
// struct people 结构全展开
struct people {
    struct $list {
        struct $list * next;
    } $node;        // 真面目

    int    free;    // 有理想
    char * ideal;   // 有文化
    double future;  // 有秩序
};

// p 为 struct people 结构 
struct people p = { .free = 100, .ideal = "59", .future = 0.0 };
```

> 下面的关系将会成立 &people == &people::$node
(void *)&p == (void *)&(p.$node)

> 因而由 &p 地址可以确定 $node 地址, 因而也获得了 $node 内部的 $next
((struct $list *)&p)->$next

读者可以画画写写感受哈, list 过于基础, 解释太多没有自己抄写 10 几类链表源码来的实在. 用 C 写业务, 几乎都是**围绕 list 相关结构的增删改查**. 

后续封装代码库基本套路整体是三思而后行, 想出大致思路, 定好基本接口, 再堆实现. 设计出优雅好用的接口, 是第一位. 在 C 中思路落地表现是基本的数据结构定型. 后续实现相关代码实现就已经妥了! 最后就是 Debug 和 Unit test 来回倒腾一段时间.

### 2.1.2 list implements

我们用 C 设计一个库的时候, 首要考虑的是**创建**和**删除(销毁)**, 事关**生命周期**的问题. list 创建比较简单采用了一个潜规则 **void * list = NULL;** 代表创建一个空链表. 链表头创建设计常见有两个套路, 其一是自带实体头结点, 这种思路在处理头结点的时候特别方便. 其二就是我们这种没有头结点一开始为从 NULL 开始, 优势在于节省空间. 对于这类链表的销毁删除操作, 使用 **list_delete**

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

- 1' 检查 pist 和 fdie 是否都不为 NULL
- 2' 为 list 每个结点执行 fdie 注入的行为
- 3' *(void **)pist = NULL 图个心安
    
渐进的继续看 list_add 实现, 直接通过注入函数决定插入的位置

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
/* file : list.h */
//
// list_next - 获取结点 n 的下一个结点
// n        : 当前结点
#define list_next(n) ((void *)((struct $list *)(n))->next)

/* file : lish.c */
//
// list_next - 获取结点 n 的下一个结点
// n        : 当前结点
#undef  list_next
#define list_next(n) ((struct $list *)(n))->next
```

可以看出他在外部用的时候, 相当于无类型指针, 只能获取值难于设值. 内部用的时候已经转为类型指针, 就可以操作了. 算是宏控制代码使用权限的一个小技巧. 继续抛砖引玉用宏带大家构造 C 的常量技巧!

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

通过宏和声明定义 static 函数, 构造 const_version const struct version 真常量. 是不是挺有意思, 希望读者有所消遣 ~ 
    
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

同样有 3 部曲, 1 检查, 2 头结点处理, 3 非头结点处理. 需要注意的是 list_pop 只是在 list 中通过 fget(left, x) 弹出结点. 后续 free or delete 操作还得依赖使用方自行控制.

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

#ifndef EACH_F
#define EACH_F

//
// each_f - 遍历行为, node 是内部结点, arg 是外部参数
// : int echo(void * node, void * arg) { return 0; }
//
typedef int (* each_f)(void * node, void * arg);

#endif//EACH_F

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

list_get 和 list_each 代码是质朴中的质朴啊. 其中 list_each 注入 each_f 函数指针, 通过返回值来精细化控制 list_each 执行行为. 不好意思到这 list 设计套路解释完了. 喜欢的朋友可以多写几遍代码去体会其中思路然后再分享运用 ~ 

## 2.2 string

有句话不知道当讲不当讲, C 中 char * 其实够用了! 写过几个 string 模型, 自我觉得都有些过度封装, 不怎么爽且应用领域很鸡肋. 但开发中也有场景需要扩展 char *. 例如说, 如果 char * 字符串长度不确定, 随时可能变化. 那我们怎么处理呢? 这会就需要为其封装个动态字符集的库去管理这种行为. 为了让大家对 string 体会更多, 先带大家为 string.h 扩展几个小接口. 磨刀不费砍柴功.

### 2.2.1 包装 string.h => strext.h

strext.h 是基于 string.h 扩展而来, 先引入 strext.h 目的是方便后续相关 char * 操作. strext.h 功能设计分成 string 相关操作和 file 相关操作. 

```C
#pragma once

/*
    继承 : string.h
    功能 : 扩展 string.h 中部分功能, 方便业务层调用
 */

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "stdext.h"

//
// BKDHash - Brian Kernighan 与 Dennis Ritchie hash 算法
// str      : 字符串内容
// return   : 返回计算后的 hash 值
//
extern unsigned BKDHash(const char * str);

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
// str_sprintf - 格化式字符串构建
// fmt      : 构建格式参照 printf
// ...      : 参数集
// return   : char * 堆上内存
//
extern char * str_sprintf(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

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

```

这个库比较简单, 所以我就多说点. 大家都懂看得也热闹 ~ 

```C
#include "strext.h"

// 19 世纪 60 年代 Brian Kernighan 与 Dennis Ritchie hash 算法
unsigned BKDHash(const char * str) {
    register unsigned u, h = 0;
    if (str) {
        while ((u = *str++))
            h = h * 131u + u;
    }
    return h;
}
```

BKDHash 延续了 C 语言之父展示一种极其简便快速 hash 算法实现. 哈希(hash)映射相当于定义数学上一个函数, f (char *) 映射为 unsigned 数值. 意图通过数值一定程度上反向确定这个字符串. 思路特别巧妙. 同样也隐含了一个问题, 如果两个串映射一样的值, 那怎么搞. 常用术语叫碰撞, 解决碰撞也好搞. 套路不少有桶式 hash, 链式 hash, 混合 hash(后面会看见相关例子). 回到问题, 即如果发生碰撞了后续怎么办? 假设把保存 hash 值集合的地方叫海藻池子. 一种思路是当池子中海藻挤在一起(碰撞)了, 就加大池子, 让海藻分开, 原理是池子越大碰撞机会越小. 另一种思路当池子中海藻挤在一块吹泡泡的时候, 那我们单独开小水沟把这些吹泡泡的海藻全引流到小水沟中, 思路是碰撞的单独放一起. 而对于 hash 最重要特性是"两个模型映射的哈希值不一样, 那么二者一定不一样!". 通过这个特性在数据查找时能够快速刷掉一批! 推荐也多查查其他资料, 把 hash 设计和编码仔细分析明白!!

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
    if (!src || !tar || !n) return -1;
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

函数写的很普通. 完全算不上"高效"(没采用编译器特定函数优化, 例如按照字长比较函数). 胜在有异常参数处理, 这也是要写这些函数原因, 不希望传入 NULL 就崩溃. 再展示 trim 函数.

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

主要思路是在字符串头尾都查找 space 字符并缩进, 最后 e - s + 2 = (e + 1 - s) + 1 (最后 + 1 是 '\0' 字符). str_trim(char []) 声明设计希望调用方传入参数是数组, 且允许修改行为. 开始讲 str_printf 之前, 先回忆下系统 api

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

当初刚学习 snprintf 时候, 总觉得第一个参数好不爽. 凭什么让我来预先构造 char * 大小. 后面看标准说, 不希望过多揉进动态内存分配平台相关的代码, **内存申请和释放都应该交给使用方**. 基于这个提示我们实现思路非常巧妙和直白

```C
//
// str_sprintf - 格化式字符串构建
// fmt      : 构建格式参照 printf
// ...      : 参数集
// return   : char * 堆上内存
//
char * 
str_sprintf(const char * fmt, ...) {
    // 确定待分配内存 size
    va_list arg;
    va_start(arg, fmt);
    int n = vsnprintf(NULL, 0, fmt, arg);
    va_end(arg);

    if (n < 0) 
        return NULL;

    // 获取待分配内存, 尝试填充格式化数据
    char * ret = malloc(++n);

    va_start(arg, fmt);
    n = vsnprintf(ret, n, fmt, arg);
    va_end(arg);

    if (n < 0) {
        free(ret);
        return NULL;
    }

    return ret;
}
```

核心思路是利用 **vsnprintf(NULL, 0, fmt, arg)** 得到 max len, 多看 man 手册还是有好处的. 后面实现部分直接先贴上一块看.  

```C
//
// str_freads - 读取整个文件内容返回, 需要事后 free
// path     : 文件路径
// return   : 文件内容字符串, NULL 表示读取失败
//
char * 
str_freads(const char * path) {
    int64_t size = fsize(path);
    if (size < 0)
        return NULL;
    if (size == 0) 
        return calloc(1, sizeof (char));

    // 尝试打开文件读取处理
    FILE * txt = fopen(path, "rb");
    if (!txt) 
        return NULL;

    // 构建最终内存
    char * str = malloc(size + 1);
    str[size] = '\0';

    size_t n = fread(str, sizeof(char), size, txt);
    assert(n == (size_t)size);
    if (ferror(txt)) {
        free(str);
        fclose(txt);
        return NULL;
    }

    fclose(txt);

    return str;
}

// str_fwrite - 按照约定输出数据到文件中
static int str_fwrite(const char * p, const char * s, const char * m) {
    int len;
    FILE * txt;
    if (!p || !*p || !s || !m) {
        return -EINVAL; // 参数无效
    }

    // 打开文件, 写入消息, 关闭文件
    if (!(txt = fopen(p, m))) {
        return -ENOENT; // 文件或路径不存在
    }

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

**str_freads** 中 **fsize** 获取文件大小功能来自于 **stdext.h** 中, 这个功能实现我们放在后面讲. str_fwrite 设计仅仅对系统的文件输出函数包装一下. 以上关于 string.h 接口扩展部分不华丽, 但又是不可或缺, 适合传授新手, 带其练手和快速上手 ~

### 2.2.2 tstr interface

经过 strext.h 接口演练, 已经可以回忆起 C string.h 基础库的部分功能. 趁热打铁开始封装一类自带扩容缓冲的字符串模型, 比较好过渡. 首先看总的接口声明, 有个感性认知. **cstr.h** 支持堆上和栈上声明使用

```C
#pragma once

#include "struct.h"

#ifndef CSTR_INT

struct cstr {
    char * str;     // 字符串
    size_t cap;     // 容量
    size_t len;     // 长度
};

// CSTR_INT 构建字符串初始化大小
#define CSTR_INT    (1 << 7)

typedef struct cstr * cstr_t;

//
// cstr_declare - 栈上创建 cstr_t 结构
// cstr_free - 释放栈上 cstr_t 结构
// var      : 变量名
//
#define cstr_declare(var)               \
struct cstr var[1] = { {                \
    .str = malloc(CSTR_INT),            \
    .cap = CSTR_INT,                    \
} }

inline void cstr_init(cstr_t cs) {
    cs->len = 0;
    // 构建字符串初始化大小
    cs->cap = CSTR_INT;
    cs->str = malloc(CSTR_INT);
}

inline cstr_t cstr_new() {
    cstr_t cs = malloc(sizeof(struct cstr));
    cstr_init(cs);
    return cs;
}

inline void cstr_free(cstr_t cs) {
    free(cs->str);
}

#endif//CSTR_INT
```

通过 struct cstr 就能猜出作者思路, str 存放内容, len 记录当前字符长度, cap 表示字符池容量. 声明字符串类型 cstr_t 用于堆上声明. 如果想在栈上声明, 可以用提供的 **cstr_declare** 操作宏. 其实很多编译器支持运行期结束自动析构操作, 通过编译器的语法糖, 内嵌析构操作. 类比下面套路(编译器协助开发者插入 free or delete 代码), 模拟自动退栈销毁栈上字符串 var 变量

```C
#define CSTR_USING(var, code)           \
do {                                    \
    TSTR_CREATE(var);                   \
    code                                \
    TSTR_DELETE(var);                   \
} while(0)
```

C 修炼入门绝不是一朝一夕的事情, 就算早已看懂, 也需要入戏匪浅. 有了上面数据结构, 关于行为的部分代码定义就好理解多了. 

多说一点, C 中没有'继承'(当然也可以搞)但是有文件依赖, 也像是文件继承. 例如上面 **#include "struct.h"** 表达的意思是 **cstr.h** 接口文件继承 **struct.h** 接口文件. 强加文件继承关系, 能够明朗文件包含关系拊顺脉络. 继续看后续接口设计

```C
//
// cstr_expand - low level 字符串扩容 api
// cs       : 可变字符串
// len      : 扩容的长度
// return   : cstr::str + cstr::len 位置的串
//
char * cstr_expand(cstr_t cs, size_t len);

//
// cstr_t 串结构中添加字符等
// cs       : cstr_t 串
// c        : 添加 char
// str      : 添加 char *
// len      : 添加串的长度
// return   : void
//
extern void cstr_appendc(cstr_t cs, int c);
extern void cstr_appends(cstr_t cs, const char * str);
extern void cstr_appendn(cstr_t cs, const char * str, size_t len);

//
// cstr_create - cstr_t 创建函数, 根据 C 串创建 cstr_t 字符串
// str      : 待创建的字符串
// len      : 创建串的长度
// return   : 返回创建的字符串
//
inline cstr_t cstr_creats(const char * str) {
    cstr_t cs = cstr_new();
    cstr_appends(cs, str);
    return cs;
}

inline cstr_t cstr_create(const char * str, size_t len) {
    cstr_t cs = cstr_new();
    if (str && len) cstr_appendn(cs, str, len);
    return cs;
}

//
// cstr_delete - cstr_t 释放函数
// cs       : 待释放的串对象
// return   : void
//
inline void cstr_delete(cstr_t cs) {
    cstr_free(cs);
    free(cs);
}

//
// cstr_get - 通过 str_t 串得到一个 C 串以'\0'结尾
// cs       : cstr_t 串
// return   : 返回构建 C 串, 内存地址 cs->str
//
inline char * cstr_get(cstr_t cs) {
    *cstr_expand(cs, 1) = '\0';
    return cs->str;
}

//
// cstr_dup - 得到 C 堆上的串, 需要自行 free
// cs       : cstr_t 串
// return   : 返回创建好的 C 串
//
extern char * cstr_dup(cstr_t cs);

//
// cstr_popup - 字符串头弹出 len 长度字符
// cs       : 可变字符串
// len      : 弹出的长度
// return   : void
//
extern void cstr_popup(cstr_t cs, size_t len);

//
// cstr_sprintf - 参照 sprintf 方式填充内容
// cs       : cstr_t 串
// fmt      : 待格式化的串
// ...      : 可变参数列表
// return   : 返回创建的 C 字符串内容
//
extern char * cstr_sprintf(cstr_t cs, const char * fmt, ...) __attribute__((format(printf, 2, 3))) ;

```

还是无外乎创建销毁, 其中 cstr_expand 表示为 cstr 扩容操作. 没加 extern 表达的意图是使用这个低等级接口要小心. cstr_get 安全的得到 C 类型 char * 串. 当然了, 如果足够自信, 也可以直接 cstr->str 走起. 安全因人而异, 这是 C 的'自由', 大神在缥缈峰上, 菜鸡在自家泥河里. 

其中 cstr_get 封装很直白, 在串的结尾强加 C 的 '\0'.

```C
//
// cstr_get - 通过 str_t 串得到一个 C 串以'\0'结尾
// cs       : cstr_t 串
// return   : 返回构建 C 串, 内存地址 cs->str
//
inline char * cstr_get(cstr_t cs) {
    *cstr_expand(cs, 1) = '\0';
    return cs->str;
}
```

我们强调一切封装从简, 最好很自然 ~ 让大家在无内耗的大道于开心奔跑 ~

### 2.2.3 cstr implement

详细谈一下 cstr 的实现, 首先看最重要的一个接口 cstr_expand 操作内存. C 中掌控了内存, 就掌控了世界.

```C
//
// cstr_expand - low level 字符串扩容 api
// cs       : 可变字符串
// len      : 扩容的长度
// return   : cstr::str + cstr::len 位置的串
//
char * 
cstr_expand(cstr_t cs, size_t len) {
    size_t cap = cs->cap;
    if ((len += cs->len) > cap) {
        if (cap < CSTR_INT ) {
            cap = CSTR_INT;
        } else {
            // 走 1.5 倍内存分配, '合理'降低内存占用
            while (cap < len) 
                cap = cap * 3 / 2;
        }

        cs->str = realloc(cs->str, cs->cap = cap);
    }
    return cs->str + cs->len;
}
```

上面 cap = cap * 3 / 2 的做法, 在内存分配的时候很常见, 初始值加幂级别增长. 也有按照分阶段增长, 不同解决增长系数不一样. 最初版本增长用的是 cap <<= 1, 并用了位操作, 一个失传已久的装波技巧.

> pow2gt 返回比 x - 1 大的最小的 2 的 n 次方
原理是 2 ^ n - 1 位数全是 1, 1 -> 11 -> 1111 -> 11111111 -> ... 

```C
// pow2gt - 2 ^ n >= x , 返回 [2 ^ n]
static inline int pow2gt(int x) {
    --x;
    x |= x >>  1;
    x |= x >>  2;
    x |= x >>  4;
    x |= x >>  8;
    x |= x >> 16;
    return x + 1;
}
```

综合而言这里内存分配策略也属于直接拍脑门, 合理的还需要很多数据支撑以及特定工程使用情况还包括相关的研究论文.

```C
//
// cstr_t 串结构中添加字符等
// cs       : cstr_t 串
// c        : 添加 char
// str      : 添加 char *
// len      : 添加串的长度
// return   : void
//
inline void 
cstr_appendc(cstr_t cs, int c) {
    // 这类函数不做安全检查, 为了性能
    cstr_expand(cs, 1);
    cs->str[cs->len++] = c;
}

inline void 
cstr_appends(cstr_t cs, const char * str) {
    if (cs && str) {
        size_t sz = strlen(str);
        if (sz > 0)
            cstr_appendn(cs, str, sz);
        cstr_get(cs);
    }
}

inline void 
cstr_appendn(cstr_t cs, const char * str, size_t len) {
    memcpy(cstr_expand(cs, len), str, len);
    cs->len += len;
}
```

思路完全是大白话, **[ 还能撑住吗 -> 不能, 请求支援 -> 援兵赶到 -> 继续 Back OFF ... ]**

到这简单完成了一个 C 字符串, 准确的说是字符池模块. 顺手展示几个应用

```C
//
// cstr_dup - 得到 C 堆上的串, 需要自行 free
// cs       : cstr_t 串
// return   : 返回创建好的 C 串
//
inline 
char * cstr_dup(cstr_t cs) {
    // 构造内存, 返回最终结果
    size_t len = cs->len + (!cs->len||cs->str[cs->len-1]);
    char * str = malloc(len * sizeof(char));
    memcpy(str, cs->str, len - 1);
    str[len - 1] = '\0';
    return str;
}

//
// cstr_popup - 字符串头弹出 len 长度字符
// cs       : 可变字符串
// len      : 弹出的长度
// return   : void
//
inline 
void cstr_popup(cstr_t cs, size_t len) {
    if (len >= cs->len)
        cs->len = 0;
    else {
        cs->len -= len;
        memmove(cs->str, cs->str + len, cs->len);
    }
}
```

cstr_dup 用于 cstr_t 到 char * 转换. cstr_popup 操作会在 str 头部弹出特定长度字符, 可用于协议解析模块. 再附加赠送个 cstr_printf 用于 cstr sprintf 操作

```C
//
// cstr_sprintf - 参照 sprintf 方式填充内容
// cs       : cstr_t 串
// fmt      : 待格式化的串
// ...      : 可变参数列表
// return   : 返回创建的 C 字符串内容
//
char * 
cstr_sprintf(cstr_t cs, const char * fmt, ...) {
    // 确定待分配内存 size
    va_list arg;
    va_start(arg, fmt);
    int n = vsnprintf(NULL, 0, fmt, arg);
    va_end(arg);

    if (n <= 0) 
        return cstr_get(cs);

    // 获取待分配内存, 尝试填充格式化数据
    cstr_expand(cs, ++n);

    va_start(arg, fmt);
    n = vsnprintf(cs->str + cs->len, n, fmt, arg);
    va_end(arg);

    if (n <= 0) 
        return cstr_get(cs);

    cs->len += n;
    return cs->str;
}
```

到这 C 字符串辅助模块也大致搞定. string 不是 C 必须的, 有时候在特定场景会用的很舒服. 这么久, 也可以看出 C 写代码方式是 **[数据结构设计 -> 内存处理设计 -> 业务设计]**. 而
大多数现代语言写代码方式只需要关心 [业务设计]. 硬要对比的话, 存在性能和生产力成反比相关性规律. 作为工作很多年菜鸟, 如果有兴趣还是多用心在现代语言上, C 更适合教学知识点拆解而不是工作技能点提升.

## 2.3 array

array 指的是动态数组. C 中同样固定数组就够用了! 顺带说一点 C99 中新加变长(变量)数组. 如下声明, 本质是编译器运行时帮我们在栈上分配和释放, 一旦分配好后也不可以再改变长度. 

```C
int n = 64;
int array[n];
```

这里要说的 array, 支持运行时容量扩容. 设计原理与上面封装 cstr 很相似, 只是 char 独立单元变成了 void * 独立单元.

**array.h**

```C
#pragma once

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
// array_declare - 栈上创建动态数组对象
// array_free    - 销毁栈上动态数组对象
// var           : 创建动态数组对象名字
// ARRAY_UINT    - 数组初始化默认大小
#define ARRAY_UINT      (1u<<5)
#define array_declare(type, var)                \
struct array var[1] = { {                       \
    .size = sizeof(type),                       \
    .cap = ARRAY_UINT,                          \
    .data = malloc(sizeof(type) * ARRAY_UINT)   \
} }

inline void array_init(array_t a, unsigned size) {
    assert(a && size > 0);
    a->size = size;
    // set default cap size
    a->cap = ARRAY_UINT;
    a->len = 0;
    a->data = malloc(size * ARRAY_UINT);
}

inline void array_free(array_t a) {
    free(a->data);
}

//
// array_create - 返回创建动态数组对象
// size     : 元素大小
// return   : 返回创建的动态数组对象
//
inline array_t array_create(unsigned size) {
    struct array * a = malloc(sizeof(struct array));
    array_init(a, size);
    return a;
}

//
// array_delete - 销毁动态数组对象
// a        : 动态数组对象 
// return   : void
//
inline void array_delete(array_t a) {
    if (a) {
        array_free(a);
        free(a);
    }
}

//
// array_push - 数组中插入一个数据
// a        : 动态数组对象
// return   : void * 压入数据首地址
//
inline void * array_push(array_t a) {
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
inline void * array_pop(array_t a) {
    assert(a && a->len > 0);
    --a->len;
    return (char *)a->data + a->size * a->len;
}

//
// array_top - 得到动态数组顶元素
// a        : 动态数组
// return   : 得到返回动态数组顶部元素
//
inline void * array_top(array_t a) {
    assert(a && a->len > 0);
    return (char *)a->data + a->size * (a->len - 1);
}

//
// array_get - 索引映射数组元素
// a        : 动态数组
// idx      : 索引位置
// return   : NULL is not found
//
inline void * array_get(array_t a, unsigned idx) {
    assert(a && idx < a->len);
    return (char *)a->data + a->size * idx;
}

//
// array_idx - 通过结点返回索引
// a        : 动态数组
// elem     : 查询元素
// return   : 索引
//
inline unsigned array_idx(array_t a, void * elem) {
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
inline void array_swap(array_t a, array_t b) {
    struct array t = *a;
    *a = *b; 
    *b = t;
}

//
// array_sort - 动态数组排序
// a        : 动态数组
// fcmp     : 数组元素的比较函数
// return   : void
//
inline void array_sort(array_t a, cmp_f fcmp) {
    assert(a && a->len && fcmp);
    qsort(a->data, a->len, a->size, 
         (int (*)(const void *, const void *))fcmp);
}

//
// array_each - 动态数组遍历
// a        : 动态数组
// func     : 遍历行为
// return   : >= 0 表示成功, < 0 表示失败
//
extern int array_each(array_t a, each_f func, void * arg);

```

在 C 中数组 [1] 这个技巧主要为了追求指针对象写法的统一, 全用 -> 去操作. 是不是有点意思. 其中 struct array 通过注册的 size 确定数组中每个对象模型内存大小, 是一种很
原始的反射套路. 高级语言做的很多工作就是把原本编译时做的事情转到了运行时. 更现代化的魔法直接跳过编译时吟唱阶段而瞬发.

array 接口设计分为两部分, 第一部分是核心围绕创建, 删除, 压入, 弹出. 第二部分是应用围绕 array 结构做一些辅助操作. 

**array.c**

```C
#include "array.h"

//
// array_each - 动态数组遍历
// a        : 动态数组
// func     : 遍历行为
// return   : >= 0 表示成功, < 0 表示失败
//
int 
array_each(array_t a, each_f func, void * arg) {
    assert(a && func);

    char * s = a->data, * e = s + a->size * a->len;
    while (s < e) {
        int ret = func(s, arg);
        if (ret < 0)
            return ret;
        s += a->size;
    }

    return 0;
}

```

代码写来写去, 也就那点东西了. 

当然了, 越是经过筛选的好东西, 理应很顺很清晰. 


顺带补充点, 对于编程而言, 尽量少 typedef, 多 struct 写全称. 谎言需要另一个谎言来弥补. 并且多用标准中推出的解决方案. 例如标准提供的 stdint.h 和 stddef.h 定义全平
台类型. 不妨传大家我这么多年习得的无上秘法, 开 血之限界 -> 血轮眼 -> 不懂装懂, 抄抄抄. 一切如梦如幻! 回到正题. 再带大家写个很傻的单元测试, 供参考. 有篇幅的话会带大家写个简单的单元测试功能设计.

```C
#include <array.h>

// array_test - array test
void array_test(void) {
    // 构建一个在栈上的动态数组
    array_declare(double, a);

    // 开始处理数据
    *(double *)array_push(a) = 1.1234;
    *(double *)array_push(a) = 2.2345;
    *(double *)array_push(a) = 4.9876;

    // 输出数据
    printf("v = %lf\n", *(double *)array_pop(a));
    printf("v = %lf\n", *(double *)array_pop(a));
    printf("v = %lf\n", *(double *)array_pop(a));

    array_free(a);
}

```

## 2.4 两篇阅读理解

### 2.4.1 stack 设计

stack 设计和上面 cstr, array 非常类似. 我们这本书强调是工程实现, 如果你还不知道 stack 干什么的特性是什么, 可以尝试看看数据结构栈的部分. 温故而知新, 一起加油.

```C
#pragma once

#include "struct.h"

// 
// struct stack 对象栈
// stack empty <=> tail = -1 
// stack full  <=> tail == cap
//
struct stack {
    int      tail;  // 尾结点
    int       cap;  // 栈容量
    void **  data;  // 栈实体
};

//
// stack_init - 初始化 stack 对象栈
// stack_free - 清除掉 stack 对象栈
// return   : void
//
#define INT_STACK   (1 << 8)
inline void stack_init(struct stack * s) {
    assert(s && INT_STACK > 0);
    s->tail = -1;
    s->cap  = INT_STACK;
    s->data = malloc(sizeof(void *) * INT_STACK);
}

inline void stack_free(struct stack * s) {
    free(s->data);
}

//
// stack_delete - 删除 stack 对象栈
// s        : stack 对象栈
// fdie     : node_f push 结点删除行为
// return   : void
//
inline void stack_delete(struct stack * s, node_f fdie) {
    if (s) {
        if (fdie) {
            while (s->tail >= 0)
                fdie(s->data[s->tail--]);
        }
        stack_free(s);
    }
}

//
// stack_empty - 判断 stack 对象栈是否 empty
// s        : stack 对象栈
// return   : true 表示 empty
//
inline bool stack_empty(struct stack * s) {
    return s->tail <  0;
}

//
// stack_top - 获取 stack 栈顶对象
// s        : stack 对象栈
// return   : 栈顶对象
//
inline void * stack_top(struct stack * s) {
    return s->tail >= 0 ? s->data[s->tail] : NULL;
}

//
// stack_pop - 弹出栈顶元素
// s        : stack 对象栈
// return   : void
//
inline void stack_pop(struct stack * s) {
    if (s->tail >= 0) --s->tail;
}

//
// stack_pop_top - 弹出并返回栈顶元素
// s        : stack 对象栈
// return   : 弹出的栈顶对象
//
inline void * stack_pop_top(struct stack * s) {
    return s->tail >= 0 ? s->data[s->tail--] : NULL;
}

//
// stack_push - 压入元素到对象栈栈顶
// s        : stack 对象栈
// m        : 待压入的对象
// return   : void
// 
inline void stack_push(struct stack * s, void * m) {
    if (s->cap <= s->tail) {
        s->cap <<= 1;
        s->data = realloc(s->data, sizeof(void *) * s->cap);
    }
    s->data[++s->tail] = m;
}

```

### 2.4.2 id hash 设计

这篇阅读理解讲述的是 id hash 业务, 对数值进行哈希映射. 针对性很强, 应用场景也多, 例如内核层给应用层的句柄 id. 无法对其规律进行假设, 那我们把他映射到特定的范围内, 通过映射值去控制. 封装系统 io 复用层的时候很常见. 下面展示一个 hash id 相关库封装. 原始思路来自云风大佬的 skynet c gate server 上设计

```C
#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct hashid_node {
    int id;
    struct hashid_node * next;
};

struct hashid {
    int hashmod;
    int cap;
    int len;
    struct hashid_node * array;
    struct hashid_node ** hash;
};

static void hashid_init(struct hashid * hi, int max) {
    int hashcap = 16;
    while (hashcap < max) {
        hashcap <<= 1;
    }
    hi->hashmod = hashcap - 1;
    hi->cap = max;
    hi->len = 0;
    hi->array = malloc(max * sizeof(struct hashid_node));
    for (int i = 0; i < max; i++) {
        hi->array[i].id = -1;
        hi->array[i].next = NULL;
    }
    hi->hash = calloc(hashcap, sizeof(struct hashid_node *));
}

static void hashid_clear(struct hashid * hi) {
    free(hi->array);
    free(hi->hash);
    hi->array = NULL;
    hi->hash = NULL;
    hi->hashmod = 1;
    hi->cap = 0;
    hi->len = 0;
}

static int hashid_lookup(struct hashid * hi, int id) {
    int h = id & hi->hashmod;
    struct hashid_node * node = hi->hash[h];
    while (node) {
        if (node->id == id)
            return node - hi->array;
        node = node->next;
    }
    return -1;
}

static int hashid_remove(struct hashid * hi, int id) {
    int h = id & hi->hashmod;
    struct hashid_node * node = hi->hash[h];
    if (NULL == node)
        return -1;

    if (node->id == id) {
        hi->hash[h] = node->next;
        goto ret_clr;
    }

    while (node->next) {
        if (node->next->id == id) {
            struct hashid_node * temp = node->next;
            node->next = temp->next;
            node = temp;
            goto ret_clr;
        }

        node = node->next;
    }
    return -1;

ret_clr:
    node->id = -1;
    node->next = NULL;
    --hi->len;
    return node - hi->array;
}

static int hashid_insert(struct hashid * hi, int id) {
    int h;
    struct hashid_node * node = NULL;
    for (h = 0; h < hi->cap; h++) {
        int index = (h + id) % hi->cap;
        if (hi->array[index].id == -1) {
            node = hi->array + index;
            break;
        }
    }
    assert(node && node->next == NULL);
    node->id = id;
    ++hi->len;

    h = id & hi->hashmod;
    node->next = hi->hash[h];
    hi->hash[h] = node;

    return node - hi->array;
}

static inline int hashid_full(struct hashid * hi) {
    return hi->len == hi->cap;
}

```

这本小册子很多思想借鉴前辈云风思路. 他的代码非常不错, 读起来抄起来都很舒服, 这里再次感谢前辈的辛苦耕耘 /{|}\

代码比注释值钱, 尝试通过代码理解代码. 一般书中会有习题, 我们这里推荐"阅读理解", 辅助思考而不是考研. 哈哈. 来一同感受设计的细节. hash id 库设计这个阅读理解, 有些飘逸, 有些巧妙. 多临摹多思考
    
- **0' return -1;**

没有找见就返回索引 -1, 作为默认错误和 POSIX 默认错误码相同. POSIX 错误码引入了 errno 机制, 不太好, 封装过度. 上层需要二次判断, 开发起来难受. 我们后续设计思路也是承接这种 POSIX 思路.

- **1' hashcap <<= 1;**

这个 hashcap 初始值必须是 2 的幂数, 方便得到 h->hashmod = 2 ^ x - 1 = hashcap - 1

- **2' hi->hash = calloc(hashcap, sizeof(struct hashid_node *));**

这里表示, 当前 hash 实体已经全部申请好了, 只能用这些了. 所以有了 hashid_full 接口.

- **3' assert(node && node->next == NULL);**

代码在 hashid_insert 中出现, 表明插入一定会成功. 那么这个接口必须在 hashid_full 之后执行. 

- **4' int index = (h + id) % hi->cap;**

一种查找策略, 依赖数据随机性, 和 O(n) 纯 for 查找区别不大.

- **5' 小总结**

有了这些阅读理解会容易点. 上面构建的 hash id api, 完成的工作就是方便 int id 的映射工作. 查找急速, 实现上采用的是桶算法. 映射到固定空间上索引. 写一遍想一遍就能感受到那些游动于指尖的美好 ~

## 2.5 拓展阅读

在 **2.2.1 包装 string.h => strext.h** 小节中我们采用直白思路去实现相关功能. 这节带有兴趣同学拓展下视野, 看看标准库级别实现, 挑选了大家都常见 **strlen**. 编程可不是儿戏.

```C
/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se);
   commentary by Jim Blandy (jimb@ai.mit.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <string.h>
#include <stdlib.h>

#undef strlen

#ifndef STRLEN
# define STRLEN strlen
#endif

/* Return the length of the null-terminated string STR.  Scan for
   the null terminator quickly by testing four bytes at a time.  */
size_t
STRLEN (const char *str)
{
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str; ((unsigned long int) char_ptr
            & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
    {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
    }
  if (sizeof (longword) > 8)
    abort ();

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      longword = *longword_ptr++;

      if (((longword - lomagic) & ~longword & himagic) != 0)
    {
      /* Which of the bytes was the zero?  If none of them were, it was
         a misfire; continue the search.  */

      const char *cp = (const char *) (longword_ptr - 1);

      if (cp[0] == 0)
        return cp - str;
      
      if (cp[1] == 0)
        return cp - str + 1;
      if (cp[2] == 0)
        return cp - str + 2;
      if (cp[3] == 0)
        return cp - str + 3;
      if (sizeof (longword) > 4)
        {
          if (cp[4] == 0)
        return cp - str + 4;
          if (cp[5] == 0)
        return cp - str + 5;
          if (cp[6] == 0)
        return cp - str + 6;
          if (cp[7] == 0)
        return cp - str + 7;
        }
    }
    }
}
libc_hidden_builtin_def (strlen)

```

**思考和分析**

**1. unsigned long int 字节多大 4 字节, 8 字节 ?**

```C
  unsigned long int longword, himagic, lomagic;
```

long 具体多长和平台有关, 例如大多数 linux , x86 sizeof (long) = 4, x64 sizeof (long) = 8. window x86, x64 sizeof (long) = 4. C 标准保证 sizeof(long) >= sizeof (int) 具体多少字节交给了标准的实现方.

**2. ((unsigned long int) char_ptr & (sizeof (longword) - 1)) 位对齐 ? **

```C
/* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str; ((unsigned long int) char_ptr
            & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;
```

起始的这些代码的作用是, 让 chart_ptr 按照 sizeof (unsigned long) 字节大小进行位对齐. 这涉及到多数计算机硬件对齐有要求和性能方面的考虑等等(性能是主要因素).

**3. himagic = 0x80808080L; lomagic = 0x01010101L; what fuck ?**

```C
/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
    {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
    }
  if (sizeof (longword) > 8)
    abort ();

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      longword = *longword_ptr++;

      if (((longword - lomagic) & ~longword & himagic) != 0)
    {
```

**3.1 (((longword - lomagic) & ~longword & himagic) != 0) ? mmp ?**

可能这就是艺术吧. 想到这个想法的, 真是个天才啊! 好巧妙. 哈哈哈.  我们会分两个小点说明下. 首次看, 感觉有点萌. 我这里用个简单的思路来带大家理解这个问题. 上面代码主要围绕sizeof (unsigned long) 4 字节和 8 字节去处理得到. 我们简单点, 通过处理 1 字节, 类比递归机制. 搞懂这个公式背后的原理 (ˇˍˇ) ～

```C
/**
 * himagic      : 1000 0000
 * lomagic      : 0000 0001
 * longword     : XXXX XXXX
 * /
unsigned long himagic = 0x80L;
unsigned long lomagic = 0x01L;

unsigned long longword ;
```

随后我们仔细分析下面公式

```C
((longword - lomagic) & ~longword & himagic)
```

( & himagic ) = ( & 1000 0000) 表明最终只在乎最高位. longword 分三种情况讨论

```C
longword     : 1XXX XXXX  128 =< x <= 255
longword     : 0XXX XXXX  0 < x < 128
longword     : 0000 0000  x = 0
```

- 第一种 longword = 1XXX XXXX 

    那么 ~longword = 0YYY YYYY 显然 ~ longword & himagic = 0000 0000 不用继续了.

- 第二种 longword = 0XXX XXXX 且不为 0, 及不小于 1

    显然 (longword - lomagic) = 0ZZZ ZZZ >= 0 且 < 127, 因为 lomagic = 1; 
    此刻 (longword - lomagic) & himagic = 0ZZZ ZZZZ & 1000 0000 = 0 , 所以也不需要继续了.

- 第三种 longword = 0000 0000

    那么 ~longword & himagic = 1111 1111 & 1000 0000 = 1000 000;
    再看 (longword - lomagic) = (0000 0000 - 0000 0001) , 由于无符号数减法是按照
    (补码(0000 0000) + 补码(-000 0001)) = (补码(0000 0000) + 补码(~000 0001 + 1))
    = (补码(0000 0000) + 补码(1111 1111)) = 1111 1111 (快捷的可以查公式得到最终结果),
    因而 此刻最终结果为 1111 1111 & 1000 0000 = 1000 0000 > 0.

综合讨论, 可以根据上面公式巧妙的筛选出值是否为 0.  对于 2字节, 4 字节, 8 字节, 思路完全相似. 

**3.2 (sizeof (longword) > 4) ? (sizeof (longword) > 8) 为什么不用宏, 大展宏图呗 ?**

宏可以做到多平台源码共享, 无法做到多平台二进制共享. glibc 这么通用项目, 可移植性影响因子可能会很重. (性能是毒酒, 想活的久还是少喝 ~ )

**4. libc_hidden_builtin_def (strlen) ? 闹哪样 ~**

解这个东西, 要引入些场外信息  (不同编译参数会不一样, 这里只抽取其中一条分支解法)

```C
// file : glibc-2.31/include/libc-symbols.h

libc_hidden_builtin_def (strlen)

#define libc_hidden_builtin_def(name) libc_hidden_def (name)

# define libc_hidden_def(name) hidden_def (name)

/* Define ALIASNAME as a strong alias for NAME.  */
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name))) \
    __attribute_copy__ (name);

/* For assembly, we need to do the opposite of what we do in C:
   in assembly gcc __REDIRECT stuff is not in place, so functions
   are defined by its normal name and we need to create the
   __GI_* alias to it, in C __REDIRECT causes the function definition
   to use __GI_* name and we need to add alias to the real name.
   There is no reason to use hidden_weak over hidden_def in assembly,
   but we provide it for consistency with the C usage.
   hidden_proto doesn't make sense for assembly but the equivalent
   is to call via the HIDDEN_JUMPTARGET macro instead of JUMPTARGET.  */
#  define hidden_def(name)    strong_alias (name, __GI_##name)

/* Undefine (also defined in libc-symbols.h).  */
#undef __attribute_copy__
#if __GNUC_PREREQ (9, 0)
/* Copies attributes from the declaration or type referenced by
   the argument.  */
# define __attribute_copy__(arg) __attribute__ ((__copy__ (arg)))
#else
# define __attribute_copy__(arg)
#endif
```

利用上面宏定义, 进行展开  

```C
libc_hidden_builtin_def (strlen)
|
hidden_def (strlen)
|
strong_alias (strlen, __GI_strlen)
|
_strong_alias (strlen, __GI_strlen)
|
extern __typeof (strlen) __GI_strlen __attribute__ ((alias ("strlen"))) __attribute_copy__ (strlen);
|
extern __typeof (strlen) __GI_strlen __attribute__ ((alias ("strlen"))) __attribute__ ((__copy__ (strlen)));
```

其中 GUN C 扩展语法

- __typeof (arg) : 获取变量的声明的类型
- __attribute__ ((__copy__ (arg))) : GCC 9 以上版本 attribute copy 复制特性
- alias_name __attribute__ ((alias (name))) : 为 name 声明符号别名 alias name.
 
总结:  libc_hidden_builtin_def (strlen) 意思是基于 strlen 符号, 重新定义一个符号别名 __GI_strlen. (@see strong_alias 注释)

strlen 工程代码有很多种, 我们这里选择一个通用 glibc 版本去思考和分析. 有兴趣可以自行查阅更多. 做人嘛开心最重要 ~

## 2.6 展望

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
