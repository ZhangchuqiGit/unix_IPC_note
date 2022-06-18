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

#include "thread_pool.h" // 自定义 线程池
#include "thread_pool.cpp" // 自定义 线程池

using namespace std;

int main()
{
    pthread_t val_id=pthread_self();
    cout<<"BEGIN : This is "<<__func__<<" ID : "<<val_id<<endl;
    cout<<"========================================="<<endl;

    string zcq="zcq";
    ThreadPool_Task mytask_ThreadPool(zcq);
    int max=7;
    string valdata[max];
    valdata[0]="123";
    valdata[1]="456";
    valdata[2]="789";
    valdata[3]="+++";
    valdata[4]="---";
    valdata[5]="***";
    valdata[6]="///";

    ThreadPool_Control mycont_ThreadPool(5);
    mycont_ThreadPool.create_Thread_Control();
    for (int i = 0; i < max; ++i)
    {
        mytask_ThreadPool.setData_Task( &valdata[i] );
        mycont_ThreadPool.addTask_Control(&mytask_ThreadPool);
        printf("----------- %d\n",i);
    }

    while (true)
    {
        printf("There are still %d tasks need to handle\n",mycont_ThreadPool.getTaskSize_Control());
        /* 任务队列 已没有任务 */
        if (mycont_ThreadPool.getTaskSize_Control() == 0)
        {
            /* 清除 线程池 */
            if (mycont_ThreadPool.stopAllThread_Control() == -1)
            {
                printf("Thread pool clear, exit\n");
                break;
            }
        }
        this_thread::sleep_for(std::chrono::milliseconds(1500)); // 线程休眠 1s
    }

    cout<<"========================================="<<endl;
    cout<<"END : This is "<<__func__<<" ID : "<<val_id<<endl;
    return 0;
}

/*如果使用常规的处理方法，即.h中是类的声明，.cpp中是类的实现，然后在main.cpp中#include .h文件会报“undefined reference to”的错误。有三种解决办法：

    解决方法一，在main.cpp中#include “xxxx.cpp”，而非"xxxx.h"，这种方式就如同将队列类的声明实现放在同一文件中。
    解决方法二，在xxxx.h中，代码部分结尾处#include “xxxx.cpp”，并且去掉xxxx.cpp里的包含语句，这与上述方式如出一辄，只是在main.cpp中看上去就像习惯中的方案一样。
    解决方法三，声明类时，在类名前加上export关键字，抱歉如今的c++编译器中这关键字多半行不通。就像foreach之类的关键字一样，多半会被认为是关键字，但是却是编译器不认识的。*/