//
// Created by zcq on 2021/5/13.
//

#pragma once

//#ifndef __POSIX_HEADER_H
//#define __POSIX_HEADER_H

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h> // POSIX Standard: Primitive System Data Types

#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <fcntl.h>

#include <csignal>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime> // <sys/time.h>

#include <iostream>

/**********************************************************/

#include "zcq_header.h"

/**********************************************************/

#include "mqueue_test.h"
#include "wrap_mqueue.h"
#include "semaphore_test.h"
#include "wrap_semphore.h"
#include "mman_test.h"





//#endif //__POSIX_HEADER_H
/**
 不同 进程间 通信 方式：
 	管道（血缘关系、匿名管道）；（使用简单）
 	FIFO（命名管道）；（使用简单）
 	本地套接字；（最稳定）
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
 	信号；（要求原子操作，开销小，易丢失）
 	（系统/用户）信号（处理函数）；（开销最小，易丢失）
 	共享内存区/映射区；（最快）
 	消息队列；（任何时候读/写）
-----------------------------------
 不同 进程间 同步 方式：
	互斥锁；条件变量；（共享全局/静态变量，开销小）
 	文件/记录锁；
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
-----------------------------------
 不同 线程间 同步 方式：
	互斥锁；条件变量；（共享全局/静态变量，开销小）
 	读写锁；（共享全局/静态变量，开销小）
 	信号量；（要求原子操作，开销大于互斥锁/条件变量/读写锁）
**/