
#include "IO_head.h" /* I/O 函数 */

/**	                     5 组 I/O 函数比较
---------------------------------------------------------------------------
    函 数             任何   仅套接字    单个     分散/集中   可选    可选      可选
                     描述符   描述符  读/写缓冲区   读/写     标志   对面地址  控制信息
read, write            *                *
readv, writev          *                           *
recv, send (TCP)               *        *                   *
recvfrom, sendto (UDP)         *        *                   *       *
recvmsg, sendmsg               *                   *        *       *       *
---------------------------------------------------------------------------
 	recvmsg() 和 sendmsg() 函数是“最通用“的I/O函数。
只要设置好参数来读取 socket_fd 数据，
可以把 read()、 readv()、 recv()和 recvfrom() 替换成 recvmsg()调用，
可以把 write()、writev()、send()和 sendto()   替换成 sendmsg()调用。
---------------------------------------------------------------------------
标准文件描述符
标准输入:		STDIN_FILENO  = fileno(stdcin)
标准输出:		STDOUT_FILENO = fileno(stdout)
标准错误输出:	STDERR_FILENO = fileno(stderr)       */

/* 	为什么引出 readv() 和 writev()：
（1）使用read()将数据读到不连续的内存，wirte()将不连续的内存发送出去，要多次调用 ！
如果要从文件中读一片连续的数据至进程的不同区域，有两种方案
① 使用read()一次将它们读至一个较大的缓冲区中，然后将它们分成若干部分复制到不同的区域；
② 调用read()若干次分批将它们读至不同区域。
同样，如果想将程序中不同区域的数据块连续地写至文件，也必须进行类似的处理。
（2）UNIX提供了另外两个函数readv()和writev()，
它们只需一次系统调用就可以实现在文件和进程的多个缓冲区之间传送数据，
免除了多次系统调用或复制数据的开销。             */


/************************** readv() writev() *****************************/
/* 	#include <sys/uio.h>
ssize_t readv  (int fd, const struct iovec *iovec, int count);
ssize_t writev (int fd, const struct iovec *iovec, int count);
参数：
	fd 		文件描述符
	iovec 	指向 iovec{} 结构数组的指针
		struct iovec {  // 数据缓冲区
			void  *io_base;   空间的基地址:数据
			size_t iov_len;   空间的长度:数据长度
		};
	count 	数据缓冲区 iovec{} 结构数组元素个数
返回值： 成功返回实际读取或写入的字节数；错误返回 -1.
备注：
 ① writev() 以顺序 iovec[0]、iovec[1] 至 iovec[count-1] 从各缓冲区中聚集输出数据到 fd
 ② readv() 则将从 fd 读入的数据 按同样的顺序散布到各缓冲区中，
 	readv() 总是先填满第一个缓冲区，然后再填下一个   */


/******************************* readv() *********************************/

/* 任何描述符 分散读（scatter read） */
void Readv_test() // 测试
{
	struct iovec iov[3]; // 数据缓冲区

	char part1[2] = {0};
	iov[0].iov_base = part1;
	iov[0].iov_len = sizeof(part1);

	char part2[2] = {0};
	iov[1].iov_base = part2;
	iov[1].iov_len = sizeof(part2);

	char part3[2] = {0};
	iov[2].iov_base = part3;
	iov[2].iov_len = sizeof(part3);

	/* ② readv() 则将从 fd 读入的数据 按同样的顺序散布到各缓冲区中，
 	readv() 总是先填满第一个缓冲区，然后再填下一个   */
	int retval = readv(STDIN_FILENO, iov, std::size(iov));
	std::cout << "readv\t" << retval << std::endl;
	/*	$ 0123456
		readv	6
		part1: 012345
		part2: 2345
		part3: 45       */
#if 1
	printf("part1: %s\n", part1);
	printf("part2: %s\n", part2);
	printf("part3: %s\n", part3);
#else
	std::cout << "part1\t" << part1 << std::endl;
	std::cout << "part2\t" << part2 << std::endl;
	std::cout << "part3\t" << part3 << std::endl;
#endif
}

/* 任何描述符 分散读（scatter read） */
int Readv()
{
	return  0;
}

/******************************* writev() ********************************/

/* 任何描述符 集中写（gather write）*/
void Writev_test() // 测试
{
	struct iovec iov[3]; // 数据缓冲区

	char part1[] = "This is iov";
	iov[0].iov_base = part1;
	iov[0].iov_len = strlen(part1);

	char part2[] = " and ";
	iov[1].iov_base = part2;
	iov[1].iov_len = strlen(part2);

	char part3[] = " writev test\n";
	iov[2].iov_base = part3;
	iov[2].iov_len = strlen(part3);

	int retval = writev(fileno(stdout), iov, std::size(iov));
	std::cout << "writev\t" << retval << std::endl;
}

/* 任何描述符 集中写（gather write） */
int Writev()
{
	return 0;
}
