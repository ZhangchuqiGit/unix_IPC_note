//
// Created by zcq on 2021/5/12.
//

#include "my_mesg.h"

#include <iostream>
//using namespace std;

/**************** 自定义 可变长度 消息体 Mesg_send *****************/

ssize_t Mesg_send(const int &fd, const struct my_mesg *ptr)
{
	return(Write(fd, ptr, MESG_HeaderSize + ptr->mesg_len));
}

/**************** 自定义 可变长度 消息体 Mesg_recv *****************/

ssize_t Mesg_recv(const int &fd, struct my_mesg *ptr)
{
	ssize_t	recvsize;

	/* read message header first, to get len of data that follows */
	if ((recvsize = Read(fd, ptr, MESG_HeaderSize+1/*不留结束符位置*/, 0)) == 0) return(0);
	if (recvsize != MESG_HeaderSize) {
		err_quit(-1, __FILE__, __func__, __LINE__,
				 "mesg_recv(): recvsize(%ld) != MESG_HeaderSize(%ld)",
				 recvsize, MESG_HeaderSize);
	}

	/* read message data */
	if (ptr->mesg_len > 0)
		if ((recvsize = Read(fd, ptr->mesg_data, ptr->mesg_len+1/*不留结束符位置*/, 0)) != ptr->mesg_len) {
			err_quit(-1, __FILE__, __func__, __LINE__,
					 "mesg_recv(): recvsize(%ld) != ptr->mesg_len(%ld)",
					 recvsize, ptr->mesg_len);
		}
	if (recvsize > 1 && ptr->mesg_data[recvsize-1] == '\n') {
		ptr->mesg_data[recvsize-1] = '\0';
		ptr->mesg_len--;
	}

//	return(MESG_HeaderSize + ptr->mesg_len);
	return(MESG_HeaderSize + recvsize);
}

/**************** 自定义 可变长度 消息体 mesg_client *****************/

void mesg_client(const int &readfd, const int &writefd)
{
	_debug_log_info("mesg_client(): begin")
	struct my_mesg mesg{}; // 自定义 可变长度 消息体
	while (true) {
		if (Read(STDIN_FILENO, mesg.mesg_data, MESG_MaxData) == 0) break;

		mesg.mesg_type = 1;
		mesg.mesg_len = strlen(mesg.mesg_data);
		Mesg_send(writefd, &mesg);

		std::cout << "send --------------------- \n"
				  << "mesg_type: " << mesg.mesg_type << "\n"
				  << "mesg_len: " << mesg.mesg_len << "\n"
				  << "mesg_data: " << mesg.mesg_data << std::endl;

		if (if_run("quit", mesg.mesg_data)) break;

		if (Mesg_recv(readfd, &mesg) == 0) break;

		std::cout << "recv --------------------- \n"
				  << "mesg_type: " << mesg.mesg_type << "\n"
				  << "mesg_len: " << mesg.mesg_len << "\n"
				  << "mesg_data: " << mesg.mesg_data
				  << "==========================" << std::endl;
	}
	_debug_log_info("mesg_client(): exit")
}

/**************** 自定义 可变长度 消息体 mesg_service *****************/

void mesg_service(const int &readfd, const int &writefd)
{
	_debug_log_info("mesg_service(): begin")
	struct my_mesg mesg{}; // 自定义 可变长度 消息体
	while (true) {
		if (Mesg_recv(readfd, &mesg) == 0) break;

		if (if_run("quit", mesg.mesg_data)) break;
		if (if_run("time", mesg.mesg_data)) {
			memset(mesg.mesg_data, 0, 64);
			time_t ticks;
/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
			ticks = time(nullptr);
			snprintf(mesg.mesg_data, 64, "%.24s\n", ctime(&ticks));
		}

		mesg.mesg_type = 2;
		mesg.mesg_len = strlen(mesg.mesg_data);
		Mesg_send(writefd, &mesg);
	}
	_debug_log_info("mesg_service(): exit")
}

/****************************************************************/
