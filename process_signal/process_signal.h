//
// Created by zcq on 2021/5/10.
//

#ifndef __PROCESS_SIGNAL_H
#define __PROCESS_SIGNAL_H

#include <csignal>

#include <iostream>

/**********************************************************/

#include "zcq_header.h"

/**********************************************************/

void process_kill(); // 用 kill() 向 进程 或 进程组 发送信号
void process_sigaction(); // sigaction() 支持信号传递信息，可用于所有信号安装
void process_shield(); // 进程(单线程)信号屏蔽
void process_signal(); // signal() 不支持信号传递信息，主要用于 非实时信号

/**************************** 信号处理 signal *****************************/

/* Type of a signal handler.  */
//typedef void (*__sighandler_t) (int);
//typedef void  Sigfunc(int) ; // 声明 函数 是 仅有一个 int 参数 且 不返回值 的类型
/**-----------------------------------------------**/
/* sigaction() 支持信号传递信息，可用于所有信号安装 */
//Sigfunc *sigaction_func(int signum, Sigfunc *Func);
__sighandler_t sigaction_func(const int &signum, __sighandler_t func);
void Signal_fork(const int &signum);
__sighandler_t Signal(const int &signum, __sighandler_t func);

/********************* 注册 多参数信号函数 + 多(实时)信号处理 ***********************/
typedef	void (*Sigfunc_rt) (int, siginfo_t *, void *);

Sigfunc_rt Sigaction_rt(const int &signum, Sigfunc_rt func);
Sigfunc_rt Signal_rt(const int &signum, Sigfunc_rt func);

/********************* 测试 多参数信号函数 + 多(实时)信号处理 ***********************/

void test01_signal_rt(); // 测试 多参数信号函数 + 多(实时)信号处理



#endif //__PROCESS_SIGNAL_H
