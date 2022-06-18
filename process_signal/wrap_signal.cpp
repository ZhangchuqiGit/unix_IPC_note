//
// Created by zcq on 2021/5/15.
//

#include "wrap_signal.h"


void Sigemptyset(sigset_t *set) // 清除 信号组 中的 所有信号
{
	if (sigemptyset(set) == -1) // 清除 信号组 中的 所有信号
		err_sys(-1, __FILE__, __func__, __LINE__,
				"Sigemptyset()");
}

void Sigfillset(sigset_t *set) // 设置 所有信号 到 信号组
{
	if (sigfillset(set) == -1) // 设置 所有信号 到 信号组
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigfillset()");
}

void Sigaddset(sigset_t *set, const int &signo) // 信号 添加到 信号组
{
	if (sigaddset(set, signo) == -1) // 信号 添加到 信号组
		err_sys(-1, __FILE__, __func__, __LINE__,
				"Sigaddset()");
}

void Sigdelset(sigset_t *set, const int &signo) // 清除 信号组 中的 信号
{
	if (sigdelset(set, signo) == -1) // 清除 信号组 中的 信号
		err_sys(-1, __FILE__, __func__, __LINE__,
				"Sigaddset()");
}

/** SIG_BLOCK 阻塞 set 而不受其信号中断打扰，直到 SIG_UNBLOCK。
 	阻塞时，信号处理程序一般在工作 **/
void Sigprocmask(const int &how,
				 const sigset_t *set, sigset_t *oset ) // 单线程:检测或改变信号屏蔽字
{
	if (sigprocmask(how, set, oset) == -1) // 单线程:检测或改变信号屏蔽字
		err_sys(-1, __FILE__, __func__, __LINE__,
				"Sigprocmask()");
}

/** 等待 zeromask(空信号) 即 永久等待，效率低（经内核） **/
void Sigsuspend(const sigset_t *set) // 等待一个信号到达
{
	if (sigsuspend(set) == -1) // 等待一个信号到达
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigsuspend()");
}

void Sigpending(sigset_t *set) // 将所有被阻止并等待发送的信号置于 set 中
{
	if (sigpending(set) == -1) // 将所有被阻止并等待发送的信号置于 set 中
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigpending()");
}

void Sigwait(const sigset_t *set, int &sig) // 等待 set 中任何信号到达并放置在 sig
{
	if (sigwait(set, &sig) == -1) // 等待 set 中任何信号到达并放置在 sig
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigwait()");
}

# ifdef __USE_POSIX199309
void Sigwaitinfo(const sigset_t *set, siginfo_t *info) // 从 set 中选择所有待处理信号并放置在 info
{
	if (sigwaitinfo(set, info) == -1) // 从 set 中选择所有待处理信号并放置在 info
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigwaitinfo()");
}

/*从 set 中选择所有待处理信号并放置在 info。如果没有信号待处理，等待指定时间。*/
void Sigtimedwait(const sigset_t *set, siginfo_t *info,
				  const struct timespec *timeout) // 等待指定时间从 set 中选择所有待处理信号并放置在 info
{
	if (sigtimedwait(set, info, timeout) == -1) // 等待指定时间从 set 中选择所有待处理信号并放置在 info
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigtimedwait()");
}

void Sigqueue(const pid_t &pid, const int &sig, const union sigval val) // 将信号SIG发送到进程PID
{
	if (sigqueue(pid, sig, val) == -1) // 将信号SIG发送到进程PID
		err_sys(-1, __FILE__, __func__, __LINE__,
				"sigqueue()");
}
#endif

/* 	pid_t waitpid(pid_t pid, int *status, int options); // 等待子进程中断或结束
	waitpid() 会暂停当前进程的执行，直到 有信号来到 或 子进程结束。
	如果在调用 wait() 时子进程已经结束，则 wait() 会立即返回子进程结束状态值。
	如果 不在意 结束状态值，则参数 status 可以设成 NULL。
---------------------------------------------------------------------------
参数 pid :
    pid>0 	等待 任何子进程识别码为 pid 的子进程。
    pid=0 	等待 pid 与 当前进程 有相同的进程组 的任何子进程。
    pid=-1 	等待 任何子进程，相当于 wait()。
    pid<-1 	等待 进程组识别码为 pid 绝对值 的任何子进程。
参数 option :	可以为 0
    WNOHANG		如果没有 任何已经结束的子进程 则马上返回，不予以等待。
    WUNTRACED 	如果子进程进入暂停执行情况则马上返回，但结束状态不予以理会。
参数 status :	子进程的 结束状态值 存于 status，下面有几个宏可判别结束情况：
    WIFEXITED ：如果子进程 正常结束 则为 非0 值。
    WEXITSTATUS ：取得子进程 exit() 返回的 结束代码，
    			一般会先用 WIFEXITED 来判断 是否 正常结束 才能使用此宏。
    WIFSIGNALED ：如果子进程是 因为信号而结束 则此宏值为 true
    WTERMSIG ：取得子进程 因信号而中止 的信号代码，
    			一般会先用 WIFSIGNALED 来判断后才使用此宏。
    WIFSTOPPED ：如果子进程处于 暂停执行 情况则此宏值为 true。
    			一般 只有 使用 option=WUNTRACED 时才会有此情况。
    WSTOPSIG ：取得引发子进程暂停的 信号代码，
    			一般会先用 WIFSTOPPED 来判断后才使用此宏。
---------------------------------------------------------------------------
返回值 : 执行成功，则返回 pid，并将 该死子进程 的状态存储在 status 中；
 		该子进程尚未死，则返回 0；错误则返回 -1。失败原因存于 errno 中。	 */
void Waitpid(const pid_t &pid, int *stat_loc, const int &option) // 等待子进程中断或结束
{
	if (waitpid(pid, stat_loc, option) == -1) // 将信号SIG发送到进程PID
		err_sys(-1, __FILE__, __func__, __LINE__,
				"waitpid()");
}

