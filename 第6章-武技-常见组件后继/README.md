# 第6章-武技-常见组件后继

        时间过得真快, 如果武技修炼完毕, 修真之旅岁月也就过去一半了. 武技本是金丹期选手在
    妖魔大战中招招实战的经验. 原始而奏效, 飞沙走石, 热浪滚滚. 说的也许很短, 但未来的传说
    由你为自己谱写 ~ 此刻只带你穿梭那种场景, 感受一刹那间神魂震动, 山河破碎, 天地不仁 ~

    带好你的剑, 
    那年学的华山剑法 ~
    ---:o- 此刻上路吧

## 6.1 传说中的线程池

        线程池是很古老的旧话题. 讨论的很多, 深入的不多. 也就在那些基础库中才能见到这种
	有点精妙的技巧. 这里也会随大流深入简述一种简单且控制性强的一种线程池实现. 先引入一个
    概念, 惊群. 简单举个例子. 春天来了, 公园出现了很多麻雀. 而你恰巧有一个玉米粒. 扔出去
    , 立马无数麻雀过来争抢. 而最终只有一只麻雀会得到. 而那些没有抢到的麻雀很累...... 编
    程中惊群, 也是个很古老的话题. 服务器框架开发中很有机会遇到. 有兴趣的可以自行搜索, 多
    数介绍的解决方案质量非常高. 而我们今天只讨论线程池中惊群现象. 采用的 POSIX 跨平台的
    线程库 pthread 来演示和克服. 

```C
//
// pthread_cond_signal - 通过条件变量激活正在等待线程
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

    上面 pthread_cond_signal 接口就是线程池中出现惊群来源. 它会激活一个或多个等待状态
    pthread_cond_wait 线程. 同样这里解决惊群的方式很普通也很实在, 定向激活, 每个实际运
    行线程对象都有自己的条件变量. 那么每次只要激活需要激活的线程变量就可以了. 惊群的现象
    自然就巧妙避免了.

***

![日出东方](./img/开始.jpg)

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
// threads_delete - 异步销毁线程池对象
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

#endif // !_THREADS_H
```

    不完全类型 threads_t 就是我们定义的线程池对象, 创建删除添加等行为. 其中使用了个函数
    参数  void * frun 的技巧. 用于去掉警告. 设计的如果不好,  那将什么都不是 ......

### 6.1.2 线程池实现

    线程池解决的问题是避免创建过多线程对象, 加重操作系统的负担. 从而换了一种方式, 将
	多个线程业务转成多个任务被固定的线程来回处理. 这个思路是不是很巧妙. 先说说线程池
	容易的实现部分, 来看任务结构的设计:

```C
// 任务链表 结构 和 构造
struct job {
	struct job * next;  // 指向下一个任务结点
	node_f run;         // 任务结点执行的函数体
	void * arg;	
};

static inline struct job * _job_new(node_f run, void * arg) {
	struct job * job = malloc(sizeof(struct job));
	if (NULL == job)
		CERR_EXIT("malloc sizeof(struct job) is error!");
	job->next = NULL;
	job->run = run;
	job->arg = arg;
	return job;
}
```

    内存处理方式很直接, 申请失败直接退出. 更加粗暴的方式是调用 abort(). 强制退出.
    但实际开发中还真没遇到过内存不足(抛开测试). 假如一个系统真的内存不足了, 可能一
	切都是未知. 继续分析线程池结构, 线程处理对象和线程池对象结构如下:

```C
// 线程结构体, 每个线程一个信号量, 定点触发
struct thread {
	struct thread * next;   // 下一个线程对象
	bool wait;              // true 表示当前线程被挂起
	pthread_t tid;          // 当前线程id
	pthread_cond_t cond;    // 线程条件变量
};

// 定义线程池(线程集)定义
struct threads {
	size_t size;            // 线程池大小, 最大线程结构体数量
	size_t curr;            // 当前线程池中总的线程数
	size_t idle;            // 当前线程池中空闲的线程数
	volatile bool cancel;   // true表示当前线程池正在 delete
	pthread_mutex_t mutx;   // 线程互斥量
	struct thread * thrs;   // 线程结构体对象集
	struct job * head;      // 线程任务链表的链头, 队列结构
	struct job * tail;      // 线程任务队列的表尾, 后插入后执行
};
```

    上面可以找出每个 struct thread 线程运行对象中, 都有个 pthread_cont_t 条件变量
	. 这就是定向激活的关键. struct threads::cancel 用于标识当前线程池是否在销毁阶段
	. 来避免使用 pthread_cancel + pthread_cleanup_push 和 pthread_cleanup_pop ,
    这类有风险的设计. 上面两个结构衍生了几个辅助行为操作如下 :

```C
// 根据 cond 内存地址熟悉, 删除 pool->thrs 中指定数据
static void _threads_del(struct threads * pool, pthread_cond_t * cond) {
	struct thread * head = pool->thrs, * front = NULL;
	
	while (head) {
		// 找见了, 否则开始记录前置位置
		if (cond == &head->cond)
			break;
		front = head;
		head = head->next;
	}

	if (head) {
		if (front)
			front->next = head->next;
		else
			pool->thrs = head->next;

		// head 结点就是我们要找的
		pthread_cond_destroy(&head->cond);
		free(head);
	}
}

// 找到线程 tid 对应的条件变量地址
static struct thread * _threads_get(struct threads * pool, pthread_t tid) {
	struct thread * head = pool->thrs;
	while (head) {
		if (pthread_equal(tid, head->tid))
			return head;
		head = head->next;
	}
	return NULL;
}

// 找到空闲的线程, 并返回起信号量 
static pthread_cond_t * _threads_getcont(struct threads * pool) {
	struct thread * head = pool->thrs;
	while (head) {
		if (head->wait)
			return &head->cond;
		head = head->next;
	}
	return NULL;
}
```

    对于 struct threads 结构中 struct job * head, * tail; 是个待处理的任务队列.
    struct thread * thrs; 链接所有线程对象的链表. 线程池对象中共用 struct threads 
	中 mutex 一个互斥量. 希望还记得前面数据结构部分扯的, 链表是 C结构中基础的内丹, 
	所有代码都是或多或少围绕它这个结构. 要在勤磨练中熟悉提高, 对于刚学习的人. 上面代
	码其实和业务代码没啥区别, 创建销毁添加查找等. 前戏营造的估计够了, 现在开搞其它接
	口简单实现代码 :

```C
// 开启的线程数
#define _UINT_THREADS		(4u)

threads_t 
threads_create(void) {
	struct threads * pool = calloc(1, sizeof(struct threads));
	if (NULL == pool) {
		RETURN(NULL, "calloc sizeof(struct threads) is error!");
	}

	pool->size = _UINT_THREADS;
	pthread_mutex_init(&pool->mutx, NULL);

	return pool;
}
```

    上面创建接口的实现代码中, calloc 相比 malloc 多调用了 bzero 的相关置零清空操作. 
    还有一个释放资源函数. 设计意图允许创建多个线程池(但是没有必要). 请看下面优雅的销
	毁操作:

```C
void 
threads_delete(threads_t pool) {
	struct job * head;
	struct thread * thrs;

	if (!pool || pool->cancel) 
		return;
	// 标识当前线程池正在销毁过程中
	pool->cancel = true;

	pthread_mutex_lock(&pool->mutx);

	// 先来释放任务列表
	head = pool->head;
	while (head) {
		struct job * next = head->next;
		free(head);
		head = next;
	}
	pool->head = pool->tail = NULL;

	pthread_mutex_unlock(&pool->mutx);

	// 再来销毁每个线程
	thrs = pool->thrs;
	while (thrs) {
		struct thread * next = thrs->next;
		// 激活每个线程让其主动退出
		pthread_cond_signal(&thrs->cond);
		pthread_join(thrs->tid, NULL);
		thrs = next;
	}

	// 销毁自己
	pthread_mutex_destroy(&pool->mutx);
	free(pool);
}
```

    用到很多 pthread api. 不熟悉的多搜索, 多做笔记. 不懂多了, 说明提升功力的机会来了.
    对于上面释放函数先监测销毁标识, 后竞争唯一互斥量, 竞争到后就开始释放过程. 先清除任
	务队列并置空, 随后解锁. 再去准备销毁每个线程, 激活它并等待它退出. 最终清除锁销毁自
	己. 优雅结束了线程池的生成周期. 

    如果不知道是不是真爱, 那就追求优雅 ~
    如果是真爱, 那么什么都不想要 ~

#### 6.1.3 线程池核心部分

    核心部分就两个函数, 一个是线程轮询处理任务的函数. 另一个是构建线程池函数. 线程轮询
	函数如下:

```C
// 线程运行的时候执行函数, 消费者线程
static void _consumer(struct threads * pool) {
	int status;
	struct thread * thrd;
	pthread_cond_t * cond;
	pthread_t tid = pthread_self();
	pthread_mutex_t * mutx = &pool->mutx;

	pthread_mutex_lock(mutx);

	thrd = _threads_get(pool, tid);
	assert(thrd);
	cond = &thrd->cond;

	// 使劲循环的主体, 开始消费 or 沉睡
	while (!pool->cancel) {
		if (pool->head) {
			struct job * job = pool->head;
			pool->head = job->next;
			// 队列尾置空监测
			if (pool->tail == job)
				pool->tail = NULL;

			// 解锁, 允许其它消费者线程加锁或生产者添加新任务
			pthread_mutex_unlock(mutx);

			// 回调函数, 后面再去删除任务
			job->run(job->arg);
			free(job);

			// 新的一轮开始, 需要重新加锁
			pthread_mutex_lock(mutx);
			continue;
		}

		// job 已经为empty , 开启线程等待
		thrd->wait = true;
		++pool->idle;

		// 开启等待, 直到线程被激活
		status = pthread_cond_wait(cond, mutx);
		if (status < 0) {
			pthread_detach(tid);
			CERR("pthread_cond_wait error status = %zu.", status);
			break;
		}
		thrd->wait = false;
		--pool->idle;
	}

	// 到这里程序出现异常, 线程退出中, 先减少当前线程
	--pool->curr;
	// 去掉这个线程链表pool->thrs中对应的数据
	_threads_del(pool, cond);

	// 一个线程一把锁, 退出中
	pthread_mutex_unlock(mutx);
}
```
	对于消费者线程 _consumer 函数运行起来, 内部出现异常进入自销毁操作 pthread_detach
	. 外部让其退出, 走 pthread_join 关联接收. 

    突然想起以前扯的一句话, 关于提升技术好办法
    1. 多看书
    2. 多写代码, 多搜搜, 多问问
    3. 多看别人的好代码, 多临摹源码
    4. 多创造, 多改进, 多实战
    等该明白的都明白了, 一切都是那样容易, 那样的美的时候. 就可以回家种田了. 哈哈 ~

    到这里线程池是结束了, 不妨为其写个测试代码吧 ~

```C
#include "scthreads.h"

// 测试开启线程量集
#define _INT_THS (8192)

//全局计时器,存在锁问题
static int _old;

//简单的线程打印函数
static inline void _ppt(const char * str) {
	printf("%d => %s\n", ++_old, str);
}

//另一个线程测试函数
static inline void _doc(void * arg) {
	printf("p = %d, 技术不决定项目的成败!我老大哭了\n", ++_old);
}

int main(void) {
	//创建线程池
	threads_t pool = threads_create();

	//添加任务到线程池中
	for (int i = 0; i<_INT_THS; ++i) {
		threads_insert(pool, _ppt, "你为你负责的项目拼命过吗.流过泪吗");
		threads_insert(pool, _doc, NULL);
	}

	//等待5s 再结束吧
	sh_msleep(5000);
	//清除当前线程池资源, 实战上线程池是常驻内存,不要清除.
	threads_delete(pool);
    return EXIT_SUCCESS;
}
```

    咱们费了老大劲写了个线程池, 9.9 业务基本都不会用到. 密集型业务目前修真界都流
	行少量线程加轮询消息队列的方式处理, 下一个出场的主角登场了 ~

### 4.2 消息轮序器

    系统开发中, 消息轮询器基本上就是整个服务器调度处理的核心! 所有待处理的业务统
	一封装 push 进去, 单独线程异步 loop 去处理, 周而复始. 等同于守护门派安定的
    无上大阵. 下面就带大家写个超酷炫的封魔大镇, 收获门派一世繁华 ~

    接口设计部分 scrunloop.h

```C
#ifndef _H_SIMPLEC_SCRUNLOOP
#define _H_SIMPLEC_SCRUNLOOP

#include "struct.h"

typedef struct srl * srl_t;

//
// srl_create - 创建轮询服务对象
// run		: 轮序处理每条消息体, 弹出消息体的时候执行
// die		: srl_push msg 的销毁函数
// return	: void 
//
srl_t srl_create_(node_f run, node_f die);
#define srl_create(run, die) srl_create_((node_f)(run), (node_f)(die))

//
// srl_delete - 销毁轮询对象,回收资源
// s		: 轮询对象
// return	: void 
//
extern void srl_delete(srl_t srl);

//
// srl_push - 将消息压入到轮询器中
// s		: 轮询对象
// msg		: 待加入的消息地址
// return	: void
// 
extern void srl_push(srl_t s, void * msg);

#endif // !_H_SIMPLEC_SCRUNLOOP
```

    通过宏函数 srl_create 启动一个消息轮序器, 需要注册两个函数 run 和 die, 前者用于处
	理每个 push 进来的消息, die 用户 push 进来消息的善后工作(销毁清除). 这个类实现的非
	常精简. 直贴代码, 比较有价值. 多感受其中的妙用, 真是, 小也能满足你 

scrunloop.c

```C
#include "mq.h"
#include "schead.h"
#include "scrunloop.h"
#include <semaphore.h>

// 具体轮询器
struct srl {
	pthread_t id;           // 具体奔跑的线程
	sem_t blocks;           // 线程阻塞

	mq_t mq;                // 消息队列
	node_f run;             // 每个消息都会调用 run(pop())
	node_f die;             // 每个消息体的善后工作
	volatile bool loop;     // true表示还在继续 
	volatile bool wait;     // true表示当前轮序器正在等待
};

static void * _srl_loop(struct srl * s) {
	while (s->loop) {
		void * pop = mq_pop(s->mq);
		if (NULL == pop) {
			s->wait = true;
			sem_wait(&s->blocks);
		} else {
			// 开始处理消息
			s->run(pop);
			s->die(pop);
		}
	}
	return s;
}

srl_t
srl_create_(node_f run, node_f die) {
	struct srl * s = malloc(sizeof(struct srl));
	assert(s && run);
	s->mq = mq_create();
	s->loop = true;
	s->run = run;
	s->die = die;
	s->wait = true;
	// 初始化 POSIX 信号量, 进程内线程共享, 初始值 0
	sem_init(&s->blocks, 0, 0);
	// 创建线程,并启动
	if (pthread_create(&s->id, NULL, (start_f)_srl_loop, s)) {
		mq_delete(s->mq, die);
		free(s);
		RETURN(NULL, "pthread_create create error !!!");
	}
	return s;
}

void
srl_delete(srl_t s) {
	if (s) {
		s->loop = false;
		sem_post(&s->blocks);
		// 等待线程结束, 然后退出
		pthread_join(s->id, NULL);
		sem_destroy(&s->blocks);
		mq_delete(s->mq, s->die);
		free(s);
	}
}

inline void 
srl_push(srl_t s, void * msg) {
	assert(s && msg);
	mq_push(s->mq, msg);
	if (s->wait) {
		s->wait = false;
		sem_post(&s->blocks);
	}
}
```
	对于 struct srl::die 可以优化, 支持为 NULL 操作, 或者添加函数行为. 但是总的而言,
	语法糖层面支持的越好, 用起来是爽了, 多次爽之后那种退烧的感觉会更有意思. C 很难把所
	有业务统一起来支持, 那样的代码臃肿和不敢维护. 体会并学会那种 C 设计的思路, 大不了
	为另一种业务写个更加贴切的支持, 如果追求性能的话.

    再分析会, 假如是多线程环境 srl_push 触发了并发操作, 相应的 sem_post 会执行多次
	P 操作. 但 _srl_loop 是单线程轮询处理的, 只会触发对映次的 sem_wait V 操作. 所以
	push 不加锁不影响业务正确性. 而且 sem_wait 是通用层面阻塞性能最好的选择. 这些都是
	高效的保证. 武技修炼中, srl 库是继 clog 库之后, 最小意外的实现 ~

    修炼到现在, 逐渐进出妖魔战场, 另一个异常恐怖 - 域外天魔正在逼近. 它会拷问你的心,
    你为什么修炼编程 ? 进入弥天幻境, 多数人在幻境的路中间 ~ 不曾解脱 ~  

    走不过去那条大道, 元婴终身无望 ~ 

### 4.3 http util by libcurl

    开发中对于 http 请求处理, 也是一个重要环节, 链接运营管理业务, 不可缺失. 同样都有固
	定套路. 这里用的是 libcurl c http 处理库. 

```Bash
// linux 安装总结
sudo apt-get install libcurl4-openssl-dev

// winds 使用方法
/*
 方法1. 查教程, 去 libcurl 官网下载最新版本编译
 方法2. 如果你怕麻烦 github 找我的项目 simplc, 参照 extern 目录下面
 方法3. 随便, 只要你搭好 hello world 就行 
 */
```

    按照上面思路, 搭建好环境开搞. 服务器中对于 http 请求, 主要围绕在 get 和 post. 
    接收别人的 http 请求和接收 tcp请求一样, 通过协议得到完整报文, 抛给业务层. 所
	以封装的 http 工具库, 接口功能很单一. 就是对外发送 get post 请求:

httputil.h

```C
#ifndef _H_SIMPLEC_HTTPUTIL
#define _H_SIMPLEC_HTTPUTIL

#include "tstr.h"
#include <curl/curl.h>

//
// http_start - http 库启动
// return	: void
//
extern void http_start(void);

//
// http_get - http get 请求 or https get请求 
// url		: 请求的url
// str		: 返回最终申请的结果, NULL表示只请求不接收数据
// return	: true表示请求成功
//
extern bool http_get(const char * url, tstr_t str);
extern bool http_sget(const char * url, tstr_t str);

//
// http_post - http post 请求 or https post 请求
// url		: 请求的路径
// params	: 参数集 key=value&.... 串
// str		: 返回结果, 需要自己释放, NULL表示只请求不接受数据返回
// return	: false表示请求失败, 会打印错误信息
//
extern bool http_post(const char * url, const char * params, tstr_t str);
extern bool http_spost(const char * url, const char * params, tstr_t str);

#endif // !_H_SIMPLEC_HTTPUTIL
```

    对于 http_start 启动, 稍微说点. 不知道有没有人好奇, winds 上面使用 socket 的
	时候需要先加载 WSAStartup 操作. 其实是我们用惯了 linux 上面的, 没有先加载操作.
    linux c 程序编译的时候, 默认加了 socket 库的初始化加载工作. 所以咱们就不需要继
    续加载了(编译器主动做了). 这就清晰了 xxx_start 相当于启动操作, 先启动车子, 才
	能操作车子不是吗. 那我们继续看吧

```C
#include "httputil.h"

// 超时时间10s
#define _INT_TIMEOUT		(4)

// libcurl 库退出操作
static inline void _http_end(void) {
	curl_global_cleanup();
}

inline void 
http_start(void) {
	//
	// CURLcode curl_global_init(long flags);
	// @ 初始化libcurl, 全局只需调一次
	// @ flags : CURL_GLOBAL_DEFAULT        // 等同于 CURL_GLOBAL_ALL
	//           CURL_GLOBAL_ALL            // 初始化所有的可能的调用
	//           CURL_GLOBAL_SSL            // 初始化支持安全套接字层
	//           CURL_GLOBAL_WIN32          // 初始化WIN32套接字库
	//           CURL_GLOBAL_NOTHING        // 没有额外的初始化
	//
	CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (code != CURLE_OK) {
		RETURN(NIL, "curl_global_init error = %s.", curl_easy_strerror(code));
	}

	atexit(_http_end);
}
```

    这类 start 操作基本都是单例, 在 main 函数启动的时候组织顺序执行一次就可以. 其中
    atexit 可有可无, 因为 http_start 操作 curl_global_init 需要跟随程序整个生命周期.
    对于这种同程序同生共死的操作, 流程操作系统回收吧. 下面 http_get 函数也只是熟悉 
	libcurl 库的简单用法:

```C
// 请求共有头部
static CURL * _http_head(const char * url, tstr_t str) {
	CURL * crl = curl_easy_init();
	if (NULL == crl) {
		RETURN(NULL, "curl_easy_init error !!!");
	}

	// 设置下载属性和常用参数
	curl_easy_setopt(crl, CURLOPT_URL, url);                    // 访问的URL
	curl_easy_setopt(crl, CURLOPT_TIMEOUT, _INT_TIMEOUT);       // 设置超时时间 单位s
	curl_easy_setopt(crl, CURLOPT_HEADER, true);                // 下载数据包括HTTP头部
	curl_easy_setopt(crl, CURLOPT_NOSIGNAL, true);              // 屏蔽其它信号

	curl_easy_setopt(crl, CURLOPT_WRITEFUNCTION, (curl_write_callback)_http_write);	// 输入函数类型
	curl_easy_setopt(crl, CURLOPT_WRITEDATA, str);              // 输入参数

	return crl;
}

// 请求共有尾部
static inline bool _http_tail(CURL * crl, tstr_t str) {
	CURLcode res = curl_easy_perform(crl);
	tstr_cstr(str);
	curl_easy_cleanup(crl);
	if (res != CURLE_OK) {
		RETURN(false, "curl_easy_perform error = %s!", curl_easy_strerror(res));
	}

	return true;
}

inline bool 
http_get(const char * url, tstr_t str) {
	CURL * crl = _http_head(url, str);
	return crl ? _http_tail(crl, str) : false;
}
```

    对于 http_sget 走 https 协议, 相比 http_get 请求也只是多了个属性设置

```C
// 添加 https 请求设置
static inline void _http_sethttps(CURL * crl) {
	curl_easy_setopt(crl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(crl, CURLOPT_SSL_VERIFYHOST, false);
}

inline bool 
http_sget(const char * url, tstr_t str) {
	CURL * crl = _http_head(url, str);
	if (crl) {
		_http_sethttps(crl);
		return _http_tail(crl, str);
	}
	return false;
}
```

    get 方面请求完了, 自然 post 也会很快就完了. 不是它太简单, 而是这种看的多了, 
    查查手册, 看看源码, 用的会贼溜 ~

```C
// 添加 post 请求设置
static inline void _http_setpost(CURL * crl, const char * params) {
	curl_easy_setopt(crl, CURLOPT_POST, true); 
	curl_easy_setopt(crl, CURLOPT_POSTFIELDS, params);
}

inline bool 
http_post(const char * url, const char * params, tstr_t str) {
	CURL * crl = _http_head(url, str);
	if (crl) {
		_http_setpost(crl, params);
		return _http_tail(crl, str);
	}
	return false;
}

bool 
http_spost(const char * url, const char * params, tstr_t str) {
	CURL * crl = _http_head(url, str);
	if (crl) {
		_http_sethttps(crl);
		_http_setpost(crl, params);
		return _http_tail(crl, str);
	}
	return false;
}
```

    前面几章内功练到好, 这里武技只是三花聚顶, 越打越带劲. 出掌震飞尸, 拔剑灭妖魔.
    当然了 http 解决方案不少, 很多知名网络库都会附赠个 http 处理小单元. libcurl 是
    我见过比较良心的了. 成熟可靠, 资料很全. 武技讲的这么多, 希望修炼的你能懂. 一切
    都是套路, 只有赤城之人才能走到道的远方 ~

#### 4.4 阅读理解时间, 不妨来个定时器

	对于定式器实现而言无外乎三大套路. 一种是有序链表用于解决, 大量重复轮询的定时结点设
	计的. 另一种是采用时间堆构建的定时器, 例如小顶堆, 时间差最小的在堆定执行速度最快. 
	还有一种时间片结构, 时间按照一定颗度转呀转, 转到那就去执行那条颗度上的链表. 总的而
	言定时器的套路不少, 具体看应用的场景. 我们这里带来的阅读理解是基于有序链表, 刚好温
	故下 list 用. 可以说起缘 list, 终于 list. 

sctimer.h

```C
#ifndef _H_SIMPLEC_SCTIMER
#define _H_SIMPLEC_SCTIMER

#include "schead.h"

//
// st_add - 添加定时器事件,虽然设置的属性有点多但是都是必要的
// intval	: 执行的时间间隔, <=0 表示立即执行, 单位是毫秒
// timer	: 定时器执行函数
// arg		: 定时器参数指针
// return	: 返回这个定时器的唯一id
//
extern int st_add_(int intval, node_f timer, void * arg);
#define st_add(intval, timer, arg) st_add_(intval, (node_f)timer, (void *)(intptr_t)arg)

//
// st_del - 删除指定事件
// id		: st_add 返回的定时器id
// return	: void
//
extern void st_del(int id);

#endif // !_H_SIMPLEC_SCTIMER
```

	结构清晰易懂, 业务只有添加和删除. 顺便用了一个去除警告的函数宏技巧 st_add . 实现部分如下, 
	重点感受数据结构内功的 list 结构的用法. 

```C
// 使用到的定时器结点
struct stnode {
    $LIST_HEAD;

    int id;                 // 当前定时器的id
    struct timespec tv;     // 运行的具体时间
    node_f timer;           // 执行的函数事件
    void * arg;             // 执行函数参数
};					   
							   
// 当前链表对象管理器			  
struct stlist {				   
    int lock;               // 加锁用的
    int nowid;              // 当前使用的最大timer id
    bool status;            // false表示停止态, true表示主线程loop运行态
    struct stnode * head;   // 定时器链表的头结点
};

// 定时器对象的单例, 最简就是最复杂
static struct stlist _st;

// 先创建链表对象处理函数
static struct stnode * _stnode_new(int s, node_f timer, void * arg) {
	struct stnode * node = malloc(sizeof(struct stnode));
	if (NULL == node)
		RETURN(NULL, "malloc struct stnode is error!");

	// 初始化, 首先初始化当前id
	node->id = ATOM_INC(_st.nowid);
	timespec_get(&node->tv, TIME_UTC);
	node->tv.tv_sec += s / 1000;
	node->tv.tv_nsec += (s % 1000) * 1000000;
	node->timer = timer;
	node->arg = arg;

	return node;
}

// 得到等待的微秒时间, <=0的时候头时间就可以执行了
static inline int _stlist_sus(struct stlist * st) {
	struct timespec t[1], * v = &st->head->tv;
	timespec_get(t, TIME_UTC);
	return (int)((v->tv_sec - t->tv_sec) * 1000000
		+ (v->tv_nsec - t->tv_nsec) / 1000);
}

// 重新调整, 只能在 _stlist_loop 后面调用, 线程安全,只加了一把锁
static void _stlist_run(struct stlist * st) {
	struct stnode * sn;

	ATOM_LOCK(st->lock); // 加锁防止调整关系覆盖,可用还是比较重要的
	sn = st->head;
	st->head = (struct stnode *)list_next(sn);
	ATOM_UNLOCK(st->lock);

	sn->timer(sn->arg);
	free(sn);
}

// 运行的主loop,基于timer管理器
static void * _stlist_loop(struct stlist * st) {
	// 正常轮询,检测时间
	while (st->head) {
		int nowt = _stlist_sus(st);
		if (nowt > 0) {
			usleep(nowt);
			continue;
		}
		_stlist_run(st);
	}

	// 已经运行结束
	st->status = false;
	return NULL;
}

// st < sr 返回 < 0, == 返回 0, > 返回 > 0
static inline int _stnode_cmptime(const struct stnode * sl, const struct stnode * sr) {
	if (sl->tv.tv_sec != sr->tv.tv_sec)
		return (int)(sl->tv.tv_sec - sr->tv.tv_sec);
	return (int)(sl->tv.tv_nsec - sr->tv.tv_nsec);
}

int 
st_add_(int intval, node_f timer, void * arg) {
	struct stnode * now;
	// 各种前戏操作
	if (intval <= 0) {
		timer(arg);
		return SufBase;
	}

	now = _stnode_new(intval, timer, arg);
	if (NULL == now) {
		RETURN(ErrAlloc, "_new_stnode is error intval = %d.", intval);
	}

	ATOM_LOCK(_st.lock); //核心添加模块 要等, 添加到链表, 看线程能否取消等

	list_add(&_st.head, _stnode_cmptime, now);

	// 这个时候重新开启线程
	if(!_st.status) {
		if (async_run(_stlist_loop, &_st)) {
			list_destroy(&_st.head, free);
			RETURN(ErrFd, "pthread_create is error!");
		}
		_st.status = true;
	}

	ATOM_UNLOCK(_st.lock);
	
	return now->id;
}

// 通过id开始查找
static inline int _stnode_cmpid(int id, const struct stnode * sr) {
	return id - sr->id;
}

void 
st_del(int id) {
	struct stnode * node;
	if (!_st.head) return;

	ATOM_LOCK(_st.lock);
	node = list_findpop(&_st.head, _stnode_cmpid, id);
	ATOM_UNLOCK(_st.lock);

	free(node);
}
```

	其中用到的 usleep 移植到 winds 上面实现为 

```C
// 为 Visual Studio 导入一些 GCC 上优质思路
#ifdef _MSC_VER

//
// usleep - 微秒级别等待函数
// usec		: 等待的微秒
// return	: The usleep() function returns 0 on success.  On error, -1 is returned.
//
int
usleep(unsigned usec) {
	int rt = -1;
	// Convert to 100 nanosecond interval, negative value indicates relative time
	LARGE_INTEGER ft = { .QuadPart = -10ll * usec };

	HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (timer) {
		// 负数以100ns为单位等待, 正数以标准FILETIME格式时间
		SetWaitableTimer(timer, &ft, 0, NULL, NULL, FALSE);
		WaitForSingleObject(timer, INFINITE);
		if (GetLastError() == ERROR_SUCCESS)
			rt = 0;
		CloseHandle(timer);
	}

	return rt;
}

#endif
```
    以上 sctime 模块中操作, 无外乎利用 list 构建了一个升序链表, 通过额外异步分离
	线程 loop 监测下去并执行. 定时器一个通病, 不要放入阻塞函数, 容易失真. 
	sctimer 使用方面也很简单, 例如一个技能, 吟唱 1s, 持续伤害 2s. 构造如下:

```C
struct skills {
	int id;
	bool exist; // 实战走状态机, true 表示施法状态中 
};

// 007 号技能 火球术, 没有释放
struct skills fireball = { 007, false };

static void _end(struct skills * kill) {
	...
	if (kill.id == 007) {
		puts("火球术持续输出结束...");
		kill.exist = false;
	}
	...
}

static void _continued(struct skills * kill) {
	...
	if (kill.id == 007) {
		puts("火球术吟唱成功, 开始持续输出...");
		kill.exist = true;
		st_add(2000, _end, kill);
	}
	...
}

static void _start(struct skills * kill) {
	...
	if (kill.id == 007) {
		puts("火球术开始吟唱...");
		kill.exist = false;
		st_add(1000, _continued, kill);
	}
	...
}
```

	调用 _start 就可以了, 火球术吟唱, 持续输出. 中间打断什么鬼, 那就自己扩展. 后期
	根据标识统一绘制显示. 以上是简单到吐的思路说不定也很有效. 有点像优化过的 select
	特定的时候出其不意 ~

***

	过的真快, 修炼之路已经走过小一半了. 从华山剑法练起, 到现在的一步两步三步. 以后
	自己可以上路了, 单纯的客户端业务的小妖魔, 分分钟可以干掉吧. 本章在实战中会用的最
	多就是日月轮 sclooprun 模块. 对于定时器, 多数内嵌到主线程轮询模块(update) 中.  
	此刻应该多出去历练求索, 在血与歌中感受生的洗礼. 聆听心中的道. Thx

	元日
	王安石 - 宋代

	爆竹声中一岁除，春风送暖入屠苏。
	千门万户曈曈日，总把新桃换旧符。

***

![道](./img/女苑姜明.jpg)
