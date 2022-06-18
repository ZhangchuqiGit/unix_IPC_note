//
// Created by zcq on 2021/3/19.
//

#include "if_quit_or_run.h"

bool if_quit_echo(const void *vptr)
{
	const char *buf = (char *)vptr;
#if 0
	std::string bufstr = buf;
		if ( !bufstr.compare(0, bufstr.find_first_of('\n'),
							 "quit", 0, std::size("quit")/* 0~4 */ ) ) return true;
#elif 1
	if(strncmp(buf, "quit", 4) == 0) return true;
#else
	if ( strcmp(buf, "quit\n") == 0 ) return true;
#endif
	return false;
}


int if_quit_0(std::string_view cmd, std::string_view buf )
{
	if (cmd == buf) return 1;
	return 0;
}

int if_quit_struct(const void *vptr)
{
	const struct result *buf = (struct result *)vptr;
	if ( buf->sum == 0 ) return true;
	return false;
}

int sctp_quit( std::string_view cmd, std::string_view buf, const int &sockfd,
			   struct sockaddr *to_addr) {
	struct sctp_sndrcvinfo sctp_SndRcv_info{}; //存放消息相关的细节信息(若设置了通知事件)
	bzero(&sctp_SndRcv_info,sizeof(sctp_SndRcv_info)); // 清 0
	if (if_run(cmd, buf)) {
		std::cout.flush();
		std::cerr << "NOW : goodbye !" << std::endl;
		sctp_SndRcv_info.sinfo_flags |= SCTP_ABORT;
		int ret_value = sctp_sendmsg(sockfd, "quit", std::size("quit"),
									 to_addr, sizeof(sockaddr), 0,
									 sctp_SndRcv_info.sinfo_flags,
//									 0,
									 0, 0, 0);
		err_sys(ret_value, __LINE__, "sctp_sendmsg()");
		return 1;
	}
	return 0;
}

/*************************************************************************/

// 	if ( if_run("time\n", buf) )
bool if_run( std::string_view cmd, std::string_view buf)
{
	if (cmd == buf) {
		return true;
	}
//	std::string buf_str = cmd.data();
//	if (buf_str[buf_str.size()] == '\n') buf_str[buf_str.size()] = '\0';
	if (buf.compare(0, 4, cmd.data()) == 0) return true;
	return false;
}

