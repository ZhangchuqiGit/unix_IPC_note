//
// Created by zcq on 2021/4/26.
//

#include "IO_head.h" /* I/O 函数 */


/* 传输“过大”消息时部分递交，其它等待该消息完整提交 */
int pdapi_recvmsg(const int &sock_fd, std::string &buf_str,
				  struct sockaddr *from, socklen_t *from_len,
				  struct sctp_sndrcvinfo *buf_info, int *msg_flags)
{
	buf_str.clear();
	char buf[SCTP_part_recv] = {'\0'};
	int msg_flag;

	int recvsize = sctp_recvmsg(sock_fd, buf, sizeof(buf),
								(struct sockaddr *)from, from_len,
								buf_info, &msg_flag);
	err_sys(recvsize, __FILE__, __func__, __LINE__, "sctp_recvmsg()");

	*msg_flags = msg_flag;
	if(recvsize == 0) return recvsize;
	buf_str += buf;

	socklen_t tmp_len = 0; // 临时保存
	int num = 0, rsize;
	while((msg_flag & MSG_EOR) == 0) {
		++num;
		_debug_log_info("传输“过大”消息时部分递交，其它等待该消息完整提交")
		_debug_log_info("NO %d : %d / %d", num, rsize, recvsize)

		bzero(buf, sizeof(buf));
		rsize = sctp_recvmsg(sock_fd, buf, sizeof(buf),
								 nullptr, &tmp_len, nullptr, &msg_flag);
		err_sys(recvsize, __FILE__, __func__, __LINE__, "sctp_recvmsg()");

		recvsize += rsize;
		buf_str += buf;
	}
	*msg_flags = msg_flag;
	return recvsize;
}
