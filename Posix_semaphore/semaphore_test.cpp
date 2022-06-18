//
// Created by zcq on 2021/5/20.
//
/** linux 命令：$ ipcs #显示所有 消息队列/共享内存段/信号量数组
 ipcs -q  显示所有 消息队列
 ipcs -m  显示所有 共享内存段
 ipcs -s  显示所有 信号量数组
 ipcrm -h 查看删除选项    **/

/**** 多线程 的 同步 与 互斥
互斥锁(同步)、条件变量(同步)、读写锁(同步)、自旋锁(同步)、信号量(同步与互斥) ****/
/* 	同步 与 互斥 的概念：
现代操作系统基本都是多任务操作系统，即同时有大量可调度实体在运行。
在多任务操作系统中，同时运行的多个任务可能：
 	都需要访问/使用同一种资源；
    多个任务之间有依赖关系，某个任务的运行依赖于另一个任务。
---------------------------------------------------------------------------
【同步】：
	是指散步在不同任务之间的若干程序片断，它们的运行必须严格按照规定的某种 先后次序 来运行，
这种 先后次序 依赖于要完成的特定的任务。
	最基本的场景就是：两个或两个以上的进程或线程在运行过程中协同步调，
按预定的 先后次序 运行。比如 A 任务的运行依赖于 B 任务产生的数据。
---------------------------------------------------------------------------
【互斥】：
 	是指散步在不同任务之间的若干程序片断，当某个任务运行其中一个程序片段时，
其它任务就不能运行它们之中的任一程序片段，只能 等 到该任务运行完这个程序片段后才可以运行。
	最基本的场景就是：一个公共资源同一时刻只能被一个进程或线程使用，
多个进程或线程 不能 同时 使用 公共资源。			 */


/** 信号量（semaphore）是一种提供不同进程之间或者一个给定进程不同线程之间的同步。
 	POSIX、SystemV 信号随内核持续。
 	Linux操作系统中，POSIX 有名 信号量 创建在虚拟文件系统中，一般挂载在 /dev/shm，
 其名字以 sem.somename 的形式存在。
 	信号量初始化的值的大小一般用于表示可用资源的数（例如缓冲区大小，之后代码中体现）；
 如果初始化为 1，则称之二值信号量，二值信号量的功能就有点像互斥锁了。
 不同的是：互斥锁的加锁和解锁必须在同一线程执行，而信号量的挂出却不必由执行等待操作的线程执行。
--------------------------------------------------------------------------
 有名信号量（基于路径名 /dev/sem/sem.zcq）：通常用于 不同进程之间的同步
 无名信号量（基于内存的信号量）：通常用于 一个给定进程的 不同线程之间的同步
--------------------------------------------------------------------------
 注意：fork()的子进程，通常不共享父进程的内存空间，
 子进程是在父进程的副本上启动的，它跟共享内存区不是一回事。
 不同进程之间的同步 若使用 无名信号量（基于内存的信号量），要考虑指针或地址的关联。
--------------------------------------------------------------------------
 一个信号量的最大值  SEM_VALUE_MAX  (2147483647)  **/


/*	#include <semaphore.h>
 	信号量 广泛用于进程或线程间的同步和互斥，信号量本质上是一个非负的整数计数器，
它被用来控制对公共资源的访问。可根据操作信号量值的结果判断是否对公共资源具有访问的权限，
当信号量值大于 0 时，则可以访问，否则将阻塞。PV 原语是对信号量的操作，
一次 P (wait) 操作使信号量减１，一次 V (post) 操作使信号量加１。
---------------------------------------------------------------------------
 Posix 信号量的调用方式：
 有名信号量          						无名信号量(基于内存的信号量)
 (基于路径名 /dev/sem/sem.zcq) 			sem_init()
 sem_open()
					信号量共用 API
 					sem_wait() 						// 等待唤醒，信号量减１
 					sem_trywait() 					// 非阻塞，信号量减１
 					sem_timedwait() 				// 超时操作将返 -1，否则信号量减１
 					sem_clockwait()
 					sem_post() 						// 唤醒，信号量加１
	 				sem_getvalue() 					// 获取信号量值
 有名信号量 								无名信号量
 sem_close()							sem_destory()
 sem_unlink()
---------------------------------------------------------------------------
 打开/创建 有名 信号量
 sem_t *sem_open(const char *name, int oflag);
 sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
	 oflag	可以设置为 O_CREAT | O_EXCL (如果存在则返回错误，同 open)。
	 mode	可以设置为 0755。
	 value	信号量初始值，不超过 SEM_VALUE_MAX。
 返回：若成功则为指向信号量的指针，若出错则为 SEM_FAILED。
---------------------------------
 int sem_close (sem_t *sem);
 int sem_unlink (const char *name);
---------------------------------------------------------------------------
 初始化 无名 信号量
 int sem_init(sem_t *sem, int pshared, unsigned int value);
	 pshared 指定该信号量是用于 进程 或 线程 同步
		0	表示用于 同进程的不同线程间 同步（所有线程可见）
		非0	表示用于 不同进程间 同步（需要放在共享内存中）
	 value	信号量初始值
---------------------------------
 int sem_destroy(sem_t *sem); // 销毁信号量
---------------------------------------------------------------------------
 int sem_wait(sem_t *sem); // 等待唤醒，P操作 信号量减１
 int sem_trywait(sem_t *sem); // 非阻塞，信号量减１
 int sem_timedwait(sem_t *sem, const struct timespec *abstimeout); // 超时操作将返 -1
 int sem_clockwait(sem_t *sem, clockid_t clock, const struct timespec *abstime)
---------------------------------------------------------------------------
 int sem_post(sem_t *sem); // 唤醒，V操作 信号量加１
---------------------------------------------------------------------------
 获取信号量值
 int sem_getvalue(sem_t *sem, int *sval); // 获取 sem 当前值并储存在 sval
 如果当前有一个或多个进程或线程正在使用 sem_wait() 等待信号量，
 返回两种结果在 sval 里：
 	返回 0：该信号量当前已上锁。
 	返回负值：其绝对值就是等待该信号量解锁的 进程或线程 数量（如阻塞在 sem_wait()）。
注意：信号量的值可能在 sem_getvalue()返回时已经被更改。
---------------------------------------------------------------------------  */


#include "semaphore_test.h"


/******************************* 生产者和消费者 **********************************/

#define	buff_len 	10
#define	items_num 	(90+buff_len)

/**---------------------- 有名信号量:生产者和消费者问题 ----------------------**/

namespace shared_named_one {
#define	SEM_MUTEX	"mutex" 	// 基于路径名 /dev/sem/sem.mutex
#define	SEM_EMPTY	"empty" 	// 基于路径名 /dev/sem/sem.empty
#define	SEM_STORED	"stored" 	// 基于路径名 /dev/sem/sem.stored
	struct {    /* data shared by producer and consumer */
		int buff[buff_len];
		sem_t *mutex, *empty_num, *stored_num;
	} shared;
	void *produce(void *arg) {
		/* 	0 ~ buff_len   ~ items_num
			< 初始化缓冲区 >  < 生产数量 >         	 */
		for (int i = 0; i < items_num; ++i) {
			Sem_wait(shared.empty_num);        // 等待至少1个空插槽
			Sem_wait(shared.mutex);
			_debug_log_info("produce_in\tbuff[%d] = %d", i % buff_len, i)
			shared.buff[i % buff_len] = i;    /* store i into circular buffer */
			Sem_post(shared.mutex);
			Sem_post(shared.stored_num);    // 已有1个存储项目
		}
		return (nullptr);
	}
	void *consume(void *arg) {
		for (int i = 0; i < items_num; ++i) {
			Sem_wait(shared.stored_num);    // 等待至少1个存储项目
			Sem_wait(shared.mutex);
			if (shared.buff[i % buff_len] != (i % buff_len)) {
				_debug_log_info("consume_out\tbuff[%d] = %d",
								i % buff_len, shared.buff[i % buff_len])
			}
			Sem_post(shared.mutex);
			Sem_post(shared.empty_num);        // 已有1个空插槽
		}
		return (nullptr);
	}
}

void test_semaphore_named() // 有名信号量:生产者和消费者问题
{
	using namespace shared_named_one;
	shared.mutex = Sem_open(SEM_MUTEX, O_CREAT, FILE_MODE, 1);
	shared.empty_num = Sem_open(SEM_EMPTY, O_CREAT, FILE_MODE, buff_len);
	shared.stored_num = Sem_open(SEM_STORED, O_CREAT, FILE_MODE, 0);

	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量
	int ret = pthread_setconcurrency(3); 			// 设置并发级别 即 线程数量
	err_sys(ret, __LINE__, "pthread_setconcurrency()");
	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("主线程 ID: %lu", pid)

	pthread_t tid_produce, tid_consume;
	ret = pthread_create(&tid_produce, nullptr, produce, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");
	ret = pthread_create(&tid_consume, nullptr, consume, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");

//	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms
	_debug_log_info("线程 ID: %lu", tid_produce)
	_debug_log_info("线程 ID: %lu", tid_consume)

	pthread_join(tid_produce, nullptr);
	pthread_join(tid_consume, nullptr);

	Sem_unlink(SEM_MUTEX);
	Sem_unlink(SEM_EMPTY);
	Sem_unlink(SEM_STORED);
}

/**---------------- 无名信号量(基于内存的信号量):生产者和消费者问题 -------------**/

namespace shared_unnamed_one {
	struct {    /* data shared by producer and consumer */
		int buff[buff_len];
		sem_t mutex, empty_num, stored_num;
	} shared;
	void *produce(void *arg) {
		/* 	0 ~ buff_len   ~ items_num
			< 初始化缓冲区 >  < 生产数量 >         	 */
		for (int i = 0; i < items_num; ++i) {
			Sem_wait(&shared.empty_num);	// 等待至少1个空插槽
			Sem_wait(&shared.mutex);
			_debug_log_info("produce_in\tbuff[%d] = %d", i % buff_len, i)
			shared.buff[i % buff_len] = i;    /* store i into circular buffer */
			Sem_post(&shared.mutex);
			Sem_post(&shared.stored_num); 	// 已有1个存储项目
		}
		return (nullptr);
	}
	void *consume(void *arg) {
		for (int i = 0; i < items_num; ++i) {
			Sem_wait(&shared.stored_num);  	// 等待至少1个存储项目
			Sem_wait(&shared.mutex);
			if (shared.buff[i % buff_len] != (i % buff_len)) {
				_debug_log_info("consume_out\tbuff[%d] = %d",
								i % buff_len, shared.buff[i % buff_len])
			}
			Sem_post(&shared.mutex);
			Sem_post(&shared.empty_num);  	// 已有1个空插槽
		}
		return (nullptr);
	}
}

void test_semaphore_unnamed() // 无名信号量(基于内存的信号量):生产者和消费者问题
{
	using namespace shared_unnamed_one;
	Sem_init(&shared.mutex, 0/*0:同进程的不同线程间*/, 1);
	Sem_init(&shared.empty_num, 0/*0:同进程的不同线程间*/, buff_len);
	Sem_init(&shared.stored_num, 0/*0:同进程的不同线程间*/, 0);

	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量
	int ret = pthread_setconcurrency(3); 			// 设置并发级别 即 线程数量
	err_sys(ret, __LINE__, "pthread_setconcurrency()");
	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("主线程 ID: %lu", pid)

	pthread_t tid_produce, tid_consume;
	ret = pthread_create(&tid_produce, nullptr, produce, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");
	ret = pthread_create(&tid_consume, nullptr, consume, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");

//	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms
	_debug_log_info("线程 ID: %lu", tid_produce)
	_debug_log_info("线程 ID: %lu", tid_consume)

	pthread_join(tid_produce, nullptr);
	pthread_join(tid_consume, nullptr);

	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.empty_num);
	Sem_destroy(&shared.stored_num);
}

/**---------------- 无名信号量(基于内存的信号量):多个生产者，单个消费者 -------------**/

#define	MAXNTHREADS	6 // 6个 生产者/消费者 线程

namespace shared_unnamed_01 {
	struct {    /* data shared by producer and consumer */
		int buff[buff_len];
		int total_produced; // 已生产总量
		int total_consumed; // 已消费总量
		sem_t mutex, empty_num, stored_num;
	} shared;
	void *produce(void *arg) {
		while (true) {
			Sem_wait(&shared.empty_num);	// 等待至少1个空插槽
			Sem_wait(&shared.mutex);

			if (shared.total_produced >= items_num) {
				Sem_post(&shared.empty_num);
				Sem_post(&shared.mutex);
				break;
			}
			shared.buff[shared.total_produced % buff_len] = shared.total_produced;
			_debug_log_info("produce_in\tbuff[%d] = %d",
							shared.total_produced % buff_len,
							shared.buff[shared.total_produced % buff_len])
			shared.total_produced++;

			Sem_post(&shared.mutex);
			Sem_post(&shared.stored_num); 	// 已有1个存储项目
			*((int *)arg) += 1; // 每个生产者线程循环次数
		}
		return (nullptr);
	}
	void *consume(void *arg) {
		for (int i = 0; i < items_num; ++i) {
			Sem_wait(&shared.stored_num);  	// 等待至少1个存储项目
			Sem_wait(&shared.mutex);
			if (shared.total_consumed < shared.total_produced &&
				shared.buff[i % buff_len] != -1) {
				_debug_log_info("consume_out\tbuff[%d] = %d",
								i % buff_len, shared.buff[i % buff_len])
				shared.buff[i % buff_len] = -1;
				shared.total_consumed++;
			}
			Sem_post(&shared.mutex);
			Sem_post(&shared.empty_num);  	// 已有1个空插槽
		}
		return (nullptr);
	}
}

void test_semaphore_unnamed_01() // 无名信号量(基于内存的信号量):多个生产者，单个消费者
{
	using namespace shared_unnamed_01;
	bzero(&shared, sizeof(shared));
	Sem_init(&shared.mutex, 0/*0:同进程的不同线程间*/, 1);
	Sem_init(&shared.empty_num, 0/*0:同进程的不同线程间*/, buff_len);
	Sem_init(&shared.stored_num, 0/*0:同进程的不同线程间*/, 0);

	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量
	int ret = pthread_setconcurrency(MAXNTHREADS+1+1); // 设置并发级别 即 线程数量
	err_sys(ret, __LINE__, "pthread_setconcurrency()");
	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("主线程 ID: %lu", pid)

	int run_count[MAXNTHREADS]; // 每个生产者线程循环次数
	pthread_t tid_produce[MAXNTHREADS], tid_consume;
	for (int i=0; i < MAXNTHREADS; ++i) {
		run_count[i] = 0;
		ret = pthread_create(&tid_produce[i], nullptr, produce, &run_count[i]);
		err_sys(ret, __LINE__, "pthread_create()");
		_debug_log_info("生产者线程 ID: %lu", tid_produce[i])
	}
	pthread_create(&tid_consume, nullptr, consume, nullptr);
	_debug_log_info("消费者线程 ID: %lu", tid_consume)
//	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms

	for (int i=0; i < MAXNTHREADS; ++i) {
		pthread_join(tid_produce[i], nullptr);
		_debug_log_info("生产者线程 ID:%lu\t循环次数: %d", tid_produce[i], run_count[i])
	}
	pthread_join(tid_consume, nullptr);

	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.empty_num);
	Sem_destroy(&shared.stored_num);
}

/**---------------- 无名信号量(基于内存的信号量):多个生产者，多个消费者 -------------**/

namespace shared_unnamed_02 {
	struct {    /* data shared by producer and consumer */
		int buff[buff_len];
		int total_produced; // 已生产总量
		int total_consumed; // 已消费总量
		sem_t mutex, empty_num, stored_num;
	} shared;
	void *produce(void *arg) {
		while (true) {
			Sem_wait(&shared.empty_num);	// 等待至少1个空插槽
			Sem_wait(&shared.mutex);

			if (shared.total_produced >= items_num) {
				Sem_post(&shared.stored_num);	/* let consumers terminate */
				Sem_post(&shared.empty_num);
				Sem_post(&shared.mutex);
				break;
			}
			shared.buff[shared.total_produced % buff_len] = shared.total_produced;
			_debug_log_info("produce_in\tbuff[%d] = %d",
							shared.total_produced % buff_len,
							shared.buff[shared.total_produced % buff_len])
			shared.total_produced++;

			Sem_post(&shared.mutex);
			Sem_post(&shared.stored_num); 	// 已有1个存储项目
			*((int *)arg) += 1; // 每个生产者线程循环次数
		}
		return (nullptr);
	}
	void *consume(void *arg) {
		int val;
		while (true) {
			Sem_wait(&shared.stored_num);  	// 等待至少1个存储项目
			Sem_wait(&shared.mutex);

			if (shared.total_consumed >= items_num) {
				Sem_post(&shared.stored_num);	/* let consumers terminate */
				Sem_post(&shared.mutex);
				break;
			}
			val = shared.total_consumed % buff_len;
			if (shared.total_consumed < shared.total_produced &&
				shared.buff[val] != -1) {
				_debug_log_info("consume_out\tbuff[%d] = %d", val, shared.buff[val])
				shared.buff[val] = -1;
				shared.total_consumed++;
			}

			Sem_post(&shared.mutex);
			Sem_post(&shared.empty_num);  	// 已有1个空插槽
			*((int *)arg) += 1; // 每个消费者线程循环次数
		}
		return (nullptr);
	}
}

void test_semaphore_unnamed_02() // 无名信号量(基于内存的信号量):多个生产者，多个消费者
{
	using namespace shared_unnamed_02;
	bzero(&shared, sizeof(shared));
	Sem_init(&shared.mutex, 0/*0:同进程的不同线程间*/, 1);
	Sem_init(&shared.empty_num, 0/*0:同进程的不同线程间*/, buff_len);
	Sem_init(&shared.stored_num, 0/*0:同进程的不同线程间*/, 0);

	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量
	int ret = pthread_setconcurrency(MAXNTHREADS*2+1); // 设置并发级别 即 线程数量
	err_sys(ret, __LINE__, "pthread_setconcurrency()");
	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("主线程 ID: %lu", pid)

	int run_produce_count[MAXNTHREADS]; // 每个生产者线程循环次数
	int run_consume_count[MAXNTHREADS]; // 每个消费者线程循环次数
	pthread_t tid_produce[MAXNTHREADS], tid_consume[MAXNTHREADS];

	for (int i=0; i < MAXNTHREADS; ++i) {
		run_produce_count[i] = 0;
		ret = pthread_create(&tid_produce[i], nullptr,
							 produce, &run_produce_count[i]);
		err_sys(ret, __LINE__, "pthread_create()");
		_debug_log_info("生产者线程 ID: %lu", tid_produce[i])
	}
	for (int i=0; i < MAXNTHREADS; ++i) {
		run_consume_count[i] = 0;
		ret = pthread_create(&tid_consume[i], nullptr,
							 consume, &run_consume_count[i]);
		err_sys(ret, __LINE__, "pthread_create()");
		_debug_log_info("消费者线程 ID: %lu", tid_consume[i])
	}
//	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms

	for (int i=0; i < MAXNTHREADS; ++i) {
		pthread_join(tid_produce[i], nullptr);
		_debug_log_info("生产者线程 ID:%lu\t循环次数: %d",
						tid_produce[i], run_produce_count[i])
	}
	for (int i=0; i < MAXNTHREADS; ++i) {
		pthread_join(tid_consume[i], nullptr);
		_debug_log_info("消费者线程 ID:%lu\t循环次数: %d",
						tid_consume[i], run_consume_count[i])
	}

	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.empty_num);
	Sem_destroy(&shared.stored_num);
}

/*********************** 生产者和消费者：经典“双缓冲”方案 **************************/
/* 	多线程 + 多缓冲区，文件复制
	在 单核 CPU 中，单进程 的 多线程 能实现 迸发 效果，
 但是，把 整个操作的 读和写 分割到两个线程中 并不影响 完成整个操作所需时间。
 如在单核CPU中，多个缓冲区 在 两个线程间 来回切换，没有得到速度上的优势。
	在 多核 CPU 或 多个 CPU 中，多进程（多线程）能实现 迸发 效果，也能得到速度上的优势。  */

namespace shared_unnamed_buffer {
	struct {    /* data shared by producer and consumer */
		struct {
			char data[MAXLINE];
			ssize_t	n; // count of #bytes in the data[BUFFSIZE]
		} buff[buff_len]; // buff_len 个
		int fd; // input file to copy to stdout
		sem_t empty_num, stored_num;
	} shared;
	void *produce_in(void *arg) { /* input file */
		int num = 0;
		while (true) {
			Sem_wait(&shared.empty_num);	// 等待至少1个空插槽

			shared.buff[num].n = Read(shared.fd, shared.buff[num].data,
									  MAXLINE, 0);
			_debug_log_info("produce_in\tsize:%zd\tbuff[%d]:\n%s",
							shared.buff[num].n, num, shared.buff[num].data)
			if (shared.buff[num].n == 0) {
				shared.buff[num].n = -10; // end of file
				Sem_post(&shared.stored_num); /* let consumers terminate */
				break;
			}
			if (++num >= buff_len) num = 0;

			Sem_post(&shared.stored_num); 	// 已有1个存储项目
		}
		return (nullptr);
	}
	void *consume_out(void *arg) { /* output to stdout */
		int num = 0;
		while (true) {
			Sem_wait(&shared.stored_num);  	// 等待至少1个存储项目

			if (shared.buff[num].n == -10) break; // end of file
			if (shared.buff[num].n != 0) {
				_debug_log_info("consume_out\tsize:%zd\tbuff[%d]:\n%s",
								shared.buff[num].n, num, shared.buff[num].data)
//				Write(STDOUT_FILENO, shared.buff[num].data, shared.buff[num].n);
				shared.buff[num].n = 0;
			}
			if (++num >= buff_len) num = 0;

			Sem_post(&shared.empty_num);  	// 已有1个空插槽
		}
		return (nullptr);
	}
}

void test_semaphore_unnamed_buffer() // 生产者和消费者：经典“双缓冲”方案
{
	using namespace shared_unnamed_buffer;
	bzero(&shared, sizeof(shared));
	shared.fd = Open("/home/zcq/.zsh_history", O_RDONLY);

	Sem_init(&shared.empty_num, 0/*0:同进程的不同线程间*/, buff_len);
	Sem_init(&shared.stored_num, 0/*0:同进程的不同线程间*/, 0);

	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量
	int ret = pthread_setconcurrency(2+1); // 设置并发级别 即 线程数量
	err_sys(ret, __LINE__, "pthread_setconcurrency()");
	std::cout << "获取并发级别 即 线程数量: "
			  << pthread_getconcurrency() << std::endl; // 获取并发级别 即 线程数量

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("主线程 ID: %lu", pid)

	pthread_t tid_produce, tid_consume;
	ret = pthread_create(&tid_produce, nullptr, produce_in, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");
	ret = pthread_create(&tid_consume, nullptr, consume_out, nullptr);
	err_sys(ret, __LINE__, "pthread_create()");

//	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms
	_debug_log_info("线程 ID: %lu", tid_produce)
	_debug_log_info("线程 ID: %lu", tid_consume)

	pthread_join(tid_produce, nullptr);
	pthread_join(tid_consume, nullptr);

	Sem_destroy(&shared.empty_num);
	Sem_destroy(&shared.stored_num);
}
