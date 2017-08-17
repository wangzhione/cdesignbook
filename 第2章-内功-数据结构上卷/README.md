## 第2章-内功-数据结构上卷

    对于 C而言, 数据结构不过关, 一切都是虚妄. 其它语言还好, 基础复合类型标准支持的很好.
    重读一点, 在 C的世界里, 只认数据结构和操作系统. 而数据结构就是核心内功, 一招一式
    都是内劲. 修炼数据结构本质是为了方便操作, 规划整个布局, 捋顺输入输出. 内功没有几个月
    苦练, 打出来的什么都是花把式. 内功上卷我们只讲简单一点 list, string, array, hash.

### 2.1 list

    如果说数据结构是 C的内功, 那么毫无疑问链表 list就是数据结构的内功. 链式结构最原始的
    抽象模型就是 list.

![list](img/list.png) 
    
    上面就是一种最原始的 list结构, next是指向下一个结点的地址. 自己对于 list结构的理解
    比较简单, list 只是个实体, 并且这个实体还能找到它保存的下一个实体.  
    这里我们简单的构建一部分接口部分

#### 2.1.1 list.h interface

```C
#ifndef _H_SIMPLEC_LIST
#define _H_SIMPLEC_LIST

#include "struct.h"

//
//	这个万能单链表库 前提所有结点都是堆上分配的,设计的比较老了可以一用
//	1.使用的时候,需要加上 _LIST_HEAD; 宏
//	2.创建的第一句话就是 list_t head = NULL; 开始从空链表开始list的生涯
//
struct $lnode {
	struct $lnode * next;
};

// 不多说了一定放在想使用链表结构的结构体头部
#define _LIST_HEAD struct $lnode $node

//
// 简单链表结构, 当你使用这个链表的时候 需要 list_t head = NULL; 开始使用之旅
//
typedef void * list_t;

//
// list_next - 主要返回结点n的下一个结点. 用法 node->next = list_next(n) or list_next(n) = node;
// n		: 当前结点
//
#define list_next(n) ((struct $lnode *)(n))->next

//
// list_destroy - 链表销毁函数.对于只是栈上数据就不用调这个api
// ph 		: 指向当前链表结点的指针
// die		: 销毁执行的函数
// return	: void
//
extern void list_destroy(list_t * ph, node_f die);

//
// list_add - 在 cmp(left, x) <= 0 x处前面插入node结点
// ph		: 指向头结点的指针
// cmp		: 比较函数,将left同 *ph中对象按个比较
// left		: cmp(left, x) 比较返回 <=0 or >0
// return	: 返回 SufBase 表示成功!
//
extern int list_add(list_t * ph, icmp_f cmp, void * left);

//
// list_findpop - 查找到要的结点,并弹出,需要你自己回收
// ph		: 指向头结点的指针
// cmp		: 比较函数,将left同 *ph中对象按个比较
// left		: cmp(left, x) 比较返回 0 >0 <0
// return	: 找到了退出/返回结点, 否则返回NULL
//
extern void * list_findpop(list_t * ph, icmp_f cmp, const void * left);

#endif // !_H_SIMPLEC_LIST
```

    对于 struct.h 可以参看第一章的设计部分, 这里轻微提及一下 

```C
//
// icmp_f   - int icmp(const void * ln, const void * rn); 标准结构
//          - return > 0 or = 0 or < 0
//          - ( ) 可以接收任意参数类型, 方便类型转换
//
typedef int    (* icmp_f )( );

//
// node_f   - 每个结点的处理函数指针
//
typedef void   (* node_f )(void * node);

```

    封装的这一套接口, 使用起来也很简单. 例如构建一个人的链表结构, 只需要采用下面方式

```C

struct people {
    _LIST_HEAD;
    int free;       // 有理想
    char * ideal;   // 有文化
    double future;  // 有秩序
};

// 构建
list_t pls = NULL;

// 添加
list_add(&pls, xxx, xxx);

// 删除 | 返回结点自己负责善后
struct people * ple = list_findpop(&pls, xxx, xxx);

// 销毁
list_destroy(&pls, NULL);

```

    对于 list部分过于基础, 说太多还没有写个10几类链表的来的实在. 做 C开发工作, 几乎就是
    围绕 'list'类个中增删改查, 高级些加个缓存层, 伪删除.
    这里多说一点, 写代码也要三思而后行. 先想好思路, 定好基调接口. 再谈实现. 设计, 优雅的
    设计, 是第一位的. 在 C中设计确定了, 多数指数据结构确定了, 那么代码实现就已经定好了!

#### 2.1.2 list implements

    