
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

/***************************
 * 		条件变量（同步）condition
 * **************************/
/*	与互斥锁不同，条件变量 是 用来 等待 而不是用来上锁的。
条件变量 用来 自动阻塞 一个线程，直到某特殊情况发生为止。
通常 条件变量 和 互斥锁 同时使用。条件变量使我们可以睡眠等待某种条件出现。
条件变量 是利用线程间 共享的 全局变量 进行同步 的一种机制，主要包括两个动作：
一个线程等待"条件变量的条件成立"而挂起；另一个线程使 “条件成立”（给出条件成立信号）。
---------------------------------------------------------------------------
【原理】：
条件的检测是在互斥锁的保护下进行的。线程在改变条件状态之前必须首先锁住互斥量。
如果一个条件为假，一个线程自动阻塞，并释放等待状态改变的互斥锁。
如果另一个线程改变了条件，它发信号给关联的条件变量，
唤醒一个或多个等待它的线程，重新获得互斥锁，重新评价条件。
如果两进程共享可读写的内存，条件变量 可以被用来实现这两进程间的线程同步。
---------------------------------------------------------------------------
【条件变量的操作流程如下】：
1. 初始化：init() 或者 pthread_cond_t cond=PTHREAD_COND_INITIALIER；属性置为NULL；
2. 等待条件成立：pthread_wait()、pthread_cond_timedwait() 释放锁,
并阻塞等待条件变量为真 timewait() 设置等待时间,
仍未 signal,返回 ETIMEOUT(加锁保证只有一个线程wait)；
3. 激活条件变量：pthread_cond_signal,pthread_cond_broadcast(激活所有等待线程)；
4. 清除条件变量：destroy;无线程等待,否则返回 EBUSY。
---------------------------------------------------------------------------
创建 条件变量
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr);
---------------------------------------------------------------------------
阻塞等待
int pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex);
---------------------------------------------------------------------------
超时等待
int pthread_cond_timewait ( pthread_cond_t *cond,pthread_mutex *mutex,
							const timespec *abstime);
---------------------------------------------------------------------------
销毁 条件变量
int pthread_cond_destroy(pthread_cond_t *cond);
---------------------------------------------------------------------------
触发 条件变量 至少唤醒一个等待该条件的线程
int pthread_cond_signal(pthread_cond_t *cond);
---------------------------------------------------------------------------
广播 条件变量 唤醒等待该条件的所有线程
int pthread_cond_broadcast(pthread_cond_t *cond);


  */


/**************** POSIX 线程 可和 c++ 线程 联合使用 *********************/

/**************** c++ 线程 ***************/
#include <thread> // C++ 多线程
#include <atomic> // C++ 多线程
#include <mutex> // 互斥锁
#include <condition_variable> // C++ 多线程
#include <future> // C++ 多线程

/**************** POSIX 线程 *********************/
#include <pthread.h>

#include <chrono> // std::chrono::system_clock
#include <ctime> // std::time_t, std::tm, std::localtime, std::mktime
//#include <cstring>
#include <cstddef>
#include <string>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cstdio>
#include <sys/types.h> // POSIX Standard: Primitive System Data Types
#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <iostream>
using namespace std;



/*	bash shell 	------------------------
	$ g++ test.cpp -o test
  	$ ./test myfile.txt youfile.txt one.sh
	c++ 		------------------------
		int main(int argc, char **argv)
----------------------------------------
  	c++    		bash shell
  	argc	=	${#argv[@]} # 参数个数
  	argv[0]	=	myfile.txt 	# 参数 : ./当前文件名
  	argv[1]	=	myfile.txt 	# 参数 1
  	argv[2]	=	youfile.txt # 参数 2
  	argv[3]	=	one.sh 	# 参数 3	  	*/
#if 0
int main(int argc, char **argv)
{



#if 1
	this_thread::sleep_for(std::chrono::milliseconds(10));//线程休眠 10ms
#else
	sleep(1); // 进程休眠 1s
	usleep(10); // 进程休眠 10 ms
#endif

	this_thread::yield();  //让出自己的CPU时间片

	cout << "当前 mian 线程退出" << endl;
	pthread_exit(nullptr); // 当前线程退出，不影响进程

	return 0;
}
#endif

/************ c++ 线程 std::thread 类 成员函数 ***********/
/*	#include <thread> // c++ 线程 std::thread 类 成员函数
---------------------------------------------------------------------------
thread::id thread::get_id(); // 获取 线程标识符，返回对象为 std::thread::id
---------------------------------------------------------------------------
thread::join(); // 阻塞当前线程，直至 子线程 结束（会立即释放资源）
---------------------------------------------------------------------------
thread::detach(); // 线程分离，线程可单独执行（会自动释放资源）
---------------------------------------------------------------------------
bool thread::joinable(); // 判断线程是否可被 join 连接
缺省构造的thread对象、已经 join 或 已经 detach 的thread对象都不是 joinable。
---------------------------------------------------------------------------
pthread_t thread::native_handle(); // 获得与操作系统相关的原生线程句柄
该函数 返回 与 std::thread 具体实现相关的线程句柄。
native_handle() 是连接 thread类 和 操作系统 SDK API 之间的桥梁，
当 thread类 的功能不能满足我们的要求的时候(比如改变某个 线程优先级)，
可以通过 thread类实例的 native_handle()返回值作为参数来调用相关的 pthread函数达到目录。
---------------------------------------------------------------------------
void thread::swap(thread &t); // 线程交换
---------------------------------------------------------------------------
static unsigned int thread::hardware_concurrency(); // 硬件最大的并发线程数目
返回计算机硬件最大的并发线程数目。基本上可以视为处理器的核心数目。		 */

/************ c++ 线程 std::this_thread 类 成员函数 ***********/
/*	#include <thread> // c++ 线程 std::this_thread 类 成员函数
std::this_thread 类 成员函数 用来辅助 std::thread 类，std::async 类。
---------------------------------------------------------------------------
thread::id this_thread::get_id(); // 获取 线程标识符，返回对象为 std::thread::id
---------------------------------------------------------------------------
void this_thread::sleep_for(const chrono::duration<Rep,Period>& rel_time);//线程休眠
阻塞 block 调用该函数的线程 一段时间
---------------------------------------------------------------------------
this_thread::sleep_until(const chrono::duration<Rep,Period>& rel_time);//线程休眠
阻塞 block 调用该函数的线程 直到 指定的时间点 timepoint
---------------------------------------------------------------------------
this_thread::yield(); // 线程让出 CPU
将 处理器 CPU 移交给另一个线程或进程，以允许其他具有相同优先级的线程获得执行的机会。
但是很有可能该线程刚刚回到可执行状态又被再次执行。		 */

