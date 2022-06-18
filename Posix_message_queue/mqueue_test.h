//
// Created by zcq on 2021/5/13.
//

#ifndef __MQUEUE_TEST_H
#define __MQUEUE_TEST_H

/** set( CMAKE_C_CXX_FLAGS_behind "-lrt" )   # 修改 后面 编译选项 ！ **/

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <ctime>
#include <unistd.h>

#include <mqueue.h>

#include "Posix_header.h"
#include "wrap_mqueue.h"

/******************************************************************************/

void test_mq_create(const int &argc, char **argv); // 只创建 消息队列
mqd_t mq_create(const mode_t &mode); // 只创建 消息队列

/******************************** 简单应答 *************************************/
void test_mq_client(); // 独立 客户端 消息队列
void test_mq_service(); // 独立 服务器 消息队列

/******************************************************************************/
void test_send_mq_notify(); // 周期 发送 消息队列

/********************** 通知 + 非阻塞 mq_receive 效率低 **************************/
void test_mq_notify_signal(); // 独立 消息队列 通知

/*************************** 通知 + sigwait 效率较高 ****************************/
void test_mq_notify_sigwait(); // 独立 消息队列 通知

/***************************** 通知 + select 效率高 *****************************/
void test_mq_notify_pipe_select(); // 独立 消息队列 通知

/******************************* 通知 + 启动线程 ********************************/
void test_mq_notify_thread(); // 独立 消息队列 通知

/******************************************************************************/

#endif //__MQUEUE_TEST_H

/** linux 命令：$ ipcs #显示所有 消息队列/共享内存段/信号量数组
 ipcs -q  显示所有 消息队列
 ipcs -m  显示所有 共享内存段
 ipcs -s  显示所有 信号量数组
 ipcrm -h 查看删除选项    **/

/* 	Linux进程间通信之POSIX消息队列  #include <mqueue.h>
	消息队列可认为是一个消息链表，它允许进程之间以消息的形式交换数据。
 有足够写权限的进程或线程可往队列中放置消息，有足够读权限的进程或线程可从队列中取走消息。
 每个消息都是一个记录，它由发送者赋予一个优先级。
 与管道不同，管道是字节流模型，没有消息边界。
 POSIX 消息队列 与 System V 消息队列的 相似之处 在于数据的交换单位是整个消息，
 但它们之间仍然存在一些显著的差异：
 1.POSIX消息队列是引用计数的。
 	只有当所有当前使用队列的进程都关闭了队列之后才会对队列进行标记以便删除。
 2.每个System V消息都有一个整数类型，并且通过msgrcv()可以以各种方式类选择消息。
 与之形成鲜明对比的是，POSIX消息有一个关联的优先级，并且消息之间是严格按照优先级顺序排队的(以及接收)。
 3.POSIX消息队列提供了一个特性允许在队列中的一条消息可用时异步地通知进程。
--------------------------------------------------------------------------------
 消息队列 和 管道、FIFO有很大的区别，主要有以下两点：
 1.一个进程向消息队列写入消息之前，并不需要某个进程在该队列上等待该消息的到达，而管道和FIFO是相反的，
 进程向其中写消息时，管道和FIFO必需已经打开来读(否则open阻塞)，那么内核会产生 SIGPIPE信号。
 2.IPC的持续性不同。
 	管道和FIFO是随进程的持续性，当管道和FIFO最后一次关闭发生时，仍在管道和FIFO中的数据会被丢弃。
 	消息队列是随内核的持续性，即一个进程向消息队列写入消息后，然后终止，
 另外一个进程可以在以后某个时刻打开该队列读取消息。只要内核没有重新自举，消息队列没有被删除。
--------------------------------------------------------------------------------
消息队列中的每条消息通常具有以下属性：
 一个表示优先级的整数；消息的数据部分的长度(可为0)；消息数据本身(若大于0)；
--------------------------------------------------------------------------------
 消息队列可以在进程之间进行共享数据，Posix消息队列有几个主要的特性：
 1.消息队列的每个消息都有自己的优先级，而且每次从中获取消息，总是得到优先级最高的一个
 2.消息队列通过msg_open进行创建，而且每个消息队列通过/somename的方式进行唯一标识。
 3.进程通过mq_close关闭本进程的队列的引用，通过mq_unlink删除内核中的消息队列
 4.使用fork()函数后，子进程复制父进程的文件描述符，那么共享同一个消息队列，而且也共享mq_flags
 5.一个消息队列描述符可以通过select、poll和epoll进行监听
 6.与Posix相对的还有一个更古老的System V消息队列。Posix消息队列接口更好用，但是相对的，
 System V的接口更灵活。根据需要合理选取即可。如果没有特殊需求，使用Posix即可
--------------------------------------------------------------------------------
mq_open()函数创建一个新消息队列或打开一个既有队列，返回后续调用中会用到的消息队列描述符。
mq_setattr()设置消息队列的属性
mq_getattr()获取消息队列的属性
mq_close()函数关闭进程之前打开的一个消息队列。
mq_unlink()函数删除一个消息队列名并当所有进程关闭该队列时对队列进行标记以便删除。
mq_send()函数向队列写入一条消息。
mq_timedsend()
mq_recevie()函数从队列中读取一条消息。
mq_timedreceive()
mq_notify()注册消息异步通知
--------------------------------------------------------------------------------
 // 创建或打开消息队列，成功时返回消息队列的描述符 mqd_t mqdes
 mqd_t mq_open (const char *name, int oflag, ...)
 // mqd_t mq_open(const char *name, int oflag);
 // mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
	 name	标识出了消息队列，消息队列对象的名字的最大长度为 NAME_MAX(255)个字符。
	 oflag	位掩码，消息队列的权限（同 open() ）：
			O_CREAT		队列不存在时创建队列
			O_EXCL		与 O_CREAT 一起使用，若消息队列已存在，则错误返回
			O_RDONLY	只读打开
			O_WRONLY	只写打开
			O_RDWR		读写打开
			O_NONBLOCK	以非阻塞模式打开
		如果使用了 O_CREAT | O_EXCL，若消息队列已经存在，则 errno 置为 EEXIST
	 mode 	消息队列的模式
	 attr 	设置的消息队列的属性，是一个结构体，结构如下：
			struct mq_attr {
				long mq_flags;	//阻塞标志，只有2个值 0 或 O_NONBLOCK
				long mq_maxmsg;	//最大消息数：消息队列中最多能容纳的消息个数
				long mq_msgsize;//每个消息的最大字节数
				long mq_curmsgs;//当前消息队列中正在排队的消息个数
				long pad[4];
			};
	    注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。
--------------------------------------------------------------------------------
 mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性。
 mq_setattr() 只设置 mq_flags，用来 设置或清除 消息队列的 非阻塞标志，忽略另外3个属性。
--------------------------------------------------------------------------------
 // 设置消息队列的属性
 int mq_setattr (mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);
 // 获取消息队列的属性
 int mq_getattr (mqd_t mqdes, struct mq_attr *mqstat);
--------------------------------------------------------------------------------
 // 关闭消息队列
 int mq_close(mqd_t mqdes);  // 类似 close
 // 删除消息队列引用
 int mq_unlink(const char *name);
--------------------------------------------------------------------------------
 // 向消息队列写入一条消息
 int mq_send ( 	mqd_t mqdes, const char *msg_ptr,
            	size_t msg_len, unsigned int msg_prio );
 int mq_timedsend (	mqd_t mqdes, const char *msg_ptr,
 					size_t msg_len, unsigned int msg_prio,
                 	const struct timespec *abs_timeout );
	mqdes：		消息队列的描述符
    msg_ptr：	需要发送的消息
    msg_len：	发送消息的长度
    msg_prio：	发送消息的优先级，不使用时设为 0
    abs_timeout：超时参数
 成功返回0，失败返回-1
-------------------------------
 mq_send()函数向消息队列发送消息，如果消息队列满了，那么该函数阻塞；
 如果队列满了，且消息队列设置为 O_NONBLOCK 模式，那么函数返回 -1，同时 errno 设置为 EAGAIN。
 mq_timedsend() 基本同上，唯一区别在于如果阻塞情况下超时，则立刻返回 -1。
--------------------------------------------------------------------------------
 // 从消息队列中读取一条消息
 ssize_t mq_receive ( 	mqd_t mqdes, char *msg_ptr,
                   		size_t msg_len, unsigned int *msg_prio );
 ssize_t mq_timedreceive ( 	mqd_t mqdes, char *msg_ptr,
                        	size_t msg_len, unsigned int *msg_prio,
                         	const struct timespec *abs_timeout );
    mqdes：		消息队列描述符
    msg_ptr：	指向放置消息的缓冲区
    msg_len：	放置消息缓冲区的长度
    msg_prio：	返回消息的优先级，不使用时设为 nullptr
    abs_timeout：超时参数
 成功返回消息长度，失败返回-1
-------------------------------
 mq_receive函数接受消息队列消息，如果队列是阻塞模式，而且队列空，那么该函数阻塞；
 如果队列空，但是设置为 O_NONBLOCK 模式，则返回 -1，同时 errno 设置为 EAGAIN。
 ma_timedreceive() 唯一的区别在于，如果队列空且超时，则立刻返回 -1。
--------------------------------------------------------------------------------
 // 注册消息异步通知
 int mq_notify ( mqd_t mqdes, const struct sigevent *notification );
	mqdes：		消息队列的标识符
    notification：参数的意义参照 sigevent 结构体：
-------------------------------
 作用：如果有新消息到达一个空的队列，允许调用线程注册一个异步通知，该通知会在消息到达时触发。
 核心的思想在异步这两个字上，也就是说，我们希望从消息队列中获取消息，
 但是不想让当前进程阻塞在一个空的消息队列上，那么通过该函数可以注册异步通知，来处理将来到达的事件。
-------------------------------
 几个注意事项：
    1.一个消息队列只能注册在一个进程。
    2.如果当前进程想要移除注册的事件，只需要把 notification 设置为 NULL注册 mq_notify()即可。
    3.如果进程注册已经存在的注册事件，那么其他注册该事件的进程不会收到信号（见 1）。
    4.只有空消息队列在接收到第一个消息才会触发通知。
-------------------------------
 要明确 sigevent{} 的作用：
 	sigevent{}的作用是为Linux/Unix系统调用函数API提供一个调用接口的通用参数；
	这类API的作用以某种特定的方式来通知进程到达的事件。
 	API根据sigevent提供的结构，来具体处理有关信息。
-------------------------------
union sigval {
  int sival_int;
  void *sival_ptr; 	// 指定 信号处理(线程)函数。
};
typedef union sigval __sigval_t;
-------------------------------
typedef struct sigevent
{
	__sigval_t sigev_value;
	int sigev_signo; 	// 通知信号
	int sigev_notify; 	// 通知方式
	union {
		int _pad[__SIGEV_PAD_SIZE];
		__pid_t _tid; 	// 线程 ID
		struct {
			void (*_function) (__sigval_t);	// 指定 信号处理(线程)函数。
			pthread_attr_t *_attribute;		// 线程属性。
		} _sigev_thread;
	} _sigev_un;
} sigevent_t;
#define sigev_notify_function   _sigev_un._sigev_thread._function // 信号处理(线程)函数。
#define sigev_notify_attributes _sigev_un._sigev_thread._attribute // 线程属性
-------------------------------
 sigev_notify 通知方式 参数取值和作用如下：
    SIGEV_NONE：	空的提醒，事件发生时不做任何事情
    SIGEV_SIGNAL：	向进程发送 sigev_signo 中指定的信号，这涉及到 sigaction() 的使用
    SIGEV_THREAD：	通知进程在一个新的线程中启动 sigev_notify_function 函数，
    	函数的实参是 sigev_value，系统API自动启动一个线程，我们不用显式启动。
-------------------------------------------------------------------------------- */
