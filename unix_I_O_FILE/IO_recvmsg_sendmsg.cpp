
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


/************************ recvmsg() sendmsg() ****************************/
/* #include <sys/types.h>, <sys/socket.h>
ssize_t recvmsg (int sock_fd, const struct msghdr *msg, int flags);
ssize_t sendmsg (int sock_fd, const struct msghdr *msg, int flags);
返回：若成功则为读入或写出的字节数，若出错则为 -1
---------------------------------------------------------------------------
sock_fd：套接字描述符
---------------------------------------------------------------------------
struct msghdr {
	void 		 *msg_name;		协议地址，是 数据报套接口 UDP 才需要设置，
						如 套接字地址结构 sockaddr_in6{}
	socklen_t 	 msg_namelen;	协议地址的大小，如 sizeof(struct sockaddr_in6)
	struct iovec *msg_iov;		指定数据缓冲区数组，即 iovec{} 结构数组
	size_t 		 msg_iovlen;	msg_iov 中的元素长度
	void 		 *msg_control;	辅助数据（控制信息）通常指向 cmsghdr{}
	size_t 		 msg_controllen;辅助数据的大小
 						类型应是socklen_t，但内核的定义与此不兼容
	int 		 msg_flags;		recvmsg()返回的标志
};
--------------------------------
	msg_name 和 msg_namelen 这两个成员用于 套接字未连接 的场合，如未socket()。
它们类似 recvfrom 和 sendto 的第五个和第六个参数：
    msg_name 指向一个套接字地址结构，调用者在其中存放接收者（对于 sendmsg 调用）
或发送者（对于 recvmsg 调用）的协议地址。
如果无需指明协议地址（如对于 TCP套接字 或 已连接 UDP套接字），msg_name 应为 空指针。
--------------------------------
	msg_iov 和 msg_iovlen 这两个成员指定输入或输出缓冲区数组（即 iovec{} 结构数组），
类似 readv 或 writev 的第二个和第三个参数。
--------------------------------
	msg_control 和 msg_controllen 这两个成员指定可选的辅助数据的位置和大小。
---------------------------------------------------------------------------
标志：
    一个是传递值的 flags 参数；
    另一个是所传递 msghdr{} 结构的 msg_flags 成员，它传递的是引用
--------------------------------
   只有 recvmsg 使用 msg_flags 成员。
recvmsg() 被调用时，flags 参数被复制到 msg_flags 成员，并由内核使用其值驱动接收处理过程。
内核还依据 recvmsg() 的结果 更新 msg_flags 成员的值：
 标记位        描述
MSG_EOR 	 当接收到记录结尾时会设置这一位。
			这通常对于 SOCK_SEQPACKET（socket_type）套接口类型十分有用。
MSG_TRUNC    这个标记位表明数据的结尾被截短，因为接收缓冲区太小不足以接收全部的数据。
MSG_CTRUNC   这个标记位表明某些控制数据(附属数据)被截短，因为缓冲区太小。
MSG_OOB      这个标记位表明接收了带外数据。
MSG_ERRQUEUE 这个标记位表明没有接收到数据，但是返回一个扩展错误。
--------------------------------
	sendmsg 则忽略 msg_flags 成员，因为它直接使用 flags 参数驱动发送处理过程。
这一点意味着如果想在某个 sendmsg() 调用中设置 MSG_DONTWAIT 标志，
那就把 flags 参数设置为该值，把 msg_flags 成员设置为该值不起作用。
---------------------------------------------------------------------------
struct iovec {  // 数据缓冲区
	void  *io_base;   空间的基地址:数据
	size_t iov_len;   空间的长度:数据长度
};
struct cmsghdr {  // 辅助数据（控制信息）   sizeof(struct cmsghdr)=16
	socklen_t cmsg_len;  	包含该头部的数据长度(不包括 填充数据部分)
    int  	  cmsg_level;   具体的协议标识，如 IPPROTO_IP，IPPROTO_IPV6，SOL_SOCKET
    int  	  cmsg_type;    协议中的类型，
				如 SOL_SOCKET 中 SCM_RIGHTS(发送接收描述字)、SCM_CREDS(发送接收用户凭证)
};
...... 两个 辅助数据 之间的 填充字节 + 数据 + 填充字节
 struct cmsghdr ...... 下一个 辅助数据（控制信息）
--------------------------------
               辅助数据 的用途
	协议	cmsg_level		cmsg_type		说明
	IPv4	IPPROTO_IP		IP_RECVDSTADDR	随UDP数据报接收目的的地址
							IP_RECVIF		随UDP数据报接收接口的索引
	IPv6	IPPROTO_IPV6	IPV6_DSTOPTS	指定/接收目的地选项
							IPV6_HOPLIMIT	指定/接收跳限
							IPV6_HOPOPTS	指定/接收步跳选项
							IPV6_NEXTHOP	指定下一跳地址
							IPV6_PKTINFO	指定/接收分组信息
							IPV6_PTHDR		指定/接收路由首部
							IPV6_TCLASS		指定/接收分组流通类别
	Unix域	SOL_SOCKET		SCM_RIGHTS		发送/接收描述符
							SCM_CREDS		发送/接收用户凭证
--------------------------------
 辅助数据 cmsghdr{} 的 函数 说明：
 #include <sys/socket.h>，<sys/param.h>

 struct cmsghdr * CMSG_FIRSTHDR (struct msghdr *mhdrptr);
    //返回：指向 msghdr{} 第一个 辅助数据 cmsghdr{} 结构的指针，若无辅助数据对象则为 NULL

 struct cmsghdr * CMSG_NXTHDR (struct msghdr *mhdrptr, struct cmsghdr *cmsghdr);
    //返回：指向 msghdr{} 下一个 辅助数据 cmsghdr{} 结构的指针，若无辅助数据对象则为 NULL

 unsigned char * CMSG_DATA (struct cmsghdr *cmsgptr);
    //返回：指向与 辅助数据 cmsghdr{} 结构关联的数据的第一个字节的指针

 int CMSG_LEN (int length);//返回：cmsghdr{}的 cmsg_len 成员值，
 	包含该头部的数据长度，不包括 两个辅助数据之间的 填充(字节)数据部分
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

 int CMSG_SPACE (int length);//返回：辅助数据对象所占用的字节数。
 	CMSG_SPACE() 会包含 两个辅助数据之间的 填充(字节)数据部分。
	传入参数 length 指的是一个 控制信息元素(即 cmsghdr{}) 后面部分的字节数，
	返回的是这个控制信息的总的字节数，即包含了 cmsghdr{}各成员、填充(字节)数据 的总和。
--------------------------------
	CMSG_LEN() 和 CMSG_SPACE() 的区别在于 后者会包含两个辅助数据之间的填充字节。
 CMSG_LEN (sizeof(int))	 	20
 CMSG_SPACE (sizeof(int)) 	24
--------------------------------
 msghdr{}、cmsghdr{} 数据之间的关系:
 msghdr{} ----------------- *msg_control
	|						|
	|		cmsghdr{} -----	cmsg_len --------------------
	|						cmsg_level		|			|
	|						cmsg_type		cmsg_len	|
 	|						__cmsg_data[]	|			|
 	|						(填充字节+数据)	|			CMSG_SPACE() 辅助数据对象
 msg_controllen				-----------------			所占用的字节数 msg_controllen				填充字节(两个辅助数据之间)		|
	|						填充字节(两个辅助数据之间)		|
	|		cmsghdr{} -----	cmsg_len --------------------
	|						cmsg_level		|			|
	|						cmsg_type		|			|
 	|						__cmsg_data[]	CMSG_LEN()	|
 	|						(填充字节+数据)	|			CMSG_SPACE() 辅助数据对象
	|						-----------------			所占用的字节数 msg_controllen				填充字节(两个辅助数据之间)		|
	|						填充字节(两个辅助数据之间)		|
	|		cmsghdr{} -----	cmsg_len --------------------
	...						...
---------------------------------------------------------------------------
char contorl[CMSG_SPACE(size_of_struct1) + CMSG_SPACE(size_of_struct2)];
struct msghdr 	msg;
struct cmsghdr 	*cmsptr;
for(cmsptr=CMSG_FIRSTHDR(&msg); cmsptr!=NULL; cmsptr=CMSG_NXTHDR(&msg, cmsptr) )
{
    // 判断是否是自己需要的 msg_level 和 msg_type
	u_char *ptr;
	ptr = CMSG_DATA(cmsptr); // 获取辅助数据
	CMSG_DATA()指向与辅助数据 cmsghdr{}关联的数据 cmsghdr.cmsg_data[]的第一个字节指针
}                  */

/*************************** 通讯数据结构 配置 *****************************/

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr(struct msghdr &msghdr_s,	 /** 通讯数据结构 */
						   void *address,	  		 // 协议地址
						   const socklen_t &addr_len,// 协议地址的大小
						   void *data_base,			/* 数据缓冲区:数据 */
						   const size_t &data_len   /* 数据缓冲区:数据长度 */ )
{
//	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	msghdr_s.msg_name    = address; 	// 协议地址 address.
	msghdr_s.msg_namelen = addr_len;	// 协议地址的大小
/**----------------------------------**/
	struct iovec iovec_s[1]; 			/** 数据缓冲区数组 */
	iovec_s[0].iov_base = data_base;	/* 数据缓冲区:数据 */
	iovec_s[0].iov_len  = data_len;		/* 数据缓冲区:数据长度 */
	msghdr_s.msg_iov    = iovec_s; 		// 指定 数据缓冲区数组
	msghdr_s.msg_iovlen = 1;			// msg_iov 中的元素长度
/**----------------------------------**/
#if SUPPORT_msg_control
	/**	在任意时刻，union 中只能有一个数据成员可以有值。
	 	union 成员共享同一块大小的内存，对某一个成员赋值，
	 	会覆盖其他成员的值，一次只能使用其中的一个成员。
	 	当给联合中某个成员赋值之后，该联合中的其它成员就变成未定义状态了。
	 	联合的存储空间至少能够容纳其最大的数据成员，其大小由最大的成员的大小决定*/
	union { 							/** 对齐数据 **/
		struct cmsghdr cmsghdr_s{}; 	/* 辅助数据 sizeof(struct cmsghdr)=16 */
		char control[ CMSG_SPACE( sizeof(int) ) ];
/* 	CMSG_LEN() 和 CMSG_SPACE() 的区别在于 后者会包含两个辅助数据之间的填充字节。
 	sizeof (struct cmsghdr) 	16
	CMSG_LEN (sizeof(int))	 	20 = 16 + 4(int)
	CMSG_SPACE (sizeof(int)) 	24 = 16 + 4(int) + 4(int, 填充字节)	  */
	} cmsg_control{};
	msghdr_s.msg_control = (caddr_t) &cmsg_control;			// 辅助数据
	msghdr_s.msg_controllen = sizeof(cmsg_control);			// 辅助数据的大小
//	msghdr_s.msg_control = cmsg_control.control; 			// 辅助数据
//	msghdr_s.msg_controllen = sizeof(cmsg_control.control);	// 辅助数据的大小
/**	实际 union cmsg_control {} 层次：
 msghdr{} ----------------- *msg_control
	|						|
	|		cmsghdr{} -----	cmsg_len
	|						cmsg_level
	|						cmsg_type
 	|						int ( __cmsg_data[] 填充字节+数据)
 msg_controllen				-----------------
 	|						int 填充字节 (两个辅助数据之间)          **/
#else
	int	newfd;
	msghdr_s.msg_accrights = (caddr_t) &newfd;		// 辅助数据
	msghdr_s.msg_accrightslen = sizeof(int);		// 辅助数据的大小
#endif
}

/****---------------------------- recvmsg() --------------------------****/

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 recv_fd 单个读/写缓冲区 */
ssize_t read_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				int &recv_fd)
{
	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	struct_msghdr_cmsghdr(msghdr_s,		// 通讯数据结构
						  nullptr,	// 协议地址
						  0,		// 协议地址的大小
						  ptr,			/* 数据缓冲区:数据 */
						  nbytes		/* 数据缓冲区:数据长度 */ );
	/**-----------------------------------------------**/
	ssize_t retval;
	if ((retval = recvmsg(sock_fd, &msghdr_s, 0)) <= 0) return(retval);
	/**-----------------------------------------------**/
#if SUPPORT_msg_control
	struct cmsghdr *cmsghdr_ptr;		/* 辅助数据 */
	/* CMSG_FIRSTHDR() 指向 msghdr{}第一个辅助数据 cmsghdr{}结构的指针 */
	if ( (cmsghdr_ptr = CMSG_FIRSTHDR(&msghdr_s)) != nullptr &&
		 cmsghdr_ptr->cmsg_len == CMSG_LEN(sizeof(int)) )/** 一个 int 数据 */
	{
/*	协议	cmsg_level		cmsg_type		说明
	Unix域	SOL_SOCKET		SCM_RIGHTS		发送/接收描述符
							SCM_CREDS		发送/接收用户凭证  */
		if (cmsghdr_ptr->cmsg_level != SOL_SOCKET) // 不是 Unix域
			err_quit(-1, __FILE__, __func__, __LINE__,
					 "control level != SOL_SOCKET (不是 Unix域)");
		if (cmsghdr_ptr->cmsg_type != SCM_RIGHTS) // 不是 发送/接收描述符
			err_quit(-1, __FILE__, __func__, __LINE__,
					 "control type != SCM_RIGHTS (不是 发送/接收描述符)");
/* CMSG_DATA()指向与辅助数据 cmsghdr{}关联的数据 cmsghdr.cmsg_data[]的第一个字节指针 */
		recv_fd = *((int *)CMSG_DATA(cmsghdr_ptr));		/** 一个 int 数据 */
	} else recv_fd = -1;		/* descriptor was not passed */
#else
	if (msghdr_s.msg_accrightslen == sizeof(int))
		*recvfd = newfd;
	else
		*recvfd = -1;		/* descriptor was not passed */
#endif
	return(retval);
}

/* recvmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 recv_fd 单个读/写缓冲区 */
ssize_t Read_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				int &recv_fd)
{
	ssize_t	retval = read_fd(sock_fd, ptr, nbytes, recv_fd);
	err_sys(retval, __FILE__, __func__, __LINE__,	"read_fd()");
	return(retval);
}

/****---------------------------- sendmsg() --------------------------****/

/* sendmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 send_fd 单个读/写缓冲区 */
ssize_t write_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				 const int &send_fd)
{
	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	struct_msghdr_cmsghdr(msghdr_s,		// 通讯数据结构
						  nullptr,	// 协议地址
						  0,		// 协议地址的大小
						  ptr,			/* 数据缓冲区:数据 */
						  nbytes		/* 数据缓冲区:数据长度 */ );
	/**-----------------------------------------------**/
	struct cmsghdr *cmsghdr_ptr;		/* 辅助数据 */
	/* CMSG_FIRSTHDR() 指向 msghdr{}第一个辅助数据 cmsghdr{}结构的指针 */
	cmsghdr_ptr = CMSG_FIRSTHDR(&msghdr_s);
	/*  CMSG_LEN (len) = len + CMSG_ALIGN (sizeof (struct cmsghdr) ) */
	cmsghdr_ptr->cmsg_len = CMSG_LEN(sizeof(int)); /** send_fd 类型是int，设置长度 */
/*	协议	cmsg_level		cmsg_type		说明
	Unix域	SOL_SOCKET		SCM_RIGHTS		发送/接收描述符
							SCM_CREDS		发送/接收用户凭证  */
	cmsghdr_ptr->cmsg_level = SOL_SOCKET;
	cmsghdr_ptr->cmsg_type = SCM_RIGHTS;
/* CMSG_DATA()指向与辅助数据 cmsghdr{}关联的数据 cmsghdr.cmsg_data[]的第一个字节指针 */
	*((int *)CMSG_DATA(cmsghdr_ptr)) = send_fd;			/** 添加 一个 int 数据 */

	return sendmsg(sock_fd, &msghdr_s, 0);
}

/* sendmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 send_fd 单个读/写缓冲区 */
ssize_t Write_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				 const int &send_fd)
{
	ssize_t	retval = write_fd(sock_fd, ptr, nbytes, send_fd);
	err_sys(retval, __FILE__, __func__, __LINE__,	"write_fd()");
	return(retval);
}

/*************************************************************************/
/*************************************************************************/

/*************************** 通讯数据结构 配置 *****************************/

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr_01(struct msghdr &msghdr_s,	/** 通讯数据结构 */
							  void *address,	  		// 协议地址
							  const socklen_t &addr_len,// 协议地址的大小
							  void *data_base,		/* 数据缓冲区:数据 */
							  const size_t &data_len/* 数据缓冲区:数据长度 */ )
{
//	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	msghdr_s.msg_name    = address; 	// 协议地址 address.
	msghdr_s.msg_namelen = addr_len;	// 协议地址的大小
/**----------------------------------**/
	struct iovec iovec_s[1]; 			/** 数据缓冲区数组 */
	iovec_s[0].iov_base = data_base;	/* 数据缓冲区:数据 */
	iovec_s[0].iov_len  = data_len;		/* 数据缓冲区:数据长度 */
	msghdr_s.msg_iov    = iovec_s; 		// 指定 数据缓冲区数组
	msghdr_s.msg_iovlen = 1;			// msg_iov 中的元素长度
/**----------------------------------**/
#if SUPPORT_msg_control
	/**	在任意时刻，union 中只能有一个数据成员可以有值。
	 	union 成员共享同一块大小的内存，对某一个成员赋值，
	 	会覆盖其他成员的值，一次只能使用其中的一个成员。
	 	当给联合中某个成员赋值之后，该联合中的其它成员就变成未定义状态了。
	 	联合的存储空间至少能够容纳其最大的数据成员，其大小由最大的成员的大小决定*/
	union { 							/** 对齐数据 **/
		struct cmsghdr cmsghdr_s{}; 	/* 辅助数据 sizeof(struct cmsghdr)=16 */
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
		char control [CMSG_SPACE(sizeof(struct in_pktinfo))];
		/**	实际 union cmsg_control {} 层次：
 msghdr{} ----------------- *msg_control
	|						|
	|		cmsghdr{} -----	cmsg_len
	|						cmsg_level
	|						cmsg_type
 	|						in_pktinfo{} ( __cmsg_data[] 填充字节+数据)
 msg_controllen				-----------------
  	|		in_pktinfo{} 填充字节 (两个辅助数据之间)          **/
#else						/* 自定义 报文(UDP)相关控制信息 */
		char control [CMSG_SPACE(sizeof(struct in_addr)) +
					  CMSG_SPACE(sizeof(struct unp_in_pktinfo)) ];
		/**	实际 union cmsg_control {} 层次：
 msghdr{} ----------------- *msg_control
	|						|
	|		cmsghdr{} -----	cmsg_len
	|						cmsg_level
	|						cmsg_type
 	|						in_addr{} ( __cmsg_data[] 填充字节+数据)
 msg_controllen				-----------------
 	|		in_addr{} 填充字节 (两个辅助数据之间)
 	|		cmsghdr{} -----	cmsg_len
	|						cmsg_level
	|						cmsg_type
 	|						unp_in_pktinfo{} ( __cmsg_data[] 填充字节+数据)
	| 						-----------------
 	|		unp_in_pktinfo{} 填充字节 (两个辅助数据之间)          **/
#endif
/* 	CMSG_LEN() 和 CMSG_SPACE() 的区别在于 后者会包含两个辅助数据之间的填充字节。
 	sizeof (struct cmsghdr) 	16
	CMSG_LEN (sizeof(int))	 	20 = 16 + 4(int)
	CMSG_SPACE (sizeof(int)) 	24 = 16 + 4(int) + 4(int, 填充字节)	  */
	} cmsg_control{};
	msghdr_s.msg_control = (caddr_t) &cmsg_control;			// 辅助数据
	msghdr_s.msg_controllen = sizeof(cmsg_control);			// 辅助数据的大小
//	msghdr_s.msg_control = cmsg_control.control; 			// 辅助数据
//	msghdr_s.msg_controllen = sizeof(cmsg_control.control);	// 辅助数据的大小
#else
	int	newfd;
	msghdr_s.msg_accrights = (caddr_t) &newfd;		// 辅助数据
	msghdr_s.msg_accrightslen = sizeof(int);		// 辅助数据的大小
#endif
}

/****---------------------------- recvmsg() --------------------------****/

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 接收标志、目的IP地址、接口/网口索引 单个读/写缓冲区 */
ssize_t recvmsg_flags(const int &sock_fd, void *ptr, const size_t &nbytes,
					  int &flags, struct sockaddr *sa, socklen_t &salenptr,
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
					  struct in_pktinfo *pktinfo)
#else						/* 自定义 报文(UDP)相关控制信息 */
struct unp_in_pktinfo *pktinfo)
#endif
{
	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	struct_msghdr_cmsghdr_01(msghdr_s,	// 通讯数据结构
							 sa,		// 协议地址
							 salenptr,	// 协议地址的大小
							 ptr,		/* 数据缓冲区:数据 */
							 nbytes		/* 数据缓冲区:数据长度 */ );
	/**-----------------------------------------------**/
	ssize_t recvsize;
	if ((recvsize = recvmsg(sock_fd, &msghdr_s, flags)) <= 0) return(recvsize);
	/**-----------------------------------------------**/
	salenptr = msghdr_s.msg_namelen;	// 协议地址的大小
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
	if (pktinfo != nullptr) bzero(pktinfo, sizeof(struct in_pktinfo));
#else						/* 自定义 报文(UDP)相关控制信息 */
	if (pktinfo != nullptr) bzero(pktinfo, sizeof(struct un_in_pktinfo));
#endif
	/**-----------------------------------------------**/
#if SUPPORT_msg_control
	flags = msghdr_s.msg_flags;		// Flags on received message.
	if ( msghdr_s.msg_controllen < sizeof(struct cmsghdr) /*没有控制信息*/ ||
		 (msghdr_s.msg_flags & MSG_CTRUNC) /*控制信息被截断*/ ||
		 pktinfo == nullptr ) return(-2);

	/* CMSG_FIRSTHDR() 指向 msghdr{}第一个辅助数据 cmsghdr{}结构的指针 */
	for (struct cmsghdr *cmsghdr_ptr = CMSG_FIRSTHDR(&msghdr_s);
		 cmsghdr_ptr != nullptr;
		 cmsghdr_ptr = CMSG_NXTHDR(&msghdr_s, cmsghdr_ptr) )
	{
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
		if (cmsghdr_ptr->cmsg_level == IPPROTO_IP &&
			cmsghdr_ptr->cmsg_type == IP_PKTINFO) {
			auto *pktinfo_ptr = (struct in_pktinfo *) CMSG_DATA(cmsghdr_ptr);
			pktinfo->ipi_ifindex = pktinfo_ptr->ipi_ifindex; // 接口索引
			memcpy(&pktinfo->ipi_addr, &pktinfo_ptr->ipi_addr,
				   sizeof(struct in_addr)); // 标头目标地址
			memcpy(&pktinfo->ipi_spec_dst, &pktinfo_ptr->ipi_spec_dst,
				   sizeof(struct in_addr)); // 路由目标地址
			continue;
		}
#endif
#ifdef	IP_RECVDSTADDR
		if (cmsghdr_ptr->cmsg_level == IPPROTO_IP &&
			cmsghdr_ptr->cmsg_type == IP_RECVDSTADDR) {
			memcpy(&pktinfo->ipi_addr, CMSG_DATA(cmsghdr_ptr),
		  			sizeof(struct in_addr));
			continue;
		}
#endif
#ifdef IP_RECVIF
		if (cmsghdr_ptr->cmsg_level == IPPROTO_IP &&
			cmsghdr_ptr->cmsg_type == IP_RECVIF) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) CMSG_DATA(cmsghdr_ptr);
			pktinfo->ipi_ifindex = sdl->sdl_index;
			continue;
		}
#endif
		err_quit(-1, __FILE__, __func__, __LINE__,
				 "unknown ancillary data, len = %d, level = %d, type = %d",
				 cmsghdr_ptr->cmsg_len, cmsghdr_ptr->cmsg_level,
				 cmsghdr_ptr->cmsg_type);
	}
#else
	flagsp = 0;					/* pass back results */
	return(-3);
#endif
	return(recvsize);
}

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 接收标志、目的IP地址、接口/网口索引 单个读/写缓冲区 */
ssize_t Recvfrom_flags(const int &sock_fd, void *ptr, const size_t &nbytes,
					   int &flagsp, struct sockaddr *sa, socklen_t &salenptr,
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
					   struct in_pktinfo *pktinfo)
#else						/* 自定义 报文(UDP)相关控制信息 */
struct unp_in_pktinfo *pktinfo)
#endif
{
	ssize_t	retval = recvmsg_flags(sock_fd, ptr, nbytes, flagsp,
									  sa, salenptr, pktinfo);
	err_sys(retval, __FILE__, __func__, __LINE__,
			"recvmsg_flags()");
	return(retval);
}

/****---------------------------- sendmsg() --------------------------****/

/*************************************************************************/
/*************************************************************************/

/*************************** 通讯数据结构 配置 *****************************/

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr_02(struct msghdr &msghdr_s,	/** 通讯数据结构 */
							  void *address,	  		// 协议地址
							  const socklen_t &addr_len,// 协议地址的大小
							  void *data_base1,			/* 数据缓冲区:数据 */
							  const size_t &data_len1,	/* 数据缓冲区:数据长度 */
							  void *data_base2,			/* 数据缓冲区:数据 */
							  const size_t &data_len2,	/* 数据缓冲区:数据长度 */
							  const int8_t &mode)
{
//	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	msghdr_s.msg_name    = address; 	// 协议地址 address.
	msghdr_s.msg_namelen = addr_len;	// 协议地址的大小
/**----------------------------------**/
	struct iovec iovec_s[2]; 			/** 数据缓冲区数组 */
	iovec_s[0].iov_base = data_base1;	/* 数据缓冲区:数据 */
	iovec_s[0].iov_len  = data_len1;	/* 数据缓冲区:数据长度 */
	iovec_s[1].iov_base = data_base2;	/* 数据缓冲区:数据 */
	iovec_s[1].iov_len  = data_len2;	/* 数据缓冲区:数据长度 */
	msghdr_s.msg_iov    = iovec_s; 		// 指定 数据缓冲区数组
	msghdr_s.msg_iovlen = 2;			// msg_iov 中的元素长度
/**----------------------------------**/
#if SUPPORT_msg_control
	/**	在任意时刻，union 中只能有一个数据成员可以有值。
	 	union 成员共享同一块大小的内存，对某一个成员赋值，
	 	会覆盖其他成员的值，一次只能使用其中的一个成员。
	 	当给联合中某个成员赋值之后，该联合中的其它成员就变成未定义状态了。
	 	联合的存储空间至少能够容纳其最大的数据成员，其大小由最大的成员的大小决定*/
	union { 							/** 对齐数据 **/
		struct cmsghdr cmsghdr_s{}; 	/* 辅助数据 sizeof(struct cmsghdr)=16 */
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
		char control [CMSG_SPACE(sizeof(struct in_pktinfo))];
		/**	实际 union cmsg_control {} 层次：
 msghdr{} ----------------- *msg_control
	|						|
	|		cmsghdr{} -----	cmsg_len
	|						cmsg_level
	|						cmsg_type
 	|						in_pktinfo{} ( __cmsg_data[] 填充字节+数据)
 msg_controllen				-----------------
  	|		in_pktinfo{} 填充字节 (两个辅助数据之间)          **/
#endif
/* 	CMSG_LEN() 和 CMSG_SPACE() 的区别在于 后者会包含两个辅助数据之间的填充字节。
 	sizeof (struct cmsghdr) 	16
	CMSG_LEN (sizeof(int))	 	20 = 16 + 4(int)
	CMSG_SPACE (sizeof(int)) 	24 = 16 + 4(int) + 4(int, 填充字节)	  */
	} cmsg_control{};
	if (mode) {
		msghdr_s.msg_control = (caddr_t) &cmsg_control;            // 辅助数据
		msghdr_s.msg_controllen = sizeof(cmsg_control);            // 辅助数据的大小
	}
//	msghdr_s.msg_control = cmsg_control.control; 			// 辅助数据
//	msghdr_s.msg_controllen = sizeof(cmsg_control.control);	// 辅助数据的大小
#else
	int	newfd;
	msghdr_s.msg_accrights = (caddr_t) &newfd;		// 辅助数据
	msghdr_s.msg_accrightslen = sizeof(int);		// 辅助数据的大小
#endif
}

/****---------------------------- recvmsg() --------------------------****/

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 	在信号处理函数中进行非局部转移
 	增加 可靠性： 	1、超时和重传：处理丢失数据；
 				 	2、序列号：验证应答是否匹配。    */
ssize_t recvmsg_seq_ts(const int &sock_fd, void *ptr1, const size_t &nbytes1,
					   void *ptr2, const size_t &nbytes2,
					   struct sockaddr *sa, socklen_t &salenptr,
					   struct in_pktinfo *pktinfo)
{
	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	struct_msghdr_cmsghdr_02(msghdr_s,// 通讯数据结构
							 sa,		// 协议地址
							 salenptr,	// 协议地址的大小
							 ptr1,		/* 数据缓冲区:数据 */
							 nbytes1,	/* 数据缓冲区:数据长度 */
							 ptr2,		/* 数据缓冲区:数据 */
							 nbytes2,	/* 数据缓冲区:数据长度 */
							 1 );
	/**-----------------------------------------------**/
	ssize_t recvsize;
	if ((recvsize = recvmsg(sock_fd, &msghdr_s, 0)) <= 0) return(recvsize);
	/**-----------------------------------------------**/
	salenptr = msghdr_s.msg_namelen;	// 协议地址的大小
	/**-----------------------------------------------**/
#if SUPPORT_msg_control
	if ( msghdr_s.msg_controllen < sizeof(struct cmsghdr) /*没有控制信息*/ ||
		 (msghdr_s.msg_flags & MSG_CTRUNC) /*控制信息被截断*/ ) return(-2);

	if (pktinfo != nullptr) {
		bzero(pktinfo, sizeof(struct in_pktinfo));

		/* CMSG_FIRSTHDR() 指向 msghdr{}第一个辅助数据 cmsghdr{}结构的指针 */
		for (struct cmsghdr *cmsghdr_ptr = CMSG_FIRSTHDR(&msghdr_s);
			 cmsghdr_ptr != nullptr;
			 cmsghdr_ptr = CMSG_NXTHDR(&msghdr_s, cmsghdr_ptr)) {
#ifdef IP_PKTINFO            /* 报文(UDP)相关控制信息 */
			if (cmsghdr_ptr->cmsg_level == IPPROTO_IP &&
				cmsghdr_ptr->cmsg_type == IP_PKTINFO) {
				auto *pktinfo_ptr = (struct in_pktinfo *) CMSG_DATA(
						cmsghdr_ptr);
				pktinfo->ipi_ifindex = pktinfo_ptr->ipi_ifindex; // 接口索引
				memcpy(&pktinfo->ipi_addr, &pktinfo_ptr->ipi_addr,
					   sizeof(struct in_addr)); // 标头目标地址
				memcpy(&pktinfo->ipi_spec_dst, &pktinfo_ptr->ipi_spec_dst,
					   sizeof(struct in_addr)); // 路由目标地址
				continue;
			}
			err_ret(-1, __FILE__, __func__, __LINE__,
					"unknown ancillary data, len = %d, level = %d, type = %d",
					cmsghdr_ptr->cmsg_len, cmsghdr_ptr->cmsg_level,
					cmsghdr_ptr->cmsg_type);
#endif
		}
	}
#else
	return(-4);
#endif
	return(recvsize);
}

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 	在信号处理函数中进行非局部转移
 	增加 可靠性： 	1、超时和重传：处理丢失数据；
 				 	2、序列号：验证应答是否匹配。    */
ssize_t Recvmsg_seq_ts(const int &sock_fd, void *ptr1, const size_t &nbytes1,
					   void *ptr2, const size_t &nbytes2,
					   struct sockaddr *sa, socklen_t &salenptr,
					   struct in_pktinfo *pktinfo)
{
	ssize_t	retval = recvmsg_seq_ts(sock_fd, ptr1, nbytes1, ptr2, nbytes2,
									   sa, salenptr, pktinfo);
	err_sys(retval, __FILE__, __func__, __LINE__, "%s()", __func__);
	return(retval);
}

/****---------------------------- sendmsg() --------------------------****/

/* sendmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 	在信号处理函数中进行非局部转移
 	增加 可靠性： 	1、超时和重传：处理丢失数据；
 				 	2、序列号：验证应答是否匹配。    */
ssize_t Sendmsg_seq_ts(const int &sock_fd, void *ptr1, const size_t &nbytes1,
					   void *ptr2, const size_t &nbytes2,
					   struct sockaddr *sa, socklen_t &salenptr)
{
	struct msghdr msghdr_s{};			/** 通讯数据结构 */
	struct_msghdr_cmsghdr_02(msghdr_s,// 通讯数据结构
							 sa,		// 协议地址
							 salenptr,	// 协议地址的大小
							 ptr1,		/* 数据缓冲区:数据 */
							 nbytes1,	/* 数据缓冲区:数据长度 */
							 ptr2,		/* 数据缓冲区:数据 */
							 nbytes2	/* 数据缓冲区:数据长度 */ );
	/**-----------------------------------------------**/
	ssize_t num = sendmsg(sock_fd, &msghdr_s, 0);
	err_sys(num, __FILE__, __func__, __LINE__, "%s()", __func__);
	return num;
}

/*************************************************************************/
/*************************************************************************/

/****---------------------------- recvmsg() --------------------------****/

/****---------------------------- sendmsg() --------------------------****/

/***************************** old_backup ************************************/
#if 0
/* sendmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 send_fd */
ssize_t write_fd(int sock_fd, void *ptr, size_t nbytes, int send_fd)
{
	struct msghdr msghdr_s{};			/* 通讯数据结构 */
	msghdr_s.msg_name = nullptr; 		// 协议地址
	msghdr_s.msg_namelen = 0;			// 协议地址的大小

	/**	在任意时刻，union 中只能有一个数据成员可以有值。
	 	union 成员共享同一块大小的内存，对某一个成员赋值，
	 	会覆盖其他成员的值，一次只能使用其中的一个成员。
	 	当给联合中某个成员赋值之后，该联合中的其它成员就变成未定义状态了。
	 	联合的存储空间至少能够容纳其最大的数据成员，其大小由最大的成员的大小决定*/
	union { 							/** 对齐数据 **/
		struct cmsghdr cmsghdr_s; 		/* 辅助数据 */
		char control[CMSG_SPACE(sizeof(int))];
/* 	CMSG_LEN() 和 CMSG_SPACE() 的区别在于 后者会包含两个辅助数据之间的填充字节。
	CMSG_LEN (sizeof(int))	 	20
	CMSG_SPACE (sizeof(int)) 	24                   */
	} cmsg_control{};
#if 0
	msghdr_s.msg_control = cmsg_control.control; 			// 辅助数据
	msghdr_s.msg_controllen = sizeof(cmsg_control.control);	// 辅助数据的大小
#else
	msghdr_s.msg_control = (caddr_t) &cmsg_control;			// 辅助数据
	msghdr_s.msg_controllen = sizeof(cmsg_control);			// 辅助数据的大小
#endif
	struct iovec iovec_s[1]; 			/* 数据缓冲区数组 */
	iovec_s[0].iov_base = ptr;			// 数据缓冲区:数据
	iovec_s[0].iov_len = nbytes;		// 数据缓冲区:数据长度
	msghdr_s.msg_iov = iovec_s; 		// 指定 数据缓冲区数组
	msghdr_s.msg_iovlen = 1;			// msg_iov 中的元素长度
/**---------------------------------------------------------------------**/
	struct cmsghdr *cmsghdr_ptr;		/* 辅助数据 */
	/* CMSG_FIRSTHDR() 指向 msghdr{}第一个辅助数据 cmsghdr{}结构的指针 */
	cmsghdr_ptr = CMSG_FIRSTHDR(&msghdr_s);
	cmsghdr_ptr->cmsg_len = CMSG_LEN(sizeof(int));
/*	协议	cmsg_level		cmsg_type		说明
	Unix域	SOL_SOCKET		SCM_RIGHTS		发送/接收描述符
							SCM_CREDS		发送/接收用户凭证  */
	cmsghdr_ptr->cmsg_level = SOL_SOCKET;
	cmsghdr_ptr->cmsg_type = SCM_RIGHTS;
/* CMSG_DATA()指向与辅助数据 cmsghdr{}关联的数据 cmsghdr.cmsg_data[]的第一个字节指针 */
	*((int *)CMSG_DATA(cmsghdr_ptr)) = send_fd;

	return sendmsg(sock_fd, &msghdr_s, 0);
}
#endif
