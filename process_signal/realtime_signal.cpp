//
// Created by zcq on 2021/5/15.
//



/**********************	信号 signal **********************/
/*	信号机制是进程之间相互传递消息的一种方法，信号全称为软中断信号。
从它的命名可以看出，它的实质和使用很象中断。所以，信号可以说是进程控制的一部分。
	软中断信号（signal，又简称为信号）用来通知进程发生了异步事件。
进程之间可以互相通过系统调用 kill 发送软中断信号。内核也可以因为内部事件而给进程发送信号，
通知进程发生了某个事件。注意，信号只是用来通知某进程发生了什么事件，并不给该进程传递任何数据。
---------------------------------------------------------------------------
进程通过系统调用 signal 来指定进程对某个信号的处理行为。
在进程表的表项中有一个软中断信号域，该域中每一位对应一个信号，
当有信号发送给进程时，对应位置位。由此可以看出，进程对不同的信号可以同时保留，
但对于同一个信号，进程并不知道在处理之前来过多少个。
---------------------------------------------------------------------------
	收到信号的进程对各种信号有不同的处理方法。
处理方法可以分为三类：
第一种方法是，自定义，类似中断的处理程序，对于需要处理的信号，
 	进程可以指定处理函数，由该函数来处理。
第二种方法是，忽略 接收到信号后不做任何反应。
 	忽略某个信号，对该信号不做任何处理，就象未发生过一样。
第三种方法是，默认 接收到信号后按默认的行为处理该信号。 这是多数应用采取的处理方式，
 	对该信号的处理保留系统的默认值，这种缺省操作，对大部分的信号的缺省操作是使得进程终止。	 */

/**********************	信号的来源 **********************/
/*
1 信号来自内核， 生成信号的请求来自以下3个地方。
（1）用户
 	用户可以通过输入Ctrl-C, Ctrl-\等命令，
 	或是终端驱动程序分配给信号控制字符的其他任何键来请求内核产生信号。
（2）内核
	当进程执行出错时， 内核给进程发送一个信号。
 	例如，非法段存取，浮点数溢出，亦或是一个非法指令，内核也利用信号通知进程特定事件发生。
（3）进程
	一个进程可以通过系统调用kill给另外一个进程发送信号， 一个进程可以和另一个进程通过信号通信。
2 信号捕获处理，进程能够通过系统调用 signal 告诉内核， 它要如何处理信号， 进程有3个选择。
（1）接收默认处理（通常是消亡）
 	SIGINT 的默认处理是消亡， 进程并不一定要使用signal接收默认处理，
 	但是进程能够通过以下调用来恢复默认处理。signal(SIGINT, SIG_DFL);
（2）忽略信号
	程序可以通过以下调用来告诉内核， 它需要忽略 SIGINT。signal(SIGINT, SIG_IGN);
（3）信号处理函数
	程序能够告诉内核，当程序到来时应该调用哪个函数。 signal(signum, functionname); */

/**********************	Linux 信号 相关函数 **********************/
/*	信号安装
		进程处理某个信号前，需要先在进程中安装此信号。
		安装过程主要是建立 信号值 和 进程对相应信息值的 动作。
int signal(int signum, sighandler_t handler); // 不支持信号传递信息，主要用于 非实时信号
int sigaction(int signum, struct sigaction *act, sigaction *oact);
	// 支持信号传递信息，主要用于实时信号，可用于所有信号(含非实时信号)
---------------------------------------------------------------------------
	信号发送
int kill(pid_t pid, int signum); // 用于向进程或进程组发送信号
int sigqueue (pid_t pid, int signum, const union sigval val);
	// 只能向 一个 进程发送信号，不能向进程组发送信号；
 	// 主要针对实时信号提出，与 sigaction() 组合使用，支持非实时信号的发送；
unsigned int alarm (unsigned int seconds); // 计时达到后给进程发送 SIGALARM 信号
int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);		//	getitimer(which, curr_value);
 	// 设置定时器，计时达到后给进程发送 SIGALRM 信号，功能比 alarm()强大 <sys/time.h>
void abort(void); // 向进程发送 中止执行 SIGABORT 信号，默认进程会异常退出。
int raise(int signum); // 向进程自身发送信号
---------------------------------------------------------------------------
信号集操作函数
sigemptyset(sigset_t *set)：信号集全部清0；
sigfillset(sigset_t *set)： 信号集全部置1，则信号集包含linux支持的64种信号；
sigaddset(sigset_t *set, int signum)：向信号集中加入signum信号；
sigdelset(sigset_t *set, int signum)：向信号集中删除signum信号；
sigismember(const sigset_t *set, int signum)：判定信号signum是否存在信号集中。
---------------------------------------------------------------------------
信号阻塞(屏蔽)函数
sigprocmask(int how, const sigset_t *set, sigset_t *oldset));//不同how参数，实现不同功能
SIG_BLOCK：将set指向信号集中的信号，添加到进程阻塞信号集；
SIG_UNBLOCK：将set指向信号集中的信号，从进程阻塞信号集删除；
SIG_SETMASK：将set指向信号集中的信号，设置成进程阻塞信号集；
sigpending(sigset_t *set))：获取已发送到进程，却被阻塞的所有信号；
sigsuspend(const sigset_t *mask))：用mask代替进程的原有掩码，并暂停进程执行，
 	直到收到信号再恢复原有掩码并继续执行进程。			*/

/**********************	Linux 支持的信号列表 **********************/
/*	值 的含义：
		第一个值 通常在 Alpha 和 Sparc 上有效，
		中间值 	对应 i386 和 ppc 以及 sh，
		最后值 	对应 mips。
		- 表示信号没有实现
---------------------------------------------------------------------------
 	处理动作 的字母含义：
					A 缺省的动作是 终止进程
					B 缺省的动作是 忽略此信号
					C 缺省的动作是 终止进程 并 进行内核映像转储（dump core）
					D 缺省的动作是 停止进程
					E 信号 不能 被捕获
					F 信号 不能 被忽略
---------------------------------------------------------------------------
	POSIX.1 中列出的信号
信号	值    处理动作	发出信号的原因
SIGHUP 	1 		 	A 	终端挂起或者控制进程终止
SIGINT	2 			A 	键盘中断（如break键被按下）
SIGQUIT 3 	 		C 	键盘的退出键被按下
SIGILL 	4	 	 	C 	非法指令
SIGABRT 6 		 	C 	由abort(3)发出的退出指令
SIGFPE 	8 		 	C 	浮点异常
SIGKILL 9 		 	AEF Kill信号
SIGSEGV 11 		 	C 	无效的内存引用
SIGPIPE 13 		 	A 	管道破裂: 写一个没有读端口的管道
SIGALRM 14 		 	A 	由alarm(2)发出的信号
SIGTERM 15 		 	A 	终止信号
SIGUSR1 30,10,16 	A 	用户自定义信号1
SIGUSR2 31,12,17 	A 	用户自定义信号2
SIGCHLD 20,17,18	B 	子进程结束信号
SIGCONT 19,18,25 		进程继续（曾被停止的进程）
SIGSTOP 17,19,23 	DEF 终止进程
SIGTSTP 18,20,24 	D 	控制终端（tty）上按下停止键
SIGTTIN 21,21,26 	D 	后台进程企图从控制终端读
SIGTTOU 22,22,27 	D 	后台进程企图从控制终端写
---------------------------------------------------------------------------
	没在 POSIX.1 中列出，而在 SUSv2 列出的信号
信号	值    		处理动作	发出信号的原因
SIGBUS 	10,7,10 	C 		总线错误(错误的内存访问)
SIGPOLL 			A 		Sys V定义的 Pollable 事件，与 SIGIO 同义
SIGPROF	27,27,29 	A 		Profiling定时器到
SIGSYS 	12,-,12 	C 		无效的系统调用 (SVID)
SIGTRAP 5 			C 		跟踪/断点捕获
SIGURG 	16,23,21 	B 		Socket出现紧急条件(4.2 BSD)
SIGVTALRM 26,26,28 	A 		实际时间报警时钟信号(4.2 BSD)
SIGXCPU 24,24,30 	C 		超出设定的CPU时间限制(4.2 BSD)
SIGXFSZ 25,25,31 	C 		超出设定的文件大小限制(4.2 BSD)
对于 SIGBUS，SIGSYS，SIGXCPU，SIGXFSZ，Linux 缺省的动作是 A ，SUSv2 是 C
---------------------------------------------------------------------------
	其它的信号
信号		值    		处理动作	发出信号的原因
SIGIOT 		6 			C 		IO捕获指令，与SIGABRT同义
SIGEMT 		7,-,7
SIGSTKFLT 	-,16,- 		A 		协处理器堆栈错误
SIGIO 		23,29,22 	A 		某I/O操作现在可以进行了(4.2 BSD)
SIGCLD 		-,-,18 		A 		与SIGCHLD同义
SIGPWR 		29,30,19 	A 		电源故障(System V)
SIGINFO 	29,-,- 		A 		与SIGPWR同义
SIGLOST 	-,-,- 		A 		文件锁丢失
SIGWINCH 	28,28,20 	B 		窗口大小改变(4.3 BSD, Sun)
SIGUNUSED 	-,31,- 		A 		未使用的信号(will be SIGSYS)
信号 29 在 Alpha 上为 SIGINFO / SIGPWR ，在 Sparc 上为 SIGLOST		 */

/**********************	Linux 信号汇总 **********************/
/*	信号列表	shell : $ kill -l
列表中，编号为 1 ~ 31 的信号为传统 UNIX 支持的信号，是 不可靠信号(非实时的)，
编号为 32 ~ 63 的信号 是 后来扩充的，称做 可靠信号(实时信号)。
不可靠信号 和 可靠信号 的区别在于 前者不支持排队，可能会造成信号丢失，而后者不会。
 1) SIGHUP		2) SIGINT	 	3) SIGQUIT	 	4) SIGILL	 	5) SIGTRAP
 6) SIGABRT	 	7) SIGBUS	 	8) SIGFPE	 	9) SIGKILL		10) SIGUSR1
11) SIGSEGV		12) SIGUSR2		13) SIGPIPE		14) SIGALRM		15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD		18) SIGCONT		19) SIGSTOP		20) SIGTSTP
21) SIGTTIN		22) SIGTTOU		23) SIGURG		24) SIGXCPU		25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF		28) SIGWINCH	29) SIGIO		30) SIGPWR
31) SIGSYS		34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX
---------------------------------------------------------------------------
信号详解 ：
1) SIGHUP 终止进程，终端线路挂断
 		本信号在用户终端连接(正常或非正常)结束时发出, 通常是在终端的控制进程结束时,
		通知同一session内的各个作业, 这时它们与控制终端不再关联.
2) SIGINT 终止进程，中断进程 Ctrl+C
 		程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl+C)时发出
3) SIGQUIT 和 SIGINT 类似, 但由 QUIT 字符(通常是Ctrl+\)来控制.
 		进程在因收到 SIGQUIT 退出时会产生core文件, 在这个意义上类似于一个程序错误信号.
4) SIGILL 执行了 非法指令. 通常是因为可执行文件本身出现错误, 或者试图执行数据段.
 		堆栈溢出时也有可能产生这个信号.
5) SIGTRAP 由断点指令或其它trap指令产生. 由debugger使用.
6) SIGABRT 执行I/O自陷，程序自己发现错误并调用abort时产生.
6) SIGIOT 跟踪自陷，在PDP-11上由iot指令产生, 在其它机器上和SIGABRT一样.
7) SIGBUS 总线错误，非法地址, 包括内存地址对齐(alignment)出错.
 		eg: 访问一个四个字长的整数，但其地址不是4的倍数.
 		某种特定的硬件异常，通常由内存访问引起
8) SIGFPE 在发生致命的算术运算错误时发出. 不仅包括浮点运算错误,
 		还包括溢出及除数为0等其它所有的算术的错误.
9) SIGKILL 用来立即结束程序（被杀）的运行. 本信号不能被阻塞, 处理和忽略.
10) SIGUSR1 留给用户使用
11) SIGSEGV 试图访问未分配给自己的内存, 或试图往没有写权限的内存地址写数据.
12) SIGUSR2 留给用户使用
13) SIGPIPE Broken pipe
14) SIGALRM 时钟定时信号, 计算的是实际的时间或时钟时间. alarm函数使用该信号.
15) SIGTERM 程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和处理.
 		通常用来要求程序自己正常退出. shell命令kill缺省产生这个信号.
17) SIGCHLD 子进程结束时, 父进程会收到这个信号.
18) SIGCONT 让一个停止(stopped)的进程继续执行. 本信号不能被阻塞.
 		可以用一个handler来让程序在由stopped状态变为继续执行时完成特定的工作.
 		例如, 重新显示提示符
19) SIGSTOP 停止(stopped)进程的执行. 注意它和terminate以及interrupt的区别:
 		该进程还未结束, 只是暂停执行. 本信号不能被阻塞, 处理或忽略.
20) SIGTSTP 停止进程的运行, 但该信号可以被处理和忽略.
 		用户键入SUSP字符时(通常是Ctrl+Z)发出这个信号
21) SIGTTIN 当后台作业要从用户终端读数据时, 该作业中的所有进程会收到SIGTTIN信号.
 		缺省时这些进程会停止执行.
22) SIGTTOU 类似于SIGTTIN, 但在写终端(或修改终端模式)时收到.
23) SIGURG 有"紧急"数据或out-of-band数据到达socket时产生.
24) SIGXCPU 超过CPU时间资源限制. 这个限制可以由getrlimit/setrlimit来读取/改变
25) SIGXFSZ 超过文件大小资源限制.
26) SIGVTALRM 虚拟时钟信号. 类似于SIGALRM, 但是计算的是该进程占用的CPU时间.
27) SIGPROF 类似于SIGALRM/SIGVTALRM, 但包括该进程用的CPU时间以及系统调用的时间.
28) SIGWINCH 窗口大小改变时发出.
29) SIGIO 文件描述符准备就绪, 可以开始进行输入/输出操作.
30) SIGPWR 电源（检测）失败
31) SIGSYS 非法的系统调用
---------------------------------------------------------------------------
在以上列出的信号中，程序 不可 捕获、阻塞 或 忽略 的信号有：SIGKILL,SIGSTOP
不能恢复至默认动作的信号有：SIGILL,SIGTRAP
默认会导致进程流产的信号有：
	SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,SIGXCPU,SIGXFSZ
默认会导致进程退出的信号有：
	SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,
 	SIGPROF,SIGSYS,SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM
默认会导致进程停止的信号有：SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU
默认进程忽略的信号有：SIGCHLD,SIGPWR,SIGURG,SIGWINCH
SIGIO 在 SVR4 是退出，在4.3BSD中是忽略；
SIGCONT 在进程挂起时是继续，否则是忽略，不能被阻塞。
---------------------------------------------------------------------------
信号表 ：
取值 名称 		解释 							默认动作
1 	SIGHUP 		挂起
2 	SIGINT 		中断
3 	SIGQUIT 	退出
4 	SIGILL 		非法指令
5 	SIGTRAP 	断点或陷阱指令
6 	SIGABRT 	abort发出的信号
7 	SIGBUS 		非法内存访问
8 	SIGFPE 		浮点异常
9 	SIGKILL 	被杀信号 						不能被忽略、处理和阻塞
10 	SIGUSR1 	用户信号1
11 	SIGSEGV 	无效内存访问
12 	SIGUSR2 	用户信号2
13 	SIGPIPE 	管道破损，没有读端的管道写数据
14 	SIGALRM 	alarm发出的信号
15 	SIGTERM 	终止信号
16 	SIGSTKFLT 	栈溢出
17 	SIGCHLD 	子进程退出 						默认忽略
18 	SIGCONT 	进程继续
19 	SIGSTOP 	进程停止 						不能被忽略、处理和阻塞
20 	SIGTSTP 	进程停止
21 	SIGTTIN 	进程停止，后台进程从终端读数据时
22 	SIGTTOU 	进程停止，后台进程想终端写数据时
23 	SIGURG 		I/O有紧急数据到达当前进程 			默认忽略
24 	SIGXCPU 	进程的CPU时间片到期
25 	SIGXFSZ 	文件大小的超出上限
26 	SIGVTALRM 	虚拟时钟超时
27 	SIGPROF 	profile时钟超时
28 	SIGWINCH 	窗口大小改变 					默认忽略
29 	SIGIO 		I/O相关
30 	SIGPWR 		关机 							默认忽略
31 	SIGSYS 		系统调用异常 								 */

#include "process_signal.h"

/********************* 注册 多参数信号函数 + 多(实时)信号处理 ***********************/

// sigaction() 支持信号传递信息，可用于所有信号安装
Sigfunc_rt Sigaction_rt(const int &signum, Sigfunc_rt func)
{
	struct sigaction sig_act{}, sig_oldact{}; // 信号处理

	sig_act.sa_sigaction = func; // 信号处理函数
//	sig_act.sa_handler = func; // 信号处理函数
	sigemptyset(&sig_act.sa_mask/*被屏蔽的信号*/); // 清除 信号组 中的所有信号
	sig_act.sa_flags = SA_SIGINFO; // 标示位: 使用三个参数而不是一个参数来调用信号捕获功能

	if (signum == SIGALRM) {
#ifdef	SA_INTERRUPT
		sig_act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	}
	else { // SA_RESTART 使被信号打断的 系统调用 syscall 自动重新发起
#ifdef	SA_RESTART
		sig_act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}

	/* 注册信号处理函数 *//* sigaction() 支持信号传递信息，可用于所有信号安装 */
	if ( sigaction(signum, &sig_act, &sig_oldact) < 0 ) return (Sigfunc_rt(SIG_ERR));
	return sig_oldact.sa_sigaction; // 返回旧信号处理程序
}

Sigfunc_rt Signal_rt(const int &signum, Sigfunc_rt func)
{
	Sigfunc_rt sigfunc = Sigaction_rt(signum, func);
	if (sigfunc == Sigfunc_rt(SIG_ERR)) // Error return
		err_sys(-1, __FILE__, __func__, __LINE__, "Sigaction_rt()");
	return sigfunc; // 返回旧信号处理程序
}

/********************* 测试 多参数信号函数 + 多(实时)信号处理 ***********************/

void sig_rt(int signo, siginfo_t *info, void *context) // 信号处理程序
{
	using namespace std;
	cout << "信号处理程序"
		 << "\tsignal " << info->si_signo
		 << "\tcode " 	<< info->si_code
		 << "\tsigval " << info->si_value.sival_int;
	if (info->si_errno != 0) cout << "\n" << strerror(info->si_errno);
#if 0
	cout << "\n"
		 << "_kill.si_pid" 				<< info->si_pid
		 << "_kill.si_uid" 				<< info->si_uid
		 << "_timer.si_overrun" 		<< info->si_overrun
		 << "_sigchld.si_status"		<< info->si_status
		 << "_sigchld.si_utime" 		<< info->si_utime
		 << "_sigchld.si_stime"			<< info->si_stime
		 << "_rt.si_sigval.sival_int" 	<< info->si_int
		 << "_rt.si_sigval.sival_ptr" 	<< info->si_ptr
		 << "_sigfault.si_addr" 		<< info->si_addr
		 << "_sigfault.si_addr_lsb" 	<< info->si_addr_lsb
		 << "_sigfault._bounds._addr_bnd._lower" << info->si_lower
		 << "_sigfault._bounds._addr_bnd._upper" << info->si_upper
		 << "_sigfault._bounds._pkey" 	<< info->si_pkey
		 << "_sigpoll.si_band" 			<< info->si_band
		 << "_sigpoll.si_fd" 			<< info->si_fd;
#if __SI_HAVE_SIGSYS
	cout << "\n"
		 << "_sigsys._call_addr" 	<< info->si_call_addr
		 << "_sigsys._syscall" 		<< info->si_syscall
		 << "_sigsys._arch" 		<< info->si_arch;
#endif
#endif
	cout << endl;
}

void test01_signal_rt() // 测试 多参数信号函数 + 多(实时)信号处理
{
	pid_t child_pid;
	if ((child_pid = Fork()) == 0) { /* 注册 信号处理程序 与 接收信号 */
		child_pid = getpid();
		_debug_log_info("test01_signal_rt(): 子进程 ID %d begin", child_pid)

		sigset_t newmask; // 要阻止，取消阻止或等待的 信号组
		Sigemptyset(&newmask); // 清除 信号组 中的所有信号
		Sigaddset(&newmask, SIGRTMAX - 0); // 信号 添加到 信号组
		Sigaddset(&newmask, SIGRTMAX - 1);
		Sigaddset(&newmask, SIGRTMAX - 2);
		Sigprocmask(SIG_BLOCK, &newmask, nullptr); // 单线程:检测或改变信号屏蔽字

		Signal_rt(SIGRTMAX - 0, sig_rt); // 注册 信号处理程序
		Signal_rt(SIGRTMAX - 1, sig_rt);
		Signal_rt(SIGRTMAX - 2, sig_rt);
		Signal_rt(SIGTERM/*终止信号*/, sig_rt);

		sleep(4);
		Sigprocmask(SIG_UNBLOCK, &newmask, nullptr);
//		sleep(2);

		_debug_log_info("test01_signal_rt(): 子进程 ID %d end", child_pid)
		exit(0);
	}

	sleep(2);
	pid_t parent_pid = getpid();
	_debug_log_info("test01_signal_rt(): 进程 ID %d begin", parent_pid)

	union sigval val{};

	/* parent sends nine signals to child */
	for (int sig_i = SIGRTMAX; sig_i >= SIGRTMAX - 2; --sig_i) {
		for (int j = 0; j <= 2; ++j) {
			val.sival_int = j; // 信号处理函数 整型 参数
			Sigqueue(child_pid, sig_i, val); // 将信号SIG发送到进程PID
			std::cout << "sent signal: " << sig_i << "\t参数: " << j << std::endl;
		}
	}

	for (int i = 0; i < 3; ++i) {
		val.sival_int = i; // 信号处理函数 整型 参数
		Sigqueue(child_pid, SIGTERM/*终止信号*/, val); // 将信号SIG发送到进程PID
		std::cout << "sent signal: " << SIGTERM << "\t参数: " << i << std::endl;
	}

	Waitpid(child_pid, nullptr, 0); // 等待子进程中断或结束
	_debug_log_info("test01_signal_rt(): 进程 ID %d end", parent_pid)
}

