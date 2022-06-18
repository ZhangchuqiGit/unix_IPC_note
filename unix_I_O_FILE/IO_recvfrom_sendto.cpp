
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


/************************* recvfrom() sendto() ***************************/
/* 	#include <sys/socket.h>
ssize_t recvfrom (int sock_fd, void *buf, size_t nbytes, int flags,
 							   struct sockaddr *from_addr, socklen_t *addr_len);
ssize_t sendto   (int sock_fd, void *buf, size_t nbytes, int flags,
 							   struct sockaddr *to_addr,   socklen_t addrlen);
参数：前 3个参数同 read()、write()，
	sock_fd	套接字描述符
 	buf 	指向一段内存的指针
 	nbytes 	想要读取或者写入的字节数
 	flags	要么是 0（同 read、write），要么是一些常值的逻辑或；常值及意义参阅头文件。
	from_addr\to_addr 存放对端地址结构，addrlen 地址结构长度。
返回值： 成功返回实际读取或写入的字节数；错误返回 -1.
备注：
（1）注意最后一个参数，recvfrom()是指向整数值的指针，sendto()是一个整数值；
 recvfrom()的最后一个参数是调用 recvfrom()时返回的对端地址结构长度，
 只有是指针型，我们才能够方便地使用该长度变量。
 一般这样使用：
 	struct sockaddr_in6 addr{};
	socklen_t addr_len = sizeof(struct sockaddr_in6);
	sendto  (sockfd,buf,sizeof(buf),0,(struct sockaddr *)&addr, addr_len);
	recvfrom(sockfd,buf,sizeof(buf),0,(struct sockaddr *)&addr,&addr_len);
（2）对于 UDP，recvfrom()返回 0 是可以接受的；而 TCP 的 read()返回 0 则表示对端关闭连接。
（3）如果不关心对端的协议地址，可以将 recvfrom()的最后两个参数置为空指针。
（4）recvfrom()和 sendto()都可以用于TCP，但通常不这么做。   */


/****************************** recvfrom() *******************************/

/* 仅套接字描述符 单个读缓冲区 可选标志 可选对面地址*/

void recv_all(const int &sock_fd, const socklen_t &salen)
{
//	struct sockaddr_storage sas{};
	auto uniqueptr = std::make_unique<char[]>(salen+1);
	auto *sa = (struct sockaddr *)uniqueptr.get();
	ssize_t num;
	char buf[MAXLINE+1] = {'\0'};
	socklen_t len;
	while (true) {
		len = salen;
		num = recvfrom(sock_fd, buf, MAXLINE, 0, sa, &len);
		if (num == -1) std::cerr << "recvfrom(): error" << std::endl;
		else if (num == 0) break;
		else {
			buf[num] = 0;	/* null terminate */
			std::cout << buf << std::endl;
		}
	}
	uniqueptr.reset();
	sa = nullptr;
}


/******************************* sendto() ********************************/

/* 仅套接字描述符 单个写缓冲区 可选标志 可选对面地址*/

#include <sys/utsname.h>

#define	SENDRATE 	5		/* send one datagram every 5 seconds */

void send_all(const int &sock_fd,
			  const struct sockaddr *sa, const socklen_t &salen)
{
	struct utsname sys_info{}; // 系统和机器的描述
	err_sys(uname(&sys_info)/* 以名称汇总系统的信息 */, __FILE__,
			__func__, __LINE__, "%s():uname()", __func__);
	std::string strbuf = "--------- NO:00";
	strbuf += "\npid: ";
	strbuf += std::to_string(getpid());
	strbuf += "\n操作系统的执行名称: ";
	strbuf += sys_info.sysname ;
	strbuf += "\n网络上此节点的名称: ";
	strbuf += sys_info.nodename;
	strbuf += "\n此实现的当前发行版: ";
	strbuf += sys_info.release;
	strbuf += "\n此版本的当前版本级别: ";
	strbuf += sys_info.version;
	strbuf += "\n系统正在运行的硬件类型的名称: ";
	strbuf += sys_info.machine;
	strbuf += "\n网络上此节点的域的名称: ";
	strbuf += sys_info.domainname;
	strbuf += "\n";
	size_t num = 0;
	const size_t pos = strbuf.find_first_of("NO:");
	while (true) {
		++num;
		if (num < 10) {
			strbuf[pos+4] = num%10+'0';
			if (num >= 10) strbuf[pos+3] = num/10+'0';
		} else {
			strbuf = "";
			sendto(sock_fd, strbuf.c_str(), strbuf.size(), 0, sa, salen);
			/* 此处父进程不能调用 close()，用 shutdown()发送的 FIN 使子进程退出 */
			err_ret(shutdown(sock_fd, SHUT_WR), __LINE__, "shutdown()");
			break;
		}
		sendto(sock_fd, strbuf.c_str(), strbuf.size(), 0, sa, salen);
		sleep(SENDRATE);
	}
}
