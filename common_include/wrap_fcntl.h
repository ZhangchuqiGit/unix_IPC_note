//
// Created by zcq on 2021/5/11.
//

#ifndef __WRAP_FCNTL_H
#define __WRAP_FCNTL_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "zcq_header.h"

int Fcntl(const int & fd, const int & cmd, const int & arg,
		  const int &line=0, const char *func=" ");

int Open(const char *pathname, int oflag, ... /* 可直接调用 */ ); // 打开/创建 指定文件



#endif //__WRAP_FCNTL_H
