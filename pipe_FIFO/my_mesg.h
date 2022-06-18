//
// Created by zcq on 2021/5/12.
//

#ifndef __MY_MESG_H
#define __MY_MESG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <climits>	// for OPEN_MAX

/*********************************/

#include "zcq_header.h"

/**************** 自定义 可变长度 消息体 *****************/

/* Our own "messages" to use with pipes, FIFOs, and message queues. */

/* PIPE_BUF： bytes in atomic write to a pipe */
/* want sizeof(struct mymesg) <= PIPE_BUF */
#define	MESG_MaxData  (PIPE_BUF/*4096*/ - sizeof(long)*2)

struct my_mesg { 	/* 自定义 可变长度 消息体 */
	long mesg_type;	/* message type, must be > 0 */
	long mesg_len;	/* bytes in mesg_data, can be 0 */
	char mesg_data[MESG_MaxData]; // 4080 = 4096 - 8*2
};

/* length of mesg_len and mesg_type */
#define	MESG_HeaderSize  (sizeof(struct my_mesg) - MESG_MaxData)

ssize_t Mesg_send(const int &fd, const struct my_mesg *ptr);
ssize_t Mesg_recv(const int &fd, struct my_mesg *ptr);

void mesg_client(const int &readfd, const int &writefd);
void mesg_service(const int &readfd, const int &writefd);


#endif //__MY_MESG_H
