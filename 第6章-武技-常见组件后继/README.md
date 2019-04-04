# 第6章-武技-常见组件后继

        时间过得真快, 此章武技修炼完毕, 修真之旅的岁月也就过去一半了. 武技本是金丹期选
    手在妖魔实战中的宝贵经验. 原始而奏效, 飞沙走石, 滔天气浪. 诉述故事也许很短, 但未来
    的传说由你自己谱写 ~ 此刻一同穿梭那场景, 感受一刹那间神魂震动, 山河破碎, 天地不仁 
    ~
    ~ 带好你的剑, 
    ~ 还记得那年的华山剑法?
    ~ ---:o- 上路吧,

## 6.1 传说中的线程池

        线程池这个很古老的旧话题, 讨论的很多, 深入也挺难. 也就在一些基础库中才能见到这
    类有点精妙的模型技巧. 此处也会随大流深入简述简单且控制性强的一种线程池实现. 先引入一
    个概念"惊群". 举个简单例子. 春天来了, 公园出现了很多麻雀. 而你恰巧有一个玉米渣. 扔
	出去, 立马无数麻雀过来争抢. 而最终会有一只或多只麻雀会得到. 而那些没有抢到的麻雀会很
	累 ... .. . 编程中惊群, 同样是个很古老的话题. 服务器框架开发中很有机会遇到. 有兴趣
    的可以自行搜索, 多数介绍的解决方案质量非常高. 而我们今天只讨论线程池中惊群现象. 采用
    的 POSIX 跨平台的线程库 pthread 来演示和克服. 

```C
//
// pthread_cond_signal - 通过条件变量激活正在等待的线程
// cond     : 条件变量
// return   : 0 is success, -1 is error, 见 errno
//
extern int __cdecl pthread_cond_signal (pthread_cond_t * cond);

//
// pthread_cond_wait - 线程等待激活
// cond     : 条件变量
// mutex    : 互斥锁
// return   : 0 is success, -1 is error, 见 errno 
//
extern int __cdecl pthread_cond_wait (pthread_cond_t * cond, 
                                      pthread_mutex_t * mutex);
```

    上面 pthread_cond_signal 接口就是线程池中出现惊群现象的原因所在. 他会激活一个或多
    个 pthread_cond_wait 等待状态线程. 同样我们解决惊群的方法很直接, 定向激活, 每个
    实际运行线程对象都有自己的条件变量. 这样每次只要激活需要激活的线程变量就可以了. 惊群
    现象就自然巧妙的避过去了.

***

![日出东方](./img/开始.jpg)

***

### 6.1.1 线程池设计

```C
#ifndef _THREADS_H
#define _THREADS_H

#include "thread.h"

//
// 简易的线程池的库
//
typedef struct threads * threads_t;

//
// threads_create - 创建线程池对象
// return   : 创建的线程池对象, NULL 表示失败
//
extern threads_t threads_create(void);

//
// threads_delete - 删除线程池对象, 同步操作
// pool     : 线程池对象
// return   : void
//
extern void threads_delete(threads_t pool);

//
// threads_insert - 线程池中添加待处理的任务
// pool     : 线程池对象
// frun     : node_f 运行的执行体
// arg      : frun 的参数
// return   : void
//
extern void threads_insert(threads_t pool, void * frun, void * arg);

#endif//_THREADS_H
```

    不完全类型 threads_t 就是我们定义的线程池对象, 并有创建删除添加行为. 其中使用函数
    参数 void * frun 是用于去掉警告. 设计的如果不好, 那还不如不设计, 同样功能越少出错
    的概率越小.

### 6.1.2 线程池实现

    线程池解决的问题是避免创建过多线程对象, 加重操作系统切换的负担. 从而换了一种方式, 将
	多个线程业务转成多个任务被固定的线程来回处理. 这个思路是牺牲优先级来换取性能. 先说说
	线程池容易实现的部分, 任务结构的设计.

```C
#include "threads.h"

// struct job 任务链表 结构 和 构造
struct job {
    struct job * next;      // 指向下一个任务结点
    node_f frun;            // 任务结点执行的函数体
    void * arg;
};

inline struct job * job_new(node_f frun, void * arg) {
    struct job * job = malloc(sizeof(struct job));
    job->next = NULL;
    job->frun = frun;
    job->arg = arg;
    return job;
}
```

	job_new 创建一个 job 任务结构, 很直白. job::frun(job::arg) 去执行任务. 继续设
    计线程池结构, 线程对象和线程池对象结构如下:

```C
// struct thread 线程结构体, 每个线程一个信号量, 定点触发
struct thread {
    struct thread * next;   // 下一个线程对象
    pthread_cond_t cond;    // 线程条件变量
    volatile bool wait;     // true 表示当前线程被挂起
    pthread_t id;           // 当前线程 id
};

// 定义线程池(线程集)定义
struct threads {
    int size;               // 线程池大小, 线程体最大数量
    int curr;               // 当前线程池中总的线程数
    int idle;               // 当前线程池中空闲的线程数
    struct thread * thrs;   // 线程结构体对象集
    pthread_mutex_t mutx;   // 线程互斥量
    volatile bool cancel;   // true 表示当前线程池正在 delete
    struct job * head;      // 线程任务链表的链头, 队列结构
    struct job * tail;      // 线程任务队列的表尾, 后进后出
};

// threads_add - 线程池中添加线程
static void threads_add(struct threads * pool, pthread_t id) {
    struct thread * thrd = malloc(sizeof(struct thread));
    thrd->next = pool->thrs;
    thrd->cond = PTHREAD_COND_INITIALIZER;
    thrd->wait = false;
    thrd->id = id;

    pool->thrs = thrd;
    ++pool->curr;
}

// threads_del - 根据 cond 内存地址删除 pool->thrs 中指定数据
static void threads_del(struct threads * pool, pthread_cond_t * cond) {
    struct thread * head = pool->thrs, * prev = NULL;
    while (head) {
        // 找见了, 否则开始记录前置位置
        if (cond == &head->cond) {
            if (prev)
                prev->next = head->next;
            else
                pool->thrs = head->next;
            return free(head);
        }
        prev = head;
        head = head->next;
    }
}

// threads_get - 找到线程 id 对应的条件变量地址
static struct thread * threads_get(struct threads * pool, pthread_t id) {
    struct thread * head = pool->thrs;
    while (head) {
        if (pthread_equal(id, head->id))
            break;
        head = head->next;
    }
    return head;
}

// threads_cond - 找到空闲的线程, 并返回其信号量 
static pthread_cond_t * threads_cond(struct threads * pool) {
    struct thread * head = pool->thrs;
    while (head) {
        if (head->wait)
            return &head->cond;
        head = head->next;
    }
    return NULL;
}
```

    通过 struct thread 可以看出线程运行对象中, 都有个 pthread_cont_t 条件变量. 这
    就是定向激活的关键. struct threads::cancel 用于标识当前线程池是否在销毁阶段. 来
    避免使用 pthread_cancel + pthread_cleanup_push 和 pthread_cleanup_pop 这
    类有风险的设计. 由上两个结构衍生了几个辅助行为 threads_del threads_get 
    threads_cond 等. 对于 struct threads 结构中 struct job * head, * tail; 是
    个待处理的任务队列. struct thread * thrs; 是线程对象的链表. 线程池对象中共用 
    struct threads 中 mutex 一个互斥量, 方便写代码. 希望还记得前面章节数据结构部分
    扯的, 链表是 C 结构中基础的内丹, 所有代码都是或多或少围绕他这个结构. 要在勤磨练中熟
    悉提高, 对于刚学习的人. 上面代码其实和业务代码没啥区别, 创建删除添加查找等. 前戏营
    造的估计够了, 现在开搞其他接口实现.

```C
// THREADS_INT - 开启的线程数是 2 * CPU
#define THREADS_INT        (8)

//
// threads_create - 创建线程池对象
// return   : 创建的线程池对象, NULL 表示失败
//
inline threads_t 
threads_create(void) {
    struct threads * pool = calloc(1, sizeof(struct threads));
    pool->size = THREADS_INT;
    pool->mutx = PTHREAD_MUTEX_INITIALIZER;
    return pool;
}
```

    创建接口的实现代码中, calloc 相比 malloc 多调用了 bzero 的相关置零清空操作. 配套
	还有一个删除释放资源函数. 设计意图允许创建多个线程池对象, 因为有了创建和删除成对操作. 
	请看下面优雅的删除销毁操作:

```C
//
// threads_delete - 异步销毁线程池对象
// pool     : 线程池对象
// return   : void
//
void 
threads_delete(threads_t pool) {
    if (!pool || pool->cancel)
        return;

    // 已经在销毁过程中, 直接返回
    pthread_mutex_lock(&pool->mutx);
    if (pool->cancel) {
        pthread_mutex_unlock(&pool->mutx);
        return;
    }

    // 标识当前线程池正在销毁过程中, 并随后释放任务列表
    pool->cancel = true;
    struct job * head = pool->head;
    while (head) {
        struct job * next = head->next;
        free(head);
        head = next;
    }
    pool->head = pool->tail = NULL;
    pthread_mutex_unlock(&pool->mutx);

    // 再来销毁每个线程
    struct thread * thrs = pool->thrs;
    while (thrs) {
        struct thread * next = thrs->next;
        // 激活每个线程让其主动退出
        pthread_cond_signal(&thrs->cond);
        pthread_join(thrs->id, NULL);
        thrs = next;
    }

    // 销毁自己
    free(pool);
}
```

    用到很多 pthread api. 不熟悉的多研究, 多做笔记. 不懂的时候说明又是提升功力的机会哈
    ! 对于删除函数, 先监测销毁标识, 后竞争唯一互斥量, 竞争到后就开始释放过程. 先清除任
    务队列并置空, 随后解锁. 再去准备销毁每个线程, 激活他并等待他退出. 最后销毁自己. 优
    雅结束了线程池的生成周期. 如果不知道是不是真爱, 那就追求优雅 ~ 如果是真爱, 那么什么
    都不想要 ~ 随后步入核心部分只有两个函数, 一个是线程轮询处理任务的函数, 另一个是构建
    线程池函数. 

```C
// thread_consumer - 消费线程
static void thread_consumer(struct threads * pool) {
    pthread_t id = pthread_self();
    pthread_mutex_t * mutx = &pool->mutx;

    pthread_mutex_lock(mutx);

    struct thread * thrd = threads_get(pool, id);
    assert(thrd);
    pthread_cond_t * cond = &thrd->cond;

    // 使劲循环的主体, 开始消费 or 沉睡
    while (!pool->cancel) {
        if (pool->head) {
            struct job * job = pool->head;
            pool->head = job->next;
            // 队列尾置空监测
            if (pool->tail == job)
                pool->tail = NULL;

            // 解锁, 允许其他消费者线程加锁或生产者添加新任务
            pthread_mutex_unlock(mutx);

            // 回调函数, 后面再去删除任务
            job->frun(job->arg);
            free(job);

            // 新的一轮开始, 需要重新加锁
            pthread_mutex_lock(mutx);
            continue;
        }

        // job 已经为 empty , 开启线程等待
        thrd->wait = true;
        ++pool->idle;

        // 开启等待, 直到线程被激活
        int status = pthread_cond_wait(cond, mutx);
        if (status < 0) {
            pthread_detach(id);
            CERR("pthread_cond_wait error status = %d.", status);
            break;
        }
        thrd->wait = false;
        --pool->idle;
    }

    // 到这里程序出现异常, 线程退出中, 先减少当前线程
    --pool->curr;
    // 去掉这个线程链表 pool->thrs 中对应的数据
    threads_del(pool, cond);

    // 所有线程共用同一把任务锁
    pthread_mutex_unlock(mutx);
}

//
// threads_insert - 线程池中添加待处理的任务
// pool     : 线程池对象
// frun     : node_f 运行的执行体
// arg      : frun 的参数
// return   : void
//
void 
threads_insert(threads_t pool, void * frun, void * arg) {
    if (!frun || !pool || pool->cancel)
        return;

    struct job * job = job_new(frun, arg);
    pthread_mutex_t * mutx = &pool->mutx;

    pthread_mutex_lock(mutx);

    // 线程池中任务队列的插入任务
    if (!pool->head)
        pool->head = job;
    else
        pool->tail->next = job;
    pool->tail = job;

    // 构建线程, 构建完毕直接获取
    if (pool->idle > 0) {
        pthread_cond_t * cond = threads_cond(pool);
        // 先释放锁后发送信号激活线程, 速度快, 缺点丧失线程执行优先级
        pthread_mutex_unlock(mutx);
        // 发送给空闲的线程, 这个信号量一定存在
        pthread_cond_signal(cond);
        return;
    }

    if (pool->curr < pool->size) { // 没有, 那就新建线程去处理
        pthread_t id;
        if (pthread_create(&id, NULL, (start_f)thread_consumer, pool))
            CERR("pthread_create error curr = %d.", pool->curr);
        else // 添加开启线程的信息
            threads_add(pool, id);
    }

    pthread_mutex_unlock(mutx);
}
```

	对于消费者线程 thread_consumer 函数运行起来后, 只有内部出现异常 status == -1 时
    候会进入自销毁 pthread_detach 操作. 外部 pool->cancel == true 的时候会让其退
    出, 走 pthread_join 关联接收. 再次想起以前扯的一句闲篇, 关于提升技术好办法
    	1' 多望书
    	2' 多写代码, 搜搜, 问问
    	3' 多看好代码, 临摹源码
    	4' 多创造, 改进, 实战
    等该明白的都明白了, 多数会变得那就这样吧. 期待这样的场景重复, 到这里线程池是结束了,
    不妨为其写段测试代码 ~

```C
#include "times.h"
#include "threads.h"

// old 全局计时器, 存在锁问题
static int old;

// ppt 简单的线程打印函数
static inline void ppt(const char * str) {
    printf("%d => %s\n", ++old, str);
}

// doc 另一个线程测试函数
static inline void doc(void * arg) {
    printf("p = %d, 技术不决定项目的成败! 我老大哭了\n", ++old);
}

void test_threads(void) {
    // 创建线程池
    threads_t pool = threads_create();

    // 添加任务到线程池中
    for (int i = 0; i < BUFSIZ; ++i) {
        threads_insert(pool, ppt, "你为你负责的项目拼命过吗. 流过泪吗");
        threads_insert(pool, doc, NULL);
    }

    //等待 5s 再结束吧
    msleep(5000);

    // 清除当前线程池资源, 实战上线程池是常驻内存, 不要清除.
    threads_delete(pool);
}
```

	线程模型有点廉颇老矣, 尚能饭否的味道. 在现代服务业务处理上面, 切换代价, 资源数量(线
    程栈大小, 线程数量)消耗大. 所以 goroutine 这类模型很吃香. 但如果你足够自信通过设置
    CPU 硬亲和性, 有时候会获得更高性能. 总而言之咱们费了老大劲写了个线程池, 99% 业务基
    本都不会用到(实战中进程线程协程模型常混搭). 密集型业务目前修真界都流行少量线程加轮询
    消息队列的方式处理, 下一个主角该登场了 ~

## 6.2 消息轮序器

    	服务开发中, 消息轮询器基本上就是整个服务器调度处理的核心! 所有待处理的业务统一封
    装 push 进去, 单独线程异步 loop 去处理, 周而复始. 等同于守护门派安定的无上大阵. 
    下面就带大家写个超炫迈的封魔大阵, 收获门派一世繁华 ~ 接口设计部分 loop.h

```C
#ifndef _LOOP_H
#define _LOOP_H

#include "q.h"
#include "atom.h"
#include "thread.h"

typedef struct loop * loop_t;

//
// loop_delete - 轮询对象销毁
// p        : 轮询对象
// return   : void
//
extern void loop_delete(loop_t p);

//
// loop_push  - 消息压入轮询器
// p        : 轮询对象
// m        : 消息
// return   : void
//
extern void loop_push(loop_t p, void * m);

//
// loop_create - 创建轮询对象
// frun     : node_f 消息处理行为
// fdie     : node_f 消息销毁行为
// return   : 轮询对象
//
extern loop_t loop_create(void * frun, void * fdie);

#endif//_LOOP_H
```

    函数 loop_create 创建一个消息轮序器并启动, 需要注册两个函数 frun 和 fdie, 前者
    用于处理每个 push 进来的消息, fdie 用于用户 push 进来消息的善后工作销毁清除操作. 
    这个库实现的非常精简. 直贴代码, 比较有价值. 多感受其中的妙用, 做戏, 小就不能满足你
    了吗. 

```C
#include "loop.h"

// loop 轮询器结构
struct loop {
    q_t rq;             // 读消息
    q_t wq;             // 写消息
    atom_t lock;        // 消息切换锁
    node_f frun;        // 消息处理行为
    node_f fdie;        // 消息销毁行为
    sem_t block;        // 线程信号量
    pthread_t id;       // 运行的线程id
    volatile bool loop; // true 线程正在运行
    volatile bool wait; // true 线程空闲等待
};

// run - 消息处理行为
inline void run(loop_t p, void * m) {
    // 开始处理消息
    p->frun(m);
    p->fdie(m);
}

//
// loop_delete - 轮询对象销毁
// p        : 轮询对象
// return   : void
//
void 
loop_delete(loop_t p) {
    //
    // delete 执行必须在 push 之后, C 代码是在刀剑上跳舞 ~ 
    //
    if (p) {
        p->loop = false;
        sem_post(&p->block);
        // 等待线程结束, 然后退出
        pthread_end(p->id);
        sem_destroy(&p->block);
        q_delete(p->rq, p->fdie);
        q_delete(p->wq, p->fdie);
        free(p);
    }
}

//
// loop_push - 消息压入轮询器
// p        : 轮询对象
// m        : 消息
// return   : void
//
void 
loop_push(loop_t p, void * m) {
    assert(p && m);
    atom_lock(p->lock);
    q_push(p->rq, m);
    atom_unlock(p->lock);
    if (p->wait) {
        p->wait = false;
        sem_post(&p->block);
    }
}

// loop_run - 轮询器执行的循环体
static void loop_run(loop_t p) {
    while (p->loop) {
        void * m = q_pop(p->rq);
        if (m) {
            run(p, m);
            continue;
        }

        // read q <- write q
        atom_lock(p->lock);
        q_swap(p->rq, p->wq);
        atom_unlock(p->lock);

        m = q_pop(p->rq);
        if (m) run(p, m);
        else {
            // 仍然没有数据, 开始睡眠
            p->wait = true;
            sem_wait(&p->block);
        }
    }
}

//
// loop_create - 创建轮询对象
// frun     : node_f 消息处理行为
// fdie     : node_f 消息销毁行为
// return   : 轮询对象
//
loop_t 
loop_create(void * frun, void * fdie) {
    loop_t p = malloc(sizeof(struct loop));
    p->lock = 0;
    q_init(p->rq);
    q_init(p->wq);
    p->frun = frun;
    p->fdie = fdie;
    p->wait = p->loop = true;
    // 初始化 POSIX 信号量, 进程内线程共享, 初始值 0
    sem_init(&p->block, 0, 0);
    if (pthread_run(&p->id, loop_run, p)) {
        sem_destroy(&p->block);
        free(p->rq->queue);
        free(p->wq->queue);
        free(p); p = NULL;
    }
    return p;    
}
```

	对于 struct loop::fdie 也支持 NULL 行为操作. 如果 C 编译器层面语法糖支持的好些,
    那就爽了. 整体思路是乒乓交换, 亮点在于 sem_wait. 分析会, 假如是多线程环境 
    loop_push 多次并发操作, 并触发相应的 sem_post 会执行多次 P 操作. 但 loop_run 
    是单线程轮询处理的, 只会触发对应次的 sem_wait V 操作. 所以 push 的 sem_post 不
    加锁不影响业务正确性. 而且 sem_wait 是通用层面阻塞性能最好的选择. 这些都是高效的保
    证. 武技修炼中, loop 库是继 clog 库之后, 基于最小意外的实现 ~ 感悟至今, 进出妖魔
    战场更加频繁, 修炼也越发坚深, 然而心境中域外天魔也逐渐在另一个次元逼近而来. 他会拷
    问你的内心, 你为何修炼编程? 随之进入弥天幻境, 太多太多人在幻境的路中间 ~ 不曾解脱 
    ~ 不愿走过那条大道, 去, 元婴终身无望 ~ 

## 6.3 阅读理解

        对于定时器常见的实现有三类套路. 一种是有序链表用于解决, 大量重复轮询的定时结点设
    计的. 另一种是采用时间堆构建的定时器, 例如小顶堆, 时间差最小的在堆顶, 最先执行. 还
    有一种时间片结构, 时间按照一定颗度转呀转, 转到那就去执行那条刻度上的链表. 总的而言定
    时器的套路取舍得看应用的场景. 我们这里带来的阅读理解是基于有序链表. 可以说起缘 list
    , 终于 list. 希望这篇阅读理解能加深你对 list 的理解.

```C
#ifndef _TIMER_H
#define _TIMER_H

#include "atom.h"
#include "list.h"
#include "times.h"
#include "thread.h"

//
// timer_del - 删除定时器事件
// id       : 定时器 id
// return   : void
//
extern void timer_del(int id);

//
// timer_add - 添加定时器事件
// ms       : 执行间隔毫秒, <= 0 表示立即执行
// ftimer   : node_f 定时器行为
// arg      : 定时器参数
// return   : 定时器 id, < 0 标识 error
//
extern int timer_add(int ms, void * ftimer, void * arg);

#endif//_TIMER_H
```

	设计清晰易懂, 业务只有添加和删除. 实现部分, 重点感受数据结构内功的 list 的用法. 

```C
#include "timer.h"

// timer_node 定时器结点
struct timer_node {
$LIST
    int id;            // 定时器 id
    void * arg;        // 执行函数参数
    node_f ftimer;     // 执行的函数事件
    struct timespec t; // 运行的具体时间
};

// timer_node id compare
inline static int timer_node_id_cmp(int id, 
                                    const struct timer_node * r) {
    return id - r->id;
}

// timer_node time compare 比较
inline static int timer_node_time_cmp(const struct timer_node * l, 
                                      const struct timer_node * r) {
    if (l->t.tv_sec != r->t.tv_sec)
        return (int)(l->t.tv_sec - r->t.tv_sec);
    return (int)(l->t.tv_nsec - r->t.tv_nsec);
}

// timer_list 链表对象管理器
struct timer_list {
    int id;                   // 当前 timer node id
    atom_t lock;              // 自旋锁
    volatile bool status;     // true is thread loop, false is stop
    struct timer_node * list; // timer list list
};

// timer_list_sus - 得到等待的微秒事件, <= 0 表示可以执行
inline int timer_list_sus(struct timer_list * list) {
    struct timespec * v = &list->list->t, t[1];
    timespec_get(t, TIME_UTC);
    return (int)((v->tv_sec - t->tv_sec) * 1000000 + 
        (v->tv_nsec - t->tv_nsec) / 1000);
}

// timer_list_run - 线程安全, 需要再 loop 之后调用
inline void timer_list_run(struct timer_list * list) {
    struct timer_node * node;
    atom_lock(list->lock);
    node = list->list;
    list->list = list_next(node);
    atom_unlock(list->lock);

    node->ftimer(node->arg);
    free(node);
}

// 定时器管理单例对象
static struct timer_list timer;

//
// timer_del - 删除定时器事件
// id       : 定时器 id
// return   : void
//
inline void 
timer_del(int id) {
    if (timer.list) {
        atom_lock(timer.lock);
        free(list_pop(&timer.list, timer_node_id_cmp, (void *)(intptr_t)id));
        atom_unlock(timer.lock);
    }
}

// timer_new - timer_node 定时器结点构建
static struct timer_node * timer_new(int s, node_f ftimer, void * arg) {
    struct timer_node * node = malloc(sizeof(struct timer_node));
    node->id = atom_inc(timer.id);
    if (node->id < 0)
        node->id = atom_and(timer.id, INT_MAX);
    node->arg = arg;
    node->ftimer = ftimer;
    timespec_get(&node->t, TIME_UTC);
    node->t.tv_sec += s / 1000;
    // nano second
    node->t.tv_nsec += (s % 1000) * 1000000;
    return node;
}

// 运行的主 loop, 基于 timer 管理器 
static void timer_run(struct timer_list * list) {
    // 正常轮循, 检查时间
    while (list->list) {
        int sus = timer_list_sus(list);
        if (sus > 0) {
            usleep(sus);
            continue;
        }

        timer_list_run(list);
    }

    // 已经运行结束
    list->status = false;
}

//
// timer_add - 添加定时器事件
// ms       : 执行间隔毫秒, <= 0 表示立即执行
// ftimer   : node_f 定时器行为
// arg      : 定时器参数
// return   : 定时器 id, < 0 标识 error
//
int 
timer_add(int ms, void * ftimer, void * arg) {
    if (ms <= 0) {
        ((node_f)ftimer)(arg);
        return 0;
    }

    struct timer_node * node = timer_new(ms, ftimer, arg);
    int id = node->id;
    atom_lock(timer.lock);

    list_add(&timer.list, timer_node_time_cmp, node);

    // 判断是否需要开启新的线程
    if (!timer.status) {
        if (!pthread_async(timer_run, &timer))
            timer.status = true;
        else {
            free(node);
            id = -1;
        }
    }

    atom_unlock(timer.lock);
    return id;
}
```

    以上 timer.c 模块实现思路, 核心是利用 list 构建了一个升序链表, 通过额外异步分离线
    程 loop 获取链表结点去执行. 定时器一个通病, 不要放入阻塞函数, 容易失真. timer 使
    用方面也很简单, 例如一个技能, 吟唱 1s, 持续伤害 2s. 构造如下:

```C
struct skills {
	int id;
	bool exist; // 实战走状态机, true 表示施法状态中 
};

// 007 号技能 火球术, 没有释放
struct skills fireball = { 007, false };

static void _end(struct skills * kill) {
	...
	if (kill.id == fireball.id) {
		puts("火球术持续输出结束...");
		kill.exist = false;
	}
	...
}

static void continued(struct skills * kill) {
	...
	if (kill.id == fireball.id) {
		puts("火球术吟唱成功, 开始持续输出...");
		kill.exist = true;
		timer_add(2000, end, kill);
	}
	...
}

static void start(struct skills * kill) {
	...
	if (kill.id == fireball.id) {
		puts("火球术开始吟唱...");
		kill.exist = false;
		timer_add(1000, continued, kill);
	}
	...
}
```

	调用 start 就可以了, 火球术吟唱, 持续输出. 中间打断什么鬼, 那就自己扩展. 后期根据
    标识统一绘制显示. 以上是简单到吐的思路说不定也很有效. 有点像优化过的 select 特定的
    时候出其不意 ~

***

	好快好快, 修炼之路已经走过小一半. 从华山剑法练起, 到现在的一步两步三步. 此刻自己可
    以出门踏草原, 风和日丽. 遇到呢些业务中的小妖精, 分分钟可以干掉了吧. loop 日月轮在实
    战中会用的最多. 对于定时器, 多数内嵌到主线程轮询模块(update) 中. 恭喜同行, 同行历
    练求索, 在血与歌中感受生的洗礼. 尝悟心中的道. 此景想起宋代大文豪的一首喜悦 ❀

***

	元日(宋-王安石)
	爆竹声中一岁除，春风送暖入屠苏。
	千门万户曈曈日，总把新桃换旧符。

***

![道](./img/女苑姜明.jpg)
