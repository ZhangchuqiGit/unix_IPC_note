
#include "zcq_header.h"

#include <sys/types.h> // POSIX Standard: Primitive System Data Types
#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <iostream>

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/* ---- 创建 服务 子进程 ---- */
void service_fork(const int &socket_listen_fd, const int &socket_accept_fd)
{
	int ret_value; // 函数返回值

	/* ---- 创建 子进程 ---- */
	pid_t child_pid = fork(); // 创建 子进程
	if (child_pid == -1) // 负值 : 创建子进程失败。
	{
		err_ret(child_pid, __LINE__, "创建子进程失败");
	}
	else if (child_pid == 0) // 0 : 当前是 子进程 运行。
	{
//		cout<<"当前是子进程运行，child ID: "<< getpid() <<endl;
		/* ---- close () ---- */
		ret_value = close(socket_listen_fd);
		err_sys(ret_value, __LINE__, "close()");

		/** process the request **/

		/* ---- close () ---- */
		ret_value = close(socket_accept_fd);
		err_sys(ret_value, __LINE__, "close()");

		exit(0);
	}
	else // 正值 : 当前是 父进程 运行。正值 为 子进程 的 进程 ID。
	{
		std::cout << "创建子进程 child ID: " << child_pid << std::endl;

		/* ---- close () ---- */
		ret_value = close(socket_accept_fd);
		err_sys(ret_value, __LINE__, "close()");
	}
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/* definite */

void process_fork(); // fork() 创建 子进程
void process_exec(); // exec类()创建一个替代当前进程的新进程，执行后进程结束
void process_system(); // 建议 system()函数 只 用来执行 shell 命令
void test_popen_pclose(); // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令

/*	bash shell 	------------------------
	$ g++ test.cpp -o test
  	$ ./test myfile.txt youfile.txt one.txt
	c++ 		------------------------
		int main(int argc, char **argv)
----------------------------------------
  	c++    		bash shell
  	argc	=	${#argv[@]} # 参数个数
  	argv[0]	=	myfile.txt 	# 参数 : ./当前文件名
  	argv[1]	=	myfile.txt 	# 参数 1
  	argv[2]	=	youfile.txt # 参数 2
  	argv[3]	=	one.txt 	# 参数 3	  	*/
#if 0
int main(int argc, char **argv)
{
//	process_fork(); // fork() 创建 子进程
//	process_exec(); // exec类()创建一个替代当前进程的新进程，执行后进程结束
//	process_system(); // 建议 system()函数 只 用来执行 shell 命令
//	process_popen_pclose(); // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令

	return 0;
}
#endif


/*  父进程(调用者) 调用 fork() 创建 子进程，该 子进程 与 (父)进程 同时运行，
	两个进程 将执行 系统调用的函数 fork（）之后的 下一条指令。
 	两个进程 使用 相同的 pc（程序计数器），相同的 CPU 寄存器，打开 相同的 文件。
 	pid_t pid = fork();
返回值	负值：	创建子进程失败。
		0 ：	当前是 子进程 运行。
		正值：	当前是 父进程 运行。正值 为 子进程 的 进程 ID。		           */
void process_fork() // fork() 创建 子进程
{
	using namespace std;

	pid_t pid = fork(); // 创建 子进程

	if(pid == -1) // 负值 : 创建子进程失败。
	{
		cout<< "创建子进程失败，pid: " << pid << endl;
	}
	else if (pid==0) // 0 : 当前是 子进程 运行。
	{
		cout<<"当前是子进程运行，child ID: "<< getpid() <<endl;
	}
	else // 正值 : 当前是 父进程 运行。正值 为 子进程 的 进程 ID。
	{
		cout << "父进程休眠 1s，让子进程有机会运行" << endl;
		sleep(1); // 进程休眠 1s，让另一个进程有机会运行
		cout << "当前是父进程运行，parant ID: " << getpid()
			 << ", child ID: " << pid << endl;
	}
}


/*	exec类() 创建一个 替代 当前进程 的 新进程，但 进程 ID 不变，
 	用于执行某路径下文件，能执行 Linux 系统文件 或 Shell 命令(如 $ pwd(/bin/pwd)。
	当 函数 exec类() 执行后 进程 结束 [ 相当执行：exit(0); ]
exec 系列 函数: execl(),execlp(),execle(),execv(),execvp(),execvpe()
	当我们创建了一个进程之后，通常将 fork()创建的 子进程 替换 exec系列 进程，
 	当然，exec系列可将 当前进程 替换掉。
 	exec--- l p v e
int execl	(const char *path, const char *arg, ...);
int execlp	(const char *file, const char *arg, ...);
int execle 	(const char *path, const char *arg, ...);
int execv	(const char *path, char *const argv[] );
int execvp	(const char *file, char *const argv[] );
int execvpe (const char *file, char *const argv[], 	char *const envp[] );
参数 ：
	path			要启动 程序的名称 包括 路径名
	arg	或 argv[]	第一个参数 : 自定义名称；
 					从第二个参数开始为 启动 path 程序 所带的 命令参数。
					不是 带路径 且 *arg and argv[] 必须 以 NULL 结束。
返回值 :	成功返回 0, 失败返回 -1
------------------------------------------------------
exec 系列 函数 区别 ：
1. 带 l ：execl(),execlp(),execle()
 		参数 arg 以 可变参数 ... 的形式给出 且 都以一个空指针结束。
		如 	execl("/bin/ls","ls","-l",NULL); //
2. 带 p : execlp(),execvp(),execvpe()
 		第一个参数 file 不用输入完整路径，只给出命令名即可，
 		它会在环境变量 $PATH 当中 自动 查找命令。
 		如 	execlp("ls","ls","-l",NULL);
3. 带 v : execv(),execvp(),execvpe()
 		参数以 *argv[] 形式给出 且 argv[] 最后一个元素必须是 NULL。
 		如 	char *argv[] = {"ls","-l",NULL};
			execvp("ls",argv);
4. 带 e : execle(),execvpe()
		将 环境变量 envp[] 传递给 需要替换进程 的 environ[] 。
		recieve.cpp -----------
				// environ[] 接收 envp[] 传递 <unistd.h>
				#include <unistd.h> // environ[]
				int main(int argc, char **argv)
				{
					for (int i=0; environ[i] != nullptr; ++i) {
						printf("%d :\t%s\n", i, environ[i]);
					}
					return 0;
				}
		send.cpp -----------
				int main() {
					char *envp[] = {"AA=11", "BB=22", "CC=33", nullptr};
					char * argv[] = {"$1", "$2", "$3", NULL};
					execvpe("../test.o", argv, envp);
					return 0;
				}		 	*/
void process_exec() // exec类()创建一个替代当前进程的新进程，执行后进程结束
{
	using namespace std;
	int select =              2 ;

	char *argv1[] = {(char *)"ls", (char *)"-l", (char *)"./", nullptr};
	char *argv2[] = {(char *)"zcq", (char *)"$1", (char *)"$2", nullptr};
	char *argv3[] = {(char *)"$1", (char *)"$2", (char *)"$3", nullptr};
	char *envp[] = {(char *)"AA=11", (char *)"BB=22", (char *)"CC=33", nullptr};

	switch (select) {
		case 1:
			cout << "---------------------------- 1" << endl;
//			int execl(const char *path, const char *arg, ...);
#if 0
			/* 执行 /bin/ls，自定义名称 ls，参数为 -al ./
 			即 $ /bin/ls -al ./ 						*/
			execl("/bin/ls", "ls", "-al", "./", NULL);
#elif 0
			/* 执行 /bin/pwd，自定义名称 pwd
 			即 $ /bin/pwd		*/
			execl("/bin/pwd", "pwd", NULL);
#else
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			execl("../test.sh", "zcq", "$1", "$2", NULL);
#endif
			break;
		case 2:
			cout << "---------------------------- 2" << endl;
//			int execlp	(const char *file, const char *arg, ...);
#if 0
			/* 执行 /bin/ls，自定义名称 ls，参数为 -al ./
 			即 $ /bin/ls -al ./ 						*/
			execlp("ls", "ls", "-al", "./", NULL);
#elif 0
			/* 执行 pwd，自定义名称 pwd
 			即 $ pwd		*/
			execlp("pwd", "pwd", NULL);
#else
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			execlp("../test.sh", "zcq", "$1", "$2", NULL);
#endif
			break;
		case 3:
			cout << "---------------------------- 3" << endl;
//			int execle 	(const char *path, const char *arg, ...);
#if 1
			/* 执行 ls，自定义名称 ls， 参数为 -al
 			即 $ ls -al		*/
//			execle("/bin/ls", "ls", "-al", NULL);
			execle("/bin/ls", "ls", "-al", NULL, NULL);
#elif 0
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			execle("../test.sh", "zcq", "$1", "$2", NULL);
#else
			/* 执行 ../test.o，参数为 $1 $2 $3
 			即 $ ../test.o	"$1" "$2" "$3"  	*/
			execle("../test.o", "$1", "$2", "$3", NULL);
#endif
			break;
		case 4:
			cout << "---------------------------- 4" << endl;
//			int execv	(const char *path, char *const argv[] );
#if 1
			/* 执行 /bin/ls，自定义名称 ls，参数为 -al ./
 			即 $ /bin/ls -al ./ 						*/
			/*	char *argv1[] = {"ls", "-al", "./", NULL};	*/
			execv("/bin/ls", argv1);
#else
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			/*	char *argv2[] = {"zcq", "$1", "$2", NULL};	*/
			execv("../test.sh", argv2);
#endif
			break;
		case 5:
			cout << "---------------------------- 5" << endl;
//			int execvp	(const char *file, char *const argv[] );
#if 0
			/* 执行 /bin/ls，自定义名称 ls，参数为 -al ./
 			即 $ /bin/ls -al ./ 						*/
			/*	char *argv1[] = {"ls", "-al", "./", NULL};	*/
			execvp("ls", argv1);
#else
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			/*	char *argv2[] = {"zcq", "$1", "$2", NULL};	*/
			execvp("../test.sh", argv2);
#endif
			break;
		case 6:
			cout << "---------------------------- 6" << endl;
//			int execvpe (const char *file, char *const argv[], char *const envp[] );
#if 0
			/* 执行 /bin/ls，自定义名称 ls，参数为 -al ./
 			即 $ /bin/ls -al ./ 						*/
			/*	char *argv1[] = {"ls", "-al", "./", NULL};	*/
			execvpe("ls", argv1, nullptr);
#elif 0
			/* 执行 ../test.sh，自定义名称 "zcq"，参数为 $1 $2
 			即 $ ../test.sh	"$1" "$2"	*/
			/*	char *argv2[] = {"zcq", "$1", "$2", NULL};	*/
			execvpe("../test.sh", argv2, nullptr);
#else
			/* 执行 ../test.o，参数为 $1 $2 $3
 			即 $ ../test.o	"$1" "$2" "$3"  	*/
			/*	char *argv3[] = {"$1", "$2", "$3", NULL};	*/
			execvpe("../test.o", argv3, envp);
#endif
			break;
		default:
			break;
	}
	cout << "exec类()执行后进程结束，相当执行：exit(0); 此句不执行" << endl;
}


#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <cstring>
/*	int system(const char * command); // <stdlib.h>
	system() 函数调用 /bin/sh 来执行参数 command 指定的命令，
	/bin/sh 一般是一个软连接，指向某个具体的 shell，比如 bash，zsh
 	在该 command 执行期间，SIGCHLD 被暂时阻塞，
 	在该 command 执行期间，SIGINT 和 SIGQUIT 被忽略，进程收到信号后无动作。
-------------------------------------
system() 函数 执行过程：
	1.先调用 fork() 函数创建一个子进程；
	2.在子进程中调用 exec类() 函数(替代该子进程) 去执行 shell 命令 command；
	3.在父进程中调用 wait() 等待 子进程结束。
-------------------------------------
返回值 :
1.对于 fork() 失败，system()函数返回 -1；
2.如果 exec类() 执行成功，即 command 顺利执行完毕，通过 exit 或 return 返回值；
	注意，command 顺利执行 不代表执行成功，比如 $ rm debuglog.txt，
	不管文件存不存在该 command 都顺利执行了；
 	如果 exec类() 执行失败，即 command 没有顺利执行，
 	比如被信号中断，或 command 命令根本不存在，system() 函数返回 127。
3.如果 command 为 NULL，则 system() 函数返回 非0，一般为 1。
-------------------------------------
system() 函数简单实现
int system(const char * command) // <stdlib.h>
{
    pid_t pid;
    int status;
	if(command == NULL)	{
		return (1); //如果command为空，返回非零值，一般为1
	}
	if( (pid = fork()) < 0 )
	{
		status = -1; //fork() 创建子进程失败，返回 -1
	}
	else if(pid == 0) // 子进程
	{
		exec类("/bin/sh", "sh", "-c", command, (char *)0); // exec类()
		_exit(127); // 子进程 exec类() 执行失败返回 127，
		// 注意，exec类() 执行成功的话，现在的 子进程 就不存在
	}
	else // 父进程
	{
		while(waitpid(pid, &status, 0) < 0) //父进程 使用 wait() 或 waitpid()对子进程收尸
		{
			if(errno != EINTR) {
				status = -1; //如果 waitpid 被信号中断，则返回 -1
				break;
			}
		}
	}
    return status; //如果 父进程 waitpid 成功，则返回 子进程 的 返回 状态
}	*/
int system_shell(const char *cmd_line) // 屏蔽因系统升级或信号处理方式改变带来的影响
{
	int ret = 0;
/* 	sighandler_t is a type of a signal handler.  */
	sighandler_t old_handler; // 记录原来的处理方式
	/* system() 执行前，SIGCHLD 信号的处理方式不再是 SIG_DFL
	 * 把 SIGCHLD 信号处理方式显式修改为 SIG_DFL 方式 */
	/*	SIGCHLD，在一个进程终止或者停止时，将 SIGCHLD 信号发送给其 父进程，
按 系统默认 将忽略 此信号，如果父进程希望被告知其子系统的这种状态，则应捕捉此信号。*/
	old_handler = signal(SIGCHLD, SIG_DFL); // <signal.h>
	ret = system(cmd_line);
	signal(SIGCHLD, old_handler); // 使用完 system() 后设为原来的处理方式
	return ret;
}

/* 使用 system() 函数的 建议
那就是 system()函数的返回值。system() 函数有时很方便，但不可滥用！
1、建议 system()函数 只 用来执行 shell 命令，
	一般来讲，system()返回值 不是 0 就说明出错了；
2、建议监控 system() 函数的执行完毕后的 errno 值，争取出错时给出更多有用信息；
3、建议考虑 popen() 替代 system()。popen() 函数较于 system() 函数的优势在于使用简单 */
void process_system_debug(const char *cmd_shell) // 监控 system() 函数执行状态，尝试打印错误信息
{
	using namespace std;

	cout << "================================" << endl;
	if(cmd_shell == nullptr) // 如果 cmd_shell 为空
	{
		printf("cmd_shell 为 空\n");
		return;
	}
#if 1
	int status = system(cmd_shell); // 父进程 使用 wait() 或 waitpid() 对子进程收尸
#else
	int status = system_shell(cmd_shell); // 屏蔽因系统升级或信号处理方式改变带来的影响
#endif
	if(status < 0) {
		/* C 库函数 char *strerror(int errno) // <string.h>
		 * 从内部数组中搜索错误号 errno，并返回一个指向错误消息字符串的指针。
		 * strerror() 生成的错误字符串的内容取决于开发平台和编译器。 */
		printf("CMD error(%s) : %s\n", cmd_shell, strerror(errno));
		printf("status : %d\terrno : %d\n", status, errno);
		if( status == -1 ) // fork() 失败，返回 -1
		{	printf("fork() 创建子进程失败\n");	}
		else if( status == 127 ) // 子进程 exec类() 执行失败返回 127
		{	printf("子进程 exec类() 执行失败\n");	}
		return;
	}
	if(WIFEXITED(status)) //取得 cmd_shell 执行结果
	{
		printf("normal termination, exit status = %d\n",
			   WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status)) // 如果 cmd_shell 被 信号 中断
	{
		printf("abnormal termination, signal number =%d\n",
			   WTERMSIG(status)); // 取得 信号值
	}
	else if(WIFSTOPPED(status))	//如果 cmd_shell 被 信号 暂停 执行
	{
		printf("process stopped, signal number =%d\n",
			   WSTOPSIG(status)); // 取得信号值
	}
}

/*************************************************************************/
/* 建议 system() 只 用来执行 shell 命令 */
void unix_process_system(std::string_view shell_CMD)
{
	using namespace std;
	if (shell_CMD.empty()) // 如果 shell_CMD 为空
		err_quit(-1, __LINE__, "shell_CMD 为空");
#if 1
	int status = system(shell_CMD.data()); // 父进程 使用 wait() 或 waitpid() 对子进程收尸
#else
	int status = system_shell(shell_CMD.data()); // 屏蔽因系统升级或信号处理方式改变带来的影响
#endif
	if (status < 0) {
		err_quit(status, __LINE__, "system(): status = %d : %s",
				 status, (status == -1) ? "fork() 创建子进程失败" :
						 ((status == 127) ? "子进程 exec类() 执行失败"
										  : "unknown"));
	}
	if(WIFEXITED(status)) //取得 cmd_shell 执行结果:是否为正常退出
	{
		std::cout << "normal termination, exit status ="
				  << WEXITSTATUS(status)// 提取子进程的返回值
				  << std::endl;
		return;
	} else {
		std::cout << "abnormal termination, exit status ="
				  << WEXITSTATUS(status) // 提取子进程的返回值
				  << std::endl;
		if (WIFSIGNALED(status)) // 如果 cmd_shell 被 信号 中断
		{
			std::cout << "abnormal termination, signal number = "
					  << WTERMSIG(status) // 取得 信号值
					  << std::endl;
		} else if (WIFSTOPPED(status))    //如果 cmd_shell 被 信号 暂停 执行
		{
			std::cout << "process stopped, signal number = "
					  << WSTOPSIG(status) << std::endl; // 取得信号值
		}
	}
}
/*************************************************************************/

void process_system() // 建议 system()函数 只 用来执行 shell 命令
{
	using namespace std;

	process_system_debug(nullptr); // 监控 system() 函数执行状态，尝试打印错误信息
	sleep(1);
	process_system_debug("ls -al ../"); // 监控 system() 函数执行状态，尝试打印错误信息
	sleep(1);
	process_system_debug("pwd"); // 监控 system() 函数执行状态，尝试打印错误信息
	sleep(1);
	process_system_debug("echo \"${USER}\"");
	sleep(1);
	string shell = R"(echo "\033[1m\033[5m\033[32m012345\e[0m6789")"; // echo 默认带 -e
	process_system_debug(shell.c_str()); // 监控 system() 函数执行状态，尝试打印错误信息
	sleep(1);
	process_system_debug("../test.sh zcq"); // 监控 system() 函数执行状态，尝试打印错误信息
	sleep(1);
	process_system_debug("../test.o zcq"); // 监控 system() 函数执行状态，尝试打印错误信息
}


/*	popen() 函数较于 system() 函数的优势在于使用简单，
 	popen() 可以执行 shell 命令 : $ sh -c command
------------------------------------------------------------------
	FILE *popen (const char *command , const char *modes);
 		// 创建一个运行给定命令的管道(标准I/O流)，子进程 执行 shell 命令 <stdio.h>
	int	pclose (FILE * stream); // 关闭 popen()创建的管道，并返回其子级的状态 <stdio.h>
------------------------------------------------------------------
参数 modes 只能是 读 或者 写 中的一种，
 	得到的返回值（标准I/O流）也具有和 modes 相应的 只读 或 只写 类型。
	如果 modes 是 "r" 则返回值(文件指针)连接到 shell 的 标准输出；
	如果 modes 是 "w" 则返回值(文件指针)连接到 shell 的 标准输入。
参数 command 参数是一个指向以 NULL 结束的 shell 命令 的 字符串指针
------------------------------------------------------------------
返回值：
 	成功返回 子进程的 status，使用 WIFEXITED WIFSIGNALED WIFSTOPPED 相关宏可得返回结果
	不能分配内存将返回 NULL；如果调用 fork() 或 pipe() 失败返回 -1，
 	我们可以使用 perror 或 strerror(errno) 得到有用的错误信息，
------------------------------------------------------------------
popen() 函数通过创建一个运行给定命令的管道，调用 fork()产生一个子进程，执行一个 shell 命令。
这个进程必须由 pclose() 函数关闭。
pclose() 函数关闭 popen()创建的管道(标准I/O流)，等待命令执行结束，返回 shell 的终止状态。
如果 shell 不能被执行，则 pclose() 返回的终止状态与 shell 已执行 exit 一样。*/
//void process_popen_pclose_debug(const char *cmd_shell, const char *modes) // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令
void process_popen_pclose_debug(const char *cmd_shell) // 创建一个运行给定命令的管道(标准I/O流)，子进程 执行 shell 命令
{
	using namespace std;

	if (cmd_shell == nullptr) // 如果 cmd_shell 为空
	{
		cout << "================================" << endl;
		printf("cmd_shell 为 空\n");
		return;
	}
	FILE *fp  = popen(cmd_shell, "r"); // 子进程 执行 shell 命令
	// "r" 则返回值(文件指针)连接到 shell 的 标准输出；
	// "w" 则返回值(文件指针)连接到 shell 的 标准输入。
	if(!fp)
	{
		perror("popen() : shell create error");

		/* C 库函数 char *strerror(int errno) // <string.h>
		 * 从内部数组中搜索错误号 errno，并返回一个指向错误消息字符串的指针。
		 * strerror() 生成的错误字符串的内容取决于开发平台和编译器。 */
		printf("CMD error ! cmd_shell : %s\n", cmd_shell);
		printf("strerror(errno) : %s\n", strerror(errno));
		printf("errno : %d\n", errno);

		exit(EXIT_FAILURE); // Failing exit status.
	}
	if (WIFEXITED(errno)) //取得 cmd_shell 执行结果
	{
		printf("normal termination, exit status = %d\n",
			   WEXITSTATUS(errno));
	}
	else if (WIFSIGNALED(errno)) // 如果 cmd_shell 被 信号 中断
	{
		printf("abnormal termination, signal number =%d\n",
			   WTERMSIG(errno)); // 取得 信号值
	}
	else if (WIFSTOPPED(errno))    //如果 cmd_shell 被 信号 暂停 执行
	{
		printf("process stopped, signal number =%d\n",
			   WSTOPSIG(errno)); // 取得信号值
	}

	/* "r" 标准输出 */
	cout << "================================ 父进程" << endl;
	char buf[2];
	memset(buf, 0, sizeof(buf)); // 清空
	while( fgets(buf, std::size(buf), fp) != nullptr )
		// 父进程 获取 子进程 shell 终端输出
	{
		printf("%s", buf); // 打印终端输出的每一行内容
		memset(buf, 0, sizeof(buf)); // 清空
	}
	cout << "-------------       ------------" << endl;

	int ret = pclose(fp); // 关闭 popen()创建的管道，并返回其子级的状态
	cout << "shell 终止状态 : exit(" << ret << ")" << endl;
	if (ret != 0)
	{
		perror("popen() : shell create error");

		/* C 库函数 char *strerror(int errno) // <string.h>
		 * 从内部数组中搜索错误号 errno，并返回一个指向错误消息字符串的指针。
		 * strerror() 生成的错误字符串的内容取决于开发平台和编译器。 */
		printf("CMD error : %s\n", cmd_shell);
		printf("strerror(errno) : %s\n", strerror(errno));
		printf("errno : %d\n", errno);

		exit(ret); // Failing exit status.
	}
}

void popen_pclose() // 创建一个运行给定命令的管道(标准I/O流)，可以执行 shell 命令
{
	using namespace std;

	process_popen_pclose_debug(nullptr);
	sleep(1);
	process_popen_pclose_debug("ls -al ../");
	sleep(1);
	process_popen_pclose_debug("pwd");
	sleep(1);
	process_popen_pclose_debug("echo \"${USER}\"");
	sleep(1);
	string shell = R"(echo "\033[1m\033[5m\033[32m012345\e[0m6789")"; // echo 默认带 -e
	process_popen_pclose_debug(shell.c_str());
	sleep(1);
	process_popen_pclose_debug("../test.sh zcq");
	sleep(1);
	process_popen_pclose_debug("../test.o zcq");
	sleep(1);
	process_popen_pclose_debug("../test.sh qcz &"); // & 后台 shell 命令
	sleep(1);
//	process_popen_pclose_debug("~/clash-linux-amd64/clash-linux-amd64-v1.4.1");
	process_popen_pclose_debug("~/clash-linux-amd64/clash-linux-amd64*");
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/




