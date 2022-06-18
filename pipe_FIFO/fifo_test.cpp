//
// Created by zcq on 2021/5/11.
//

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

/************ 管道（匿名管道） pipe 具有血缘关系(公共祖先)的进程间通信 ***********/
/* 	进程间的通信方式 pipe（管道）
	管道的原理: 管道实为内核使用环形队列机制，借助内核缓冲区(如4k)实现。
	管道 是一种最基本的 IPC 机制，作用于有血缘关系(公共祖先)的进程之间，完成数据传递。
调用 pipe() 系统函数即可创建一个管道。
 	管道创建成功以后，创建该管道的进程（父进程）同时掌握着管道的读端和写端。
---------------------------------------------------------------------------
特质：
1. 其本质是一个伪文件(实为内核缓冲区)
2. 由两个文件描述符引用，一个表示读端，一个表示写端。
3. 规定数据从管道的写端流入管道，从读端流出。
---------------------------------------------------------------------------
局限性：
① 数据自己读却不能自己写。
② 数据一旦被读走，便不在管道中存在，不可反复读取。
③ 由于管道采用半双工通信方式。因此，数据只能在一个方向上流动。
④ 只能在有 公共祖先 的进程间使用管道。
常见的通信方式有，单工通信、半双工通信、全双工通信。
---------------------------------------------------------------------------
特点:
1.管道只允许具有血缘关系(公共祖先)的进程间通信，如父子进程间的通信，半双工通信。
2.管道只允许单向通信。 从消息队列中读出消息，消息队列中对应的数据都会被删除。
3.管道内部保证同步机制，从而保证访问数据的一致性。
4.面向字节流。
5.管道随进程，进程在管道在，进程消失管道对应的端口也关闭，两个进程都消失管道也消失。
---------------------------------------------------------------------------
总结：
	如果一个管道的写端一直在写，而读端的引⽤计数是否⼤于0决定管道是否会堵塞，
引用计数大于0，只写不读再次调用 write 会导致管道堵塞；
	如果一个管道的读端一直在读，而写端的引⽤计数是否⼤于0决定管道是否会堵塞，
引用计数大于0，只读不写再次调用 read 会导致管道堵塞；
	而当他们的引用计数等于0时，
只写不读 会导致 写端的进程 收到一个 SIGPIPE 信号，导致进程终止，
只读不写 会导致 read 返回 0,就像读到⽂件末尾⼀样。		*/

/* 	管道有自身的缺陷：
 	管道更适合进程端对端的通信，即在两个进程之间建立一个通信管道，如果有多个进程想要通过管道进行通信，
 那么需要建立多个管道。
 	如果建立管道，那么必须先有管道的读出端进程，否则仅仅有写入端的管道是无意义，或者说是不存在的。
 	管道的消息是没有优先级的，即数据是先进先出(FIFO)的，这样如果有紧急数据，无法立刻进行处理，
 只能等到前面的数据处理完毕后，才可以处理当前紧急消息  */


#include "fifo_test.h"

#include <iostream>
using namespace std;


/******************************************************************************/

#define	FIFO_00 	"/tmp/test_process_fifo.00"
#define	FIFO_01 	"/tmp/test_process_fifo.01"

void creat_two_fifo()
{
	/* create two FIFOs; OK if they already exist */
	Mkfifo(FIFO_00, FILE_MODE);
	int ret = mkfifo(FIFO_01, FILE_MODE);
	if (ret < 0 && errno != EEXIST/*File exists*/ ) {
		unlink(FIFO_00); // 删除
		err_sys(ret, __FILE__, __func__, __LINE__,
				"can't create %s", FIFO_01);
	}
}

/******************************************************************************/

void fifo_service(const int &readfd, const int &writefd)
{
	int flags = Fcntl(writefd, F_GETFL, 0); 		// 获取 I/O 状态
	Fcntl(writefd, F_SETFL, flags | O_NONBLOCK); 	// 设置 I/O 非阻塞
//	flags = Fcntl(readfd, F_GETFL, 0); 			// 获取 I/O 状态
//	Fcntl(readfd, F_SETFL, flags | O_NONBLOCK); 	// 设置 I/O 非阻塞

	char buff[MAXLINE];
	pid_t pid = getpid();
	ssize_t bufsize;
	while (true) {
		if ( (bufsize = Read(readfd, buff, MAXLINE)) <= 0 &&
			 bufsize != -1234) break;
		if (bufsize != -1234) {
			cout << "fifo_service[" << pid << "] (" << bufsize << "): "
				 << buff << endl;
			if (if_run("quit", buff)) break;
			if (if_run("time", buff)) {
				memset(buff, 0, 64);
				time_t ticks;
/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
				ticks = time(nullptr);
				snprintf(buff, sizeof(buff), "%.24s\n", ctime(&ticks));
			}
			Write(writefd, buff, strlen(buff));
		}
//		_debug_log_info("while() pid: %d", pid)
	}
	_debug_log_info("while() exit bufsize=%zd", bufsize)
}

void fifo_client(const int &readfd, const int &writefd)
{
	int flags = Fcntl(writefd, F_GETFL, 0); 		// 获取 I/O 状态
	Fcntl(writefd, F_SETFL, flags | O_NONBLOCK); 	// 设置 I/O 非阻塞
//	flags = Fcntl(readfd, F_GETFL, 0); 			// 获取 I/O 状态
//	Fcntl(readfd, F_SETFL, flags | O_NONBLOCK); 	// 设置 I/O 非阻塞

	char sendbuf[MAXLINE];
	char recvbuf[MAXLINE];
	pid_t pid = getpid();
	ssize_t bufsize;
	while (true) {
		if ((bufsize = Read(STDIN_FILENO, sendbuf, MAXLINE)) <= 0) break;
		Write(writefd, sendbuf, bufsize);
		if (if_run("quit", sendbuf)) break;
		bufsize = Read(readfd, recvbuf, MAXLINE);
		cout << "fifo_client[" << pid << "] (" << bufsize << "): "
			 << recvbuf << endl;
//		_debug_log_info("while() pid: %d", pid)
	}
	_debug_log_info("while() exit bufsize=%zd", bufsize)
}

/******************************************************************************/

void test_process_fifo() // 建立父子进程间通信的管道 FIFO（命名管道）
{
	creat_two_fifo();
	int read_fd, write_fd;

	pid_t child_pid = Fork();
	if (child_pid == 0) {
		_debug_log_info("fifo_service: %d", getpid())
		read_fd = Open(FIFO_00, O_RDONLY, 0777);
		write_fd = Open(FIFO_01, O_WRONLY, 0777);
		fifo_service(read_fd, write_fd);
		exit(0);
	}

	_debug_log_info("fifo_client: %d", getpid())
	write_fd = Open(FIFO_00, O_WRONLY, 0777);
	read_fd = Open(FIFO_01, O_RDONLY, 0777);
	fifo_client(read_fd, write_fd);

	_debug_log_info("waitpid: %d", child_pid)
	/* pid=-1, option=0 等待 任何子进程，相当于 wait() */
	int ret = waitpid(child_pid, nullptr, 0); // wait for child to terminate
	err_sys(ret, __LINE__, "waitpid()");

	Close(write_fd);
	Close(read_fd);
	unlink(FIFO_00); // 删除
	unlink(FIFO_01);
}

/******************************************************************************/

void test_fifo_service() // 独立 服务器 FIFO（命名管道）
{
	creat_two_fifo();
	int read_fd = Open(FIFO_00, O_RDONLY, 0777);
	int write_fd = Open(FIFO_01, O_WRONLY, 0777);
	fifo_service(read_fd, write_fd);
}

void test_fifo_client() // 独立 客户端 FIFO（命名管道）
{
	int write_fd = Open(FIFO_00, O_WRONLY, 0777);
	int read_fd = Open(FIFO_01, O_RDONLY, 0777);
	fifo_client(read_fd, write_fd);
	Close(write_fd);
	Close(read_fd);
	unlink(FIFO_00); // 删除
	unlink(FIFO_01);
}

/******************************************************************************/

#define	FIFO_one_multiple	"/tmp/FIFO_one_multiple"

void test_fifo_one_service() // 单个 独立 服务器 FIFO（命名管道）: 一对多
{
	pid_t ppid = getpid(), cpid;
	_debug_log_info("test_fifo_one_service(): %d", ppid)

	string pathname = FIFO_one_multiple;
	pathname += ".service";
	Mkfifo(pathname.c_str(), FILE_MODE);

	/* 	每当有一个 client 终止时，将 close(write_fd)
	 	导致 service 的 read(read_fd, x, x)返回 0，表示是一个文件结束符，从而 service 退出；
	 	但是要求 service 不得退出 ！必须用 Open() 创建 FIFO 的 O_RDONLY 且 不使用。 */
	/* 不使用 O_RDWR，防止 open(x, O_RDONLY, x) 阻塞 */
//	int tmp_fd = Open(pathname.c_str(), O_RDWR, 0777); // 不使用
//	_debug_log_info("while(): fifo_read_fd=%d", tmp_fd)
//	int fifo_read_fd = Open(pathname.c_str(), O_RDONLY, 0777);
	int fifo_read_fd = open(pathname.c_str(), O_RDWR, 0777);
	_debug_log_info("fifo_read_fd=%d", fifo_read_fd)
	if (fifo_read_fd < 0) {
		Close(fifo_read_fd);
		unlink(pathname.c_str()); // 删除
		err_quit(-1, __LINE__, "open(): %s", pathname.c_str());
	}
//	Close(tmp_fd); // 不使用
//	tmp_fd = Fcntl(fifo_read_fd, F_GETFL, 0); 			// 获取 I/O 状态
//	Fcntl(fifo_read_fd, F_SETFL, tmp_fd & (~(int)O_NONBLOCK) ); // 设置 I/O 非阻塞

	int fifo_write_fd, fd;
	string filename;
	ssize_t bufsize;
	char buff[MAXLINE], *ptr= nullptr;
	while (true) {
		if ((bufsize = Read(fifo_read_fd, buff, MAXLINE)) < 0 &&
			bufsize != -1234) break;
		if (bufsize == 0) continue;
		cout << "test_fifo_one_service[" << ppid << "] (" << bufsize << "): "
			 << buff << endl;

		/* buff = "PID pathname" */
//			if ( (ptr = strchr(buff, ' ')) == nullptr) {
//				err_msg(-1, __LINE__, "strchr(): %s", buff);
//				continue;
//			}
//			*ptr++ = 0; // null terminate PID, ptr = pathname
		cpid = strtol(buff, &ptr, 10);
		while (true) {
			if (*ptr == ' ') ptr++; // null terminate PID, ptr = pathname
			else break;
		}
		if (ptr == nullptr) continue;

		/* client FIFO pathname */
		filename.clear();
		filename = FIFO_one_multiple;
		filename += ".client.";
		filename += to_string(cpid);
		if ((fifo_write_fd = open(filename.c_str(), O_WRONLY, 0777)) < 0) {
			err_msg(-1, __LINE__, "open(): %s", filename.c_str());
			continue;
		}
		_debug_log_info("%d", fifo_write_fd)

		if ( (fd = open(ptr, O_RDONLY)) < 0) {
			filename.clear();
			filename += "service[";
			filename += to_string(ppid);
			filename += "] echo me[";
			filename += to_string(cpid);
			filename += "] : can't open \"";
			filename += ptr;
			filename += "\"";
			Close(fifo_write_fd);
		} else {
			while ( (bufsize = Read(fd, buff, MAXLINE)) > 0) {
				Write(fifo_write_fd, buff, bufsize);
			}
			Close(fd);
			Write(fifo_write_fd, "", 1); // 通知结束
			Close(fifo_write_fd);
		}
		_debug_log_info("while() ppid: %d", ppid)
	}
//	}
	_debug_log_info("EXIT ppid: %d", ppid)
	Close(fifo_read_fd);
	unlink(pathname.c_str());// 删除
}

void test_fifo_multiple_client() // 多个 独立 客户端 FIFO（命名管道）: 多对一
{
	pid_t cpid = getpid();
	_debug_log_info("test_fifo_multiple_client(): %d", cpid)

	/******************** service: open **************************/
	string pathname_serv = FIFO_one_multiple;
	pathname_serv += ".service";
	_debug_log_info("open(): %s", pathname_serv.c_str())

	int fifo_write_fd = open(pathname_serv.c_str(), O_WRONLY, 0777);
	_debug_log_info("%d", fifo_write_fd)
	if (fifo_write_fd < 0) {
		Close(fifo_write_fd);
		err_quit(-1, __LINE__, "open(): %s", pathname_serv.c_str());
	}

	/******************* client: Mkfifo + open ******************/
	string pathname = FIFO_one_multiple;
	pathname += ".client.";
	pathname += to_string(cpid);
	Mkfifo(pathname.c_str(), FILE_MODE);
	_debug_log_info("open(): %s", pathname.c_str())

	/* 	每当有一个 client 终止时，将 close(write_fd)
	 	导致 service 的 read(read_fd, x, x)返回 0，表示是一个文件结束符，从而 service 退出；
	 	但是要求 service 不得退出 ！必须用 Open() 创建 FIFO 的 O_RDONLY 且 不使用。 */
	/* 不使用 O_RDWR，防止 open(x, O_RDONLY, x) 阻塞 */
//	int tmp_fd = open(pathname.c_str(), O_RDWR, 0777); // 不使用
//	_debug_log_info("tmp_fd=%d", tmp_fd)
//	int fifo_read_fd = open(pathname.c_str(), O_RDONLY, 0777);
	int fifo_read_fd = open(pathname.c_str(), O_RDWR, 0777);
	_debug_log_info("fifo_read_fd=%d", fifo_read_fd)
//	Close(tmp_fd); // 不使用
	if (fifo_read_fd < 0) {
		Close(fifo_write_fd);
		Close(fifo_read_fd);
		unlink(pathname.c_str()); // 删除
		err_quit(-1, __LINE__, "open(): %s", pathname.c_str());
	}
//	tmp_fd = Fcntl(fifo_read_fd, F_GETFL, 0); 			// 获取 I/O 状态
//	Fcntl(fifo_read_fd, F_SETFL, tmp_fd & (~(int)O_NONBLOCK) ); // 设置 I/O 阻塞

	char buff[MAXLINE];
	snprintf(buff, sizeof(buff), "%d ", cpid);
	ssize_t buflen = strlen(buff);
	char *ptr = buff + buflen;

	cout << "input file: ";
	Read(STDIN_FILENO, ptr, MAXLINE - buflen);

	cout << "send: " << buff << endl;
	Write(fifo_write_fd, buff, strlen(buff));

	while ( (buflen = Read(fifo_read_fd, buff, MAXLINE)) > 0)
		Write(STDOUT_FILENO, buff, buflen);

	Close(fifo_read_fd);
	Close(fifo_write_fd);

	_debug_log_info("EXIT cpid: %d", cpid)
	unlink(pathname.c_str()); // 删除
}

/************************* 自定义 可变长度 消息体 *********************************/

void test_fifo_mesg_service() // 独立 服务器 FIFO（命名管道）自定义 可变长度 消息体
{
	creat_two_fifo();

	int read_fd = Open(FIFO_00, O_RDONLY, 0777);
	_debug_log_info("test_fifo_mesg_service(): read_fd=%d", read_fd)

	sleep(3);

	int write_fd = Open(FIFO_01, O_WRONLY, 0777);
	_debug_log_info("test_fifo_mesg_service(): write_fd=%d", write_fd)

	mesg_service(read_fd, write_fd);
}

void test_fifo_mesg_client() // 独立 客户端 FIFO（命名管道）自定义 可变长度 消息体
{
	int write_fd = Open(FIFO_00, O_WRONLY, 0777); // 阻塞到 FIFO_00 已 O_RDONLY
	_debug_log_info("test_fifo_mesg_client(): write_fd=%d", write_fd)

	sleep(3);

	int read_fd = Open(FIFO_01, O_RDONLY, 0777); // 阻塞到 FIFO_01 已 O_WRONLY
	_debug_log_info("test_fifo_mesg_client(): read_fd=%d", read_fd)

	mesg_client(read_fd, write_fd);
	Close(write_fd);
	Close(read_fd);
	unlink(FIFO_00); // 删除
	unlink(FIFO_01);
}

/******************************************************************************/
