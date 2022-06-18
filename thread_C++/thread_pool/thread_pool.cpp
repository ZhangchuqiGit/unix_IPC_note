/*如果使用常规的处理方法，即.h中是类的声明，.cpp中是类的实现，然后在main.cpp中#include .h文件会报“undefined reference to”的错误。有三种解决办法：

    解决方法一，在main.cpp中#include “xxxx.cpp”，而非"xxxx.h"，这种方式就如同将队列类的声明实现放在同一文件中。
    解决方法二，在xxxx.h中，代码部分结尾处#include “xxxx.cpp”，并且去掉xxxx.cpp里的包含语句，这与上述方式如出一辄，只是在main.cpp中看上去就像习惯中的方案一样。
    解决方法三，声明类时，在类名前加上export关键字，抱歉如今的c++编译器中这关键字多半行不通。就像foreach之类的关键字一样，多半会被认为是关键字，但是却是编译器不认识的。*/

#ifndef __THREAD_POOL_CPP
#define __THREAD_POOL_CPP

#include "thread_pool.h" // 自定义 线程池

//#define detach_join      // 转换 当前线程 为 可分离线程

/* 静态成员 初始化 */
vector<ThreadPool_Task*> ThreadPool_Control::tasklist_Control;//任务列表
bool ThreadPool_Control::exit_Thread_Control = false;//线程退出标志
pthread_mutex_t ThreadPool_Control::lock_Mutex_Control = PTHREAD_MUTEX_INITIALIZER;//线程同步 互斥锁
pthread_cond_t ThreadPool_Control::val_Cond_Control = PTHREAD_COND_INITIALIZER;//线程同步 条件变量
int ThreadPool_Control::num_func= 0;

//设置任务数据
void ThreadPool_Task::setData_Task(void *data)
{
    this->data_Task = data;
}

//得到任务数据
void *ThreadPool_Task::getData_Task()
{
    return  this->data_Task;
}

ThreadPool_Task::ThreadPool_Task(string &taskname)
{
    printf("任务的名称 : %s\n", taskname.c_str() );
    this->name_Task=taskname;//任务的名称
}

//创建 线程池中的 线程
int ThreadPool_Control::create_Thread_Control()
{
    printf("I will create %d threads.\n", this->num_ThreadRun_Control);
    this->id_Thread_Control = new pthread_t[this->num_ThreadRun_Control];
    for (int i = 0; i < this->num_ThreadRun_Control ; i++)
    {
        pthread_create(&(
                this->id_Thread_Control[i]), nullptr,
                       funcThread_Control, nullptr);
#ifdef detach_join      // 转换 当前线程 为 可分离线程
        pthread_detach(this->id_Thread_Control[i]); // 转换 当前线程 为 可分离线程
#endif
    }
    return 0;
}

//线程函数
void *ThreadPool_Control::funcThread_Control(void *arg)
{
    int num=getfuncNum();
    pthread_t id_Thread = pthread_self(); // 获取当前线程 ID 值
    while (true) // 线程函数 一直运行
    {
        pthread_mutex_lock(&lock_Mutex_Control); // 上锁

        /* 如果任务队列为空，等待新任务进入任务队列 */
        while (tasklist_Control.empty() && !exit_Thread_Control) {
            printf("[%d Thread id: %lu]\tidle\n",num,id_Thread);
            /* 线程同步 条件变量 */
            pthread_cond_wait(
                    &val_Cond_Control, &lock_Mutex_Control);
        }
        if (exit_Thread_Control)        //关闭线程
        {
            printf("[%d Thread id: %lu]\texit\n",num,id_Thread);
            pthread_mutex_unlock(&lock_Mutex_Control); // 解锁
            pthread_exit(nullptr); // 当前线程退出，不影响进程！
        }
#if 0
        if ( !tasklist_Control.empty() ) // 如果队列不为空
            {
#endif
        /* 取出一个任务并处理之 */
        auto iter = tasklist_Control.begin();
        ThreadPool_Task *task = *iter;
        if (iter != tasklist_Control.end()) // 如果队列不为空
        {
            task = *iter;                   // 取出一个任务
            tasklist_Control.erase(iter);   // 删除一个任务
        }
        printf("[%d Thread id: %lu]\trun: %s\n",num,
               id_Thread, ((string *) task->getData_Task())->c_str());
#if 0
        }
#endif
        pthread_mutex_unlock(&lock_Mutex_Control); // 解锁
    }
//        task->Run_Task();    //执行任务
    return (void*)nullptr;
}

//往 任务队列里 添加任务 并发出 线程 同步信号
int ThreadPool_Control::addTask_Control(ThreadPool_Task *task)
{
    pthread_mutex_lock(&lock_Mutex_Control); // 上锁
    tasklist_Control.push_back(task);        // 添加一个任务
    pthread_mutex_unlock(&lock_Mutex_Control); // 解锁

    pthread_cond_signal(&val_Cond_Control); // 发出 线程 同步信号
    return 0;
}

//停止 所有 线程
int ThreadPool_Control::stopAllThread_Control()
{
    //避免重复调用
    if (exit_Thread_Control) // 线程退出标志
        return -1;
    printf("\nNow I will end all threads!\n\n");

    //唤醒 所有等待 线程，线程池 也要 销毁了
    exit_Thread_Control = true;                 // 唤醒 所有等待 线程
    pthread_cond_broadcast(&val_Cond_Control);  // 发出 线程 同步信号

#ifndef detach_join      // 转换 当前线程 为 可分离线程
    for (int i = 0; i < num_ThreadRun_Control; i++)    //清楚僵尸
        pthread_join(id_Thread_Control[i], nullptr);
#endif

    delete[] id_Thread_Control;
    id_Thread_Control = nullptr;

    //销毁 互斥量 和 条件变量
    pthread_mutex_destroy(&lock_Mutex_Control); // 销毁 互斥量
    pthread_cond_destroy(&val_Cond_Control); // 销毁 条件变量

    return 0;
}

//获取当前队列中的任务数
int ThreadPool_Control::getTaskSize_Control() {
    return tasklist_Control.size();
}

int ThreadPool_Control::getfuncNum() {
    num_func++;
    return num_func;
}


#endif
