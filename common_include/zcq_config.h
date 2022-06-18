
/* configuration options for current OS */
/********** 配置选项 ************/

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/**
 不同 进程间 通信 方式：
 	管道（血缘关系、匿名管道）；（使用简单）
 	FIFO（命名管道）；（使用简单）
 	本地套接字；（最稳定）
 	信号量；（要求原子操作，开销最小，易丢失）
 	（系统/用户）信号（处理函数）；（开销最小，易丢失）
 	共享内存区/映射区；（最快）
 	消息队列；（任何时候读/写）
-----------------------------------
 不同 进程间 同步 方式：
	互斥锁；条件变量；（共享静态变量）
 	文件/记录锁；
 	信号量；（要求原子操作，开销最小，易丢失）
-----------------------------------
 不同 线程间 同步 方式：
	互斥锁；条件变量；（共享全局/静态变量）
 	读写锁；（共享全局/静态变量）
 	信号量；（要求原子操作，开销最小，易丢失）
**/

/*************************************************************************/

/* $ cat /proc/sys/net/ipv4/tcp_wmem
	4096    16384   4194304
第一个值是一个限制值，socket 发送缓存区的最少字节数；
第二个值是默认值----16384(16K)；
第三个值是一个限制值，socket 发送缓存区的最大字节数；  */

#define	MAXLINE		( 4 * 1024)		/*  4 kB：socket 发送缓存区的最少字节数 */
#define	BUFFSIZE	(16 * 1024)		/* 16 kB：socket 发送缓存区的默认字节数 */
#define	MAXSIZE		(4 * 1024 * 1024)/* 4 MB：socket 发送缓存区的最大字节数 */

#define	MAXFD	64   /* 不需要的文件描述符 */

/*可以从<syssocket.h>中的 SOMAXCONN 派生以下内容，
但是许多内核仍将其定义为5，同时实际上还支持更多内容*/
#define	MAX_listen 	64	/* 最大客户端连接数 */

/* ---- 修改 接收 缓冲区 大小 ---- */
//#define Receiver_MAXBuf_mode
#define	Receiver_MAXBuf    		MAXSIZE // 修改 缓冲区大小

/********************************* SCTP ***************************************/

#define SERV_SCTP_stream_max 	10	/* normal maximum streams SCTP 流 数目 */

/**上限，可以通过 std::string::max_size() 函数获得
在某些十六位嵌入式开发环境中，这个值是65535，也就是十六进制的0xFFFF
大部分32位环境或者系统中，这个值是 4,294,967,295，
 也就是十六进制0xFFFFFFFF。这大约有4G字节的样子。
大部分64位环境或者系统中，这个值是 4611686018427387903。
 也就是十六进制 0x3FFFFFFF FFFFFFFF。这大约有4G字节的样子。**/
/* 传输“过大”消息时部分递交，其它等待该消息完整提交 */
#ifndef Receiver_MAXBuf_mode
#define SCTP_part_recv 			65535 	/* 触发 SCTP 部分递交消息 */
#else
#define SCTP_part_recv 		Receiver_MAXBuf /* 触发 SCTP 部分递交消息 */
#endif

/*************************************************************************/

// 最好用绝对路径名！
/* Unix domain stream cli-serv */
#define	UNIX_path_stream 	"/tmp/unix_local.stream"
/* Unix domain datagram cli-serv */
#define	UNIX_path_datagram 	"/tmp/unix_local.datagram"

/**	注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234               */
#define POSIX_mqueue_PATH  "/zcq.mqueue"

/*************************************************************************/

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // 0644 rw-r--r--
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH) // 0755 rwxr-xr-x

#ifndef ACCESSPERMS
#define ACCESSPERMS  (S_IRWXU|S_IRWXG|S_IRWXO) // 0777 rwxrwxrwx
#endif
#ifndef ALLPERMS
#define ALLPERMS  (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
					// 07777 UDOrwxrwxrwx
#endif
#ifndef DEFFILEMODE
#define DEFFILEMODE  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
					// 0666 rw-rw-rw-
#endif

/*************************************************************************/

/* Posix.1g requires that an #include of <poll.h> DefinE INFTIM, but many
   systems still DefinE it in <sys/stropts.h>.  We don't want to include
   all the streams stuff if it's not needed, so we just DefinE INFTIM here.
   This is the standard value, but there's no guarantee it is -1. */
#ifndef INFTIM
#define INFTIM          (-1)    /* infinite poll timeout */
#ifdef	HAVE_POLL_H
#define	INFTIM_UNPH				/* tell unpxti.h we defined it */
#endif
#endif

/*************************************************************************/

