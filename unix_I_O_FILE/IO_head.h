//
// Created by zcq on 2021/3/18.
//

#ifndef __IO_HEAD_H
#define __IO_HEAD_H

#include "zcq_header.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>  //for read
#include <sys/uio.h>
#include <fstream> // 文件流 <istream> <ostream>
#include <iostream> // 标准流 <istream> <ostream>
#include <sys/stat.h> //  POSIX Standard: File Characteristics
#include <sys/param.h> // ALIGN macro for CMSG_NXTHDR() macro

/**************************** Standard file descriptors.
	STDIN_FILENO	0	标准输入。
	STDOUT_FILENO	1	标准输出。
	STDERR_FILENO	2	标准错误输出。
	FILE *stdin;		标准输入流。
	FILE *stdout;	标准输出流。
	FILE *stderr;	标准错误输出流。
	int fp = fileno(stdin); 	转化文件描述符 标准输入
	int fp = fileno(stdout); 	转化文件描述符 标准输出
	int fp = fileno(stderr); 	转化文件描述符 标准错误输出        **/

#define SUPPORT_msg_control   	1

/************************** read() write() *******************************/

//慢速版-仅用于示例
ssize_t readline_slow(const int &fd, const void *vptr, const size_t &size_val);

/* 任何描述符 单个读缓冲区 */
ssize_t Read(const int &fd, void *ptr, const size_t &size_val, const uint8_t &mode=1);

/* 任何描述符 单个写缓冲区 */
ssize_t Write(const int &fd, const void *vptr, const size_t &size_val);
// Write(fileno(stdout), buf, strlen(buf));

/************************** readv() writev() *****************************/

/* 任何描述符 分散读（scatter read） */
void Readv_test(); // 测试

/* 任何描述符 分散读（scatter read） */
int Readv();

/* 任何描述符 集中写（gather write）*/
void Writev_test(); // 测试

/* 任何描述符 集中写（gather write） */
int Writev();

/*************************** recv() send() *******************************/

/* 仅套接字描述符 单个读缓冲区 可选标志 */
ssize_t Recv(const int &fd, void *buf, const size_t &n, const int &flags);


/* 仅套接字描述符 单个写缓冲区 可选标志 */
ssize_t Send(const int &fd, const void *buf, const size_t &n, const int &flags);


/************************ recvfrom() sendto() ****************************/

/* 仅套接字描述符 单个读缓冲区 可选标志 可选对面地址 */
void recv_all(const int &sock_fd, const socklen_t &salen);

/* 仅套接字描述符 单个写缓冲区 可选标志 可选对面地址 */
void send_all(const int &sock_fd,
			  const struct sockaddr *sa, const socklen_t &salen);

/************************ recvmsg() sendmsg() ****************************/

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr(struct msghdr &msghdr_s,	     /** 通讯数据结构 */
						   void *address = nullptr,		 // 协议地址
						   const socklen_t &addr_len = 0,// 协议地址的大小
						   void *data_base = (char *)"", /* 数据缓冲区:数据 */
						   const size_t &data_len = sizeof("")/* 数据缓冲区:数据长度 */ );

/* recvmsg() 仅套接字描述符 单个读缓冲区 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 recv_fd 单个读/写缓冲区 */
ssize_t Read_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				int &recv_fd);

/* sendmsg() 仅套接字描述符 单个写缓冲区 可选标志 可选对面地址 可选控制信息 */
/* 进程间传递文件描述符 send_fd 单个读/写缓冲区 */
ssize_t Write_fd(const int &sock_fd, void *ptr, const size_t &nbytes,
				 const int &send_fd);

/****-----------------------------------------------------------------****/

#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
/* Structure used for IP_PKTINFO.  */
//	struct in_pktinfo
//	{
//		int ipi_ifindex;			/* Interface index  */
//		struct in_addr ipi_spec_dst;/* Routing destination address  */
//		struct in_addr ipi_addr;	/* destination IPv4 address  */
//	};
#else						/* 自定义 报文(UDP)相关控制信息 */
/* The structure returned by recvfrom_flags() */
struct unp_in_pktinfo {
	struct in_addr ipi_addr;	/* destination IPv4 address */
	int ipi_ifindex;			/* received interface index 接口/网口索引 */
};
#endif

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr_01(struct msghdr &msghdr_s,		/** 通讯数据结构 */
							  void *address = nullptr,	 	// 协议地址
							  const socklen_t &addr_len = 0,// 协议地址的大小
							  void *data_base = (char *)"",	/* 数据缓冲区:数据 */
							  const size_t &data_len = sizeof("")/* 数据缓冲区:数据长度 */ );

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 接收标志、目的IP地址、接口/网口索引 单个读/写缓冲区 */
ssize_t Recvfrom_flags(const int &sock_fd, void *ptr, const size_t &nbytes,
					   int &flagsp, struct sockaddr *sa, socklen_t &salenptr,
#ifdef IP_PKTINFO  			/* 报文(UDP)相关控制信息 */
					   struct in_pktinfo *pktinfo);
#else						/* 自定义 报文(UDP)相关控制信息 */
struct unp_in_pktinfo *pktinfo);
#endif

/****-----------------------------------------------------------------****/

/*************************** 通讯数据结构 配置 *****************************/

struct seq_ts {		/** 序列号：验证应答 **/
	uint32_t seq;	/* sequence */
	uint32_t ts;	/* timestamp when sent */
};

/* 通讯数据结构 配置 */
void struct_msghdr_cmsghdr_02(struct msghdr &msghdr_s,	/** 通讯数据结构 */
							  void *address,	  		// 协议地址
							  const socklen_t &addr_len,// 协议地址的大小
							  void *data_base1,			/* 数据缓冲区:数据 */
							  const size_t &data_len1,	/* 数据缓冲区:数据长度 */
							  void *data_base2,			/* 数据缓冲区:数据 */
							  const size_t &data_len2,	/* 数据缓冲区:数据长度 */
							  const int8_t &mode = 0);

/* recvmsg() 仅套接字描述符 分散读 可选标志 可选对面地址 可选控制信息 */
/* 	在信号处理函数中进行非局部转移
 	增加 可靠性： 	1、超时和重传：处理丢失数据；
 				 	2、序列号：验证应答是否匹配。    */
ssize_t Recvmsg_seq_ts(const int &sock_fd, void *ptr1, const size_t &nbytes1,
					   void *ptr2, const size_t &nbytes2,
					   struct sockaddr *sa, socklen_t &salenptr,
					   struct in_pktinfo *pktinfo);

/* sendmsg() 仅套接字描述符 集中写 可选标志 可选对面地址 可选控制信息 */
/* 	在信号处理函数中进行非局部转移
 	增加 可靠性： 	1、超时和重传：处理丢失数据；
 				 	2、序列号：验证应答是否匹配。    */
ssize_t Sendmsg_seq_ts(const int &sock_fd, void *ptr1, const size_t &nbytes1,
					   void *ptr2, const size_t &nbytes2,
					   struct sockaddr *sa, socklen_t &salenptr);


/*********************************** SCTP *************************************/

/* 传输“过大”消息时部分递交，其它等待该消息完整提交 */
int pdapi_recvmsg(const int &sock_fd, std::string &buf_str,
				  struct sockaddr *from, socklen_t *from_len,
				  struct sctp_sndrcvinfo *buf_info, int *msg_flags);


#endif //__IO_HEAD_H
