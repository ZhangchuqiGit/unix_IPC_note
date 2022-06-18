
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


/**************************** recv() send() ******************************/
/* 	#include <sys/socket.h>
ssize_t recv (int sock_fd,       void *buf, size_t nbytes, int flags);
ssize_t send (int sock_fd, const void *buf, size_t nbytes, int flags);
参数：前 3个参数同 read()、write()
 sock_fd 	套接字描述符
 buf 		指向一段内存的指针
 nbytes 	想要读取或者写入的字节数
------------------------------------------------
 flags	一般设置为 0（同 read、write），要么是一些常值的逻辑或:
	MSG_OOB 		处理带外数据 out-of-band
	MSG_PEEK		偷看传入的消息。
	MSG_DONTROUTE 	取消路由表查询，不要使用本地路由
	MSG_DONTWAIT 	非阻塞IO。设置为不可阻断运作
	MSG_NOSIGNAL 	此动作不愿被 SIGPIPE 信号中断
	MSG_CTRUNC		控制数据在交付前丢失。
	MSG_PROXY		提供或询问第二个地址。
	MSG_EOR		 	记录结束。
	MSG_WAITALL		等待完整的请求。
	MSG_CONFIRM		确认路径有效性。
	MSG_ERRQUEUE	从错误队列中获取消息。
	MSG_NOSIGNAL	不生成 SIGPIPE。
	MSG_MORE		发件人将发送更多。
	MSG_WAITFORONE	等待至少一个数据包返回。
	MSG_BATCH		更多消息即将到来。
	MSG_ZEROCOPY	在内核路径中使用用户数据。
	MSG_FASTOPEN	在TCP SYN中发送数据。
	MSG_CMSG_CLOEXEC	为文件设置close_on_exit
-------------------------------------------------
返回值： 成功返回实际读取或写入的字节数；错误返回 -1.
备注：比 read()、write()多了一个参数 flags，
可以理解为比 read()、write() 操作更细化的函数，但仅用于套接字。       */


/******************************** recv() *********************************/

/* 仅套接字描述符 单个读缓冲区 可选标志 */
ssize_t Recv(const int &fd, void *buf, const size_t &n, const int &flags)
{
	bzero(buf, n);
	ssize_t len = recv(fd, buf, n, flags);
	err_sys(len, __FILE__, __func__, __LINE__, "recv()");
	return len;
}


/******************************** send() *********************************/

/* 仅套接字描述符 单个写缓冲区 可选标志 */
ssize_t Send(const int &fd, const void *buf, const size_t &n, const int &flags)
{
	ssize_t len = send(fd, buf, n, flags);
	err_sys(len, __FILE__, __func__, __LINE__, "send()");
	return len;
}
