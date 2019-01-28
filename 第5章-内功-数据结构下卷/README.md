# 第5章-内功-数据结构下卷

        恭喜你到这. 此刻会是新开始的临界点. 本章算开发中数据结构使用的实战阶段. 将会展示
	金丹, 元婴战斗中时常出现数据结构部分的内功. 漫天飛絮, 气流涌动 ~ 也许随后你觉得点复
    杂, 也许觉得点简单. 因为 C 修真一个要求就是, 你需要懂得实现. 才能运用流畅. 一切都将
    钻木取火, 自生自灭. 扯一点, 编译型语言要是有那种万能数据结构 array 或者 table, 那
    生产力预估会提升 10 倍吧. 写代码就和玩似的 ~ 本章完工等价于数据结构已经登堂入世. C 
    的代码写的越多, 越发觉得喜欢就好! 也许谁都想在这个元气稀薄的江湖成就元婴, 何不乘早, 
    打码穿键盘 ~ 看书出心眼 ~ 

## 5.1 红黑树, 一道坎

        红黑树的理论, 推荐搜索多方资料恶补. 它解决的问题是, 防止二叉搜索树退化为有序的双
    向链表. 相似替代有跳跃表, hash 桶. 但具体使用什么, 因个人喜好. 作者只能站在自己框架
    用到的, 实现角度出发. 带大家感受, 那些瞎逼调整的二叉树节点 ~ 是如何张狂的出现在编程的
    世界里 ~ 哈哈 ~ 首先瞄一下总设计野路子 rtree.h

```C
#ifndef _RTREE_H
#define _RTREE_H

#include "struct.h"

//
// 红黑树通用结构, 需要将 $RTREE 放在结构开头部位
//

struct $rtree {
    uintptr_t parentc;
    struct $rtree * left;
    struct $rtree * right;
};

#define $RTREE struct $rtree $node;

typedef struct {
    struct $rtree * root;
    cmp_f fget; // cmp_f 节点查找时比较行为
    cmp_f fcmp;
    new_f fnew;
    node_f fdie;
} * rtree_t;

//
// rtee_create - 创建一个红黑树对象
// fcmp     : cmp_f 节点插入时比较行为
// fnew     : new_f 节点插入时构造行为
// fdie     : node_f 节点删除时销毁行为
// return   : 返回构建红黑树对象
//
extern rtree_t rtree_create(void * fcmp, void * fnew, void * fdie);

//
// rtree_delete - 红黑树销毁函数
// tree     : 待销毁的红黑树
// return   : void
//
extern void rtree_delete(rtree_t tree);

//
// rtree_search - 红黑树查找函数
// tree     : 待查找的红黑树结构
// return   : 返回查找的节点
//
extern void * rtree_search(rtree_t tree, void * pack);

//
// rtree_insert - 红黑树中插入节点 fnew(pack)
// tree     : 红黑树结构
// pack     : 待插入基础结构
// return   : void
//
extern void rtree_insert(rtree_t tree, void * pack);

//
// rtree_remove - 红黑树中删除节点
// tree     : 红黑树结构
// pack     : 待删除基础结构
// return   : void
//
extern void rtree_remove(rtree_t tree, void * pack);

#endif//_RTREE_H
```

    向链表. 相似替代有跳跃表, hash 桶. 但具体使用什么, 因个人喜好. 作者只能站在自己框架
    通过上面结构先体会下设计意图, 例如 rtree_t 结构中 fget, fnew, fcmp, fdie 分别用
    于红黑树中 search 查找节点, insert 创建节点, insert 查找节点, delete 销毁节点时
    候的处理规则. 使用的套路是基于注册的设计思路. $RTREE 和 $LIST 是一样的套路. C 中一
    种结构继承的技巧, 运用地址重叠确定 struct $rtree 首地址. 而对于 struct $rtree 中
    uintptr_t parentc 它表示红黑树的父亲节点地址和当前是红是黑标识两种状态. 利用原理是,
    当前结构的内存布局以指针(x86 4 字节, x64 8 字节)位对齐的. 因为地址一定是 4 的倍数, 
    即 0100 的倍数. 所以最后两位默认用不上, 扩展用于标识结点的红黑属性. 有了这些知识那么
    就有如下代码.

```C
#include "rtree.h"

//
// struct $rtree 结构辅助操作宏
// r    : 当前节点
// p    : 父节点
// c    : 当前节点颜色, 1 is black, 0 is red
//
#define rtree_parent(r)      ((struct $rtree *)((r)->parentc & ~3))
#define rtree_color(r)       ((r)->parentc & 1)
#define rtree_is_red(r)      (!rtree_color(r))
#define rtree_is_black(r)    rtree_color(r)
#define rtree_set_red(r)     (r)->parentc &= ~1
#define rtree_set_black(r)   (r)->parentc |= 1

inline void rtree_set_parent(struct $rtree * r, struct $rtree * p) {
    r->parentc = (r->parentc & 3) | (uintptr_t)p;
}

inline void rtree_set_color(struct $rtree * r, int color) {
    r->parentc = (r->parentc & ~1) | (1 & color);
}

inline static int rtree_default_cmp(const void * ln, const void * rn) {
    return (int)((intptr_t)ln - (intptr_t)rn);
}
```

    rtree_parent 通过当前节点获取父节点, rtree_color 获取当前节点颜色标识. 有了这些
    我们就可以写些简单的部分代码. rtree_create 红黑树创建, rtree_delete 红黑树删除.

```C
//
// rtee_create - 创建一个红黑树对象
// fcmp     : cmp_f 节点插入时比较行为
// fnew     : new_f 节点插入时构造行为
// fdie     : node_f 节点删除时销毁行为
// return   : 返回构建红黑树对象
//
inline rtree_t 
rtree_create(void * fcmp, void * fnew, void * fdie) {
    rtree_t tree = malloc(sizeof *tree);
    tree->root = NULL;
    tree->fcmp = fcmp ? fcmp : rtree_default_cmp;
    tree->fnew = fnew;
    tree->fdie = fdie;
    tree->fget = NULL;
    return tree;
}

// rtree_die - 后序删除树节点
static void rtree_die(struct $rtree * root, node_f fdie) {
    if (root->left)
        rtree_die(root->left, fdie);
    if (root->right)
        rtree_die(root->right, fdie);
    fdie(root);
}

//
// rtree_delete - 红黑树销毁函数
// tree     : 待销毁的红黑树
// return   : void
//
inline void 
rtree_delete(rtree_t tree) {
    if (NULL == tree) return;
    if (tree->root && tree->fdie)
        rtree_die(tree->root, tree->fdie);
    tree->root = NULL;
    free(tree);
}
```

    同样 rtree_search 红黑树查找操作也是很普通, 和有序二叉树操作一样. 只是多了步查找行
    为 tree->fget 的选择. 

```C
//
// rtree_search - 红黑树查找函数
// tree     : 待查找的红黑树结构
// return   : 返回查找的节点
//
void * 
rtree_search(rtree_t tree, void * pack) {
    cmp_f fcmp;
    struct $rtree * node;
    if (!tree) return NULL;

    fcmp = tree->fget ? tree->fget : tree->fcmp;
    node = tree->root;
    while (node) {
        int diff = fcmp(node, pack);
        if (diff == 0)
            break;
        //
        // tree left less, right greater 
        //
        node = diff > 0 ? node->left : node->right;
    }
    return node;
}
```

### 5.1.1 红黑树调整

    后续逐渐拉开了红黑树大戏. 继续细化代码, 逐个分析. 红黑树中有个重要调整旋转操作. 被称
    为左旋和右旋, 例如左旋代码如下:

```C
/* 
 * 对红黑树的节点 [x] 进行左旋转
 *
 * 左旋示意图 (对节点 x 进行左旋):
 *      px                             px
 *     /                              /
 *    x                              y
 *   /  \       --- (左旋) -->       / \
 *  lx   y                         x  ry
 *     /   \                      /  \
 *    ly   ry                    lx  ly  
 *
 */
static void rtree_left_rotate(rtree_t tree, struct $rtree * x) {
    // 设置 [x] 的右孩子为 [y]
    struct $rtree * y = x->right;
    struct $rtree * xparent = rtree_parent(x);

    // 将 [y的左孩子] 设为 [x的右孩子]；
    x->right = y->left;
    // 如果 y的左孩子 非空，将 [x] 设为 [y的左孩子的父亲]
    if (y->left != NULL)
        rtree_set_parent(y->left, x);

    // 将 [x的父亲] 设为 [y的父亲]
    rtree_set_parent(y, xparent);

    if (xparent == NULL) {
        // 如果 [x的父亲] 是空节点, 则将 [y] 设为根节点
        tree->root = y;
    } else {
        if (xparent->left == x) {
            // 如果 [x] 是它父节点的左孩子, 则将 [y] 设为 [x的父节点的左孩子]
            xparent->left = y;
        } else {
            // 如果 [x] 是它父节点的左孩子, 则将 [y] 设为 [x的父节点的左孩子]
            xparent->right = y;
        }
    }

    // 将 [x] 设为 [y的左孩子]
    y->left = x;
    // 将 [x的父节点] 设为 [y]
    rtree_set_parent(x, y);
}
```

    后续逐渐拉开了红黑树大戏. 继续细化代码, 逐个分析. 红黑树中有个重要调整旋转操作. 被称
    为什么有这些额外辅助调整操作呢, 主要是为了满足红黑树五大特性.
        特性 1: 每个节点或者是黑色, 或者是红色.
        特性 2: 根节点是黑色.
        特性 3: 每个叶子节点(NIL)是黑色. [这里是指为空(NIL或NULL)的叶子节点]
        特性 4: 如果一个节点是红色的, 则它的子节点必须是黑色的.
        特性 5: 从一个节点到该节点的子孙节点的所有路径上包含相同数目的黑色节点.
    而左右旋转操作, 就是为了调整出特性 5设计的. 相应的右旋操作如下:

```C
/* 
 * 对红黑树的节点 [y] 进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                            py
 *           /                             /
 *          y                             x                  
 *         /  \     --- (右旋) -->       /  \
 *        x   ry                        lx   y  
 *       / \                                / \
 *      lx  rx                             rx  ry
 * 
 */
static void rtree_right_rotate(rtree_t tree, struct $rtree * y) {
    // 设置 [x] 是当前节点的左孩子。
    struct $rtree * x = y->left;
    struct $rtree * yparent = rtree_parent(y);

    // 将 [x的右孩子] 设为 [y的左孩子]
    y->left = x->right;
    // 如果 x的右孩子 不为空的话，将 [y] 设为 [x的右孩子的父亲]
    if (x->right != NULL)
        rtree_set_parent(x->right, y);

    // 将 [y的父亲] 设为 [x的父亲]
    rtree_set_parent(x, yparent);
    if (yparent == NULL) {
        // 如果 [y的父亲] 是空节点, 则将 [x] 设为根节点
        tree->root = x;
    } else {
        if (y == yparent->right) {
            // 如果 [y] 是它父节点的右孩子, 则将 [x] 设为 [y的父节点的右孩子]
            yparent->right = x;
        } else {
            // 如果 [y] 是它父节点的左孩子, 将 [x] 设为 [x的父节点的左孩子]
            yparent->left = x;
        }
    }

    // 将 [y] 设为 [x的右孩子]
    x->right = y;
    // 将 [y的父节点] 设为 [x]
    rtree_set_parent(y, x);
}
```

    后续逐渐拉开了红黑树大戏. 继续细化代码, 逐个分析. 红黑树中有个重要调整旋转操作. 被称
    注释很详细, 看起来也有点头痛. 木办法, 需要各自课下下功夫. 疯狂的吸收相关元气, 照着算
	法解释抄一遍代码, 写一遍代码, 理解一遍, 坎就过了. 毕竟上面是工程代码, 和教学过家家有
	些不同. 旋转调整相比后面的插入调整, 删除调整, 要简单很多. 因而趁着它简单, 我们再额外
    补充些. 红黑树存在父亲节点, 左右旋转相对容易点. 如果一个普通二叉数没有父亲节点, 该如
    何旋转呢? 我们以陈启峰 2006 高中的时候构建的 Size Balanced Tree 为例子演示哈. 分
    别观望数据结构和旋转实现, 坐山观虎斗(眼睛看), 净收渔翁之利(动手写). 

```C
#ifndef _STREE_H
#define _STREE_H

typedef unsigned long long stree_key_t;

typedef union {
    void * ptr;
    double number;
    signed long long i;
    unsigned long long u;
} stree_value_t;

struct stree {
    struct stree * left;    // 左子树
    struct stree * right;   // 右子树
    unsigned size;          // 树节点个数

    stree_key_t key;        // tree node key
    stree_value_t value;    // tree node value
};

typedef struct stree * stree_t;

inline unsigned stree_size(const struct stree * const node) {
    return node ? node->size : 0;
}

#endif//_STREE_H
```

![SBT](./img/sbt.png)

```C
// stree_left_rotate - size tree left rotate
static void stree_left_rotate(stree_t * pode) {
    stree_t node = *pode;
    stree_t right = node->right;

    node->right = right->left;
    right->left = node;
    right->size = node->size;
    node->size = stree_size(node->left) + stree_size(node->right) + 1;

    *pode = right;
}

// stree_right_rotate - size tree right rotate
//
//           (y)   left rotate      (x)
//          /   \  ------------>   /   \
//        (x)   [C]              [A]   (y)
//       /   \     <------------      /   \
//      [A]  [B]    right rotate    [B]   [C]
//
static void stree_right_rotate(stree_t * pode) {
    stree_t node = *pode;
    stree_t left = node->left;

    node->left = left->right;
    left->right = node;
    left->size = node->size;
    node->size = stree_size(node->left) + stree_size(node->right) + 1;

    *pode = left;
}
```

### 5.1.2 红黑树 insert

    请原谅作者拾(chao)人(xi)牙(mei)慧(lian), 弄了个大概无关痛痒的步骤说明. 将一个节点
    插入到红黑树中. 首先, 将红黑树当作一颗二叉查找树, 将节点插入. 然后, 将节点着色为红色
    . 最后通过旋转和重新着色等方法来修正该树, 使之重新成为一颗红黑树. 详细过程描述.
    第一步: 
	将红黑树当作一颗二叉查找树, 将节点插入红黑树本身就是一颗二叉查找树, 将节点插入后, 该树
    仍然是一颗二叉查找树. 也就意味着, 树的键值仍然是有序的. 此外, 无论是左旋还是右旋, 若
    旋转之前这棵树是二叉查找树, 旋转之后它一定还是二叉查找树. 这也就意味着, 任何的旋转和重
    新着色操作, 都不会改变它仍然是一颗二叉查找树的事实.
    第二步:
	将插入的节点着色为'红色'. 将插入的节点着色为红色, 不会违背'特性 5'!
    第三步: 
	通过一系列的旋转或着色等操作, 使之重新成为一颗红黑树. 第二步中, 将插入节点着色为'红色'
    之后, 不会违背'特性 5'. 那它会违背哪些特性呢? 
    对于'特性 1': 不会违背. 因为我们已经将它涂成红色了.
    对于'特性 2': 也不会违背. 在第一步中, 我们是将红黑树当作二叉查找树, 然后执行的插入操
        作. 而根据二叉查找数的特点, 插入操作不会改变根节点. 所以，根节点仍然是黑色.
    对于'特性 3': 不会违背. 叶子节点是指的空叶子节点, 插入非空节点并不会对其造成影响
    对于'特性 4': 是有可能违背的！
    那接下来工作就是想办法'满足特性 4', 就可以将搜索树重新生成一棵红黑树了. 来看看热闹, 代
    码到底是怎样实现这三大步的.

```C
/*
 * 红黑树插入修正函数
 *
 * 在向红黑树中插入节点之后(失去平衡), 再调用该函数.
 * 目的是将它重新塑造成一颗红黑树.
 *
 * 参数说明:
 *     tree 红黑树的根
 *     node 插入的节点        // 对应 <<算法导论>> 中的 z
 */
static void rtree_insert_fixup(rtree_t tree, struct $rtree * node) {
    struct $rtree * parent, * gparent, * uncle;

    // 若 [父节点] 存在，并且 [父节点] 的颜色是红色
    while ((parent = rtree_parent(node)) && rtree_is_red(parent)) {
        gparent = rtree_parent(parent);

        //若 [父节点] 是 [祖父节点的左孩子]
        if (parent == gparent->left) {
            // Case 1 条件: 叔叔节点是红色
            uncle = gparent->right;
            if (uncle && rtree_is_red(uncle)) {
                rtree_set_black(uncle);
                rtree_set_black(parent);
                rtree_set_red(gparent);
                node = gparent;
                continue;
            }

            // Case 2 条件: 叔叔是黑色, 且当前节点是右孩子
            if (parent->right == node) {
                rtree_left_rotate(tree, parent);
                uncle = parent;
                parent = node;
                node = uncle;
            }

            // Case 3 条件: 叔叔是黑色, 且当前节点是左孩子
            rtree_set_black(parent);
            rtree_set_red(gparent);
            rtree_right_rotate(tree, gparent);
        } else { // 若 [z的父节点] 是 [z的祖父节点的右孩子]
            // Case 1 条件: 叔叔节点是红色
            uncle = gparent->left;
            if (uncle && rtree_is_red(uncle)) {
                rtree_set_black(uncle);
                rtree_set_black(parent);
                rtree_set_red(gparent);
                node = gparent;
                continue;
            }

            // Case 2 条件: 叔叔是黑色, 且当前节点是左孩子
            if (parent->left == node) {
                rtree_right_rotate(tree, parent);
                uncle = parent;
                parent = node;
                node = uncle;
            }

            // Case 3 条件: 叔叔是黑色, 且当前节点是右孩子
            rtree_set_black(parent);
            rtree_set_red(gparent);
            rtree_left_rotate(tree, gparent);
        }
    }

    // 将根节点设为黑色
    rtree_set_black(tree->root);
}

// rtree_new - 插入时候构造一个新节点 | static 用于解决符号重定义
static inline struct $rtree * rtree_new(rtree_t tree, void * pack) {
    struct $rtree * node = tree->fnew ? tree->fnew(pack) : pack;
    // 默认构建节点是红色的
    return  memset(node, 0, sizeof *node);
}

//
// rtree_insert - 红黑树中插入节点 fnew(pack)
// tree     : 红黑树结构
// pack     : 待插入基础结构
// return   : void
//
void 
rtree_insert(rtree_t tree, void * pack) {
    if (!tree || !pack) return;

    cmp_f fcmp = tree->fcmp;
    struct $rtree * x = tree->root, * y = NULL;
    // 1. 构造插入节点, 并设置节点的颜色为红色
    struct $rtree * node = rtree_new(tree, pack);

    // 2. 将红黑树当作一颗二叉查找树, 将节点添加到二叉查找树中. 默认 从小到大
    while (x) {
        y = x;
        if (fcmp(x, node) > 0)
            x = x->left;
        else
            x = x->right;
    }
    rtree_set_parent(node, y);

    if (NULL == y) {
        // 情况 1: 若 y是空节点, 则将 node设为根
        tree->root = node;
    } else {
        if (fcmp(y, node) > 0) {
            // 情况 2: 若 "node所包含的值" < "y所包含的值", 则将 [node] 设为 [y的左孩子]
            y->left = node;
        } else {
            // 情况 3：若 "node所包含的值" >= "y所包含的值", 将 [node] 设为 [y的右孩子] 
            y->right = node;
        }
    }

    // 3. 将它重新修正为一颗二叉查找树
    rtree_insert_fixup(tree, node);
}
```

    手写红黑树是个挑战. 参照无数元婴前辈们筑基期历练的手稿, 顺带从 linux 上拔下来的原始
    代码敲个几遍遍. 构造一个上面库. 学习的话 1 查 2 抄 3 默写, 应该能蒙蒙胧吧 ~

### 5.1.3 红黑树 remove

    将红黑树内某一个节点删除. 需要执行的操作顺序是: 首先, 将红黑树当作一颗二叉查找树, 将
    该节点从二叉查找树中删除. 然后, 通过'旋转和重新着色' 等一系列来修正该树, 使之重新成
    为一棵红黑树. 详细描述参照下面步骤.
    第一步: 
	将红黑树当作一颗二叉查找树, 将节点删除. 这和'删除常规二叉查找树中删除节点的方法是一样
    的'. 分 3 种情况:
    情况 1. 被删除节点没有儿子, 即为叶节点. 那么直接将该节点删除就 OK 了.
    情况 2. 被删除节点只有一个儿子. 那么直接删除该节点, 并用该节点的儿子节点顶替它的位置.
    情况 3. 被删除节点有两个儿子. 那么, 先找出它的后继节点. 然后把'它的后继节点的内容'复
           制给'该节点的内容'. 之后, 删除'它的后继节点'. 在这里, 后继节点相当于替身, 
           在将后继节点的内容复制给'被删除节点'之后, 再将后继节点删除. 这样就巧妙的将问
           题转换为'删除后继节点'的情况了, 下面就考虑后继节点. 在'被删除节点'有两个非空
           子节点的情况下, 它的后继节点不可能是双子非空. 既然'被删除节点的后继节点'不可
           能双子都非空, 就意味着'该节点的后继节点'要么没有儿子, 要么只有一个儿子. 若没
           有儿子, 则按'情况 1' 进行处理. 若只有一个儿子, 则按'情况 2' 进行处理.
    第二步: 
	通过'旋转和重新着色'等一系列来修正该树, 使之重新成为一棵红黑树. 因为'第一步'中删除节
    点之后, 可能会违背红黑树的特性. 所以需要通过'旋转和重新着色'来修正该树, 使之重新成为
    一棵红黑树. 
    最终对着算法说明和 linux 红黑树源码构造的一种工程实现 rtree_remove 纯属一起作死一
    起嗨.

```C
/*
 * 红黑树删除修正函数
 *
 * 在从红黑树中删除插入节点之后(红黑树失去平衡), 再调用该函数.
 * 目的是将它重新塑造成一颗红黑树.
 *
 * 参数说明:
 *     tree 红黑树的根
 *     node 待修正的节点
 */
static void rtree_remove_fixup(rtree_t tree, struct $rtree * node, struct $rtree * parent) {
    struct $rtree * other;

    while ((!node || rtree_is_black(node)) && node != tree->root) {
        if (parent->left == node) {
            other = parent->right;
            if (rtree_is_red(other)) {
                // Case 1: x的兄弟 w 是红色的  
                rtree_set_black(other);
                rtree_set_red(parent);
                rtree_left_rotate(tree, parent);
                other = parent->right;
            }
            if ((!other->left || rtree_is_black(other->left)) &&
                (!other->right || rtree_is_black(other->right))) {
                // Case 2: x的兄弟 w 是黑色, 且 w的俩个孩子也都是黑色的  
                rtree_set_red(other);
                node = parent;
                parent = rtree_parent(node);
            } else {
                if (!other->right || rtree_is_black(other->right)) {
                    // Case 3: x的兄弟 w 是黑色的, 并且 w的左孩子是红色, 右孩子为黑色  
                    rtree_set_black(other->left);
                    rtree_set_red(other);
                    rtree_right_rotate(tree, other);
                    other = parent->right;
                }
                // Case 4: x的兄弟 w 是黑色的, 并且 w的右孩子是红色的, 左孩子任意颜色
                rtree_set_color(other, rtree_color(parent));
                rtree_set_black(parent);
                rtree_set_black(other->right);
                rtree_left_rotate(tree, parent);
                node = tree->root;
                break;
            }
        } else {
            other = parent->left;
            if (rtree_is_red(other)) {
                // Case 1: x 的兄弟 w 是红色的  
                rtree_set_black(other);
                rtree_set_red(parent);
                rtree_right_rotate(tree, parent);
                other = parent->left;
            }
            if ((!other->left || rtree_is_black(other->left)) &&
                (!other->right || rtree_is_black(other->right))) {
                // Case 2: x 的兄弟 w 是黑色, 且 w的俩个孩子 也都是黑色的  
                rtree_set_red(other);
                node = parent;
                parent = rtree_parent(node);
            } else {
                if (!other->left || rtree_is_black(other->left)) {
                    // Case 3: x的兄弟 w 是黑色的, 并且 w的左孩子是红色, 右孩子为黑色
                    rtree_set_black(other->right);
                    rtree_set_red(other);
                    rtree_left_rotate(tree, other);
                    other = parent->left;
                }
                // Case 4: x的兄弟 w 是黑色的, 并且 w的右孩子是红色的, 左孩子任意颜色.
                rtree_set_color(other, rtree_color(parent));
                rtree_set_black(parent);
                rtree_set_black(other->left);
                rtree_right_rotate(tree, parent);
                node = tree->root;
                break;
            }
        }
    }
    if (node) rtree_set_black(node);
}

//
// rtree_remove - 红黑树中删除节点
// tree     : 红黑树结构
// pack     : 待删除基础结构
// return   : void
//
void 
rtree_remove(rtree_t tree, void * pack) {
    int color;
    struct $rtree * child, * parent, * node = pack;
    if (NULL != tree) return;

    // 被删除节点的 "左右孩子都不为空" 的情况
    if (NULL != node->left && node->right != NULL) {
        // 被删节点的后继节点. (称为 "取代节点")
        // 用它来取代 "被删节点" 的位置, 然后再将 "被删节点" 去掉
        struct $rtree * replace = node;

        // 获取后继节点
        replace = replace->right;
        while (replace->left != NULL)
            replace = replace->left;

        // "node节点" 不是根节点(只有根节点不存在父节点)
        if ((parent = rtree_parent(node))) {
            if (parent->left == node)
                parent->left = replace;
            else
                parent->right = replace;
        } else // "node节点" 是根节点, 更新根节点
            tree->root = replace;

        // child 是 "取代节点" 的右孩子, 也是需要 "调整的节点"
        // "取代节点" 肯定不存在左孩子! 因为它是一个后继节点
        child = replace->right;
        parent = rtree_parent(replace);
        // 保存 "取代节点" 的颜色
        color = rtree_color(replace);

        // "被删除节点" 是 "它的后继节点的父节点"
        if (parent == node)
            parent = replace; 
        else {
            // child不为空
            if (child)
                rtree_set_parent(child, parent);
            parent->left = child;

            replace->right = node->right;
            rtree_set_parent(node->right, replace);
        }

        rtree_set_parent(replace, rtree_parent(node));
        rtree_set_color(replace, rtree_color(node));
        replace->left = node->left;
        rtree_set_parent(node->left, replace);

        goto ret_out;
    }

    if (NULL != node->left)
        child = node->left;
    else 
        child = node->right;

    parent = rtree_parent(node);
    // 保存 "取代节点" 的颜色
    color = rtree_color(node);

    if (child)
        rtree_set_parent(child, parent);

    // "node节点" 不是根节点
    if (NULL == parent)
        tree->root = child;
    else {
        if (parent->left == node)
            parent->left = child;
        else
            parent->right = child;
    }

ret_out:
    if (color) // 黑色节点重新调整关系, 并销毁节点操作
        rtree_remove_fixup(tree, child, parent);
    if (tree->fdie)
        tree->fdie(node);
}
```

    红黑树代码是非线性的, 需要一点看材料的功夫. 就当扩展视野, 吸收成自己的代码库. 写代码
    很多时候要和姑苏慕容学习, 以彼之道还治彼身. 这里关于红黑树的梗过去了, 飞云逐日, 不如
    , 一切安好 ~

## 5.2 dict

        趁热打铁 map -> dict. 红黑树是上层 map 库的实现基石, 同样另一种相似的查找库 
    dict, 有时候采用的是 hash 桶算法去实现, 综合查找性能高于 map. 主要应用场景是通过
    key -> value 映射查找业务操作. 由于应用场景更多, 我们带大家走遍形式, 直接看接口设
    计 dict.h

```C
#ifndef _DICT_H
#define _DICT_H

#include "struct.h"
#include "strext.h"

//
// dict_t - C 字符串为 k 的字典结构
//
typedef struct dict * dict_t;

//
// dict_delete - 字典删除
// d        : dict_create 创建的字典对象
// return   : void 
//
extern void dict_delete(dict_t d);

//
// dict_create - 字典创建
// fdie     : node_f 销毁行为
// return   : dict_t
//
extern dict_t dict_create(node_f fdie);

//
// dict_get - 获取字典中对映的 v
// d        : dict_create 创建的字典对象
// k        : 查找的 key 
// return   : 查找的 v, NULL 表示没有
//
extern void * dict_get(dict_t d, const char * k);

//
// dict_set - 设置一个 <k, v> 结构
// d        : dict_create 创建的字典对象
// k        : 插入的 key
// v        : 插入数据的值, NULL 会销毁 k
// return   : void
//
extern void dict_set(dict_t d, const char * k, void * v);

#endif//_DICT_H
```

    内功练到后面是不是有种势如破竹的感觉, 清晰易懂, 简单明了. (前提是自己手熟) 那就开始
    实现意图剖析, 来看 dict 内功的气海结构.

```C
struct keypair {
    struct keypair * next;
    unsigned hash;
    void * val;
    char key[];
};

// keypair_delete - 销毁结点数据
inline void keypair_delete(node_f fdie, struct keypair * pair) {
    if (pair->val && fdie)
        fdie(pair->val);
    free(pair);
}

// keypair_create - 创建结点数据
inline struct keypair * keypair_create(unsigned hash, void * v, const char * k) {
    size_t len = strlen(k) + 1;
    struct keypair * pair = malloc(sizeof(struct keypair) + len);
    pair->hash = hash;
    pair->val = v;
    memcpy(pair->key, k, len);
    return pair;
}

struct dict {
    node_f fdie;                // 结点注册的销毁函数
    unsigned idx;               // 使用 _primes 质数表索引
    unsigned used;              // 用户已经使用的结点个数
    struct keypair ** table;    // size = primes[idx][0]
};
```

    内功练到后面是不是有种势如破竹的感觉, 清晰易懂, 简单明了. (前提是自己手熟) 那就开始
    dict::table 就是我们的 keypair 池子, 存放所有 struct keypair 结构. 如果冲突了
    , 那就向 keypair::next 链式结构中接着插入. 如果池子满了, 那就重新挖一个大点的池子
    , 重新调整所有关系. 这就是核心思想! 不妨详细看看池子漫了的时候的策略.

```C
//
// primes - 质数表
//
const unsigned primes[][2] = {
    { (1<<6)-1  ,         53 },
    { (1<<7)-1  ,         97 },
    { (1<<8)-1  ,        193 },
    { (1<<9)-1  ,        389 },
    { (1<<10)-1 ,        769 },
    { (1<<11)-1 ,       1543 },
    { (1<<12)-1 ,       3079 },
    { (1<<13)-1 ,       6151 },
    { (1<<14)-1 ,      12289 },
    { (1<<15)-1 ,      24593 },
    { (1<<16)-1 ,      49157 },
    { (1<<17)-1 ,      98317 },
    { (1<<18)-1 ,     196613 },
    { (1<<19)-1 ,     393241 },
    { (1<<20)-1 ,     786433 },
    { (1<<21)-1 ,    1572869 },
    { (1<<22)-1 ,    3145739 },
    { (1<<23)-1 ,    6291469 },
    { (1<<24)-1 ,   12582917 },
    { (1<<25)-1 ,   25165843 },
    { (1<<26)-1 ,   50331653 },
    { (1<<27)-1 ,  100663319 },
    { (1<<28)-1 ,  201326611 },
    { (1<<29)-1 ,  402653189 },
    { (1<<30)-1 ,  805306457 },
    { UINT_MAX  , 1610612741 },
};

static void dict_resize(struct dict * d) {
    unsigned size, prime, i;
    struct keypair ** table;
    unsigned used = d->used;

    if (used < primes[d->idx][0])
        return;
    
    // 构造新的内存布局大小
    size = primes[d->idx][1];
    prime = primes[++d->idx][1];
    table = calloc(prime, sizeof(struct keypair *));

    // 开始转移数据
    for (i = 0; i < size; ++i) {
        struct keypair * pair = d->table[i];
        while (pair) {
            struct keypair * next = pair->next;
            unsigned index = pair->hash % prime;

            pair->next = table[index];
            table[index] = pair;
            pair = next;
        }
    }

    // table 重新变化
    free(d->table);
    d->table = table;
}
```

    内功练到后面是不是有种势如破竹的感觉, 清晰易懂, 简单明了. (前提是自己手熟) 那就开始
    dict_resize 中选择了 calloc prime 和 pair->hash % prime 写法. 还有种技巧是

```C
// init
unsigned prime = 1<<5; 

// resize
prime <<= 1;
table = calloc(prime, sizeof(struct keypair *));

// get
unsigned index = pair->hash & (prime - 1);
```

    取 2 的幂大小当做 table 容量去操作. 这样写优势在于, 可以用 & 替代 % 运算. 减少运
    算指令. 存在的隐患就是 hash 取余的不够随机导致最终池子中数据分布不均, 依赖链表解冲
    突. 因而最终选取了素数表, 希望减少冲突, 提升性能. 接着看 dict_resize 做的工作, 
    判断容量是否够, 不够开始重构 table. 有了这些可以看看 dict 的 delete 和 create
    操作实现.    

```C
//
// dict_delete - 字典销毁
// d        : dict_create 创建的字典对象
// return   : void 
//
void 
dict_delete(dict_t d) {
    unsigned i, size;
    if (NULL == d) return;
    size = primes[d->idx][1];
    for (i = 0; i < size; ++i) {
        struct keypair * pair = d->table[i];
        while (pair) {
            struct keypair * next = pair->next;
            keypair_delete(d->fdie, pair);
            pair = next;
        }
    }
    free(d->table);
    free(d);
}

//
// dict_create - 字典创建
// fdie     : v 销毁函数
// return   : dict_t
//
inline dict_t 
dict_create(node_f fdie) {
    struct dict * d = malloc(sizeof(struct dict));
    unsigned size = primes[d->idx = 0][1];
    d->used = 0;
    d->fdie = fdie;
    // 默认构建的第一个素数表 index = 0
    d->table = calloc(size, sizeof(struct keypair *));
    return d;
}
```

    聊到现在大家会发现, create and delete 都是成对出现. 命名很固定, 这是良好一种编码
    态度, 创建和删除一定要做好. 特别是删除, 做人当如君子(伪君子也行). 有骨气. 渡人清风
    , 知错能改, 不留坑. 苦练内功正能量(爱自己, 护亲人). 之后是获取字典中数据.

```C
//
// dict_get - 获取字典中对映的 v
// d        : dict_create 创建的字典对象
// k        : 查找的 key 
// return   : 查找的 v, NULL 表示没有
//
void * 
dict_get(dict_t d, const char * k) {
    unsigned hash, index;
    struct keypair * pair;
    assert(NULL != d && k != NULL);

    hash = str_hash(k);
    index = hash % primes[d->idx][1];
    pair = d->table[index];

    while (pair) {
        if (!strcmp(pair->key, k))
            return pair->val;
        pair = pair->next;
    }

    return NULL;
}
```

    dict_get hash 之后 cmp 操作很普通. 随后 进入 dict 修炼最后一关, dict_set 操作.

```C
//
// dict_set - 设置一个 <k, v> 结构
// d        : dict_create 创建的字典对象
// k        : 插入的 key
// v        : 插入数据的值, NULL 会销毁 k
// return   : void
//
void 
dict_set(dict_t d, const char * k, void * v) {
    unsigned hash, index;
    struct keypair * pair, * prev;
    assert(NULL != d && k != NULL);

    // 检查一下内存, 看是否要扩充
    dict_resize(d);

    // 开始寻找数据
    hash = str_hash(k);
    index = hash % primes[d->idx][1];
    pair = d->table[index];
    prev = NULL;

    while (pair) {
        // 找见了数据
        if (pair->hash == hash && !strcmp(pair->key, k)) {
            // 相同数据直接返回什么都不操作
            if (pair->val == v)
                return;

            // 删除操作
            if (NULL == v) {
                if (NULL == prev)
                    d->table[index] = pair->next;
                else
                    prev->next = pair->next;

                // 销毁结点, 直接返回
                return keypair_delete(d->fdie, pair);
            }

            // 更新结点
            if (d->fdie)
                d->fdie(pair->val);
            pair->val = v;
            return;
        }

        prev = pair;
        pair = pair->next;
    }

    // 没有找见设置操作, 直接插入数据
    if (NULL != v) {
        pair = keypair_create(hash, v, k);
        pair->next = d->table[index];
        d->table[index] = pair;
        ++d->used;
    }
}
```

    dict_get hash 之后 cmp 操作很普通. 随后 进入 dict 修炼最后一关, dict_set 操作.
    这里为 dict_set(d, k, NULL) 赋予新语义 dict_del 删除字典 k 操作. 单纯的 set 
    操作仍然是三部曲 reset -> get -> set, 到这里 dict 完工了. 是不是感觉很简单, 苦
    练内功学什么都快. 类比奇遇太子张无忌 ~

## 5.2 来个消息队列吧

        消息队列极其重要, 基本偏 C 系列的开发中不是链表, 就是消息队列. 消息队列可以比喻
    为咱们排队等待进入火车站, 那个一排排的栏杆让人一个个的检查过去, 就是消息队列作用. 消
    息队列最大的功效是让异步编程变成同步并发. 说白了就是将异步程序变成顺序同步程序. 开发
    起来很爽. 而在 C 中消息队列的至高王的装逼是无锁, 而我们这里还是会老老实实带大家用原
    子锁实现. 大伙还记得 atom.h 吗? 金丹期之后的战斗, 无不是消息队列领域的对撞. 随我步
    入简单高效的消息队列的世界中. 

### 5.2.1 简单队列

	消息队列本质还是队列, 直白思路是通过动态循环数组和原子锁构建 pop 和 push. 凡事先看
    接口, 熟悉起用法, 请看 q.h

```C
#ifndef _Q_H
#define _Q_H

#include "struct.h"

//
// pop empty  <=> tail == -1 ( head == 0 )
// push full  <=> head == (tail + 1) % size
//
typedef struct q {
    int head;           // 头结点
    int tail;           // 尾结点
    int size;           // 队列大小
    void ** queue;      // 队列实体
} q_t[1];

//
// q_init - 初始化
// q      : 队列对象
// return : void
// Q_INT  - 队列初始大小, 必须是 2 的幂
#define Q_INT     (1<< 6)
inline void q_init(q_t q) {
    q->head = 0;
    q->tail = -1;
    q->size = Q_INT;
    q->queue = malloc(sizeof(void *) * Q_INT);
}

//
// q_swap - q swap
// r      : q one
// w      : q two
// return : void
//
inline void q_swap(q_t r, q_t w) {
    q_t q;
    q[0] = r[0];
    r[0] = w[0];
    w[0] = q[0];
}

//
// q_pop - 队列中弹出消息数据
// q      : 队列对象
// return : 若队列 empty, 返回 NULL
//
extern void * q_pop(q_t q);

//
// q_push - 队列中压入数据
// q      : 队列对象
// m      : 压入消息
// return : void
// 
extern void q_push(q_t q, void * m);

//
// q_delete - 队列删除
// q        : 队列对象
// fdie     : node_f push 节点删除行为
// return   : void
//
extern void q_delete(q_t q, node_f fdie);

#endif//_Q_H
```

    消息队列本质还是队列, 直白思路是通过动态循环数组和原子锁构建 pop 和 push. 凡事先看
    我写的循环队列, 喜欢用 q::head == (q::tail + 1) & (q::size - 1) 标识队列为满
    , q::tail == -1 标识队列为空. 读者可以思考下还有没有其它方式标识 empty 和 full
    状态, 再互相对比方式差异好处和坏处! 那可以先看看 q_delete 实现.

```C
//
// q_delete - 队列删除
// q        : 队列对象
// fdie     : node_f push 节点删除行为
// return   : void
//
void 
q_delete(q_t q, node_f fdie) {
    // 销毁所有对象
    if (q->tail >= 0 && fdie) {
        for (;;) {
            fdie(q->queue[q->head]);
            if (q->head == q->tail)
                break;
            q->head = (q->head + 1) & (q->size - 1);
        }
    }

    free(q->queue);
}
```

    q->head == q->tail 是查找结束条件. 整个删除销毁操作, 等同于 array range. 重点
    在于 q->head = (q->head + 1) & (q->size - 1); 找到数组下一个位置的索引. 那看
    看 q_pop 在队列中弹出元素.

```C
//
// q_pop - 队列中弹出消息数据
// q      : 队列对象
// return : 若队列 empty, 返回 NULL
//
void * 
q_pop(q_t q) {
    void * m = NULL;
    if (q->tail >= 0) {
        m = q->queue[q->head];
        if (q->tail != q->head)
            q->head = (q->head + 1) & (q->size - 1);
        else {
            q->head = 0; // empty 情况, 重置 tail 和 head
            q->tail = -1;
        }
    }
    return m;
}
```

    q_push 操作包含了 q_expand 内存扩充操作, 用于内存重建和前面的 dict_resize 思路
    相似. 此刻 q.c 实现部分 50 多行, 已经全部贴完了. 是不是觉得工程中用到的数据结构也
    不过如此.   

```C
// q_expand - expand memory by twice
static void q_expand(q_t q) {
    int i, size = q->size << 1;
    void ** p = malloc(sizeof(void *) * size);
    for (i = 0; i < q->size; ++i)
        p[i] = q->queue[(q->head + i) & (q->size - 1)];
    free(q->queue);

    // 重新构造内存关系
    q->head = 0;
    q->tail = q->size;
    q->size = size;
    q->queue = p;
}

//
// q_push - 队列中压入数据
// q      : 队列对象
// m      : 压入消息
// return : void
// 
void 
q_push(q_t q, void * m) {
    int tail = (q->tail + 1) & (q->size - 1);
    // 队列 full 直接扩容
    if (tail == q->head && q->tail >= 0)
        q_expand(q);
    else
        q->tail = tail;
    q->queue[q->tail] = m;
}
```

### 4.3.2 线程安全

    看 q.h 实现也会发现, 它不是线程安全的. 并发的 push 和 pop 将未定义. 我们不妨将其
    包装成线程安全的x消息队列 mq.h

```C
#ifndef _MQ_H
#define _MQ_H

#include "q.h"
#include "atom.h"

struct mq {
    q_t q;             // 队列
    atom_t lock;       // 自旋锁
};

typedef struct mq * mq_t;

//
// mq_delete - 消息队列删除
// q        : 消息队列对象
// fdie     : node_f 行为, 删除 push 进来的节点
// return   : void
//
inline void mq_delete(mq_t q, node_f fdie) {
    // 销毁所有对象
    q_delete(q->q, fdie);
    free(q);
}

//
// mq_create - 消息队列创建
// return   : 消息队列对象
//
inline mq_t mq_create(void) {
    struct mq * q = malloc(sizeof(struct mq));
    q_init(q->q);
    q->lock = 0;
    return q;
}

//
// mq_pop - 消息队列中弹出消息, 并返回数据
// q        : 消息队列对象
// return   : 若 mq empty return NULL
//
inline void * mq_pop(mq_t q) {
    atom_lock(q->lock);
    void * m = q_pop(q->q);
    atom_unlock(q->lock);
    return m;
}

//
// mq_push - 消息队列中压入数据
// q        : 消息队列对象
// m        : 压入的消息
// return   : void
//
inline void mq_push(mq_t q, void * m) {
    atom_lock(q->lock);
    q_push(q->q, m);
    atom_unlock(q->lock);
}

//
// mq_len - 消息队列的长度
// q        : 消息队列对象
// return   : 返回消息队列长度
//
inline static int mq_len(mq_t q) {
    int head, tail, size;
    atom_lock(q->lock);
    if ((tail = q->q->tail) == -1) {
        atom_unlock(q->lock);
        return 0;
    }

    head = q->q->head;
    size = q->q->size;
    atom_unlock(q->lock);

    // 计算当前时间中内存队列的大小
    tail -= head - 1;
    return tail>0 ? tail : tail+size;
}

#endif//_MQ_H
```

    不知道有没有同学好奇 mq_delete 为什么不是线程安全的? 这个是这样的 mq_delete 一旦
    执行后, 那么 mq 随后的所有的操作都不应该被调用. 因为内存都没了, 别让野指针大魔头冲出
    封印. 基于这个 mq_delete 只能在所有业务都停下的时候调用. 所以无需画蛇添足. mq_len
    额外添加的函数用于线上监控当前循环队列的峰值. 用于观测和调整代码内存分配策略. 这套骚
    操作, 主要是感悟(临摹)化神巨擘云风 skynet mq 残留的意境而构建的. 欢迎道友修炼 ~    

### 5.2.3 消息队列

    

### 4.3.1 消息队列设计实现

    队列的设计, 一大重点, 就是如何判断队列是否为空, 是否为满. 下面所传的queue内功不
	亚于小易筋经. 首先看结构:

```C
#include "mq.h"
#include "scatom.h"

// 2 的 幂
#define _INT_MQ				(1 << 6)

//
// pop empty	<=> tail == -1 ( head = 0 )
// push full	<=> head + 1 == tail
//
struct mq {
	int lock;			// 消息队列锁
	int cap;			// 消息队列容量, 必须是2的幂
	int head;			// 消息队列头索引
	int tail;			// 消息队列尾索引
	void ** queue;		// 具体的使用消息

	volatile bool fee;	// true表示销毁退出
};
```

    上面注释表述了意图, 额外修改 tail == -1 表示队列为空. 这样做的意图是为了给
    head + 1 == tail 让步, 很多消息队列设计 head == tail 的时候表示队列需要扩充
	内存. 所以会造成队列永远不满的情况, 浪费内存. 这里设计的原理是每次 push check 
	full, 并且已经满了才会扩充内存, 重新调整布局. 相比其它套路省内存

    详细看看 create 和 delete 操作

```C
inline mq_t 
mq_create(void) {
	struct mq * q = malloc(sizeof(struct mq));
	assert(q);
	q->lock = 0;
	q->cap = _INT_MQ;
	q->head = 0;
	q->tail = -1;
	q->queue = malloc(sizeof(void *) * _INT_MQ);
	assert(q->queue);
	q->fee = false;
	return q;
}

void 
mq_delete(mq_t mq, node_f die) {
	if (!mq || mq->fee) return;
	ATOM_LOCK(mq->lock);
	mq->fee = true;
	// 销毁所有对象
	if (mq->tail >= 0 && die) {
		for(;;) {
			die(mq->queue[mq->head]);
			if (mq->tail == mq->head)
				break;
			mq->head = (mq->head + 1) & (mq->cap - 1);
		}
	}
	free(mq->queue);
	ATOM_UNLOCK(mq->lock);
	free(mq);
}
```

    到这都是大白话, 申请内存随后删除, 删除采用轮询到数据一直为空结束. 
	核心到了, mq push 的时候, 检查 queue 是否满了, 然后 xxxx 俗套剧情进行:

```C
// add two cap memory, memory is do not have assert
static void _mq_expand(struct mq * mq) {
	int i, cap = mq->cap << 1;
	void ** nqueue = malloc(sizeof(void *) * cap);
	assert(nqueue);
	
	for (i = 0; i < mq->cap; ++i)
		nqueue[i] = mq->queue[(mq->head + i) & (mq->cap - 1)];
	
	mq->head = 0;
	mq->tail = mq->cap;
	mq->cap  = cap;
	free(mq->queue);
	mq->queue = nqueue;
}

void 
mq_push(mq_t mq, void * msg) {
	int tail;
	if (!mq || mq->fee || !msg) return;
	ATOM_LOCK(mq->lock);

	tail = (mq->tail + 1) & (mq->cap - 1);
	// 队列为full的时候申请内存
	if (tail == mq->head && mq->tail >= 0)
		_mq_expand(mq);
	else
		mq->tail = tail;

	mq->queue[mq->tail] = msg;

	ATOM_UNLOCK(mq->lock);
}
```

    看上面加锁的方式, 哪里有竞争的地方哪里加锁. 没一点含蓄, 很实在. pop 也相同

```C
void * 
mq_pop(mq_t mq) {
	void * msg = NULL;
	if (!mq || mq->fee) return NULL;

	ATOM_LOCK(mq->lock);

	if (mq->tail >= 0) {
		msg = mq->queue[mq->head];
		if(mq->tail != mq->head)
			mq->head = (mq->head + 1) & (mq->cap - 1);
		else {
			// 这是empty,情况, 重置
			mq->tail = -1;
			mq->head = 0;
		}
	}

	ATOM_UNLOCK(mq->lock);

	return msg;
}
```

    当 pop 为空的时候, 会设置 tail = -1 和 head = 0. 来标识empty状态. 最后统计
	中用的 mq_len 返回的只是当时状态的消息队列情况:

```C
int 
mq_len(mq_t mq) {
	int head, tail, cap;
	if (!mq || mq->fee) return 0;

	ATOM_LOCK(mq->lock);
	tail = mq->tail;
	if (tail < 0) {
		ATOM_UNLOCK(mq->lock);
		return 0;
	}
	cap = mq->cap;
	head = mq->head;
	ATOM_UNLOCK(mq->lock);

	tail -= head - 1;
	return tail > 0 ? tail : tail + cap;
}
```

    有一点需要注意, push 的时候 tail 移动一格, 此时 tail == head 标识队列满.
    普通时候 tail == head 表示队列中只有一个元素. 无锁消息队列已经搞完了, 强烈
	推荐引入到自己的项目中, 什么双缓冲太重, 上面那种最省内存. 代码很短, 思路会在
	很多地方用到, 随后利用上面思路构建一个网络IO 收发队列. 用于解决 TCP socket 
	流式协议无边界问题.

### 4.4 收发消息的环形队列

    本章已经轻微剧透了些元婴功法的消息. 在我们处理服务器通信的时候, 采用 UDP 报文
	套接子很好处理边界问题. 因为 UDP包有固定大小. 而 TCP 流式套接字一直在收发, 还可
	能重传. 业务层默认它的报文是无边界的. 因此 TCP的报文边切割需要程序员自己处理. 通
	过上面表述的问题, 就需要我们来定义收发报文协议, 用于区分每一个消息包. 这就是这个库
	的由来, 首先看 rsmq.h 结构设计:

```C
//
// recv msg : 
//  这仅仅是一个是处理网络序列接收端的解析库. 
//  通过 len [统一小端网络字节, sizeof uint32] -> data
//
// need send msg :
//	one send sizeof uint32_t + data
//

typedef struct {
	// uint8_t type + uint8_t check + uint16_t len
	uint32_t sz;
	char data[];
} * msgrs_t ;

typedef struct rsmq * rsmq_t;

//
// RSMQ_TYPE - 得到当前消息8bit字节
// RSMQ_SIZE - 得到当前消息长度 0x00 + 2 字节 sz
// RSMQ_SZ	- 8bit type + 24 bit len -> uint32_t
//
#define MSGRS_TYPE(sz)		(uint8_t)((uint32_t)(sz) >> 24)
#define MSGRS_LEN(sz)		((uint32_t)(sz) & 0xffffff)
#define MSGRS_SZ(type, len)	(((uint32_t)((uint8_t)type) << 24) | (uint32_t)(len))
```

    我们采用 len + data 构建 bit 流传输, len = 8 bit type + 24 bit size. 用于服务器
	和客户端基础通信协议. 

### 4.4.1 发的构建部分

    发送部分比较简单. 主要就是将 data -> data + sz -> type size data 消息流构建过程.
    代码比文字更有说服力

```C
//
// msgrs_create - msgrs构建函数, 客户端发送 write(fd, msg->data, msg->sz);
// msg		: 待填充的消息体
// data		: 客户端待发送的消息体
// sz		: data 的长度
// type		: 发送的消息类型, 默认0是 RSMQ_TYPE_INFO
// return	: 创建好的消息体
//
inline msgrs_t 
msgrs_create(const void * data, uint32_t sz) {
	DEBUG_CODE({
		if (!data || sz <= 0 || sz > USHRT_MAX)
			CERR_EXIT("msgrs_create params data = %p, sz = %u", data, sz);
	});
	uint32_t szn = sz + sizeof(uint32_t); 
	msgrs_t msg = malloc(sizeof(*msg) + szn);
	if (NULL == msg)
		CERR_EXIT("malloc sizeof uint32_t + %u err!", sz);
	msg->sz = szn;

	// type + sz -> 协议值 -> 网络传输约定值
	szn = MSGRS_SZ(0, sz);
	szn = sh_hton(szn);
	// 开始内存填充
	memcpy(msg->data, &szn, sizeof(uint32_t));
	memcpy((char *)msg->data + sizeof(uint32_t), data, sz);

	return msg;
}

inline void 
msgrs_delete(msgrs_t msg) {
	if (msg) free(msg);
}
```

    顺带扯一点, 有兴趣的朋友也可以查查手册熟悉 inline 关键字. 目前这个关键字有点
    鸡肋, 只是表达一种意愿, 而且不同编译平台语义也不一样. 单纯的内联函数是没有函数
    地址的. static inline 会生成单独生成一份函数地址. 看着用吧 ~ 欢喜就好 ~

### 4.4.2 收的构建部分

    同 mq 无锁环形消息队列设计差不多. 先看 create 和 delete 

```C
#define _INT_RECVMQ	(1 << 8)

//
// tail == -1 ( head = 0 ) -> queue empty
// push head == tail + 1 -> queue full
//
struct rsmq {
	int lock;
	int cap;
	int head;
	int tail;
	char * buff;
	// buffq.sz is msg header body size length
	uint32_t sz;
};

inline rsmq_t
rsmq_create(void) {
	struct rsmq * buff = malloc(sizeof(struct rsmq));
	buff->lock = 0;
	buff->cap = _INT_RECVMQ;
	buff->head = 0;
	buff->tail = -1;
	buff->buff = malloc(buff->cap);
	buff->sz = 0;
	return buff;
}

inline void 
rsmq_delete(rsmq_t q) {
	if (q) {
		free(q->buff);
		free(q);
	}
}
```

    随后无外乎是 push 和 pop, push 的时候利用了 memcpy 进行了简单优化

```C
static inline int _rsmq_len(rsmq_t q) {
	int tail = q->tail;
	if (tail < 0)
		return 0;

	tail -= q->head - 1;
	return tail > 0 ? tail : tail + q->cap;
}

static inline void _rsmq_expand(rsmq_t q, int sz) {
	// 确定是够需要扩充内存
	int cap = q->cap;
	int head = q->head;
	int len = _rsmq_len(q);
	if (len + sz <= cap)
		return;

	// 开始构建所需内存
	do cap <<= 1; while (len + sz > cap);
	char * nbuf = malloc(cap);
	assert(NULL != nbuf);

	if (len <= 0)
		q->tail = -1;
	else {
		if (head + len <= cap)
			memcpy(nbuf, q->buff + head, len);
		else {
			memcpy(nbuf, q->buff + head, q->cap - head);
			memcpy(nbuf + q->cap - head, q->buff, len + head - q->cap);
		}
		q->tail = len;
	}

	// 数据定型操作
	free(q->buff);
	q->cap = cap;
	q->head = 0;
	q->buff = nbuf;
}

void 
rsmq_push(rsmq_t q, const void * data, uint32_t sz) {
	int tail, cap, len = (int)sz;
	ATOM_LOCK(q->lock);

	// 开始检测一下内存是否足够
	_rsmq_expand(q, len);

	cap = q->cap;
	tail = q->tail + 1;
	// 分布填充数据
	if (tail + len < cap)
		memcpy(q->buff + tail, data, len);
	else {
		memcpy(q->buff + tail, data, cap - tail);
		memcpy(q->buff, (const char *)data + cap - tail, tail + len - cap);
	}
	q->tail = (q->tail + len) & (cap - 1);

	ATOM_UNLOCK(q->lock);
}
```

    这类代码不是很好理解, 强烈推荐抄一遍, 再抄一遍, 以后全部都懂了. 一次书写终生受用.
    (难在业务的杂糅和代码的优化) pop 思路是: 先 pop size, 后根据 size, pop data.

```C
static void _rsmq_pop_dn(rsmq_t q, void * d, int len) {
	char * nbuf = d;
	int head = q->head, cap = q->cap;

	if (head + len <= cap)
		memcpy(nbuf, q->buff + head, len);
	else {
		memcpy(nbuf, q->buff + head, q->cap - head);
		memcpy(nbuf + q->cap - head, q->buff, len + head - q->cap);
	}

	q->head = (head + len) & (cap - 1);
	// 这是empty,情况, 重置
	if (_rsmq_len(q) == q->cap) {
		q->tail = -1;
		q->head = 0;
	}
}

static inline void _rsmq_pop_sz(rsmq_t q) {
	_rsmq_pop_dn(q, &q->sz, sizeof(q->sz));
	q->sz = sh_ntoh(q->sz);
}

//
// rsmq_pop - 数据队列中弹出一个解析好的消息体
// q		: 数据队列对象, rsmq_create 创建
// pmsg		: 返回的消息体对象指针
// return	: ErrParse 协议解析错误, ErrEmpty 数据不完整, SufBase 解析成功
//
int 
rsmq_pop(rsmq_t q, msgrs_t * pmsg) {
	int len, cnt;
	msgrs_t msg;
	ATOM_LOCK(q->lock);

	cnt = _rsmq_len(q);

	// step 1 : 报文长度 buffq.sz check
	if (q->sz <= 0 && cnt >= sizeof(uint32_t)) {
		// 得到报文长度, 小端网络字节转成本地字节
		_rsmq_pop_sz(q);
		cnt -= sizeof(q->sz);
	}

	
	len = MSGRS_LEN(q->sz);
	// step 2 : check data parse is true
	if (len > USHRT_MAX || (q->sz > 0 && len <= 0)) {
		ATOM_UNLOCK(q->lock);
		return ErrParse;
	}

	// step 3 : buffq.sz > 0 继续看是否有需要的报文内容
	if (len <= 0 || len > cnt) {
		ATOM_UNLOCK(q->lock);
		return ErrEmpty;
	}

	// 索要的报文长度存在, 构建好给弹出去
	msg = malloc(sizeof(*msg) + len);
	assert(NULL != msg);
	// 返回数据
	msg->sz = q->sz;
	_rsmq_pop_dn(q, msg->data, len);
	q->sz = 0;
	*pmsg = msg;

	ATOM_UNLOCK(q->lock);
	return SufBase;
}
```

    小端 size + data 传输环形字符消息队列构建完毕. 从此网络中流式传输问题就解决了. 此刻
	不知道有没有感觉出来, 现在代码的能量越来越高, 适用性越来越针对. 码多了会发现, 很多极
	致优化的方案, 都是偏方, 心智成本高. 咱们这里传授的武功秘籍, 只要你多比划多实战. 必定
	不会被天外飞仙这种失传的绝技一招干死, 怎么着也 Double kill.

江湖中杜撰过一句话, 内功决定能飞多高, 武技能决定能跑多久~

***
    ...
    每一条路的尽头 是一个人
    看不透 世上的真
    你还是 这样天真
    假如我可以再生
    像太极为两仪而生
    动静间如行云流水
    追一个豁达的眼神
    ...

***

### 4.5 阅读理解, 插入个配置解析库

    内功修炼中写了个 dict 数据结构, 不妨为其出个阅读理解吧. 构建一个高效的配置解析库.
    还是觉得代码比文字值钱, 说再多抵不上简单实在的代码. 毕竟对于实践派而言 run 起来才
	是真理. 这个配置库没有什么思维逻辑过程, 仿照 php 变量解析设计的. 看接口设计:

scconf.h

```C
#ifndef _H_SIMPLEC_SCCONF
#define _H_SIMPLEC_SCCONF

#include "dict.h"

/*
 * 这里是配置文件读取接口,
 * 写配置,读取配置,需要指定配置的路径
 * 
 * 配置规则 , 参照 php 变量定义.
 * 举例, 文件中可以配置如下:

	 $heoo = "Hello World\n";

	 $yexu = "\"你好吗\",
	 我很好.谢谢!";

	 $end = "coding future 123 runing, ";
 
 * 
 * 后面可以通过, 配置文件读取出来. conf_get("heoo") => "Hello World\n"
 */

//
// conf_xxxx 得到配置写对象, 失败都返回NULL 
// path		: 配置所在路径
// conf		: conf_create 创建的对象
// key		: 查找的key
// return	: 返回要得到的对象, 失败为NULL 
//
extern dict_t conf_create(const char * path);
extern void conf_delete(dict_t conf);
extern const char * conf_get(dict_t conf, const char * key);

#endif // !_H_SIMPLEC_SCCONF
```

    当前配置解析的思路是, 逐个读取文件中字符. 当然也可以自行优化, 批量读取. 
    逐个读取优势是节约内存. 其它都是老套路, 依赖 dict::set, dict::get.

```C
#include "scconf.h"
#include "tstr.h"

// 函数创建函数, kv 是 [ abc\012345 ]这样的结构
static void _sconf_create(dict_t conf, char * key) {
	char * value = key;
	while (*value++)
		;
	dict_set(conf, key, strdup(value));
}

// 将这一行读取完毕
#define READBR(txt, c) \
	while (c != EOF && c != '\n') \
		c = fgetc(txt)

// 开始解析串
static void _sconf_parse(dict_t root, FILE * txt) {
	char c, n;
	TSTR_CREATE(tstr);

	//这里处理读取问题
	while ((c = fgetc(txt)) != EOF) {
		//1.0 先跳过空白字符
		while (c != EOF && isspace(c))
			c = fgetc(txt);

		//2.0 如果遇到第一个字符不是 '$'
		if (c != '$') { 
			READBR(txt, c);
			continue;
		}
		//2.1 第一个字符是 $ 合法字符, 开头不能是空格,否则也读取完毕
		if ((c = fgetc(txt)) != EOF && isspace(c)) {
			READBR(txt, c);
			continue;
		}

		//开始记录了
		tstr->len = 0;

		//3.0 找到第一个等号 
		while (c != EOF && c != '=') {
			if(!isspace(c))
				tstr_appendc(tstr, c);
			c = fgetc(txt);
		}
		if (c != '=') // 无效的解析直接结束
			break;

		c = '\0';
		//4.0 找到 第一个 "
		while (c != EOF && c != '\"') {
			if (!isspace(c))
				tstr_appendc(tstr, c);
			c = fgetc(txt);
		}
		if (c != '\"') // 无效的解析直接结束
			break;

		//4.1 寻找第二个等号
		for (n = c; (c = fgetc(txt)) != EOF; n = c) {
			if (c == '\"' ) {
				if (n != '\\')
					break;
				// 回退一个 '\\' 字符
				--tstr->len;
			}
			tstr_appendc(tstr, c);
		}
		if (c != '\"') //无效的解析直接结束
			break;

		// 这里就是合法字符了,开始检测 了, 
		_sconf_create(root, tstr_cstr(tstr));

		// 最后读取到行末尾
		READBR(txt, c);
		if (c != '\n')
			break;
	}

	TSTR_DELETE(tstr);
}

dict_t 
conf_create(const char * path) {
	dict_t conf;
	FILE * txt = fopen(path, "rb");
	if (NULL == txt) {
		RETURN(NULL, "fopen  r is error! path = %s.", path);
	}

	// 创建具体配置二叉树对象
	conf = dict_create(free);
	if (conf) {
		// 解析添加具体内容
		_sconf_parse(conf, txt);
	}

	fclose(txt);
	return conf;
}

inline void 
conf_delete(dict_t conf) {
	dict_delete(conf);
}

inline const char * 
conf_get(dict_t conf, const char * key) {
	return dict_get(conf, key);
}
```

    _sconf_parse 过程篇幅大点, 解析文件中内容. 其它使用 dict.h 接口的方法. 也许这就
	是追求的学以致用(但真相是, 我是想写个 sconf, 不得不写个 dict 哈).  
    内功部分, 带大家走了一遍 rbtree -> dict -> mq -> rsmq -> sconf 围绕常用容器.
    一个人内部容器有多大, 就能容纳多少, 就会有多深内功. 有种错觉, 看书的人到这里可以
	结束一段时间了. 以后将不会那么平静 ~ 妖魔战场已经向你发起了召唤令 ~

***

	日月神教，战无不胜。东方教主，文成武德。千秋万载，一统江湖。

***

![兰花草](./img/兰花草.jpg)