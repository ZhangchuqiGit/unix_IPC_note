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

/**************** c++ 线程 ***************/
#include <thread> // C++ 多线程
#include <atomic> // C++ 多线程
#include <mutex> // C++ 多线程
#include <condition_variable> // C++ 多线程
#include <future> // C++ 多线程

/**************** c++ 线程 可和 POSIX 线程 联合使用 *********************/
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

/***********************************************/

void func_getnowtime(); // 获取并打印当前时间

void mythread_join(); // 可连接线程
void thread_join_move(); // 可连接线程
void thread_join_to_detach(); // 线程函数 可连接 转为 可分离


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
	mythread_join(); // 可连接线程
	thread_join_move(); // 可连接线程
	thread_join_to_detach(); // 线程函数 可连接 转为 可分离

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


/*************** c++ 线程 ***************/

/* 线程函数 结构体 参数 */
struct struct_thread // 结构体 参数
{
	int num;
	string str;
};
void thread_none() /*  (子)线程函数 无参数 */
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "可分离(子)线程 " << __func__ << " (标识符: "
		 << id_thread << " )无参数" << endl;

	func_getnowtime(); // 获取并打印当前时间

	using std::chrono::system_clock; // using namespace chrono
	time_t time=system_clock::to_time_t(system_clock::now());
	struct std::tm *ptm=std::localtime(&time);
	ptm->tm_sec=ptm->tm_sec + 5; // 增加 5 s

	/* 线程 sleep_until 休眠到 指定时间 */
	this_thread::sleep_until(system_clock::from_time_t(mktime(ptm)));

	func_getnowtime(); // 获取并打印当前时间
}
void thread_one(int num) /*  (子)线程函数 一个参数 */
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "(子)线程 " << __func__ << " (标识符: " << id_thread << " )的一个参数: ";

	cout << num << endl;
}
void thread_multiParameter(struct_thread *mystruct, int num, string_view str,
						   const char *ctr) /*  (子)线程函数 多参数 */
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "(子)线程 " << __func__ << " (标识符: "
		 << id_thread << " ) 的多参数: " << endl;

	cout << "结构体参数:\t" << mystruct->num << "\t" << mystruct->str << endl;
	cout << "整型参数:\t" << num << endl;
	cout << "string参数:\t" << str << endl;
	cout << "数组参数:\t" << ctr << endl;
}

void mythread_join() // 可连接线程
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "线程 " << __func__ << " 标识符: " << id_thread << endl;

	/* 线程创建 形式 01 : 先定义再初始化 和 遍历数组 */
	thread ary_thread[5]; 								// 01: 先 定义
	for (int i = 0; i < 5; ++i) {
		ary_thread[i]= thread(thread_one /* 线程函数 */,
							  i /* 线程函数 参数 */); 	// 02: 再 初始化
	}
	for (auto &tmp : ary_thread)tmp.join(); // 遍历数组，等待子线程

	/* 线程创建 形式 02 : 定义并初始化 */
	struct_thread mystruct{};
	mystruct.num=1234;
	mystruct.str="mystruct";
	string str="string";
	char ctr[]="char";
	thread val_thread(thread_multiParameter /* 线程函数 */, /* 线程函数 多参数: */
					  &mystruct, 6666, str, ctr); 	// 定义并初始化
	val_thread.join(); // 等待子线程
}


/*************** c++ 线程 ***************/

void thread_one_move(int &num) /*  (子)线程函数 一个参数 */
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "(子)线程 " << __func__ << " (标识符: " << id_thread << " )" << endl;

	++num;
	this_thread::sleep_for(chrono::milliseconds(500)); //等待 500毫秒
}

void thread_join_move() // 可连接线程
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "线程 " << __func__ << " 标识符: " << id_thread << endl;

	int num = 0;
	cout << "num = " << num << endl;

	thread t1(thread_one_move, ref(num));   //ref(num)是取n的引用
//	thread t1(thread_one_move, num);  Error !!!

	thread t2(move(t1)); // move() 移动后，原来对象不存在 ！
	t2.join(); // 等待子线程

	cout << "move() 移动后 num = " << num << endl;
}


/*************** c++ 线程 ***************/

void thread_join_to_detach() // 线程函数 可连接 转为 可分离
{
	thread::id id_thread=this_thread::get_id(); // 获取当前线程 ID 值
	cout << "线程 " << __func__ << " 标识符: " << id_thread << endl;

	thread val_thread(/* 线程函数 */thread_none/* 无参数 */); // 定义并初始化
	val_thread.detach(); // 转换为 可分离线程
}


/******************************/

void func_getnowtime() // 获取并打印当前时间
{
//    struct timespec {
//        long int tv_sec;  /* seconds.  */
//        long int tv_nsec;  /* Nanoseconds.  */
//        ...  ...
//    }
	timespec time{};

//    /* ISO C `broken-down time' structure.  */
//    struct tm {
//        int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
//        int tm_min;			/* Minutes.	[0-59] */
//        int tm_hour;			/* Hours.	[0-23] */
//        int tm_mday;			/* Day.		[1-31] */
//        int tm_mon;			/* Month.	[0-11] */
//        int tm_year;			/* Year	- 1900.  */
//        ... ...
//    };
	struct std::tm nowtime{};

	clock_gettime(CLOCK_REALTIME, &time); // 获取相对于1970到现在的秒数

	/* localtime_r 用来获取 系统时间，运行于 linux 平台 多线程 */
	localtime_r(&time.tv_sec, &nowtime);

	printf("%04d-%02d-%02d %02d:%02d:%02d\n",
		   nowtime.tm_year + 1900,
		   nowtime.tm_mon+1,
		   nowtime.tm_mday,
		   nowtime.tm_hour,
		   nowtime.tm_min,
		   nowtime.tm_sec);
}


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


/***** std::async、std::future、std::promise 和 std::packaged_task ******/
/*
 */

/************ c++ 线程 异步接口 std::async ***********/
/*	异步接口 std::async ：可以很方便的 获取 线程函数 的 执行结果
std::async 会自动创建一个线程去调用 线程函数，它返回一个 std::future，
这个 future 中存储了 线程函数 的 返回结果，当我们需要线程函数的结果时，
直接从 future 中获取，非常方便。
---------------------------------------------------------------------------
std::async 提供的便利：
	首先解耦了线程的创建和执行，使得我们可以在需要的时候获取 异步操作 的结果；
	其次它还提供了线程的 创建策略（比如可以通过延迟加载的方式去创建线程），
使得我们可以以多种方式去创建线程。
---------------------------------------------------------------------------
建议用 std::async 代替 thread 线程的创建。 */

/************ c++ 线程 std::future 类 ***********/
/*	std::future 提供了一种访问 异步操作结果 的机制
从字面意思来理解，它表示未来；一个异步操作是不可能马上就被获取操作结果的，
只能在未来某个时候获取，但是我们可以以同步等待的方式来获取结果，
可以通过查询 future 的 三种状态（future_status）来获取异步操作的结果。
---------------------------------------------------------------------------
enum class future_status  // 状态码
{
	ready,		//异步操作已经完成
	timeout,	//异步操作超时
	deferred	//异步操作还没开始
};
---------------------------------------------------------------------------
// 查询future的状态
std::future_status status;
future<int> future;
int retvalue = future.get();//等待异步操作结束并返回结果
future.wait();//只是等待异步操作完成，无返回结果
status = future.wait_for(std::chrono::seconds(1));//超时等待返回结果
do {
	status = future.wait_for(std::chrono::seconds(1));//超时等待返回结果
	if (status == std::future_status::deferred) {
    	std::cout << "deferred\n";
	} else if (status == std::future_status::timeout) {
    	std::cout << "timeout\n";
	} else if (status == std::future_status::ready) {
    	std::cout << "ready!\n";
	}
} while (status != std::future_status::ready);		 */

/************ c++ 线程 std::promise 类 ***********/
/*	std::promise 为获取线程函数中的某个值提供便利，保存了一个共享状态的值
在线程函数中给外面传进来的 promise 赋值，
当线程函数执行完成之后就可以通过 promise 获取该值了，
值得注意的是取值是间接的通过 promise 内部提供的 future 来获取的。
---------------------------------------------------------------------------
std::promise<int> mypromise;
std::thread mythread( [](std::promise<int>& p)
					  {	p.set_value_at_thread_exit(9); },
					  std::ref(mypromise) );
std::future<int> myfurture = mypromise.get_future();
auto retvalue = myfurture.get();				 */

/************ c++ 线程 std::packaged_task 类 ***********/
/*	std::packaged_task 包装了一个可调用的目标（如 function, lambda, bind）,
以便异步调用，它和 promise 在某种程度上有点像，
promise 保存了一个共享状态的值，而 packaged_task 保存的是一个函数
---------------------------------------------------------------------------
std::packaged_task<int()> task([](){ return 7;} );
std::thread mythread(std::ref(task));
std::future<int> myfuture = task.get_future();
auto retvalue = myfuture.get();					 */



