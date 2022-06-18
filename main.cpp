
/**
 互斥锁：是为上锁而优化；
 条件变量：是为等待而优化；
 信号量：既可上锁，也可等待，故开销大于前二者。
 **/

/**
 不同 进程间 通信 方式：
 	管道（血缘关系、匿名管道）；（使用简单）
 	FIFO（命名管道）；（使用简单）
 	本地套接字；（最稳定）
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
 	信号；（要求原子操作，开销小，易丢失）
 	（系统/用户）信号（处理函数）；（开销最小，易丢失）
 	共享内存区/映射区；（无需系统调用、最快，但必须同步；如大量文件的复制粘贴）
 	消息队列；（任何时候读/写）
-----------------------------------
 不同 进程间 同步 方式：
	互斥锁；条件变量；（共享全局/静态变量，开销小）
 	文件/记录锁；
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
-----------------------------------
 不同 线程间 同步 方式：
	互斥锁；条件变量；（共享全局/静态变量，开销小）
 	读写锁；（共享全局/静态变量，开销小）
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
**/

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

#include "zcq_header.h"

#include <iostream> // 标准流 <istream> <ostream>
using namespace std;

/*	bash shell 	------------------------
		$ g++ test.cpp -o test
		$ ./test myfile.txt youfile.txt one.txt
	c++ 		------------------------
		int main(int argc, char **argv)
----------------------------------------
  	c++    		bash shell
  	argc	=	${#argv[@]} # 参数个数
  	argv[0]	=	myfile.txt 	# 参数 : ./当前文件名
  	argv[1]	=	myfile.txt 	# 参数 1
  	argv[2]	=	youfile.txt # 参数 2
  	argv[3]	=	one.txt 	# 参数 3	  	*/
#if 1
int main(int argc, char **argv)
{
	int num = -1;
	while (true)
	{
		cout << "==============================\n"
			 << "0 test_process_pipe();\n\t// 建立父子进程间通信的管道：先 父写 子读，后 子写 父读\n"
			 << "1 test_popen_pclose();\n\t// 创建一个运行给定命令的管道(标准I/O流)，(等待)可以执行 shell 命令\n"
			 << "2 test_process_fifo()\n\t// 建立父子进程间通信的管道 FIFO（命名管道\n"
			 << "3 test_fifo_service() // 独立 服务器 FIFO（命名管道）\n"
			 << "4 test_fifo_client() // 独立 客户端 FIFO（命名管道）\n"
			 << "5 test_fifo_one_service()\n\t// 单个 独立 服务器 FIFO（命名管道）: 一对多\n"
			 << "6 test_fifo_multiple_client()\n\t// 多个 独立 客户端 FIFO（命名管道）: 多对一\n"
			 << "7 test_fifo_mesg_service()\n\t// 独立 服务器 FIFO（命名管道）自定义 可变长度 消息体\n"
			 << "8 test_fifo_mesg_client()\n\t// 独立 客户端 FIFO（命名管道）自定义 可变长度 消息体\n"
			 << "9 test_mq_create(const int &argc, char **argv) // 只创建 消息队列\n"
			 << "10 test_mq_service() // 独立 服务器 消息队列\n"
			 << "11 test_mq_client() // 独立 客户端 消息队列\n"
			 << "12 test_send_mq_notify() // 周期 发送 消息队列\n"
			 << "13 test_mq_notify_signal() // 独立 消息队列 通知\n"
			 << "14 test_mq_notify_sigwait() // 独立 消息队列 通知\n"
			 << "15 test_mq_notify_pipe_select() // 独立 消息队列 通知\n"
			 << "16 test_mq_notify_thread(); // 独立 消息队列 通知\n"
			 << "17 test01_signal_rt() // 测试 多参数信号函数 + 多(实时)信号处理\n"
			 << "18 test_msg_service(); // 独立 服务器 消息队列\n"
			 << "19 test_msg_client(); // 独立 客户端 消息队列\n"
			 << "20 test_msg_service_echo() // 独立 迭代服务器 消息队列\n"
			 << "21 test_msg_client_call() // 独立 客户端 消息队列\n"
			 << "22 test_msg_service_fork() // 独立 迸发服务器 消息队列\n"
			 << "23 test_msg_client_fcall() // 独立 客户端 消息队列\n"
			 << "24 test_file_flock_01()\n\t// 建议性锁：对 fd1 上锁，并不影响程序 dup() 复制为 fd2 上锁\n"
			 << "25 test_file_flock_02()\n\t// 建议性锁：子进程持有锁，并不影响父进程通过相同的fd获取锁，反之亦然\n"
			 << "26 test_file_lock_fcntl_01()\n\t// 如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁\n"
			 << "27 test_file_lock_fcntl_02()\n\t// 如果某进程申请了 写锁，其它进程 可以 申请  读锁，不能申请 写锁。\n"
			 << "28 test_file_lock_fcntl_03() // 测试 优先级\n"
			 << "29 test_file_lock_fcntl_04(); // 模拟软件只启动 1次/1个 进程\n"
			 << "30 test_semaphore_named(); // 有名信号量:生产者和消费者问题\n"
			 << "31 test_semaphore_unnamed()\n\t// 无名信号量(基于内存的信号量):生产者和消费者问题\n"
			 << "32 test_semaphore_unnamed_01()\n\t// 无名信号量(基于内存的信号量):多个生产者，单个消费者\n"
			 << "33 test_semaphore_unnamed_02()\n\t// 无名信号量(基于内存的信号量):多个生产者，多个消费者\n"
			 << "34 test_semaphore_unnamed_buffer()\n\t// 生产者和消费者：经典“双缓冲”方案\n"
			 << "35 test_mman_1_1()\n\t// 共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)\n"
			 << "36 test_mman_1_2()\n\t// 共享内存区：计数器：父子进程 + Posix匿名信号量(互斥锁)\n"
			 << "37 test_mman_2_1()\n\t// (匿名)共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)\n"
			 << "38 test_mman_01(); // 测试 mmap()开辟的 内存映射区的大小\n"
			 << "39 test_mman_3_service() // 共享内存区：计数器：Posix有名信号量(互斥锁)\n"
			 << "40 test_mman_3_client() // 共享内存区：计数器：Posix有名信号量(互斥锁)\n"
			 << "41 test_mman_3_consumer_service()\n\t// 共享内存区：消息：Posix匿名信号量(互斥锁)\n"
			 << "42 test_mman_3_producer_client()\n\t// 共享内存区：消息：Posix匿名信号量(互斥锁)\n"
			 << "------------------------ please input : ";
		cout.flush();
		cerr.flush();
		cin >> std::dec >> num;
		if (!cin.good())
		{
			cin.clear();  // 清除错误标志
			cin.ignore(); //当输入缓冲区没有数据时，也会阻塞等待数据的到来
			num = -1;
		}
		switch ( num ) {
			case 0:
				test_process_pipe(); // 建立父子进程间通信的管道：先 父写 子读，后 子写 父读
				exit(0);
			case 1:
				test_popen_pclose(); // 创建一个运行给定命令的管道(标准I/O流)，(等待)可以执行 shell 命令
				exit(0);
			case 2:
				test_process_fifo(); // 建立父子进程间通信的管道 FIFO（命名管道）
				exit(0);
			case 3:
				test_fifo_service(); // 独立 服务器 FIFO（命名管道）
				exit(0);
			case 4:
				test_fifo_client(); // 独立 客户端 FIFO（命名管道）
				exit(0);
			case 5:
				test_fifo_one_service(); // 单个 独立 服务器 FIFO（命名管道）: 一对多
				exit(0);
			case 6:
				test_fifo_multiple_client(); // 多个 独立 客户端 FIFO（命名管道）: 多对一
				exit(0);
			case 7:
				test_fifo_mesg_service(); // 独立 服务器 FIFO（命名管道）自定义 可变长度 消息体
				exit(0);
			case 8:
				test_fifo_mesg_client(); // 独立 客户端 FIFO（命名管道）自定义 可变长度 消息体
				exit(0);
			case 9:
				test_mq_create(argc, argv); // 只创建 消息队列
				exit(0);
			case 10:
				test_mq_service(); // 独立 服务器 消息队列
				exit(0);
			case 11:
				test_mq_client(); // 独立 客户端 消息队列
				exit(0);
			case 12:
				test_send_mq_notify(); // 周期 发送 消息队列
				exit(0);
			case 13:
				test_mq_notify_signal(); // 独立 消息队列 通知
				exit(0);
			case 14:
				test_mq_notify_sigwait(); // 独立 消息队列 通知
				exit(0);
			case 15:
				test_mq_notify_pipe_select(); // 独立 消息队列 通知
				exit(0);
			case 16:
				test_mq_notify_thread(); // 独立 消息队列 通知
				exit(0);
			case 17:
				test01_signal_rt(); // 测试 多参数信号函数 + 多(实时)信号处理
				exit(0);
			case 18:
				test_msg_service(); // 独立 服务器 消息队列
				exit(0);
			case 19:
				test_msg_client(); // 独立 客户端 消息队列
				exit(0);
			case 20:
				test_msg_service_echo(); // 独立 迭代服务器 消息队列
				exit(0);
			case 21:
				test_msg_client_call(); // 独立 客户端 消息队列
				exit(0);
			case 22:
				test_msg_service_fork(); // 独立 迸发服务器 消息队列
				exit(0);
			case 23:
				test_msg_client_fcall(); // 独立 客户端 消息队列
				exit(0);
			case 24:
				test_file_flock_01(); // 建议性锁：对 fd1 上锁，并不影响程序 dup() 复制为 fd2 上锁
				exit(0);
			case 25:
				test_file_flock_02(); // 建议性锁：子进程持有锁，并不影响父进程通过相同的fd获取锁，反之亦然
				exit(0);
			case 26:
				// 如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁；
				test_file_lock_fcntl_01();
				exit(0);
			case 27:
				// 如果某进程申请了 写锁，其它进程 可以 申请 读锁，不能申请 写锁。
				test_file_lock_fcntl_02();
				exit(0);
			case 28:
				test_file_lock_fcntl_03();
				exit(0);
			case 29:
				test_file_lock_fcntl_04(); // 模拟软件只启动 1次/1个 进程
				exit(0);
			case 30:
				test_semaphore_named(); // 有名信号量:生产者和消费者问题
				exit(0);
			case 31:
				test_semaphore_unnamed(); // 无名信号量(基于内存的信号量):生产者和消费者问题
				exit(0);
			case 32:
				test_semaphore_unnamed_01(); // 无名信号量(基于内存的信号量):多个生产者，单个消费者
				exit(0);
			case 33:
				test_semaphore_unnamed_02(); // 无名信号量(基于内存的信号量):多个生产者，多个消费者
				exit(0);
			case 34:
				test_semaphore_unnamed_buffer(); // 生产者和消费者：经典“双缓冲”方案
				exit(0);
			case 35:
				test_mman_1_1(); // 共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)
				exit(0);
			case 36:
				test_mman_1_2(); // 共享内存区：计数器：父子进程 + Posix匿名信号量(互斥锁)
				exit(0);
			case 37:
				test_mman_2_1(); // (匿名)共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)
				exit(0);
			case 38:
				test_mman_01(); // 测试 mmap()开辟的 内存映射区的大小
				exit(0);
			case 39:
				 test_mman_3_service(); // 共享内存区：计数器：Posix有名信号量(互斥锁)
				exit(0);
			case 40:
				 test_mman_3_client(); // 共享内存区：计数器：Posix有名信号量(互斥锁)
				exit(0);
			case 41:
				 test_mman_3_consumer_service(); // 共享内存区：消息：Posix匿名信号量(互斥锁)
				exit(0);
			case 42:
				 test_mman_3_producer_client(); // 共享内存区：消息：Posix匿名信号量(互斥锁)
				exit(0);
			default:
				num = -1;
				break;
		}
		cout << endl;
	}
}
#endif

/*
现今常用的进程间通信方式有：
 ① 管道(匿名管道) (使用最简单)( FIFO为命名管道)
 ② 信号 (开销最小)
 ③ 共享内存/映射区 (无血缘关系)
 ④ 本地套接字 (最稳定)
 ⑤ 消息队列
--------------------------------------------------------------------------------
一、管道（匿名管道）
1.什么是管道？
把一个进程连接到另一个进程的一个数据流称为一个“管道”，通常是用作把一个进程的输出通过管道连接到另一个进程的输入。管道本质上是内核的一块缓存，管道是Unix IPC的最老形式，而且所有的Unix系统都支持这种通信机制。
2.管道的特点
（1）管道是半双工的，任意时刻数据只能在一个方向上流动；
（2）管道只能在具有公共祖先的进程（父进程与子进程，或者两个兄弟进程）之间使用;；
（3）管道的生命周期随进程，由进程创建，进程终止，管道占用的内存也随之归还给操作系统；
（4）内核对管道操作进行同步与互斥，读完了就不读了，写满了就不写了，这种访问方式是典型的“生产者——消费者”模型；
（5）管道面向字节流，传输的数据时没有数据结构的字节流。
3.管道的一端被关闭，下列规则起作用：
（1）当读一个写端已被关闭的管道时,在所有数据都被读取后,，read返回0,以指示达到了文件结束处；
（2）如果写一个读端已被关闭的管道,则产生信号SIGPIPE。如果忽略该信号或者捕捉该信号并从其处理程序返回,则write出错返回,进程终止，ernno置为EPIPE。
4.管道的创建 管道的数据在内核（内存）中流动
#include<unistd.h>
int pipe(int fd[2]);   //成功0，失败-1
5.管道的用法
单进程中的管道几乎没有任何用处。通常调用 pipe的进程接着调用fork创建从父进程到子进程或反之的IPC通道。
（1）对于从父进程到子进程的管道,父进程关闭管道的读端(fd [0])，子进程则关闭写端(fd [1])
（2）对于从子进程到父进程的管道,父进程关闭管道的写端(fd [1])，子进程则关闭读端(fd [0])
6.popen和pclose函数
使用常见的操作就是，创建一个连接到另一个进程的管道，然后读其输出或向输入端发送数据，所以标准 I/O库提供了两个函数 popen 和 pclose。这两个函数的(自带)操作是：创建一个管道， fork 一个子进程，关闭未使用的管道端，执行一个 shell 命令，然后等待命令终止。
#include<stdio.h>
FILE *popen(const char*cmdstring, const char* type); //返回：成功则返回文件指针，失败则为NULL
int pclose(FILE fp*) //返回：cmdstring终止状态，若出错则返回-1
函数 popen 先执行 fork，然后调用 exec 执行 cmdstring，并且返回一个标准 I/O 文件。
如果 type 是“r”， 则文件连接到 cmdstring 的标准输出；
如果 type 是“w”，则文件链接到 cmdstring 的标准输入
pclose 函数关闭标准 I/O 流，等待命令终止，然后返回 shell 的终止状态。
popen和pclose函数是对前面讲述的函数的封装，可自行实现。
7.用 socketpair()创建任何进程间全双工通信的unix local 本地套接字
#include <sys/types.h>  #include <sys/socket.h>  int socketpair(int d, int type, int protocol, int sv[2])；
基本用法：
1.这对套接字可以用于全双工通信，每一个套接字既可以读也可以写。
例如，可以往sv[0]中写，从sv[1]中读；或者从sv[1]中写，从sv[0]中读；
2.如果往一个套接字(如sv[0])中写入后，再从该套接字读时会阻塞，只能在另一个套接字中(sv[1])上读成功；
3.读、写操作可以位于同一个进程，也可以分别位于不同的进程，如父子进程。如果是父子进程时，一般会功能分离，一个进程用来读，一个用来写。因为文件描述副sv[0]和sv[1]是进程共享的，所以读的进程要关闭写描述符, 反之，写的进程关闭读描述符。
--------------------------------------------------------------------------------
二、FIFO（命名管道）
1.什么是FIFO
FIFO有时被称为命名管道。管道只能由具有血缘关系的进程使用，它们共同的祖先进程创建了管道。但是，通过FIFO，不具有血缘关系的进程也能交换数据。
FIFO是一种文件类型，创建FIFO类似于创建文件，而且FIFO的路径名存在与文件系统中，通过stat结构成员st_mode的编码指明文件是否为FIFO类型，可以用S_ISFIFO宏对此进行测试。
2.创建FIFO
(1) 在控制台shell，通过mkfifo命令直接创建 $ mkfifo test.fifo
(2) 在程序中通过mkfifo函数创建
#include<sys/types.h>  #include<sys/stat.h>
int mkfifo(const char *pathname,mode_t mode);  // 成功为0，失败为-1
特别说明：mkfifo只创建管道，不打开，一旦已经用mkfifo创建了一个FIFO，就可用open打开它，而且一般的文件 I / O函数(close、read、write、unlink等)都可用于FIFO;
path 参数 指明路径   		mode 参数 这里要注意一下，创建的管道文件的权限是 mode & ~umask
创建屏蔽字umask可以通过 umask 函数调整，例如 umask(0000)
mode与打开普通文件的 open() 函数中的 mode 参数相同
3.FIFO的注意事项
当 open 一个 FIFO 时，非阻塞标志（O_NONBLOCK）会产生下列影响(阻塞与非阻塞模式)
(1) 没有指定 O_NONBLOCK ，只读 open 要阻塞到某个其他进程为写而打开这个 FIFO 为止；只写 open 要阻塞到某个其他进程为读而打开它为止。
(2) 如果指定了 O_NONBLOCK，则只读 open 立即返回。但是，如果没有进程为读read而打开这个 FIFO，那么write只写 open 将返回 -1，并将 errno 设置成 ENXIO。
类似于管道，若 write 一个尚无进程为读而打开的 FIFO ，则产生 SIGPIPE 信号，若忽略此信号则进程终止；若某个 FIFO 的最后一个写进程关闭了该 FIFO，则将为该 FIFO 的读进程产生一个文件结束标志。
一个给定的 FIFO 有多个写进程是常见的。如果不希望多个进程所写的数据交叉，则必须考虑原子写操作：常数PIPE_BUF说明了可被原子写到FIFO的最大数据量，每次写入的长度要小于PIPE_BUF字节，这样就能避免多个进程各次写之间的穿插。
4.FIFO的用途
(1) shell 命令使用 FIFO 将数据从一条管道传送到另一条时，无需创建临时文件;
(2) 客户进程-服务器进程应用程序中，FIFO 用作汇聚点，在客户进程和服务器进程之间传递数据(如图)。
--------------------------------------------------------------------------------
三、消息队列
1.什么是消息队列
消息队列是消息的链表，存放在内核中并由消息队列标识符标识，内核管理，我们只需要调用内核提供给我们的接口就行。
我们可以将内核中的某个特定的消息队列画为一个消息链表，如图假设有一个具有三个消息的队列，消息长度分别为1字节，2字节和3字节，而且这些消息就是以这样的顺序写入该队列的。再假设这三个消息的类型分别为100,200,300.
在bash终端可以使用 ipcs -q 查看内核现有队列，使用ipcrm -q msqid 删除指定队列。
2.消息队列的特点
（1）管道和FIFO都是随进程持续的，SYSTEM V IPC(消息队列、信号量、共享内存)都是随内核持续的。当一个管道或FIFO的最后一次关闭发生时，仍在该管道或FIFO上的数据将被丢弃。消息队列，除非内核自举或显式删除，否则其一直存在。在某个进程往一个队列写入消息之前，并不需要另外某个进程在该队列上等待消息的到达（和管道、FIFO不同）。
（2）消息队列是双向通信的，通过消息的类型可以标识是服务器要读的还是客户机要读的，或者是那个客户机插入的等。
（3）消息队列具有一定的先入先出特性，但是它可以实现消息的非先入先出查询；
3.标识符和关键字
标识符：每个内核中的 IPC结构（消息队列、信号量或共享存储段）都用一个非负整数的标识符( identifier )加以引用。当一个 IPC 结构被创建，然后又被删除时，与这种结构相关的标识符连续加 1，直到达到一个整型数的最大正值，然后又回转到0。
关键字：标识符是IPC对象的内部名。为使多个合作进程能够在同一 IPC 对象上汇聚，需要提供一个外部命名方案，为此，每个 IPC 对象都和与一个键相关联，将这个键作为该对象的外部名。无论何时创建一个 IPC 结构，都应指定一个键，这个键的数据类型是基本系统数据类型key_t，通常在 <sys/types.h>中被定义为长整型。
可以调用函数 ftok 生成一个键，ftok提供的唯一服务就是由一个路径名和课题 ID 产生一个关键字。
#include <sys/ipc.h>
key_t ftok(const char * path, int id);  //若成功，返回键；若出错，返回 (key_t) - 1,path 参数必须引用一个现有文件
4.创建或打开一个消息队列msgget
#include <sys/msg.h>       int msgget(key_t key, int msgflg);
//返回值是一个整数标识符msgid，其他三个msg函数就用它来指代该队列。它是基于指定的key产生的，而key既可以是ftok的返回值，也可以是常值IPC_PRIVATE。
key  就是我们刚刚说的键了，key既可以是ftok的返回值，也可以是常值IPC_PRIVATE。
msgflg  是读写权限值的组合。它还可以与IPC_CREAT或IPC_CREAT | IPC_EXCL按位或，IPC_NOWAIT --- 读写消息队列要求无法得到满足时，不阻塞。
每个消息队列都有一个 msqid_ds 结构与其关联，这个结构定义了队列的当前状态。
struct msqid_ds {
	struct ipc_perm msg_perm;   ipc_perm 结构
	msgqnum_t msg_qnum; 		队列的消息条数
	msglen_t msg_qbytes;		最大消息占用字节数
	pid_t msg_lspid;    		最后一条发送消息的进程 ID
	pid_t msg_lrpid;    		最后一条接收消息的进程 ID
	time_t msg_stime;   		last-msgsnd() time
	time_t msg_rtime;   		last-msgrcv() time
	time_t msg_ctime;   		last-change time
…
};
当创建一个新消息队列时，msqid_ds结构的如下成员被初始化。
(1) msg_perm结构的uid和cuid成员被设置成当前进程的有效用户ID，gid和cgid成员被设置成当前进程有效组ID。
(2) msgflg中的读写权限位存放在msg_perm.mode中。
(3) msg_qnum,msg_lspid,msg_lrpid,msg_stime和msg_rtime被置为0.
(4) msg_ctime被设置成当前时间。
(5) msg_qbytes被设置成系统限制值。
(6) msg_qbytes被设置成系统限制值。
5.对队列执行多种操作msgctl
msgctl函数提供在一个消息队列上的各种控制操作。
#include<sys/msg.h>    int msgctl(int msgid,int cmd,struct msqid_d *buf);   //成功为0 失败为-1
cmd 参数指定 msqid 指定队列要执行的命令
IPC_STAT：取此消息队列的 msqid_ds 结构，并将它存放在 buf 指向的结构中；
IPC_SET：根据buf指向结构的值，设置此队列的msqid_ds 结构中的msg_perm.uid、msg_perm.gid、msg_perm.mode 和 msg_qbytes 字段（此命令要求有效ID或者超级用户）；
IPC_RMID：从系统中删除该消息队列以及仍在该队列中的所有数据。
6.调用 msgsnd 将数据放到消息队列中
#include<sys/msg.h>  int msgsnd(int msgid,const void *ptr,size_t nbytes,int flag);   //成功为0 失败为-1
		msqid 是 msgget 的返回值
		消息组成：消息都由三部分组成,它们是:正长整型类型字段、非负长度（nbytes）以及实际数据字节(对应于长度)。消息总是放在队列尾端。巧妙在第2个参数ptr，通过一个参数知道了消息类型和实际值(配合ntypes)
ptr 是一个数据块指针，指向一个结构体mymsg；
struct mymsg {
	long mtype; 消息类型. 必须大于 0
	char mtext[1];  Message text.
} //这个结构体的第二个字段可以根据自己需要进行调整；
nbytes指明mymsg结构体第二个字段的大小，即消息数据的实际大小；
msgflag 的值可以指定为 IPC_NOWAIT， 设置非阻塞
		如果消息队列已满（或者是队列中的消息总数等于系统限制值,或队列中的字节总数等于系统限制值），msgsnd 立即出错返回 EAGAIN如果没有设置 IPC_NOWAIT，进程会一直阻塞到：
(1）有空间可以容纳要发送的消息；
(2）从系统中删除了此消息队列；返回 EIDRM 错误
(3）捕捉到一个信号，并从信号处理程序返回。返回 EINTE 错误
		如果从系统中删除某个消息队列：没有维护引用计数，删了就删了使用这一消息队列的进程下次将出错返回。
7.msgrcv从队列中取用消息
#include<sys/msg.h>
int msgrcv(int msqid,void * ptr,size_t nbytes,long type, int flag);  //成功则返回消息数据部分的长度，出错则返回-1
msqid 是 msgget 的返回值；  ptr指定所接收消息的存放位置。
nbytes指定了数据部分大小(只想要多长的数据)，如果返回的消息长度大于 nbytes，并且flag 设置了MSG_NOERROR，
消息将被截断。
type指定我们希望从队列中去除什么类型的消息（用它来进行非先进先出方式的读消息）
type == 0返回队列中的第一个消息
		type > 0 返回队列中消息类型为type的第一个消息
		type < 0 返回队列中消息类型值小于或等于type绝对值的消息，如果这种消息有若干个。则取类型值最小的消息。
flag可以被设置为 IPC_NOWAIT ，使操作不阻塞
		如果没有所指定类型的消息，msgrcv 直接返回 -1，errno 设置为 ENOMSG
如果没有指定 IPC_NOWAIT，则进程会一直阻塞到：
(1) 有指定消息可用；
(2) 从系统中删除此消息队列；
(3) 捕捉到一个信号并从信号处理程序返回。
注意：msgsnd 和 msgrcv 在执行成功之后，内核才会更新与该消息队列相关连的 msgid_ds 结构中的相关信息（调用者进程 ID，时间）
--------------------------------------------------------------------------------
四、信号量
1.什么是信号量
进程间通信方式之一，用于实现进程间同步与互斥（不传递数据）。多个进程同时操作一个临界资源的时候就需要通过同步与互斥机制来实现对临界资源的安全访问。
信号量与前面介绍的管道、FIFO以及消息队列不同，信号量是具有一个等待队列的计数器（0代表现在还有没有资源可以使用），用于为多个进程提供对共享数据的访问。
常用的信号量形式被称之为双态信号量(binary semaphore)。它控制单个资源，其初始值为1。但是，一般而言，信号量的初值可以是任一正值，该值说明有多少个共享资源单位可供共享应用。SYSTEM V的信号量和POSIX的信号量思想相同，但是前者很复杂，而后者的使用非常简单，这里介绍的是SYSTEM V的信号量。
2.实现方法
当信号量没有资源可用时，这时候需要阻塞等待，常用的信号量形式被称之为双态信号量(binary semaphore)。它控制单个资源，其初始值为1。但是，一般而言，信号量的初值可以是任一正值，该值说明有多少个共享资源单位可供共享应用。
同步：只有信号量资源计数大于0的时候，会通知别人，打断等待，去操作临界资源，也就是说，别人释放了资源（+1）之后你才能获取资源（-1）然后进行操作。
互斥：信号量如果要实现互斥，那么它的计数只能是 0/1 (一元信号量，双态信号量)，一个进程获取临界资源后，在他没释放临界资源之前，别的进程无法获取该临界资源。
为了获取共享资源，进程需要执行下列操作：
(1)测试控制该资源的信号量。
(2)若此信号量的值为正（大于0），则进程可以使用该资源。在这种情况下，进程会将信号量值减一，表示使用了一个资源单位。
(3)若此信号量值为0，则进程进入休眠状态，直至信号量大于0，进程被唤醒后，继续步骤（1）。
当进程不再使用由一个信号量控制的共享资源时，该信号量值增1。如果有进程正在休眠等待此信号量，则唤醒他们。
为了正确地实现信息量，信号量值的测试及减 1 操作是原子操作。为此,信号量通常是在内核中实现的。
3. 内核中信号量集合的结构
		内核为每个信号量集设置了一个 semid_ds结构
struct semid_ds {
	struct ipc_permsem_perm ;
	structsem*sem_base ; 	//信号数组指针
	ushort sem_nsem ; 		//此集中信号个数
	time_t sem_otime ; 		//最后一次semop时间
	time_t sem_ctime ; 		//最后一次创建时间
} ;
某个指定的信号量的结构体
struct sem {
	ushort_t semval ; //信号量的值
	short sempid ; //最后一个调用semop的进程ID
	ushort semncnt ; //等待该信号量值大于当前值的进程数（一有进程释放资源 就被唤醒）
	ushort semzcnt ; //等待该信号量值等于0的进程数
} ;
4.创建或打开一个信号量 semget
#include <sys/sem.h>  int semget(ket_t key, int nsems, int flag); //成功则返回信号量ID，出错则返回-1
		key 关键字，可以通过函数 ftok 创建，也可以自己指定
		nsems 是该集合中的信号量数。如果是创建新集合(一般在服务器中),则必须指定nsems。如果引用一个现存的集合(一个客户机),则将nsems指定为0。
Oflag 可以是SEM_R(read)和SEM_A(alter)常值的组合（打开时用到），也可以是IPC_CREAT或IPC_EXCL 。
5.semctl 包含的多种信号量操作
#include <sys.sem.h>  int semctl(int semid, int semnum, int cmd, union semun arg );
semid 指定的信号量集合
semnum 指定该集合中的一个成员，取值范围：[0, nsems)
cmd  指定下列 10 种命令中的一种，使其在semid指定的信号量集合上执行此命令。其中有五条命令是针对一个特定的信号量值的，它们用semnum指定该集合中的一个成员。
IPC_STAT 对此集合取 semid_ds 结构，存储在 arg.buf 指向的结构中
IPC_SET 按 arg.buf 指向的结构中的值，设置与此集合相关的 semid_ds 结构
IPC_RMID 从系统中删除该信号量集合
GETVAL 返回成员 semnum 的 semval 值
SETVAL 设置成员 semnum 的 semval 值，该值由 arg.val 指定
GETPID 返回成员 semnum 的 sempid 值
GETNCNT 返回成员 semnum 的 semncnt 值
GETZCNT 返回成员 semnum 的 semzcnt 值
GETALL 取该集合中所有的信号量值，这些值存储在 arg.array 指向的数组中
SETALL 将集合中所有的信号量设置成 arg.array 指向的数组中的值
arg 可选，是否使用取决于所请求命令
		如果使用该参数，则其类型是 semun，是多个命令特定参数的联合（union）:
union semun {
	int val;
	struct semid_ds * buf;
	unsigned short * array;
}; 这个选项参数是一个联合，而非指向联合的指针
		返回值  对 GETALL 以外的所有 GET 命令，semctl 函数都返回相应值
对其他命令：若成功，返回 0    若出错，设置 errno 并返回 -1
6.semop自动执行信号量集合上的操作数组
#include <sys/sem.h>
int semop(int semid, struct sembuf semoparray[], size_t nops);  //成功返回0 失败返回-1
semop 函数具有原子性，要么执行数组中的所有操作，要么一个也不做。
semid 指定的信号量集合
semoparray 是一个指针，指向一个由 sembuf 结构表示的信号量操作数组：
struct sembuf {
	unsigned short sem_num;
	short sem_op;
	short sem_flg;
};
nops 规定该数组中操作的数量，对集合中每个成员的操作由相应的 sem_op 值规定
		sem_op 值可以是负值、0或正值
信号量的 “undo” 标志，此标志对应于 sem_flg 成员的 SEM_UNDO 位
（1）sem_op 为正值，表示的是进程释放的占用资源数，sem_op 值会加到信号量的值上，如果指定了 undo 标志，则也从该进程的此信号量调整值中减去 sem_op 。
（2）sem_op 为负值，表示要获取由该信号量控制的资源
		如若该信号量的值大于等于 sem_op 的绝对值，则从信号量中减去 sem_op 的绝对值。
如若信号量小于 sem_op 的绝对值：
a. 若指定了 IPC_NOWAIT，则 semop 出错返回 EAGAIN ；
b. 若未指定 IPC_NOWAIT，则该信号量的 semncnt 加 1，然后调用进程被挂起等待直至下列事件之一发生：
i. 此信号量值变为大于等于 sem_op 的绝对值
		ii. 从系统中删除了此信号量
		iii. 进程捕捉到一个信号，并从信号处理程序返回
（3）若 sem_op 为 0，这表示调用进程希望等待到该信号量变为 0.
若信号量值当前为 0 , 则此函数立即返回。
如果此信号量值非 0，则适用于下列条件：
a. 若指定了 IPC_NAWAIT，则出错返回 EAGAIN
b. 若未指定 IPC_NOWAIT，则该信号量的 semzcnt 值加 1，然后调用进程被挂起，直至下列的一个事件发生。
i. 此信号量值变为 0。此信号量的 semzcnt 值减 1。
ii. 从系统中删除了此信号量。
iii. 进程捕捉到一个信号，并从信号处理程序返回。在这种情况下，此信号量的 semzcnt 值减 1，并且函数出错返回 EINTR。
7.exit 时的信号调整
一个进程终止时，如果它占用了经由信号量分配的资源（执行P操作，减1），并且没有归还给（执行V 操作，加1），等待的其他进程将会一直阻塞，那么就会出现问题。这也是信号量必须处理的问题，它是这样做的：
无论何时只要为信号量操作指定了 SEM_UNDO 标志，然后分配资源（sem_op 值小于 0），那么内核就会记住对于该特定信号量，分配给调用进程多少资源（sem_op 的绝对值）。所以设置SEM_UNDO是很有用途的。
当该进程终止时，不论自愿或不自愿内核都将检验该进程是否还有尚未处理的信号量调整值，如果有，则按调整值对相应信号量进行处理。如果用带SETVAL 或SETALL命令的semctl 设置一个信号量的值，则在所有进程中，该信号量的调整值都将设置为 0。
--------------------------------------------------------------------------------
五、共享内存
1.什么是共享内存
共享存储允许两个或多个进程共享一给定的存储区。因为数据不需要在客户机和服务器之间复制，所以这是最快的一种 I P C（读写同一块物理内存）。
2.如何实现
使用共享存储的唯一窍门是多个进程之间对一给定存储区的同步存取。若服务器将数据放入共享存储区，则在服务器做完这一操作之前，客户机不应当去取这些数据。通常，信号量被用来实现对共享存储存取的同步。(记录锁/读写锁也可用于这种场合。)
3.内核中共享内存的属性结构
struct shmid_ds {
	 struct ipc_perm shm_perm;  	ipc_perm 结构
	 struct anon_map *shm_amp; 		pointer in kernel
	 int shm_segsz;  				size of segment in bytes
	 ushort shm_lkcnt;  			number of times segment is being locked
	 pid_t shm_lpid;  				pid of last shmop()
	 pid_t shm_cpid;  				pid of creator
	 ulong shm_nattch;  			number of current attaches
	 ulong shm_cnattach;  			used only for shminfo
	 time_t shm_atime;  			last-attach time
	 time_t shm_dtime;  			last-detach time
	 time_t shm_ctime;  			last-change time
	 ...
	};
4.shmget 函数创建共享内存或获得一个共享内存标识符
#include<sys/shm.h>  int shmget(key_t key,int size,int flag);  //成功返回共享内存ID，出错返回-1
		key 关键字，可以通过函数 ftok 创建，也可以自己指定
		size 是该共享存储段的最小值。如果正在创建一个新段(一般在服务器中)，则必须指定其size。如果正在存访一个现存的段(一个客户机)，则将size指定为0。
oflag 读写权限值的组合
5.shmctl 函数对共享内存执行多种操作
#include <sys/shm.h>  int shmctl(int shmid, int cmd, struct shmid_ds * buf); //成功为0，出错为-1
		shmid 标识符 shmget 函数返回的共享内存标识符
		cmd 指定下列 5 种命令中的一种，使其在 shmid 指定的段上执行
IPC_STAT    取此段的 shmid_ds 结构，并将它存储在由 buf 指向的结构中
IPC_SET    将 buf 指向的结构中的值设置到此共享存储段的 shmid_ds 结构
IPC_RMID   从系统中删除该共享存储段
IPC_LOCK   在内存中对共享存储段加锁
IPC_UNLOCK 解锁共享存储段
6.shmat 函数将共享内存连接到调用进程的地址空间中
#include <sys/shm.h>
void * shmat(int shmid, const void * addr, int flag); //成功则返回执行共享内存段的指针，出错则为-1
shmid 标识符 shmget 函数返回的共享内存标识符
addr 共享存储段连接到调用进程的哪个地址上与 addr 参数以及 flag 中是否指定 SHM_RND 位有关。
SHM_RND 命令的意思是 “取整”；
如果 addr 为 0，则此段连接到由内核选择的第一个可用地址上（一般应指定 addr为0,以便由内核选择地址。）
如果 addr 非 0，并且没有指定 SHM_RND ，则此段连接到 addr 所指定的地址上；
如果 addr 非 0，并且指定了 SHM_RND，则此段连接到 (addr - (addr mod SHMLBA)) 所表示的地址上，该算式是将地址向下取最近 1 个 SHMLBA 的倍数。
SHMLBA 的意思是 “低边界地址倍数”，它总是 2 的乘方。
参数 flag 如果指定了 SHM_RDONLY 位，则以只读方式连接此段，否则以读写方式连接此段；
如果 shmat 执行成功，那么内将与该共享存储段相关的 shmid_ds 结构中的 shm_nattch 计数器加 1。
7.函数 shmdt 使共享存储段与该进程分离
#include <sys/shm.h>  int shmdt(void * addr);      //成功返回0，出错返回-1
addr调用 shmat 函数的返回值，共享内存在进程中的实际地址；
如果执行成功，shmdt 将使相关的 shmid_ds 结构中的 shm_nattch 计数器减1。
注意，这并不从系统中删除其标识符以及其数据结构。该标识符仍然存在,直至某个进程(一般是服务器)调用 shmctl
		(带命令IPC_RMID)特地删除它。
8.共享内存操作步骤：  1.创建共享内存  2.映射共享内存到虚拟地址空间
3.多线程同步读写（一般任务记得进行同步和互斥操作）  4.通信结束（解除映射关系，删除共享内存）
9.关于删除共享内存的问题
在删除共享内存的时候，并不是直接删的。
如果有进程依然与共享内存保持映射连接关系，
那么共享内存将不会被立即删除，
而是等最后一个映射断开后删除，
在这期间，将拒绝其他进程使用shmat连接此共享内存！
*/


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

