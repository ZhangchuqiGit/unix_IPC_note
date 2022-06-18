
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


/************************** read() write() *******************************/
/* 	#include <unistd.h>
size_t read  ( int fd,       void *buf, size_t nbytes);
size_t write ( int fd, const void *buf, size_t nbytes);
参数：
	fd 		文件描述符
 	buf 	指向一段内存的指针
 	nbytes 	想要读取或者写入的字节数
返回值： 成功返回实际读取或写入的字节数；错误返回 -1.
备注：
 	读取时如在到达文件尾还有30字节，而实际要求读100字节，则read返回30，下次调用read返回0；
 	写入时一般实际写入 nbytes字节，但由于磁盘空间限制或中断等原因，
实际写入的字节数可能会小于 nbytes字节。在unix中，所有的设备都可以看成是一个文件，
所以我们可以用 read() 来读取 socket_fd 数据。       */


/******************************* read() **********************************/

//慢速版-仅用于示例
ssize_t readline_slow(const int &fd, const void *vptr, const size_t &size_val)
{
	char *ptr = (char *)vptr;
	size_t numval, retval;
	char c;
	for (numval = 1; numval < size_val; ++numval) {
again:
		if ((retval = read(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\n') break;	/* newline is stored, like fgets() */
		}
		else if (retval == 0) {
			*ptr = 0;
			return(numval - 1);	/* EOF, n - 1 bytes were read */
		}
		else {
			if (errno == EINTR)	goto again;
			return(-1);		/* error, errno set by read() */
		}
	}
	*ptr = 0;	/* null terminate like fgets() */
	return(numval);
}

/* 任何描述符 单个读缓冲区 */
/* read()函数是负责从fd中读取 最多 nbytes 个字节到buf，
 * 在读取数据时会将最后的回车(\n)同时读入到buf中，
 * 但是 不会在后面加上字符串结束符(\0)。 */
//Read(fd, ptr->mesg_data, ptr->mesg_len+1/*不留结束符位置*/, 0);
ssize_t Read(const int &fd, void *ptr, const size_t &size_val, const uint8_t &mode)
{
	std::cout.flush();
	std::cerr.flush();
	ssize_t retval;
/*	ptr[num]; num = strlen( ptr ) = sizeof( ptr ) / sizeof( T ) */
//	memset(ptr, 0, size_val*sizeof(T));
	memset((char *)ptr, 0, size_val);  /*留个结束符位置*/
//	bzero(ptr, size_val);
	while (true) {
		/* 留个 结束符 位置: read()不会在后面加上字符串结束符(\0)。 */
		retval = read(fd, ptr, size_val-1 /*留个结束符位置*/);
		if (retval > 0) break;
		else if (retval == 0) {
			return 0; // TCP connection closed (TCP 断开) or end of file
		}
		else { /* retval < 0  */
			if (errno == EAGAIN/* O_NONBLOCK : Try again */ ) {
#if 0
				_debug_log_info("read : %s", errno == EINTR ? "EINTR":"EAGAIN")
				continue;
#else
				return -1234;
#endif
			}
			if (errno == EINTR/* Interrupted system call */ ) continue;
			if (errno == ECONNRESET) return 0;/* connection reset by client */
			if (errno == ECHILD) return 0;/* No child processes */
			err_sys(retval, __FILE__, __func__, __LINE__, "read()");
//			return -1;
		}
	}
//	if (retval == 1 && *(char *) ptr == ' ') return 0;
	if (mode == 1 && retval > 1) {
		ptr = (char *) ptr + retval - 1;
		/* 去除 换行符: read()读取数据时会将最后的回车(\n)同时读入到buf中 */
		if (*(char *) ptr == '\n') {
			*(char *) ptr = '\0';
			retval--;
		}
//		/* 加 结束符: read()不会在后面加上字符串结束符(\0)。
//		memset((char *)ptr, 0, size_val); 留个结束符位置	*/
//		else {
//			*((char *)ptr+1) = '\0';
//		}
	}
	return retval;
}

/******************************* write() **********************************/

/* 任何描述符 单个写缓冲区 */
ssize_t Write(const int &fd, const void *ptr, const size_t &size_val)
{
	char *pptr = (char *)ptr;
	size_t nleft = size_val;
	ssize_t	nwritten;
	while (nleft > 0) {
		if ( (nwritten = write(fd, pptr, nleft) ) < 0 ) {
			if (nwritten < 0) {		/* error */
				if (errno == EINTR) nwritten = 0; // and call write() again
				err_sys(nwritten, __FILE__, __func__, __LINE__, "write()");
			}
			else return size_val - nleft;
		}
		nleft -= nwritten;
		pptr  += nwritten;
	}
	return size_val;
}
