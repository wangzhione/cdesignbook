## 第5章-内功-数据结构下卷

    恭喜到这了. 此刻是一种新的开始, 本章可以算开发中数据结构使用的末尾阶段了. 会展
    示金丹, 元婴 战斗中时常出现的内劲. 漫天飛絮, 气流涌动 ~
    随后也许你觉得好复杂, 也许觉得很简单. 因为 C程序一个要求就是, 你需要懂得实现. 
    才能运用流畅. 一切都是钻木取火, 自生自灭. 扯一点, 编译型语言要是有那种万能数据
    结构 array 或者 table 那生产率预估会提升 10倍. 写代码就和玩似的. 
    今天完工等价于 C基础数据结构已经全线通工. C的代码写的越多, 越发觉得喜欢就好!

    谁都想成就元婴, 何不乘早, 码穿键盘 ~ 书出心眼 ~ 

### 5.1 红黑树, 一道坎

    红黑树的理论, 推荐找材料恶补. 它打到效果是, 能够防止二叉搜索树退化为有序的双向
    链表. 相似的替代有跳跃表, hash 桶. 但具体使用什么, 因个人喜好. 作者只能站在自
    己框架用到的, 实现的角度出发. 带大家感受, 那些瞎比调整的二叉树结点 ~
    首先看总的设计野路子:

rbtree.h

```C
#ifndef _H_SIMPLEC_RBTREE
#define _H_SIMPLEC_RBTREE

#include <struct.h>

struct $rbnode {
	uintptr_t parent_color;
	struct $rbnode * right;
	struct $rbnode * left;
};

typedef struct {
	struct $rbnode * root;
	vnew_f new;
	node_f die;
	icmp_f cmp;
} * rbtree_t;

/*
 * 每个想使用红黑树的结构, 需要在头部插入下面宏. 
 * 例如 :
	struct person {
		_HEAD_RBTREE;
		... // 自定义信息
	};
 */
#define _HEAD_RBTREE	struct $rbnode $node

/*
 * 创建一颗红黑树头结点 
 * new		: 注册创建结点的函数
 * die		: 注册程序销毁函数
 * cmp		: 注册比较的函数
 * return	: 返回创建好的红黑树结点
 */
extern rbtree_t rb_create(vnew_f new, node_f die, icmp_f cmp);

/*
 * 插入一个结点, 会插入 new(pack)
 * root		: 红黑树头结点
 * pack		: 待插入的结点当cmp(x, pack) 右结点
 */
extern void rb_insert(rbtree_t tree, void * pack);

/*
 * 删除能和pack匹配的结点
 * root		: 红黑树结点
 * pack		: 当cmp(x, pack) 右结点
 */
extern void rb_remove(rbtree_t tree, void * pack);

/*
 * 得到红黑树中匹配的结点
 * root		: 匹配的结点信息
 * pack		: 当前待匹配结点, cmp(x, pack)当右结点处理
 */
extern void * rb_find(rbtree_t tree, void * pack);

/*
 * 销毁这颗二叉树
 * root		: 当前红黑树结点
 */
extern void rb_delete(rbtree_t tree);

#endif /* _H_SIMPLEC_RBTREE */
```

    这里先通过结构来感受设计的意图, 例如 rbtree_t 结构中 new, die, cmp 分别用于红黑树中
    insert 创建结点的时候, delete 销毁结点, find 查找结点的时候的比对规则. 基于注册的设
    计思路. 

    _HEAD_RBTREE 宏结点同样是插入到想实现红黑树结构中. C 中一种结构继承的技巧, 默认偏移量
    为 0, 隐含的潜规则是 $node 结点的首地址和当前结构的首地址一样. 
    
    对于 uintptr_t parent_color 它既能表示红黑树的父亲结点, 也能表示当前是红是黑标识. 
    主要利用原理是, 当前结构的内存布局以指针(x86 4字节, x64 8字节)位对齐的. 因为地址一定
    是4的倍数, 0100 的倍数, 所以最后两位用不上, 这里用于标识红黑.

    设计方面主要就如上了. 随后看详细实现部分: 

### 5.1.1 红黑树初步构建

    首先为 struct $rbnode 添加一些辅助操作, 方便判断父亲结点, 颜色标识等

```C
/*
 * 操作辅助宏, 得到红黑树中具体父结点, 颜色. 包括详细设置信息
 * r	: 头结点
 * p	: 父结点新值
 * c	: 当前颜色
 */
#define rb_parent(r)		((struct $rbnode *)((r)->parent_color & ~3))
#define rb_color(r)			((r)->parent_color & 1)
#define rb_is_red(r)		(!rb_color(r))
#define rb_is_black(r)		rb_color(r)
#define rb_set_red(r)		(r)->parent_color &= ~1
#define rb_set_black(r)		(r)->parent_color |= 1

static inline void rb_set_parent(struct $rbnode * r, struct $rbnode * p) {
     r->parent_color = (r->parent_color & 3) | (uintptr_t)p;
}

static inline void rb_set_color(struct $rbnode * r, int color) {
     r->parent_color = (r->parent_color & ~1) | (1 & color);
}
```

    通过上面接口设计和结构的辅助操作设计, 基本上 struct $rbnode 接口可以顺手使用了.
    有个默认小前提, 需要在学习之前, 默认就对二叉搜索树查找删除套路有个模子. 红黑树
    仅在普通搜索树基础上加了颜色标识, 来回调整. 首先看一下基于注册的创建函数

```C
static inline void * _rb_dnew(void * node) { return node; }
static inline void _rb_ddie(void * node) { }
static inline int _rb_dcmp(const void * ln, const void * rn) { 
	return (int)((intptr_t)ln - (intptr_t)rn); 
}

inline rbtree_t 
rb_create(vnew_f new, node_f die, icmp_f cmp) {
	rbtree_t tree = malloc(sizeof(*tree));
	if(NULL == tree) {
		RETURN(NULL, "rb_new malloc is error!");
	}
	
	tree->root = NULL;
	tree->new = new ? new : _rb_dnew;
	tree->die = die ? die : _rb_ddie;
	tree->cmp = cmp ? cmp : _rb_dcmp;

	return tree;
}
```

    对于用户注册 _rb_new 只能初始化用户使用的结构内存, 我们自己的 $node 还需要初始化,
    因而要包装一层. 初始化全零数据:

```C
static inline struct $rbnode * _rb_new(rbtree_t tree, void * pack) {
	struct $rbnode * node = tree->new(pack);
	memset(node, 0, sizeof(struct $rbnode));
	return node;
}
```

    随后查找和删除都是老套路, 和二叉搜索树是一模样

```C
void * 
rb_find(rbtree_t tree, void * pack) {
	icmp_f cmp;
	struct $rbnode * node;
	if((!tree) || !pack) {
		RETURN(NULL, "rb_get param is empty! tree = %p, pack = %p.\n", tree, pack);	
	}
	
	cmp = tree->cmp;
	node = tree->root;
	while(node) {
		int ct = cmp(node, pack);
		if(ct == 0)
			break;
		node = ct > 0 ? node->left : node->right;
	}

	return node;
}

// 后序遍历删除操作
static void _rb_delete(struct $rbnode * root, node_f die) {
	if(NULL == root) return;
	_rb_delete(root->left, die);
	_rb_delete(root->right, die);
	die(root);
}

inline void
rb_delete(rbtree_t tree) {
	if(!tree || !tree->root || tree->die == _rb_ddie)
		return;

	// 后续递归删除
	_rb_delete(tree->root, tree->die);

	// 销毁树本身内存
	tree->root = NULL;
	free(tree);
}
```

    后续会逐渐展开红黑树大戏了, 继续细化代码, 逐个分析. 红黑树中有个重要操作.
    左旋和右旋, 例如左旋代码如下

```C
/* 
 * 对红黑树的节点(x)进行左旋转
 *
 * 左旋示意图(对节点x进行左旋)：
 *      px                              px
 *     /                               /
 *    x                               y                
 *   /  \      --(左旋)-->           / \                #
 *  lx   y                          x  ry     
 *     /   \                       /  \
 *    ly   ry                     lx  ly  
 *
 */
static void _rbtree_left_rotate(rbtree_t tree, struct $rbnode * x) {
    // 设置x的右孩子为y
    struct $rbnode * y = x->right;
	struct $rbnode * xparent = rb_parent(x);

    // 将 “y的左孩子” 设为 “x的右孩子”；
    x->right = y->left;
	// 如果y的左孩子非空，将 “x” 设为 “y的左孩子的父亲”
    if (y->left != NULL)
		rb_set_parent(y->left, x);

    // 将 “x的父亲” 设为 “y的父亲”
	rb_set_parent(y, xparent);

    if (xparent == NULL)
        tree->root = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
    else {
        if (xparent->left == x)
            xparent->left = y;     // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
        else
            xparent->right = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
    }
    
    // 将 “x” 设为 “y的左孩子”
    y->left = x;
    // 将 “x的父节点” 设为 “y”
	rb_set_parent(x, y);
}
```

    为什么有这么搞的事情呢, 主要是为了满足红黑树五大特性

    特性1. 每个节点或者是黑色, 或者是红色.

    特性2. 根节点是黑色.

    特性3. 每个叶子节点（NIL）是黑色. [这里叶子节点，是指为空(NIL或NULL)的叶子节点!]

    特性4. 如果一个节点是红色的, 则它的子节点必须是黑色的.

    特性5. 从一个节点到该节点的子孙节点的所有路径上包含相同数目的黑节点.

    左右旋转就是为了满足特性5设计的. 相应的右旋操作

```C
/* 
 * 对红黑树的节点(y)进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                               py
 *           /                                /
 *          y                                x                  
 *         /  \      --(右旋)-->            /  \                     #
 *        x   ry                           lx   y  
 *       / \                                   / \                  #
 *      lx  rx                                rx  ry
 * 
 */
static void _rbtree_right_rotate(rbtree_t tree, struct $rbnode * y) {
    // 设置x是当前节点的左孩子。
    struct $rbnode * x = y->left;
	struct $rbnode * yparent = rb_parent(y);

    // 将 “x的右孩子” 设为 “y的左孩子”；
	y->left = x->right;
    // 如果"x的右孩子"不为空的话，将 “y” 设为 “x的右孩子的父亲”
    if (x->right != NULL)
		rb_set_parent(x->right, y);

    // 将 “y的父亲” 设为 “x的父亲”
    rb_set_parent(x, yparent);
    if (yparent == NULL) 
        tree->root = x;				// 如果 “y的父亲” 是空节点，则将x设为根节点
    else {
        if (y == yparent->right)
            yparent->right = x;		// 如果 y是它父节点的右孩子，则将x设为“y的父节点的右孩子”
        else
            yparent->left = x;		// (y是它父节点的左孩子) 将x设为“x的父节点的左孩子”
    }

    // 将 “y” 设为 “x的右孩子”
    x->right = y;
    // 将 “y的父节点” 设为 “x”
	rb_set_parent(y, x);
}
```

    扯一点, 注释很详细, 看起来也很头痛. 木办法, 需要课下下功夫. 疯狂的查阅相关资料, 照着算法
    解释抄一遍代码, 写一遍代码, 理解一遍, 坎就过了.

### 5.1.2 红黑树 insert

    首先拾人牙慧, 弄了个步骤说明. 
    
    将一个节点插入到红黑树中. 首先, 将红黑树当作一颗二叉查找树, 将节点插入. 然后, 将节点着
    色为红色. 最后, 通过旋转和重新着色等方法来修正该树, 使之重新成为一颗红黑树. 
    详细描述如下:
    第一步: 将红黑树当作一颗二叉查找树，将节点插入
       红黑树本身就是一颗二叉查找树, 将节点插入后, 该树仍然是一颗二叉查找树. 也就意味着, 
       树的键值仍然是有序的. 此外, 无论是左旋还是右旋, 若旋转之前这棵树是二叉查找树, 旋转之
       后它一定还是二叉查找树. 这也就意味着, 任何的旋转和重新着色操作, 都不会改变它仍然是
       一颗二叉查找树的事实.

    第二步：将插入的节点着色为"红色"
       将插入的节点着色为红色, 不会违背'特性5'!

    第三步: 通过一系列的旋转或着色等操作，使之重新成为一颗红黑树
       第二步中, 将插入节点着色为'红色'之后, 不会违背'特性5'. 那它会违背哪些特性呢?
       对于'特性1', 显然不会违背了. 因为我们已经将它涂成红色了。
       对于'特性2', 显然也不会违背. 在第一步中，我们是将红黑树当作二叉查找树, 然后执行
        的插入操作. 而根据二叉查找数的特点, 插入操作不会改变根节点. 所以，根节点仍然是黑色.
       对于'特性3'，显然不会违背. 叶子节点是指的空叶子节点, 插入非空节点并不会对其造成影响
       对于'特性4', 是有可能违背的！
       那接下来, 想办法使之'满足特性4', 就可以将树重新构造成红黑树了。
    下面看看代码到底是怎样实现这三步的:

```C
/*
 * 红黑树插入修正函数
 *
 * 在向红黑树中插入节点之后(失去平衡)，再调用该函数；
 * 目的是将它重新塑造成一颗红黑树。
 *
 * 参数说明：
 *     tree 红黑树的根
 *     node 插入的结点        // 对应《算法导论》中的z
 */
static void _rbtree_insert_fixup(rbtree_t tree, struct $rbnode * node) {
    struct $rbnode * parent, * gparent, * uncle;

    // 若“父节点存在，并且父节点的颜色是红色”
    while ((parent = rb_parent(node)) && rb_is_red(parent)) {
        gparent = rb_parent(parent);

        //若“父节点”是“祖父节点的左孩子”
        if (parent == gparent->left) {
            // Case 1条件：叔叔节点是红色
            uncle = gparent->right;
            if (uncle && rb_is_red(uncle)) {
                rb_set_black(uncle);
                rb_set_black(parent);
                rb_set_red(gparent);
                node = gparent;
                continue;
            }

            // Case 2条件：叔叔是黑色，且当前节点是右孩子
            if (parent->right == node) {
                _rbtree_left_rotate(tree, parent);
                uncle = parent;
                parent = node;
                node = uncle;
            }

            // Case 3条件：叔叔是黑色，且当前节点是左孩子。
            rb_set_black(parent);
            rb_set_red(gparent);
            _rbtree_right_rotate(tree, gparent);
        } 
        else { //若“z的父节点”是“z的祖父节点的右孩子”
            // Case 1条件：叔叔节点是红色
            uncle = gparent->left;
            if (uncle && rb_is_red(uncle)) {
                rb_set_black(uncle);
                rb_set_black(parent);
                rb_set_red(gparent);
                node = gparent;
                continue;
            }

            // Case 2条件：叔叔是黑色，且当前节点是左孩子
            if (parent->left == node) {
                _rbtree_right_rotate(tree, parent);
                uncle = parent;
                parent = node;
                node = uncle;
            }

            // Case 3条件：叔叔是黑色，且当前节点是右孩子。
            rb_set_black(parent);
            rb_set_red(gparent);
            _rbtree_left_rotate(tree, gparent);
        }
    }

    // 将根节点设为黑色
    rb_set_black(tree->root);
}

void 
rb_insert(rbtree_t tree, void * pack) {
	icmp_f cmp;
	struct $rbnode * node, * x, * y;
	if((!tree) || (!pack) || !(node = _rb_new(tree, pack))) {
		RETURN(NIL, "rb_insert param is empty! tree = %p, pack = %p.\n", tree, pack);
	}
	
	cmp = tree->cmp;
	// 开始走插入工作
	y = NULL;
	x = tree->root;

	// 1. 将红黑树当作一颗二叉查找树，将节点添加到二叉查找树中。从小到大
	while (x != NULL) {
		y = x;
		if (cmp(x, node) > 0)
			x = x->left;
		else
			x = x->right;
	}
	rb_set_parent(node, y);

	if (y != NULL) {
		if (cmp(y, node) > 0)
			y->left = node;             // 情况2：若“node所包含的值” < “y所包含的值”，则将node设为“y的左孩子”
		else
			y->right = node;            // 情况3：(“node所包含的值” >= “y所包含的值”)将node设为“y的右孩子” 
	}
	else
		tree->root = node;              // 情况1：若y是空节点，则将node设为根

	// 2. 设置节点的颜色为红色
	rb_set_red(node);

	// 3. 将它重新修正为一颗二叉查找树
	_rbtree_insert_fixup(tree, node);
}
```

    手写红黑树是个挑战. 参照前辈们无数历练的手稿, 顺带把上面从 linux上拔下来的代码
    敲个一遍. 还是可以容易做的的. 毕竟xx哪有那么简单

### 5.1.3 红黑树 remove

    将红黑树内的某一个节点删除. 需要执行的操作依次是: 首先, 将红黑树当作一颗二叉
    查找树, 将该节点从二叉查找树中删除. 然后, 通过'旋转和重新着色' 等一系列来修
    正该树, 使之重新成为一棵红黑树. 详细描述如下:

    第一步: 将红黑树当作一颗二叉查找树, 将节点删除
       这和'删除常规二叉查找树中删除节点的方法是一样的'. 
       分3种情况:
        1. 被删除节点没有儿子, 即为叶节点. 那么, 直接将该节点删除就OK了.

        2. 被删除节点只有一个儿子. 那么, 直接删除该节点, 并用该节点的唯一子节点顶替它的位置.

        3. 被删除节点有两个儿子. 那么, 先找出它的后继节点. 然后把'它的后继节点的内容'复制给
           '该节点的内容'. 之后, 删除'它的后继节点'. 在这里, 后继节点相当于替身, 在将后继
           节点的内容复制给'被删除节点'之后, 再将后继节点删除. 这样就巧妙的将问题转换为'删
           除后继节点'的情况了, 下面就考虑后继节点. 在'被删除节点'有两个非空子节点的情况下,
           它的后继节点不可能是双子非空. 既然'的后继节点'不可能双子都非空, 就意味着'该节点
           的后继节点'要么没有儿子, 要么只有一个儿子. 若没有儿子, 则按'情况1' 进行处理.若
           只有一个儿子, 则按'情况2' 进行处理.

    第二步: 通过'旋转和重新着色'等一系列来修正该树, 使之重新成为一棵红黑树.
       因为'第一步'中删除节点之后, 可能会违背红黑树的特性. 所以需要通过
       '旋转和重新着色'来修正该树, 使之重新成为一棵红黑树. 

    最终对着算法说明和linux红黑树源码, 一种实现如下:

```C
/*
 * 红黑树删除修正函数
 *
 * 在从红黑树中删除插入节点之后(红黑树失去平衡)，再调用该函数；
 * 目的是将它重新塑造成一颗红黑树。
 *
 * 参数说明：
 *     tree 红黑树的根
 *     node 待修正的节点
 */
static void _rbtree_delete_fixup(rbtree_t tree, struct $rbnode * node, struct $rbnode * parent) {
    struct $rbnode * other;

    while ((!node || rb_is_black(node)) && node != tree->root) {
        if (parent->left == node) {
            other = parent->right;
            if (rb_is_red(other)) {
                // Case 1: x的兄弟w是红色的  
                rb_set_black(other);
                rb_set_red(parent);
                _rbtree_left_rotate(tree, parent);
                other = parent->right;
            }
            if ((!other->left || rb_is_black(other->left)) &&
                (!other->right || rb_is_black(other->right))) {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                rb_set_red(other);
                node = parent;
                parent = rb_parent(node);
            }
            else {
                if (!other->right || rb_is_black(other->right)) {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  
                    rb_set_black(other->left);
                    rb_set_red(other);
                    _rbtree_right_rotate(tree, other);
                    other = parent->right;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。
                rb_set_color(other, rb_color(parent));
                rb_set_black(parent);
                rb_set_black(other->right);
                _rbtree_left_rotate(tree, parent);
                node = tree->root;
                break;
            }
        }
        else {
            other = parent->left;
            if (rb_is_red(other)) {
                // Case 1: x的兄弟w是红色的  
                rb_set_black(other);
                rb_set_red(parent);
                _rbtree_right_rotate(tree, parent);
                other = parent->left;
            }
            if ((!other->left || rb_is_black(other->left)) &&
                (!other->right || rb_is_black(other->right))) {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                rb_set_red(other);
                node = parent;
                parent = rb_parent(node);
            }
            else {
                if (!other->left || rb_is_black(other->left)) {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  
                    rb_set_black(other->right);
                    rb_set_red(other);
                    _rbtree_left_rotate(tree, other);
                    other = parent->left;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。
                rb_set_color(other, rb_color(parent));
                rb_set_black(parent);
                rb_set_black(other->left);
                _rbtree_right_rotate(tree, parent);
                node = tree->root;
                break;
            }
        }
    }
    if (node)
        rb_set_black(node);
}

void 
rb_remove(rbtree_t tree, void * pack) {
	struct $rbnode * child, * parent, * node = NULL;
	int color;
	
	if ((!tree) || !(node = (struct $rbnode *)pack)) {
		RETURN(NIL, "rb_remove check is error, tree = %p, node = %p.", tree, node);
	}

	// 被删除节点的"左右孩子都不为空"的情况。
	if (NULL != node->left && node->right != NULL) {
		// 被删节点的后继节点。(称为"取代节点")
		// 用它来取代"被删节点"的位置，然后再将"被删节点"去掉。
		struct $rbnode * replace = node;

		// 获取后继节点
		replace = replace->right;
		while (replace->left != NULL)
			replace = replace->left;

		// "node节点"不是根节点(只有根节点不存在父节点)
		if ((parent = rb_parent(node))) {
			if (parent->left == node)
				parent->left = replace;
			else
				parent->right = replace;
		} 
		else 
			// "node节点"是根节点，更新根节点。
			tree->root = replace;

		// child是"取代节点"的右孩子，也是需要"调整的节点"。
		// "取代节点"肯定不存在左孩子！因为它是一个后继节点。
		child = replace->right;
		parent = rb_parent(replace);
		// 保存"取代节点"的颜色
		color = rb_color(replace);

		// "被删除节点"是"它的后继节点的父节点"
		if (parent == node)
			parent = replace; 
		else {
			// child不为空
			if (child)
				rb_set_parent(child, parent);
			parent->left = child;

			replace->right = node->right;
			rb_set_parent(node->right, replace);
		}
		
		rb_set_parent(replace, rb_parent(node));
		rb_set_color(replace, rb_color(node));
		replace->left = node->left;
		rb_set_parent(node->left, replace);

		if (color) // 黑色结点重新调整关系
			_rbtree_delete_fixup(tree, child, parent);
		// 结点销毁操作
		tree->die(node);
		return ;
	}

	if (node->left !=NULL)
		child = node->left;
	else 
		child = node->right;

	parent = rb_parent(node);
	// 保存"取代节点"的颜色
	color = rb_color(node);

	if (child)
		rb_set_parent(child, parent);

	// "node节点"不是根节点
	if (parent) {
		if (parent->left == node)
			parent->left = child;
		else
			parent->right = child;
	}
	else
		tree->root = child;

	if (!color)
		_rbtree_delete_fixup(tree, child, parent);
	tree->die(node);
}
```

    上面代码非线性的, 有一定深度. 可以看看, 扩展视野, 笑话吸收成自己的代码库. 写代码
    很多时候要和姑苏慕容学习, 以彼之道还治彼身. 这里关于红黑树的梗过去了, 心安~

### 5.2 趁热打铁 map -> dict

    红黑树是上层 map库的实现基石, 同样另一种相似的查找库 dict, 采用的是 hash 桶算法.
    主要应用场景就是通过 key -> value, 一种查找业务操作. 直接上接口设计:

dict.h

```C
#ifndef _H_SIMPLEC_DICT
#define _H_SIMPLEC_DICT

#include "struct.h"

typedef struct dict * dict_t;

//
// dict_create - 创建一个以C字符串为key的字典
// die		: val 销毁函数
// return	: void
//
extern dict_t dict_create(node_f die);
extern void dict_delete(dict_t d);

//
// dict_set - 设置一个<k, v> 结构
// d		: dict_create 创建的字典对象
// k		: 插入的key, 重复插入会销毁已经插入的
// v		: 插入数据的值
// return	: void
//
extern void dict_set(dict_t d, const char * k, void * v);
extern void dict_die(dict_t d, const char * k);

extern void * dict_get(dict_t d, const char * k);

#endif//_H_SIMPLEC_DICT
```

    内功练到后面是不是有种势如破竹的感觉, 清晰易懂, 简单明了. 
    随后开始详细设计意图剖析, 来看结构部分, 内功中的气海结构 :

```C
#include "dict.h"
#include "tstr.h"

struct keypair {
	struct keypair * next;
	unsigned hash;
	char * key;
	void * val;
};

struct dict {
	node_f die;
	unsigned used; // 用户使用的
	unsigned size; // table 的 size, 等同于桶数目
	struct keypair ** table;
};
```

    dict::table 就是我们的 hash池子, 存放所有 struct keypair结构. 如果冲突了, 
    那就向 keypair桶结构中插入. 如果 hash池子满了, 那就重新挖一个大点的池子, 重新
    调整所有关系. 这就是核心思想, 真清除易懂! 不妨详细看看创建的设计思路:

```C
#define _UINT_INITSZ	(2 << 7)

dict_t 
dict_create(node_f die) {
	struct dict * d = malloc(sizeof(struct dict));
	if (NULL == d) {
		RETURN(NULL, "malloc sizeof struct dict is error!");
	}

	d->table = calloc(_UINT_INITSZ, sizeof(struct keypair *));
	if (NULL == d->table) {
		free(d);
		RETURN(NULL, "calloc sizeof(struct keypair) is error!");
	}

	d->die = die;
	d->used = 0;
	d->size = _UINT_INITSZ;

	return d;
}
```

    dict_create 所做的工作, 对外记录玩家注册的结点清除行为. 对内安排内存并初始化.
    有了构建那么删除也必须有:

```C
// 销毁数据
static inline void _keypair_delete(struct dict * d, struct keypair * pair) {
	free(pair->key);
	if (pair->val && d->die)
		d->die(pair->val);
	free(pair);
}

void 
dict_delete(dict_t d) {
	if (NULL == d)
		return;

	for (unsigned i = 0; i < d->size; ++i) {
		struct keypair * pair = d->table[i];
		while (pair) {
			struct keypair * next = pair->next;
			_keypair_delete(d, pair);
			pair = next;
		}
	}
	free(d->table);
	free(d);
}
```

    聊到现在大家也能看出来, create and delete 都是成对出现. 名字可以随意, 但是这是
    基本的态度, 创建和销毁一定要做好. 特别是销毁, 做人当如君子(伪君子也行). 有骨气.
    渡人清风, 苦练内功正能量(爱自己,  护亲人). 哈哈

    上面是彻底销毁创建的 dict, 单独删除 dict中子节点呢?

```C
void 
dict_die(dict_t d, const char * k) {
	unsigned hash, idx;
	struct keypair * pair, * front;
	if (!d || !k)
		return;

	hash = tstr_hash(k);
	idx = hash & (d->size - 1);
	pair = d->table[idx];

	front = NULL;
	while (pair) {
		if (pair->hash == hash && !strcmp(pair->key, k)) {
			// 找见数据, 调整结点关系和开始删除
			if (front == NULL)
				d->table[idx] = pair->next;
			else
				front->next = pair->next;
			
			// 删除数据
			_keypair_delete(d, pair);
			--d->used;
		}
		front = pair;
		pair = pair->next;
	}
}
```

    思路无外乎, 字符串映射为 hash值, 通过 hash 取余得到查找的桶. 对桶进行详细勘察
    strcmp. 随后就是走链表销毁那一套了. 性能高效不失简单. 同样得到查的结点, 也是上面
    思路, hash 取余返回

```C
inline void * 
dict_get(dict_t d, const char * k) {
	unsigned hash, idx;
	struct keypair * pair;
	assert(d != NULL && k != NULL);

	hash = tstr_hash(k);
	idx = hash & (d->size - 1);
	pair = d->table[idx];

	return pair ? pair->val : NULL;
}
```

    dict 修炼最后一关, set 设值

```C
// 重新调整hash表大小
static void _dict_resize(dict_t d) {
	unsigned size = d->size;
	unsigned used = d->used;
	struct keypair ** table;

	if (used < size)
		return;
	
	// 开始构建新内存
	do size <<= 1; while (size > used);
	table = calloc(size, sizeof(struct keypair *));
	if (NULL == table) {
		RETURN(NIL, "_dict_resize struct keypair * size = %u.", size);
	}

	// 开始转移数据
	for (unsigned i = 0; i < d->size; ++i) {
		struct keypair * pair = d->table[i];
		while (pair) {
			struct keypair * next = pair->next;
			unsigned idx = pair->hash & (size - 1);
			struct keypair * npair = table[idx];
			if (npair) {
				pair = npair->next;
				npair->next = pair;
			}
			else {
				table[idx] = pair;
				pair->next = NULL;
			}

			pair = next;
		}
	}
	free(d->table);

	d->size = size;
	d->table = table;
}

//
// dict_set - 设置一个<k, v> 结构
// d		: dict_create 创建的字典对象
// k		: 插入的key, 重复插入会销毁已经插入的
// v		: 插入数据的值
// return	: void
//
void 
dict_set(dict_t d, const char * k, void * v) {
	unsigned hash, idx;
	struct keypair * pair;
	assert(d != NULL && k != NULL);
	
	// 检查一下内存, 看是否需要重构
	_dict_resize(d);

	// 开始插入数据
	hash = tstr_hash(k);
	idx = hash & (d->size - 1);
	pair = d->table[idx];

	// 数据 modify
	while (pair) {
		if (pair->hash == hash && !strcmp(pair->key, k)) {
			if (d->die)
				d->die(pair->val);
			pair->val = v;
			return;
		}
		pair = pair->next;
	}

	// 没有找见直接创建数据
	pair = _keypair_create(k, v, hash);
	if (pair) {
		++d->used;
		pair->next = d->table[idx];
		d->table[idx] = pair;
	}
}
```

    _dict_resize 工作是当 d->size == d->used 的时候, 需要扩充内存. 随后编译原先
    hash 池子中所有值, 放入新的 hash池子中.
    dict_set 工作是查找待插入的字符串, 开始找到它, 找到了重新设置. 找不见添加值.
    到这里dict 基本完工了. 是不是感觉很简单, 苦练内功学什么都快. 例如张无忌 ~

### 4.3 来个消息队列吧

    消息队列重要, 基本偏C系列的开发中不是链表, 就是消息队列. 消息队列可以理解为咱们
    排队进入火车站. 那个一排栏杆让人一个个的检查过去, 就是消息队列. 消息队列最大的功
    效是让异步开发编程同步并发(多个消息队列). 说白了就是将异步程序变成顺序同步程序.
    开发起来很爽. 而在C中消息队列的标配就是无锁, 大伙时候还记得 scatom.h ~
    金丹期之后的战斗无不是消息队列领域的对撞. 随我走入一个简单高效的无锁消息队列的世
    界中. 

    消息队列本质还是队列, 思路就是通过动态数组和原子锁构建不冲突的 pop 和 push.
    凡事先看接口, 熟悉用法:

mq.h

```C
#ifndef _H_SIMPLEC_MQ
#define _H_SIMPLEC_MQ

#include "struct.h"

typedef struct mq * mq_t;

//
// mq_create - 创建一个消息队列类型
// return	: 返回创建好的消息队列对象, NULL表示失败
//
extern mq_t mq_create(void);

//
// mq_delete - 删除创建消息队列, 并回收资源
// mq		: 消息队列对象
// die		: 删除push进来的结点
// return	: void
//
extern void mq_delete(mq_t mq, node_f die);

//
// mq_push - 消息队列中压入数据
// mq		: 消息队列对象
// msg		: 压入的消息
// return	: void
// 
extern void mq_push(mq_t mq, void * msg);

//
// mq_pop - 消息队列中弹出消息,并返回
// mq		: 消息队列对象
// return	: 返回队列尾巴, 队列为empty返回NULL
//
extern void * mq_pop(mq_t mq);

//
// mq_len - 得到消息队列的长度,并返回
// mq		: 消息队列对象
// return	: 返回消息队列长度
// 
extern int mq_len(mq_t mq);

#endif // !_H_SIMPLEC_MQ
```

    养成好习惯, 先写接口后写实现. 上面几个接口中 mq_len 是个辅助接口, 查询当前消息队列
    中长度. 用于线上监测, 当然这些都可以后期自行添加. 

### 4.3.1 消息队列详细设计

    队列的设计, 一大重点, 就是如何判断队列是否为空, 是否为满. 所传的此门内功不亚于小
    易筋经. 首先看结构:

```C
#include "mq.h"
#include "scatom.h"

// 2 的 幂
#define _INT_MQ				(1 << 6)

//
// 队列empty	<=> tail == -1 ( head = 0 )
// 队列full	<=> head == cap
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

    看上面注释能够理解意图, 额外修改 tail == -1 表示队列为空. 这样做的意图是为了给
    head == cap让步, 很多消息队列设计 head == cap 的时候表示队列需要扩充内存了. 所
    以会造成队列永远不满的情况, 浪费内存. 所以这里设计的原理是每次 push check full
    的时候, 并且已经满了才会扩充内存, 重新调整布局.

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

    纯属大白话, 申请内存随后删除, 删除采用轮序到数据一直为空操作. 核心随后就到,
    mq push 的时候, 检查 queue是否满了, 然后 xxxx 俗套剧情进展

```C
// add two cap memory, memory is do not have assert
static void
_expand_queue(struct mq * mq) {
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
		_expand_queue(mq);
	else
		mq->tail = tail;

	mq->queue[mq->tail] = msg;

	ATOM_UNLOCK(mq->lock);
}
```

    从上面加锁的态度可以看出, 哪里有竞争的地方哪里加锁. 没有一点含蓄, 很实在. 
    pop 也是一样

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

    当 pop 为空的时候, 会设置 tail = -1 和 head = 0. 来标识空状态, 最后统计中用的
    mq_len 返回的只是当时状态的消息队列情况:

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

    有一点需要注意, push的时候 tail移动一格, 此时 tail == head 标识队列满. 
    普通时候 tail == head 标识队列中只有一个元素. 无锁消息队列已经搞完了, 
    强烈推荐引入到自己的项目中, 什么双缓冲都太重, 上面那种最省内存. 上面代码
    很短, 但是思路很多地方都能用到, 随后利用上面思路构建一个网络IO 收发队列.
    用于提供 TCP socket 流式协议无边界一个解决方案.

### 4.4 收发消息的环形队列

    本章已经轻微剧透了元婴功法的消息了. 在我们处理服务器通信的时候, 采用 UDP 报文套接
    子很好处理边界问题. 因为 UDP包有固定大小. 而 TCP 流式套接字一直在收发, 还可能重传
    . 业务层默认它的报文是无边界的. 因此 TCP的报文边界需要程序员自己定义. 
    通过上面分析, 那么我们就来定义收发报文协议, 用于区分每一个消息包. 
    首先看下面结构

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

    我们采用 len + data 构建 bit流传输, len = 8 bit type + 24 bit size. 用于服务器和
    客户端基础通信协议. 

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
    鸡肋, 只是表达一种意愿, 而且不同平台歧义也不一样. 单纯的内联函数是没有函数
    地址的. static inline 会生成单独生成一份函数地址. 看着用吧~

### 4.4.2 收的构建部分

    同样和 mq 无锁环形消息队列设计差不多. 先看 create 和 delete 

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

    随后无外乎就是 push 和 pop, push 的时候这里利用 memcpy 进行了简单优化

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
``

    这类代码不是很好理解, 强烈推荐抄一遍, 再抄一遍, 以后全部都懂了. 一次书写终生受用.
    pop 的思路是先 pop size 后面 得到 size之后 pop data.

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

    小端 size + data的传输环形字符池消息队列构建完毕了. 从此网络中流式传输问题就解决了. 
    此刻不知道有没有感觉出来, 代码的能量越来越高, 适用越来越窄. 很多极致优化的方案, 都是
    偏方, 心智成本高. 咱们这里传授的武功秘籍, 多比划多实战. 内功决定能飞多高, 武技能决定
    能跑多久~

    ...
    每一条路的尽头 是一个人
    看不透 世上的真
    你还是 这样天真
    假如我可以再生
    像太极为两仪而生
    动静间如行云流水
    追一个豁达的眼神
    ...

### 4.5 阅读理解, 插入个配置解析库

    咱们这章其中写了个 dict 数据结构, 不妨为其出个阅读理解吧. 构建一个高效的配置解析库.
    其实一直觉得代码比文字值钱, 说再多抵不上简单, 并且能够跑的很好的代码有说服力. 毕竟
    写的代码思路很直白, 没有什么思维逻辑过程. 继续看接口设计:

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
 * 后面可以通过, 配置文件读取出来. conf_get("heoo") => "你好!"
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

    当前设计思路是, 逐个读取文件中字符. 当然也可以自行优化, 批量读取. 
    逐个读取优势是节约内存. 其它都是老套路重度依赖 dict.h 接口.

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

    篇幅比较大就是 _sconf_parse 过程, 解析文件中内容. 其它只是 dict.h 接口的使用方法. 
    最终还是学以致用, 在软件行业是这样. 基础科学最好还是追求道的高度. 
    内功部分, 带大家搞了一遍 rbtree -> dict -> mq -> rsmq -> sconf 主要是围绕容器.
    一个人内部容器有多大, 就能容纳多少, 就会有多深内功. 有种错觉, 看书的人到这里就可以
    结束一段时间了. 以后将不会那么平静 ~ 妖魔战场已经向你发起了召唤令 ~

***

![兰花草](./img/兰花草.jpg)