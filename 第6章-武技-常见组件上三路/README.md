## 第6章-武技-常见组件上三路

    时间过得真快, 此章武技修炼完毕, 修真之旅就过去一半了. 
    武技本是金丹期选手在妖魔大战中招招带血的操作. 原始而奏效. 飞沙走石, 热浪滚滚.
    也许很短, 但未名传说等待你去填写 ~
    我们只带你来回穿梭, 感受一瞬间的震耳欲动, 山河破碎, 天地不仁 ~

    带好你的剑, 
    那年学的华山剑法 ~
    此刻上路吧 ---:o

### 6.1 传说中的线程池

    线程池很普通的老话题, 讨论的很多. 深入的不多, 也就那些基础库中才能见到这种精妙
    完备的技巧. 这里也会随大流想深入简述一种高效控制性强的一种线程池实现.
    先引入一个概念, 惊群. 简单举个例子. 春天来了, 公园出现了很多麻雀. 而你恰巧有一
    个玉米粒. 扔出去, 立马无数麻雀过来争抢. 而最终只有一只麻雀得到了. 而那些没有抢
    到的麻雀很累.......
    编程中惊群, 是个很古老的编程话题了. 在服务器开发有机会遇到. 有兴趣的可以自行搜
    索, 多数介绍的质量非常高. 而我们今天只讨论线程池中惊群现象. 采用的POSIX跨平台
    的线程库 pthread. 

```C
extern int __cdecl pthread_cond_signal (pthread_cond_t * cond);
```

    上面函数就是线程池中出现惊群来源. 它会激活 pthread_cond_wait 等待态一个或多个
    线程. 而这里避免的思路同样很实在, 定向激活, 每个线程都有自己的条件变量.

***

![日出东方](./img/开始.jpg)

#### 6.1.1 线程池接口设计

```C
#ifndef _H_SIMPLEC_SCTHREADS
#define _H_SIMPLEC_SCTHREADS

#include "schead.h"

//
// 这是个简易的线程池的库.
//

typedef struct threads * threads_t;

//
// threads_create - 创建一个线程池处理对象
// return	: 返回创建好的线程池对象, NULL表示失败
//
extern threads_t threads_create(void);

//
// threads_delete - 异步销毁一个线程池对象
// pool		: 线程池对象
// return	: void
//
extern void threads_delete(threads_t pool);

//
// threads_insert - 线程池中添加要处理的任务
// pool		: 线程池对象
// run		: 运行的执行题
// arg		: run的参数
// return	: void
//
extern void threads_insert_(threads_t pool, node_f run, void * arg);
#define threads_insert(pool, run, arg) threads_insert_(pool, (node_f)run, arg)

#endif // !_H_SIMPLEC_SCTHREADS
```

    定义了不完全类型 threads_t, 创建销毁添加等行为. 其中使用了个函数宏的技巧. 用于
    去掉警告. 也许 n 年后, 这类 void (*)(long *) convert void (*)(void *) 不再弹
    出 warning. 那 threads_insert 就可以寿终正寝了. 
    到这里接口设计部分已经完工了. 没有好的设计, 什么都不是 ......

#### 6.1.2 线程池部分实现

    先说容易的实现部分. 线程池解决的问题是避免创建过多线程, 增加操作系统的负担. 从而
    换了一种方式, 将多个线程业务转成, 多个任务被固定的线程来回处理. 是不是很巧妙, 先
    看这里的任务结构设计:

```C
// 任务链表 结构 和 构造
struct job {
	struct job * next;			// 指向下一个任务结点
	node_f run;					// 任务结点执行的函数体
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

    内存处理方式很直接, 申请失败直接退出. 更加粗暴的方式直接调用 abort(). 强制退出.
    线程处理对象和线程池对象结构如下:

```C
// 线程结构体, 每个线程一个信号量, 定点触发
struct thread {
	struct thread * next;		// 下一个线程对象
	bool wait;					// true 表示当前线程被挂起
	pthread_t tid;				// 当前线程id
	pthread_cond_t cond;		// 线程条件变量
};

// 定义线程池(线程集)定义
struct threads {
	size_t size;				// 线程池大小, 最大线程结构体数量
	size_t curr;				// 当前线程池中总的线程数
	size_t idle;				// 当前线程池中空闲的线程数
	volatile bool cancel;		// true表示当前线程池正在 delete
	pthread_mutex_t mutx;		// 线程互斥量
	struct thread * thrs;		// 线程结构体对象集
	struct job * head;			// 线程任务链表的链头, 队列结构
	struct job * tail;			// 线程任务队列的表尾, 后插入后执行
};
```

    可以看出每个 struct thread 线程运行对象中, 都有个 pthread_cont_t 条件变量.
    这就是定向激活的关键. struct threads::cancel 用于标识当前线程池是否在销毁阶段.
    避免使用 pthread_cancel + pthread_cleanup_push 和 pthread_cleanup_pop ...
    这类有风险的设计. 为上面结构顺带写的几个行为如下 :

```C
// 根据cond 内存地址熟悉, 删除pool->thrs 中指定数据
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

// 找到线程tid 对应的条件变量地址
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

    对于struct threads 结构中 struct job * head, *t ail; 是个线程任务队列.
    struct thread * thrs; 是个线程链表. 当前文件中共用 struct threads 中 mutex 一个
    互斥量. 是否想起前面扯的, 链表是C结构中基础的基础, 所有代码都是围绕它这个结构. 一定
    要磨练中熟悉提高, 对于刚学习的人. 上面代码都是业务代码, 申请销毁添加查找等. 前戏讲
    完了, 现在讲解其它简单接口实现代码 :

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

    上面就是创建接口的实现代码, calloc 相比 malloc 多调用了 bzero 的相关清空置零操作. 
    还有一个释放资源函数. 设计意图允许创建多个线程池(但是没有必要). 请看优雅的销毁操作

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
    对于上面释放函数先监测销毁标识, 后竞争唯一互斥量, 竞争到了那么就开始释放了. 先清除
    任务队列并置空, 随后解锁. 再去准备销毁每个线程, 激活它并等待它退出. 最终清除锁销毁
    自己. 优雅结束了线程池的生成周期. 

    如果是不是真爱, 那就追求优雅 ~
    如果是真爱, 那么什么都不想要 ~

#### 6.1.3 线程池核心部分

    核心部分就两个函数, 一个是线程轮询处理任务的函数. 一个是构建线程池函数. 线程轮序函
    数如下:

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

    突然想起以前扯的一句话, 关于提升技术好办法
    1. 多看书
    2. 多写代码, 多搜搜, 多问问
    3. 多看别人的好代码, 多临摹源码
    4. 多创造, 多改进, 多实战
    等该明白的都明白了, 一切都是那样容易, 那样的美的时候. 就可以回家种田了. 哈哈

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

    咱们废了老大劲写了个线程池, 9.9 业务基本都不会用. 密集型业务目前修真界都流行
    轮询消息队列的方式处理, 下一个出场的主角登场了 ~

### 4.2 消息轮序器

    在系统开发中, 消息轮询器基本上就是整个服务器调度处理的核心! 所有待处理的业务
    统一封装 push 进去, 单独线程异步 loop 去处理, 周而复始. 等同于守护门派安定的
    无上大阵. 

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

    通过宏函数 srl_create 启动一个消息轮序器, 需要注册两个函数 run 和 die, 前者用于处理
    每个 push 进来的消息, die 用户 push 进来消息的善后工作(销毁清除). 这个类实现的非常
    精简. 直接贴代码, 比较有价值. 多感受其中的妙用, 真是短小精悍 

scrunloop.c

```C
#include "mq.h"
#include "schead.h"
#include "scrunloop.h"
#include <semaphore.h>

// 具体轮询器
struct srl {
	pthread_t id;			// 具体奔跑的线程
	sem_t blocks;			// 线程阻塞

	mq_t mq;				// 消息队列
	node_f run;				// 每个消息都会调用 run(pop())
	node_f die;				// 每个消息体的善后工作
	volatile bool loop;		// true表示还在继续 
	volatile bool wait;		// true表示当前轮序器正在等待
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

    这里假如是多线程环境 srl_push 触发了并发操作, 那么 sem_post 会执行多次 P操作. 但是
    _srl_loop 处理是单线程的, 只会触发对映次的 sem_wait V操作. 所以 push 不加锁不影响
    业务正确性. 而且 sem_wait 基本是通用层面阻塞性能最好的选择. 这些都是高效的保证. 武技
    修炼中, srl 库是继 clog 库之后, 最小意外的突破.

    修炼到现在, 逐渐进出妖魔战场, 另一个异常恐怖 - 域外天魔正在逼近. 它会拷问你的内心,
    你为什么修炼编程 ? 进入弥天幻境, 多数人在幻境的路上 ~ 不曾解脱 ~  

    走不过去那条大道, 终身无望元婴 ~ 

### 4.3 http util by libcurl

    开发中对于 http 请求的处理, 也是一个环节, 不可缺失. 同样都有固定套路. 这里用的是
    libcurl c http 处理库. 

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
    接收别人的 http请求和接收 tcp请求一样, 通过协议得到完整报文, 抛给业务层. 所以
    封装的 http 工具库, 接口功能很单一

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

    其中对于 http_start 启动, 稍微说点. 不知道有没有人好奇, winds 上面使用 socket
    的时候需要先加载 WSAStartup 操作. 其实是我们用惯了 linux 上面不用先加载操作了.
    linux c 程序编译的时候, 默认加了 socket 库的初始化加载工作. 所以咱们就不需要继
    续加载了(语法糖). 这就清晰了 xxx_start 相当于启动操作, 先启动车子, 才能操作车子
    不是吗. 那我们继续看吧

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
	// @ flags : CURL_GLOBAL_DEFAULT		// 等同于 CURL_GLOBAL_ALL
	//			 CURL_GLOBAL_ALL			// 初始化所有的可能的调用
	//			 CURL_GLOBAL_SSL			// 初始化支持安全套接字层
	//			 CURL_GLOBAL_WIN32			// 初始化WIN32套接字库
	//			 CURL_GLOBAL_NOTHING		// 没有额外的初始化
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
    对于这种痛程序同生共死的操作, 流程操作系统回收吧. 
    http_get 也只是熟悉 libcurl 库的简单用法:

```C
// 请求共有头部
static CURL * _http_head(const char * url, tstr_t str) {
	CURL * crl = curl_easy_init();
	if (NULL == crl) {
		RETURN(NULL, "curl_easy_init error !!!");
	}

	// 设置下载属性和常用参数
	curl_easy_setopt(crl, CURLOPT_URL, url);					// 访问的URL
	curl_easy_setopt(crl, CURLOPT_TIMEOUT, _INT_TIMEOUT);		// 设置超时时间 单位s
	curl_easy_setopt(crl, CURLOPT_HEADER, true);				// 下载数据包括HTTP头部
	curl_easy_setopt(crl, CURLOPT_NOSIGNAL, true);				// 屏蔽其它信号

	curl_easy_setopt(crl, CURLOPT_WRITEFUNCTION, (curl_write_callback)_http_write);	// 输入函数类型
	curl_easy_setopt(crl, CURLOPT_WRITEDATA, str);				// 输入参数

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

    对于 http_sget 走 https 协议的 get 请求也只是多了个属性设置

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

    get 请求完了, 自然 post 也会很快就完了. 不是它太简单, 而是怎么看的多了, 
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

    前面内功练到好, 这里武技只是三花聚顶, 越打越带劲. 出掌震飞尸, 拔剑灭妖魔.
    当然了 http 解决方案不少, 很多知名网络库都会附赠个 http 处理小单元. libcurl 是
    我见过比较良心的了. 成熟可靠, 资料很全. 武技讲的这么多, 希望修炼的你能懂. 一切
    都是套路, 只有赤城之人才能走到道的远方 ~

#### 4.4 阅读理解时间, 不妨来个定时器

    