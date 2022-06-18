/*如果使用常规的处理方法，即.h中是类的声明，.cpp中是类的实现，然后在main.cpp中#include .h文件会报“undefined reference to”的错误。有三种解决办法：

    解决方法一，在main.cpp中#include “xxxx.cpp”，而非"xxxx.h"，这种方式就如同将队列类的声明实现放在同一文件中。
    解决方法二，在xxxx.h中，代码部分 结尾处 #include “xxxx.cpp”，并且去掉xxxx.cpp里的包含语句，这与上述方式如出一辄，只是在main.cpp中看上去就像习惯中的方案一样。
    解决方法三，声明类时，在类名前加上 export 关键字，抱歉如今的c++编译器中这关键字多半行不通。就像foreach之类的关键字一样，多半会被认为是关键字，但是却是编译器不认识的。*/

#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

#include <atomic> // C++ 多线程
#include <thread> // C++ 多线程 含 #include <pthread.h>
#include <mutex> // C++ 多线程
#include <condition_variable> // C++ 多线程
#include <future> // C++ 多线程
#include <unistd.h> // sleep

using namespace std;

/*执行任务的类：设置任务数据并执行*/
class ThreadPool_Task
{
protected:
	string name_Task;   	//任务的名称
	void *data_Task=nullptr;		//要执行的任务的具体数据

public:
	ThreadPool_Task() = default;
	~ThreadPool_Task() = default;
	explicit ThreadPool_Task(string &taskname);
    void setData_Task(void *data);   //设置任务数据
    void *getData_Task();   //得到任务数据
    string *getname_Task() { return &(this->name_Task) ; }
};


/*线程池管理类*/
class ThreadPool_Control
{
private:
	static vector<ThreadPool_Task*> tasklist_Control;    //任务列表
	static bool exit_Thread_Control;   		//线程退出标志
	pthread_t *id_Thread_Control=nullptr;			//线程 ID
	static pthread_mutex_t lock_Mutex_Control; //线程同步 互斥锁
	static pthread_cond_t val_Cond_Control;//线程同步 条件变量
    int num_ThreadRun_Control=0;	//线程池中启动的线程数
    static int num_func; //线程池  线程函数 编号

    static int getfuncNum();

protected:
	static void *funcThread_Control(void *arg_ptr);//线程函数
    static int moveTOidle_Control(pthread_t id_Thread) //线程执行结束后，把自己放入空闲线程中
    { return 0; }
    static int moveTObusy_Control(pthread_t id_Thread) //移入到忙碌线程中去
    { return 0;	}

public:
	ThreadPool_Control() = default;
	~ThreadPool_Control() = default;
    explicit ThreadPool_Control(int threadNum)
    {
		this->num_ThreadRun_Control=threadNum;//线程池中启动的线程数
    }
	static int addTask_Control(ThreadPool_Task *task);//把任务添加到任务队列中
	int stopAllThread_Control();	//使线程池中的所有线程退出
	static int getTaskSize_Control();  	//获取当前任务队列中的任务数
    int create_Thread_Control();//创建线程池中的线程
};


/*线程池类*/
//class ThreadPool : public ThreadPool_Task, public ThreadPool_Control
//{
//public:
//	ThreadPool() = default;
//	explicit ThreadPool(string &taskname, int threadNum)
//	{
//		this->name_Task=taskname;//任务的名称
//		this->num_ThreadRun_Control=threadNum;//线程池中启动的线程数
//		this->data_Task=nullptr;//要执行的任务的具体数据
//	}
//	~ThreadPool() = default;
//};


#endif

/* 解决方法二，在xxxx.h中，代码部分 结尾处 #include “xxxx.cpp”，并且去掉xxxx.cpp里的包含语句，这与上述方式如出一辄，只是在main.cpp中看上去就像习惯中的方案一样。 */

#include "thread_pool.cpp" // 自定义 线程池

