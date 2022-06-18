//
// Created by zcq on 2021/5/11.
//

#ifndef __FIFO_TEST_H
#define __FIFO_TEST_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*********************************/

#include "zcq_header.h"

/*********************************/

void test_process_fifo(); // 建立父子进程间通信的管道 FIFO（命名管道）

void test_fifo_service(); // 独立 服务器 FIFO（命名管道）
void test_fifo_client(); // 独立 客户端 FIFO（命名管道）

void test_fifo_one_service(); // 单个 独立 服务器 FIFO（命名管道）: 一对多
void test_fifo_multiple_client(); // 多个 独立 客户端 FIFO（命名管道）: 多对一

void test_fifo_mesg_service(); // 独立 服务器 FIFO（命名管道）自定义 可变长度 消息体
void test_fifo_mesg_client(); // 独立 客户端 FIFO（命名管道）自定义 可变长度 消息体


#endif //__FIFO_TEST_H


/*
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
*/

/* 	管道有自身的缺陷：
 	管道更适合进程端对端的通信，即在两个进程之间建立一个通信管道，如果有多个进程想要通过管道进行通信，
 那么需要建立多个管道。
 	如果建立管道，那么必须先有管道的读出端进程，否则仅仅有写入端的管道是无意义，或者说是不存在的。
 	管道的消息是没有优先级的，即数据是先进先出(FIFO)的，这样如果有紧急数据，无法立刻进行处理，
 只能等到前面的数据处理完毕后，才可以处理当前紧急消息  */

