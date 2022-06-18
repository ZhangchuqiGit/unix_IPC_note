//
// Created by zcq on 2021/5/11.
//

#ifndef __WRAP_FORK_H
#define __WRAP_FORK_H

#include <unistd.h>

#include "zcq_header.h"

pid_t Fork(); // 创建 子进程

void Close(const int &fd);


#endif //__WRAP_FORK_H
