/** 创建新的线程不影响已打开的描述符的引用计数，这一点不同于 fork()
 最后总结下std::thread对比于pthread的优缺点：
优点：
1. 简单，易用
2. 跨平台，pthread只能用在 POSIX(ANSI C) 系统上（其他系统有其独立的thread实现）
3. 提供了更多高级功能，比如future
4. 更加C++（跟匿名函数，std::bind，RAII等C++特性更好的集成）

缺点：
1. 没有RWlock。有一个类似的shared_mutex，不过它属于C++14,你的编译器很有可能不支持。
2. 操作线程和Mutex等的API较少。毕竟为了跨平台，只能选取各原生实现的子集。如果你需要设置某些属性，
 需要通过API调用返回原生平台上的对应对象，再对返回的对象进行操作。
 **/

/************** POSIX 线程 ******************/


/**************** c++ 线程 ***************/
#include <thread> // C++ 多线程
#include <atomic> // C++ 多线程
#include <mutex> // 互斥锁
#include <condition_variable> // C++ 多线程
#include <future> // C++ 多线程

/**************** c++ 线程 可和 POSIX 线程 联合使用 *********************/
#include <pthread.h>
#include <chrono> // std::chrono::system_clock
#include <ctime> // std::time_t, std::tm, std::localtime, std::mktime
#include <cstring>
#include <csignal>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <future>
#include <string>
#include <sys/types.h> // POSIX Standard: Primitive System Data Types
#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <iostream>
using namespace std;


/* 输出自定义错误信息 */
void perr(const int &retval, const int &code_line=0, string_view ptr="");
//perr(retval, __LINE__, "info");//输出自定义错误信息

void fun_clear(void *arg); // 线程清理函数
void mypthread_state(const pthread_t &pid); // 获取线程属性

//---------------------------------------

void mypthread_join_args(); // 线程函数(参数)
void mypthread_join_signal(); // 线程函数(可连接)
void mypthread_join_cancel(); // 线程函数(可连接)
void mypthread_join_detach(); // 线程函数(可连接转为可分离)
void mypthread_detach(); // 线程函数(可分离)


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
	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	cout << "线程函数 " << __func__ << "() (标识符: "
		 << pid << " )" << endl;

	mypthread_join_args(); // 线程函数(参数)
	mypthread_join_signal(); // 线程函数(可连接)
	mypthread_join_cancel(); // 线程函数(可连接)
	mypthread_join_detach(); // 线程函数(可连接转为可分离)
	mypthread_detach(); // 线程函数(可分离)


#if 1
	this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms
#else
	sleep(1); // 进程休眠 1s
	usleep(10); // 进程休眠 10 ms
#endif
	cout << "当前 mian 线程退出" << endl;
	/* 会触发 pthread_cleanup_push() pthread_cleanup_pop() 可连接线程清理处理程序
	 调用 线程清理函数 fun_clear() */
	pthread_exit(nullptr); // 当前线程退出，不影响进程
	return 0;
}
#endif


/**------------------------ POSIX 线程 -----------------------------**/

#define datamode 0 // 参数 选择

#if datamode==3 // 无 参数;  全局变量 -> 共享 进程数据
int count=0; // 全局变量 = 共享 进程数据，将会在 主线程 和 子线程 中用到
#endif
#if datamode==1 // 结构体 参数
/* 线程函数 结构体 参数 */
struct struct_pthread // 结构体 参数
{
	int num;
	string str;
};
#endif

void *mypthread_seed_args(void *arg) /*  (子)线程函数 */
{
	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	cout << "(子)线程函数 " << __func__ << "() (标识符: " << pid << ")" << endl;

#if datamode==0 // long int 参数
	long *longnum=(long *)(arg);
	cout << "*longnum: " << * longnum << endl;
	++(*longnum);
	cout << "++(*longnum): " << * ((long *)arg) << endl;
#elif datamode==1 // 结构体 参数
	auto *mystruct=(struct struct_pthread *)arg;
	cout << "mystruct->num: " << mystruct->num << endl
		 << "mystruct->str: " << mystruct->str << endl
		 << "mystruct->str.size: " << mystruct->str.size() << endl;
#elif datamode==2 // 字符指针 参数
	const char *str=(char *)(arg);
	cout << "str: " << str << endl;
#elif datamode==3 // 无 参数;  全局变量 -> 共享 进程数据
	while (true) {
		++count; // 全局变量 -> 共享 进程数据，将会在 主线程 和 子线程 中用到
		if (count >= 1000000)break;
	}
#endif
	return (void *)nullptr;
}


void mypthread_join_args() // 线程函数(参数)
{
	pthread_t pid; // 线程标识符
	int retval; // 返回值

#if datamode==0 // long int 参数
	long num = 1234567890;
	retval = pthread_create(&pid, nullptr /* 默认属性 */,
							mypthread_seed_args, &num /* long int 参数 */);//创建线程
#elif datamode==1 // 结构体 参数
	struct struct_pthread mystruct{ 665, "0"};
	++mystruct.num;
	mystruct.str="zcq";
	retval = pthread_create(&pid, nullptr /* 默认属性 */, mypthread_none,
							(void *)&mystruct /* 结构体 参数 */);//创建线程
#elif datamode==2 // 字符指针 参数
	const char *str="abcdefg";
	retval = pthread_create(&pid, nullptr /* 默认属性 */, mypthread_none,
							(void *)str /* 字符指针 参数 */);//创建线程
#elif datamode==3 // 无 参数;  全局变量 -> 共享 进程数据
	retval = pthread_create(&pid, nullptr /* 默认属性 */,
							mypthread_none, nullptr /* 无参数 */);//创建线程
	cout << "count: " << count << endl;
#endif
	perr(retval, __LINE__, "pthread_create");//输出自定义错误信息

	pthread_join(pid,nullptr/*退出状态*/);//阻塞等待可连接线程结束并回收线程
}


/**------------------------ POSIX 线程 -----------------------------**/

#define  retval_pthread  1     // 可连接 子线程退出码

static void func_pth_signal(int signum) //信号处理函数
{
	cout << "信号处理函数 " << __func__ << "() (标识符: "
		 << pthread_self() << ")信号 signum：" << signum << endl;

	/* 会触发 pthread_cleanup_push() pthread_cleanup_pop() 可连接线程清理处理程序
	 调用 线程清理函数 fun_clear() */
	pthread_exit(nullptr); // 当前线程退出，不影响进程
}

void *mypthread_seed_join_signal(void *arg) /*  (子)线程函数 */
{
	signal(SIGQUIT, func_pth_signal); // 注册信号处理函数 func_pth_signal()

	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	cout << "(子)线程函数 " << __func__ << "() (标识符: " << pid << ")" << endl;

	pthread_cleanup_push(fun_clear,arg); // 安装可连接线程清理处理程序

		int8_t num=10;
		while (--num)
		{
//			pthread_testcancel(); // 线程取消测试点
			cout << "num=" << (int)num <<endl;
			this_thread::sleep_for(chrono::milliseconds(1));//线程休眠 1ms
		}

	pthread_cleanup_pop(0); // 弹出可连接线程清理处理程序执行并卸载

#if retval_pthread  // 可连接 子线程退出码
	static int val_ret=987654321; // 自定义 退出码 用 static , 静态变量
	return (void *)(&val_ret);
#else
	return (void *)nullptr;
#endif
}

void mypthread_join_signal() // 线程函数(可连接)
{
	pthread_t pid; // 线程标识符
	int retval; // 返回值

	retval = pthread_create(&pid,
							nullptr /* 默认属性 */,
							mypthread_seed_join_signal,
							nullptr /* 无参数 */);//创建线程
	perr(retval, __LINE__, "pthread_create");//输出自定义错误信息

	this_thread::sleep_for(chrono::milliseconds(2));//线程休眠 10ms

	retval=pthread_kill(pid,0); // 判断子线程是否存在
	perr(retval, __LINE__, "pthread_kill");//输出自定义错误信息
	if (retval == 0) // 子线程 存在
	{
		pthread_kill(pid, SIGQUIT); // 发送 SIGQUIT 信号
		cout << "pthread_kill() 发送 SIGQUIT 信号" << endl;
	}
#if retval_pthread  // 可连接 子线程退出码
	void *val_exit=nullptr; // 子线程退出码
	retval=pthread_join(pid, &val_exit/*退出状态*/);//阻塞等待可连接线程结束并回收线程
	perr(retval, __LINE__,"pthread_join");//输出自定义错误信息
/*	static int val_ret=43210; // 自定义 退出码 用 static , 静态变量
	return (void *)(&val_ret); 	*/
	cout<< "子线程退出码 : " << * ((int *)val_exit) <<endl;
#else
	pthread_join(pid,nullptr/*退出状态*/);//阻塞等待可连接线程结束并回收线程
#endif
}


/**------------------------ POSIX 线程 -----------------------------**/

void mypthread_join_cancel() // 线程函数(可连接)
{
	pthread_t pid; // 线程标识符
	int retval; // 返回值

	retval = pthread_create(&pid,
							nullptr /* 默认属性 */,
							mypthread_seed_join_signal,
							nullptr /* 无参数 */);//创建线程
	perr(retval, __LINE__, "pthread_create");//输出自定义错误信息

	this_thread::sleep_for(chrono::milliseconds(2));//线程休眠 10ms

	retval=pthread_kill(pid,0); // 判断子线程是否存在
	perr(retval, __LINE__, "pthread_kill");//输出自定义错误信息
	if (retval == 0) // 子线程 存在
	{
		cout << "pthread_cancel() 取消线程" << endl;
		pthread_cancel(pid); // 取消线程
	}

	pthread_join(pid,nullptr/*退出状态*/);//阻塞等待可连接线程结束并回收线程
}


/**------------------------ POSIX 线程 -----------------------------**/

void mypthread_state(const pthread_t &pid) // 获取线程属性
{
	int retval; // 返回值
	int pth_state; // 线程 可分离状态
	pthread_attr_t attr_pth; // 线程 属性值

	/* 获取 线程 属性值 */
	retval=pthread_getattr_np(pid,&attr_pth); // 获取线程属性值
	perr(retval, __LINE__, "pthread_getattr_np");//输出自定义错误信息

	retval=pthread_attr_getdetachstate(&attr_pth,&pth_state);//从属性值获取线程可分离状态
	perr(retval, __LINE__, "pthread_attr_getdetachstate");//输出自定义错误信息
	if (retval == 0) {
		cout << "线程可分离状态: "
			 << ((pth_state == PTHREAD_CREATE_JOINABLE) ?
				 "PTHREAD_CREATE_JOINABLE" : "PTHREAD_CREATE_DETACHED")
			 << endl;
	}

	size_t stack_size; // 栈大小
	retval=pthread_attr_getstacksize(&attr_pth,&stack_size); // 获得 当前线程 栈大小
	perr(retval, __LINE__, "pthread_attr_getstacksize");//输出自定义错误信息
	cout << "栈大小 stack size: " << stack_size << " byte"<<endl;

	/* 销毁 线程 属性值 */
	retval=pthread_attr_destroy(&attr_pth); // 销毁线程属性值
	perr(retval, __LINE__, "pthread_attr_destroy");//输出自定义错误信息
}

void *mythread_seed_join_detach(void *arg) /*  (子)线程函数 */
{
	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	cout << "(子)线程函数 " << __func__ << "() (标识符: " << pid << ")" << endl;

	pthread_cleanup_push(fun_clear,arg); // 安装可连接线程清理处理程序

		mypthread_state(pid); // 获取线程属性

		cout << "(子)线程函数 设置 可分离线程" << endl;
		pthread_detach(pid); // 设置可分离线程

		mypthread_state(pid); // 获取线程属性

		/* 会触发 pthread_cleanup_push() pthread_cleanup_pop() 可连接线程清理处理程序
		 调用 线程清理函数 fun_clear() */
		pthread_exit(nullptr); // 当前线程退出，不影响进程

	pthread_cleanup_pop(0); // 弹出可连接线程清理处理程序执行并卸载

	return (void *)nullptr;
}

void mypthread_join_detach() // 线程函数(可连接转为可分离)
{
	pthread_t pid; // 线程标识符
	int retval; // 返回值

	retval = pthread_create(&pid,
							nullptr /* 默认属性 */,
							mythread_seed_join_detach,
							nullptr /* 无参数 */);//创建线程
	perr(retval, __LINE__, "pthread_create");//输出自定义错误信息

	pthread_join(pid,nullptr/*退出状态*/);//阻塞等待可连接线程结束并回收线程
}

/**------------------------ POSIX 线程 -----------------------------**/

void *mythread_seed_detach(void *arg) /*  (子)线程函数 */
{
	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	cout << "(子)线程函数 " << __func__ << "() (标识符: " << pid << ")" << endl;

	mypthread_state(pid); // 获取线程属性

	return (void *)nullptr;
}

void mypthread_detach() // 线程函数(可分离)
{
	pthread_t pid; // 线程标识符
	int retval; // 返回值
	pthread_attr_t attr_pth; // 线程 属性值

	/* 初始化 线程 默认属性值 */
	retval=pthread_attr_init(&attr_pth); // 默认属性初始化
	perr(retval, __LINE__, "pthread_attr_init");//输出自定义错误信息

	/* 设置线程属性为可分离状态 */
	retval=pthread_attr_setdetachstate(&attr_pth,PTHREAD_CREATE_DETACHED);
	perr(retval, __LINE__, "pthread_attr_setdetachstate");//输出自定义错误信息

	retval = pthread_create(&pid,
							&attr_pth /* 指定 属性 */,
							mythread_seed_detach,
							nullptr /* 无参数 */);//创建线程
	perr(retval, __LINE__, "pthread_create");//输出自定义错误信息

	/* 销毁 线程 属性值 */
	retval=pthread_attr_destroy(&attr_pth); // 销毁线程属性值
	perr(retval, __LINE__, "pthread_attr_destroy");//输出自定义错误信息
}


/************ POSIX 线程 操纵 函数 ***********/
/*
创建线程
int pthread_create (pthread_t *new_thread_pid, const pthread_attr_t *attr,
					void *(*start_routine) (void *), void *arg);
new_thread_pid：新 线程标识符
attr：用于设置新线程的属性，如果为 NULL 则表示使用默认线程属性
start_routine：新线程将运行的函数；线程函数
---------------------------------------------------------------------------
当前线程退出，不影响进程
void pthread_exit (void *retval);
---------------------------------------------------------------------------
阻塞等待可连接线程结束并回收线程
int pthread_join (pthread_t pid, void **return_value);
如果 return_value 不为 NULL，则存储 线程的退出状态。
---------------------------------------------------------------------------
尝试等待可连接线程结束，检查可连接线程是否已终止，回收线程
int pthread_tryjoin_np (pthread_t pid, void **return_value);
如果 return_value 不为 NULL，则存储 线程的退出状态。
---------------------------------------------------------------------------
超时等待可连接线程终止，回收线程
int pthread_timedjoin_np (pthread_t pid, void **return_value,
						  const struct timespec *abstime);
如果 return_value 不为 NULL，则存储 线程的退出状态。
---------------------------------------------------------------------------
时钟等待可连接线程终止，回收线程，直到根据 clockid 指定的时钟测得的 TIMEOUT 为止。
int pthread_clockjoin_np (pthread_t pid, void **return_value,
						  clockid_t clockid, const struct timespec *abstime);
如果 return_value 不为 NULL，则存储 线程的退出状态。
---------------------------------------------------------------------------
线程分离，设置可分离线程
int pthread_detach (pthread_t pid);
---------------------------------------------------------------------------
获取当前线程自身的标识符
pthread_t pthread_self (void);
---------------------------------------------------------------------------
比较两个线程的线程标识符。
int pthread_equal (pthread_t pidread1, pthread_t pidread2);
---------------------------------------------------------------------------
设置需要仅执行一次的函数
int pthread_once (pthread_once_t *once_control, void (*init_routine) (void));
参数 once_control：pthread_once_t 类型，是内部实现的互斥锁，保证在程序全局仅执行一次。
once_control 必须指向初始化为 PTHREAD_ONCE_INIT 的静态或变量。
参数 init_routine() 只会被调用一次 的 线程函数。
---------------------------------------------------------------------------
决定本线程是否允许被取消
int pthread_setcancelstate (int state, int *oldstate);
state：设置线程是否允许被取消，有两个可选值：
PTHREAD_CANCEL_ENABLE：允许线程被取消，它是线程被创建时默认的取消状态
PTHREAD_CANCEL_DISABEL：禁止线程被取消，这时如果收到取消线程请求，
	则将请求挂起，直到该线程允许被取消。
oldstate：用来记录线程原来的取消状态，如果不为 NULL，则返回旧状态。
---------------------------------------------------------------------------
设置线程的取消类型
int pthread_setcanceltype (int type, int *oldtype);
type：取消类型，用于设置线程如何被取消，有两个可选值：
PTHREAD_CANCEL_DEFERRED：默认类型允许目标线程推迟行动，
	直到它调用了几个所谓的取消点函数中的一个（ 包括 pthread_join、
	pthread_testcancel、pthread_cond_wait、pthread_cond_timewait、
	sem_wait 和 sigwait ）
PTHREAD_CANCEL_ASYNCHRONOUS：线程随时可以被取消，
	它将使得接收到取消请求的目标线程立即行动。
oldtype：用来记录线程原来的取消状态，如果不为 NULL，则返回旧状态。
---------------------------------------------------------------------------
取消线程，或者说异常终止一个线程
int pthread_cancel (pthread_t pid);
---------------------------------------------------------------------------
取消点测试，测试是否收到取消请求
void pthread_testcancel (void);
测试当前线程的取消，如果收到取消，则按照 pthread_exit（PTHREAD_CANCELED）终止线程。
---------------------------------------------------------------------------
获取或更改线程调用的的信号掩码。参数的含义与 sigprocmask() 的含义相同
int pthread_sigmask (int how, const sigset_t *newmask, sigset_t *oldmask);
---------------------------------------------------------------------------
将信号 signo 发送到给定线程
int pthread_kill (pthread_t threadid, int signo);
---------------------------------------------------------------------------
将信号和数据排队到线程中
int pthread_sigqueue(pthread_t threadid, int signo, const union sigval value); */


/************ POSIX 线程 属性 函数 ***********/
/*
初始化线程的默认属性
线程状态为 JOINABLE，调度策略为 SCHED_OTHER，无用户提供的堆栈
操作系统支持的线程的所有属性的默认值。
int pthread_attr_init (pthread_attr_t *attr);
---------------------------------------------------------------------------
销毁线程的属性，属性不用时要销毁
int pthread_attr_destroy (pthread_attr_t *attr);
---------------------------------------------------------------------------
获取分离状态属性
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
---------------------------------------------------------------------------
设置分离状态属性
int pthread_attr_setdetachstate (pthread_attr_t *attr, int detachstate);
---------------------------------------------------------------------------
获取为堆栈溢出保护而创建的保护区的大小
int pthread_attr_getguardsize (const pthread_attr_t *attr, size_t *guardsize);
---------------------------------------------------------------------------
设置为堆栈溢出保护而创建的保护区域的大小
int pthread_attr_setguardsize (pthread_attr_t *attr, size_t guardsize);
---------------------------------------------------------------------------
获取线程属性变量的 sched_param 属性，即调用的优先级
int pthread_attr_getschedparam (const pthread_attr_t *attr,
								struct sched_param *param);
---------------------------------------------------------------------------
设置线程属性变量的 sched_param 属性，即调用的优先级
int pthread_attr_setschedparam (pthread_attr_t *attr,
								const struct sched_param *param);
---------------------------------------------------------------------------
根据 policy 返回 attr 的调度策略
int pthread_attr_getschedpolicy (const pthread_attr_t *attr, int *policy);
---------------------------------------------------------------------------
根据 policy 设置 attr 的调度策略
int pthread_attr_setschedpolicy (pthread_attr_t *attr, int policy);
---------------------------------------------------------------------------
在 inherit 中返回 attr 的调度继承模式
int pthread_attr_getinheritsched (const pthread_attr_t *attr, int *inherit)；
---------------------------------------------------------------------------
在 inherit 中设置 attr 的调度继承模式
int pthread_attr_setinheritsched (pthread_attr_t *attr, int inherit);
---------------------------------------------------------------------------
在 scope 中返回 attr 的调度争用范围
int pthread_attr_getscope (const pthread_attr_t *attr, int *scope);
---------------------------------------------------------------------------
在 scope 中设置 attr 的调度争用范围
int pthread_attr_setscope (pthread_attr_t *attr, int scope);
---------------------------------------------------------------------------
返回堆栈的先前设置的起始地址
int pthread_attr_getstackaddr (const pthread_attr_t *attr, void **stackaddr);
---------------------------------------------------------------------------
设置要创建的线程堆栈的起始地址
根据堆栈是向上增长还是向下增长，该值必须高于或低于存储块中的所有地址。
块的最小的大小必须为 PTHREAD_STACK_MIN。
int pthread_attr_setstackaddr (pthread_attr_t *attr, void *stackaddr);
---------------------------------------------------------------------------
返回当前使用的最小堆栈大小
int pthread_attr_getstacksize (const pthread_attr_t *attr, size_t *stacksize);
---------------------------------------------------------------------------
设置当前使用的最小堆栈大小
添加有关启动线程所需的最小堆栈大小的信息
该大小不得小于 PTHREAD_STACK_MIN，并且也不得超过系统限制
int pthread_attr_setstacksize (pthread_attr_t *attr, size_t __stacksize);
---------------------------------------------------------------------------
返回堆栈的先前设置的地址
int pthread_attr_getstack (const pthread_attr_t *attr, void **stackaddr,
						   size_t *stacksize);
---------------------------------------------------------------------------
它们需要设置地址和大小，因为仅设置地址将使某些体系结构上的实现无法实现
int pthread_attr_setstack (pthread_attr_t *attr, void *stackaddr,
						   size_t stacksize);
---------------------------------------------------------------------------
获取在此过程中 pthread_create 使用的默认属性
int pthread_getattr_default_np (pthread_attr_t *attr);
---------------------------------------------------------------------------
设置在此过程中 pthread_create 使用的默认属性
int pthread_setattr_default_np (const pthread_attr_t *attr);
---------------------------------------------------------------------------
获取指定线程的属性。
int pthread_getattr_np (pthread_t pid, pthread_attr_t *attr);	 */


/************ POSIX 线程 调度控制功能 函数 ***********/
/*
根据 policy 和 param 设置 target_thread 的调度参数
int pthread_setschedparam (pthread_t target_thread, int policy,
						   const struct sched_param *param);
---------------------------------------------------------------------------
根据 policy 和 param 返回 target_thread 的调度参数
int pthread_getschedparam (pthread_t target_thread, int *policy,
						   struct sched_param *param);
---------------------------------------------------------------------------
设置 target_thread 的调度优先级
int pthread_setschedprio (pthread_t target_thread, int prio);
---------------------------------------------------------------------------
获取在内核及其接口中可见的线程名称
int pthread_getname_np (pthread_t target_thread, char *buf, size_t buflen);
---------------------------------------------------------------------------
设置线程名称在内核及其接口中可见
int pthread_setname_np (pthread_t target_thread, const char *name);
---------------------------------------------------------------------------
获取并发级别 即 线程数量
int pthread_getconcurrency (void);
---------------------------------------------------------------------------
设置并发级别 即 线程数量
int pthread_setconcurrency (int level);
---------------------------------------------------------------------------
让出 CPU 而（暂时）挂起
int pthread_yield (void);
---------------------------------------------------------------------------
将指定线程限制为仅在 cpuset 中表示的处理器上运行
int pthread_setaffinity_np (pthread_t pid, size_t cpusetsize,
 							const cpu_set_t *cpuset);
---------------------------------------------------------------------------
获取 cpuset 中代表的处理器可以运行的位
int pthread_getaffinity_np (pthread_t pid, size_t cpusetsize,
							cpu_set_t *cpuset); 						*/


/************ POSIX 线程 可连接线程(手动增加)线程清理处理程序 函数 ***********/
/*	pthread_cleanup_push 和 pthread_cleanup_pop 是宏，
	必须始终在 大括号的 相同嵌套级别中 匹配使用。
---------------------------------------------------------------------------
安装 可连接线程 清理处理程序
void pthread_cleanup_push (void (*routine)(void *), void *arg);
调用 pthread_cancel()取消线程 或 pthread_exit()退出线程 时，将使用参数 arg 调用 routine()。
---------------------------------------------------------------------------
弹出 可连接线程 清理处理程序执行并卸载
void pthread_clean_pop (int execute);
删除由匹配的 pthread_cleanup_push() 安装的清理处理程序。
当 pthread_cleanup_pop() 函数的参数 execute 为 0 时，
仅仅在 线程自己调用 pthread_exit()函数 或者 其它线程对本线程调用 pthread_cancel()函数 时，
才 在弹出“清理函数”的 同时执行 该“清理函数”。
---------------------------------------------------------------------------
像 pthread_cleanup_push() 一样安装清除处理程序，
但还会保存当前的取消类型并将其设置为延迟取消。
pthread_cleanup_push_defer_np ( routine, arg );
---------------------------------------------------------------------------
像 pthread_cleanup_pop() 一样删除清除处理程序，
但是还可以恢复调用匹配的 pthread_cleanup_push_defer() 时生效的取消类型。
pthread_cleanup_pop_restore_np(execute);		 */


/*


 */


//perr(retval, __LINE__, "pthread_getattr_np");// 输出自定义错误信息
void perr (const int &retval, const int &code_line, string_view ptr)//输出自定义错误信息
{
//	code_line--;
	if (retval != 0) {
		cerr << "Error:";
		if (!ptr.empty()) {
			cerr << "theme[" << ptr.data() <<"];";
		}
		if (code_line != 0) {
			cerr << "line[" << code_line << "];";
		}
		cerr << "value[" << retval << "]";
		if (retval > 0) {
			if (retval != errno){
				cerr << "{";
				switch (retval) {
					case ESRCH:
						cerr << "No such process";
						break;
					case EINVAL:
						cerr << "Invalid argument"; // 信号无效
						break;
					default:
						break;
				}
				cerr << "};";
			}
		}
		else cerr << ";" << endl;
		cerr << "errno[" << errno << "]";
		if (errno != 0){
			cerr << ":{" << strerror(errno) << "};";//返回描述 errno 代码含义的字符串
		}
		else cerr << ";" << endl;
	}
//	else if(retval > 0)
//	{
//
//	}
//	else
//	{
//
//	}
}

#define data_mode 0
void fun_clear(void *arg) // 线程清理函数
{
	cout << "线程清理函数 "<< __func__ << "()" << endl;
#if data_mode==0 	// 无 参数

#elif data_mode==1 	// 整型 参数
	//	int *p=(long *)(arg);
//	cout << "整型 参数: " << *p << endl;
	cout << "线程清理函数即将退出: " << *((long *)arg) << endl; // 取消点
#elif data_mode==2 	// 字符串 参数
	char *p=(char *)(arg);
	cout << "字符串 参数: " << *p << endl;
#elif data_mode==3 	// 结构体 参数
	auto *mystruct=(struct struct_data *)arg;
	cout << "结构体 参数\tnum: " << mystruct->num << "\tstr:" << mystruct->str << endl;
#elif data_mode==4 	// 共享进程数据
	++count; // 全局变量 -> 共享 进程数据，将会在 主线程 和 子线程 中用到
#endif
//	printf("fun_clear "); // 取消点
}


