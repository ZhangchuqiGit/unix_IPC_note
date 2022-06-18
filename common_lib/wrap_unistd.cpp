//
// Created by zcq on 2021/5/11.
//

#include "wrap_unistd.h"

pid_t Fork() // 创建 子进程
{
	pid_t child_pid = fork();
	if (child_pid == -1) {
		err_sys(child_pid, __FILE__, __func__, __LINE__, "fork()");
	}
	return child_pid;
}

void Close(const int &fd)
{
	if (close(fd) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "close()");
}
