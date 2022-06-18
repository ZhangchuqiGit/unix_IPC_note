//
// Created by zcq on 2021/5/15.
//

#ifndef __WRAP_SIGNAL_H
#define __WRAP_SIGNAL_H

#include <csignal>
#include <iostream>

#include "zcq_header.h"

void Sigemptyset(sigset_t *set); // 清除 信号组 中的 所有信号

void Sigfillset(sigset_t *set); // 设置 所有信号 到 信号组

void Sigaddset(sigset_t *set, const int &signo); // 信号 添加到 信号组

void Sigdelset(sigset_t *set, const int &signo); // 清除 信号组 中的 信号

/** SIG_BLOCK 阻塞 set 而不受其信号中断打扰，直到 SIG_UNBLOCK。
 	阻塞时，信号处理程序一般在工作 **/
void Sigprocmask(const int &how,
				 const sigset_t *set, sigset_t *oset ); // 单线程:检测或改变信号屏蔽字

/** 等待 zeromask(空信号) 即 永久等待，效率低（经内核） **/
void Sigsuspend(const sigset_t *set); // 等待一个信号到达

void Sigpending(sigset_t *set); // 将所有被阻止并等待发送的信号置于 set 中

void Sigwait(const sigset_t *set, int &sig); // 等待 set 中任何信号到达并放置在 sig

# ifdef __USE_POSIX199309
void Sigwaitinfo(const sigset_t *set, siginfo_t *info); // 从 set 中选择所有待处理信号并放置在 info

/*从 set 中选择所有待处理信号并放置在 info。如果没有信号待处理，等待指定时间。*/
void Sigtimedwait(const sigset_t *set, siginfo_t *info,
				  const struct timespec *timeout); // 等待指定时间从 set 中选择所有待处理信号并放置在 info

void Sigqueue(const pid_t &pid, const int &sig, const union sigval val); // 将信号SIG发送到进程PID
#endif

void Waitpid(const pid_t &pid, int *stat_loc, const int &option); // 等待子进程中断或结束


#endif //__WRAP_SIGNAL_H
