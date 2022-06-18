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


#include "wrap_semphore.h"


/********************* 有名信号量(基于路径名 /dev/shm/sem.zcq) ********************/

/* 打开/创建 有名信号量 */
sem_t *Sem_open(const char *sem_name, int oflag, ... /* 可直接调用 */ )
{
	sem_t *sem;
	if (oflag & O_CREAT) {
		mode_t mode;
		uint value;
		va_list	ap;
		va_start(ap, oflag); // <stdarg.h>
		mode = va_arg(ap, mode_t);
		value = va_arg(ap, uint);
		sem = sem_open(sem_name, oflag, mode, value);
		va_end(ap);
	}
	else {
		sem = sem_open(sem_name, oflag);
	}
	if (sem == SEM_FAILED) {
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sem_open(): %s", sem_name);
	}
	return(sem);
}

void Sem_close(sem_t *sem)
{
	if (sem_close(sem) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_close()");
}

void Sem_unlink(const char *sem_name)
{
	if (sem_unlink(sem_name) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_unlink()");
}

/************************* 无名信号量(基于内存的信号量) ****************************/

/* 初始化 无名 信号量 */
void Sem_init(sem_t *sem, const int &pshared, const uint &value)
{
	if (sem_init(sem, pshared, value) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_init()");
}

void Sem_destroy(sem_t *sem) // 销毁信号量
{
	if (sem_destroy(sem) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_destroy()");
}

/******************************* 信号量共用 API *********************************/

void Sem_wait(sem_t *sem) // 等待唤醒，信号量减１
{
	if (sem_wait(sem) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_wait()");
}

int Sem_trywait(sem_t *sem) // 非阻塞，信号量减１
{
	int rc;
	if ( (rc = sem_trywait(sem)) == -1 && errno != EAGAIN)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_trywait()");
	return(rc);
}

/*  超时操作将返 -1，否则信号量减１ */
int Sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
	int rc;
	if ( (rc = sem_timedwait(sem, abstime)) == -1 && errno != ETIMEDOUT/*超时*/)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_timedwait()");
	return(rc);
}

int Sem_clockwait(sem_t *sem, const clockid_t &clock,
				  const struct timespec *abstime)
{
	int rc;
	if ( (rc = sem_clockwait(sem, clock, abstime)) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_clockwait()");
	return(rc);
}

void Sem_post(sem_t *sem) // 唤醒，信号量加１
{
	if (sem_post(sem) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_post()");
}

void Sem_getvalue(sem_t *sem, int *valp) // 获取信号量值
{
	if (sem_getvalue(sem, valp) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "sem_getvalue()");
}
