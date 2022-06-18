//
// Created by zcq on 2021/5/10.
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

#include "pipe_test.h"

#include <iostream>
using namespace std;

/**********************	管道 创建 步骤 **********************/
/*	实现父子进程间0通信步骤:
1.父进程调用 pipe()函数创建管道，得到两个文件描述符 fd[0]、fd[1] 指向管道的 读端 和 写端。
2.父进程调用 fork()创建子进程，那么子进程也有两个文件描述符指向同一管道。
3.父进程 关闭 管道 读端，子进程 关闭 管道 写端。
  父进程 可以向管道中 写入 数据，子进程 将管道中的数据 读出。
  由于管道是利用环形队列实现的，数据从写端流入管道，从读端流出，这样就实现了进程间通信。 */

/*	int pipe(int filedes[2]); // 建立管道
filedes[0]:读取端;
filedes[1]:写入端。
返回值: 成功返回 0，否则返回 -1，错误原因存于 errno 中。
常见错误代码:
    EMFILE：进程 已用完 文件描述符 fd 最大数量（已无 文件描述符 可用）。
    EFAULT：参数 filedes 数组地址 不合法	 */
void test_process_pipe() // 建立父子进程间通信的管道：先 父写 子读，后 子写 父读
{
	int fd1[2]; // 文件描述符
	int fd2[2]; // 文件描述符
	if (pipe(fd1) == -1) // 创建管道
	{
		perror("pipe");
		exit(1);
	}
	if (pipe(fd2) == -1) // 创建管道
	{
		perror("pipe");
		exit(1);
	}
	cout << "fd1 " << fd1[0] << endl
		 << "fd1 " << fd1[1] << endl
		 << "fd2 " << fd2[0] << endl
		 << "fd2 " << fd2[1] << endl;

	pid_t pid = fork(); // 创建 子进程
	if(pid == -1) // 负值 : 创建子进程失败。
	{
		cerr << "创建子进程失败，pid: " << pid << endl;
		exit(1);
	}
	else if (pid==0) // 0 : 当前是 子进程 运行。
	{
		sleep(1);
		cout <<"当前是子进程运行，fifo_client ID: "<< getpid() <<endl;

/* 先 父写 子读 */
		close(fd1[1]); //关闭 写 描述符
		char buf[128];
		memset(buf, 0, sizeof(buf)); // 清空.
		int len = read(fd1[0], buf, sizeof(buf));  //等待管道上的数据
		cout << "fifo_client process wait to read: " << buf << "\tlen: " << len << endl;
		close(fd1[0]); //关闭 读 描述符
//		write(STDOUT_FILENO, buf, len);

/* 后 子写 父读 */
		close(fd2[0]); // 关闭 读 描述符
		string buf1 = "qcz";
		printf("fifo_client process send: %s\n", buf1.c_str());
		write(fd2[1], buf1.c_str(), buf1.size());   //向管道写入字符串数据
		close(fd2[1]); //关闭 写 描述符
	}
	else // 正值 : 当前是 父进程 运行。正值 为 子进程 的 进程 ID。
	{
		pid_t pid_parent = getpid();
		cout << "当前是父进程运行，parant ID: " << pid_parent
			 << ", fifo_client ID: " << pid << endl;

/* 先 父写 子读 */
		close(fd1[0]); // 关闭 读 描述符
		string buf2 = "zcq";
		printf("fifo_service process send: %s\n", buf2.c_str());
		write(fd1[1], buf2.c_str(), buf2.size());   //向管道写入字符串数据
//		write(fd[1], buf, strlen(buf));   //向管道写入字符串数据
		close(fd1[1]); //关闭 写 描述符

/* 后 子写 父读 */
		close(fd2[1]); //关闭 写 描述符
		char buf[128];
		memset(buf, 0, sizeof(buf)); // 清空.
		int len = read(fd2[0], buf, sizeof(buf));  //等待管道上的数据
		cout << "fifo_service process wait to read: " << buf << "\tlen: " << len << endl;
		close(fd2[0]); //关闭 读 描述符

		pid = wait(nullptr); // 阻塞自己 等待 有一个 子进程 死(结束)。
		cout << "Child process (pid:" << pid << ") is finish." << endl;
	}
}


/*	popen() 函数较于 system() 函数的优势在于使用简单，
 	popen() 可以执行 shell 命令 : $ sh -c command
------------------------------------------------------------------
	FILE *popen (const char *command , const char *modes);
 		// 创建一个运行给定命令的管道(标准I/O流)，子进程 执行 shell 命令 <stdio.h>
	int	pclose (FILE * stream); // 关闭 popen()创建的管道，并返回其子级的状态 <stdio.h>
------------------------------------------------------------------
参数 modes 只能是 读 或者 写 中的一种，
 	得到的返回值（标准I/O流）也具有和 modes 相应的 只读 或 只写 类型。
	如果 modes 是 "r" 则返回值(文件指针)连接到 shell 的 标准输出；
	如果 modes 是 "w" 则返回值(文件指针)连接到 shell 的 标准输入。
参数 command 参数是一个指向以 NULL 结束的 shell 命令 的 字符串指针
------------------------------------------------------------------
返回值：
 	成功返回 子进程的 status，使用 WIFEXITED WIFSIGNALED WIFSTOPPED 相关宏可得返回结果
	不能分配内存将返回 NULL；如果调用 fork() 或 pipe() 失败返回 -1，
 	我们可以使用 perror 或 strerror(errno) 得到有用的错误信息，
------------------------------------------------------------------
popen() 函数通过创建一个运行给定命令的管道，调用 fork()产生一个子进程，执行一个 shell 命令。
这个进程必须由 pclose() 函数关闭。
pclose() 函数关闭 popen()创建的管道(标准I/O流)，等待命令执行结束，返回 shell 的终止状态。
如果 shell 不能被执行，则 pclose() 返回的终止状态与 shell 已执行 exit 一样。*/
//void process_popen_pclose_debug(const char *cmd_shell, const char *modes) // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令
void process_popen_pclose_debug(const char *cmd_shell) // 创建一个运行给定命令的管道(标准I/O流)，等待子进程 执行 shell 命令
{
	cout << "================================" << endl;
	if (cmd_shell == nullptr) // 如果 cmd_shell 为空
	{
		printf("cmd_shell 为 空\n");
		return;
	}
	cout << "popen(): " << cmd_shell << endl;

	FILE *fp  = popen(cmd_shell, "r"); // 等待 子进程 执行 shell 命令
	// "r" 则返回值(文件指针)连接到 shell 的 标准输出；
	// "w" 则返回值(文件指针)连接到 shell 的 标准输入。
	if(!fp)
	{
		perror("popen() : shell create error");

		/* C 库函数 char *strerror(int errno) // <string.h>
		 * 从内部数组中搜索错误号 errno，并返回一个指向错误消息字符串的指针。
		 * strerror() 生成的错误字符串的内容取决于开发平台和编译器。 */
		printf("CMD error ! cmd_shell : %s\n", cmd_shell);
		printf("strerror(errno) : %s\n", strerror(errno));
		printf("errno : %d\n", errno);

		exit(EXIT_FAILURE); // Failing exit status.
	}
	if (WIFEXITED(errno)) //取得 cmd_shell 执行结果
	{
		printf("normal termination, exit status = %d\n",
			   WEXITSTATUS(errno));
	}
	else if (WIFSIGNALED(errno)) // 如果 cmd_shell 被 信号 中断
	{
		printf("abnormal termination, signal number =%d\n",
			   WTERMSIG(errno)); // 取得 信号值
	}
	else if (WIFSTOPPED(errno))    //如果 cmd_shell 被 信号 暂停 执行
	{
		printf("process stopped, signal number =%d\n",
			   WSTOPSIG(errno)); // 取得信号值
	}

	/* "r" 标准输出 */
	cout << "fileno(fp): " << fileno(fp) /*转化文件描述符*/ << endl;
	cout << "-------------------------------- 父进程" << endl;
	char buf[2];
	memset(buf, 0, sizeof(buf)); // 清空
	while( fgets(buf, std::size(buf), fp) != nullptr )
		// 父进程 获取 子进程 shell 终端输出
	{
		printf("%s", buf); // 打印终端输出的每一行内容
		memset(buf, 0, sizeof(buf)); // 清空
	}
	cout << "-------------       ------------" << endl;

	int ret = pclose(fp); // 关闭 popen()创建的管道，并返回其子级的状态
	cout << "shell 终止状态 : exit(" << ret << ")" << endl;
	if (ret != 0)
	{
		perror("popen() : shell create error");

		/* C 库函数 char *strerror(int errno) // <string.h>
		 * 从内部数组中搜索错误号 errno，并返回一个指向错误消息字符串的指针。
		 * strerror() 生成的错误字符串的内容取决于开发平台和编译器。 */
		printf("CMD error : %s\n", cmd_shell);
		printf("strerror(errno) : %s\n", strerror(errno));
		printf("errno : %d\n", errno);

		exit(ret); // Failing exit status.
	}
}

void test_popen_pclose() // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令
{
	process_popen_pclose_debug(nullptr);
	sleep(1);
	process_popen_pclose_debug("ls -al ../");
	sleep(1);
	process_popen_pclose_debug("pwd");
	sleep(1);
	process_popen_pclose_debug("echo \"${USER}\"");
	sleep(1);
	string shell = R"(echo "\033[1m\033[5m\033[32m012345\e[0m6789")"; // echo 默认带 -e
	process_popen_pclose_debug(shell.c_str());
	sleep(1);
	process_popen_pclose_debug("../process/test.sh zcq");
	sleep(1);
	process_popen_pclose_debug("../process/test.sh qcz &"); // & 后台 shell 命令
	sleep(1);
	process_popen_pclose_debug("g++ ../process/test.cpp -o ../process/test.o");
	sleep(1);
	process_popen_pclose_debug("../process/test.o zcq");
	sleep(1);
	process_popen_pclose_debug("~/.config/clash/clash-linux-amd64*");
}
