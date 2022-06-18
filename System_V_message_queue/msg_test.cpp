//
// Created by zcq on 2021/5/10.
//

/** linux 命令：$ ipcs #显示所有 消息队列/共享内存段/信号量数组
 ipcs -q  显示所有 消息队列
 ipcs -m  显示所有 共享内存段
 ipcs -s  显示所有 信号量数组
 ipcrm -h 查看删除选项    **/

/* shell : ipcs
 消息队列与管道不同的是，消息队列是基于消息的，而管道是基于字节流的，且消息队列的读取不一定是先入先出。
 消息队列也有管道一样的不足，就是每个消息的最大长度是有上限的（MSGMAX），
 每个消息队列的总的字节数是有上限的（MSGMNB），
 系统上消息队列的总数也有一个上限（MSGMNI），
 这三个参数都可以查看：
 cat /proc/sys/kernel/msgmax  # 一条消息的长度/字节数   		8192
 cat /proc/sys/kernel/msgmnb  # 一个消息队列总的容纳长度 		8192*2=16384
 cat /proc/sys/kernel/msgmni  # 系统中创建消息队列的最大个数 	32000
 ipcs  -q      		# 查看IPC消息队列命令
 ipcrm -q msqid 	# 删除命令
 ipcrm -Q key  		# 删除命令,如何key为0,则不能删除   */

/**************** 消息队列 msg 两个不相关的进程之间通信 ****************/
/*	消息队列提供了一种在两个不相关的进程之间传递数据的简单有效方法，
独立于发送和接收进程而存在。消息队列在进程间以数据块为单位传递数据，
每个数据块都有一个类型标记，接受进程可以独立地接受含有不同类型值的数据块。
Linux 系统有两个宏定义 MSGMAX 与 MSGMNB，
它们以字节为单位分别定义了一条消息和一个队列的最大长度。
---------------------------------------------------------------------------
特点:
	1.消息队列可以实现消息的随机查询。消息不一定要以先进先出的次序读取，
编程时可以按消息的类型读取。
	2.消息队列允许一个或多个进程向它写入或者读取消息。
	3.与无名管道、命名管道一样，从消息队列中读出消息，消息队列中对应的数据都会被删除。
	4.每个消息队列都有消息队列标识符，消息队列的标识符在整个系统中是唯一的。
	5.消息队列是消息的链表，存放在内存中，由内核维护。只有内核重启或人工删除消息队列时，
该消息队列才会被删除。若不人工删除消息队列，消息队列会一直存在于系统中。
---------------------------------------------------------------------------
	同一个地址才能保证是同一个银行，同一个银行双方才能借助它来托管，
同一个保险柜号码才能保证是对方托管给自己的东西。
 	而在消息队列操作中，键（key）值相当于地址，消息队列标示符相当于具体的某个银行，
消息类型相当于保险柜号码。
 	同一个 键值（key） 可以保证是同一个消息队列，
 	同一个 消息队列标示符 才能保证不同的进程可以相互通信，
 	同一个 消息类型 才能保证某个进程取出是对方的信息。		*/

/**********************	消息队列 msg 常用操作函数  **********************/
/*	<sys/msg.h>  <sys/types.h>  <sys/ipc.h>
 	同一个 键值（key） 可以保证是 同一个 消息队列(标示符)，
 	同一个 消息队列(标示符) 才能保证不同的进程可以相互通信，
 	同一个 消息类型 才能保证某个进程取出是对方的信息。
---------------------------------------------------------------------------
key_t ftok(const char *pathname, int id); // 生成键值(可能会变，建议指定一个固定key值)
参数：pathname: 路径名；	id: 项目 ID，非 0 整数(只有低 8 位有效,1~127)
返回值：成功返回 key 值；	失败返回 -1
---------------------------------------------------------------------------
int msgget(key_t key, int msgflag); // 创建 新的 或 打开 一个消息队列。
不同的进程调用此函数，同一个 key 值就能得到 同一个 消息队列的标识符。
参数：
key: 键值；	特殊 键值：IPC_PRIVATE((__key_t)0)创建 私有队列
msgflag:	IPC_CREAT | IPC_EXCL | 0777(文件权限)
	IPC_CREAT	如果消息队列不存在，则创建消息队列，否则进行打开操作。
	IPC_EXCL	如果消息队列存在，则失败。必须配合 IPC_CREAT，表示“不存则创，存则失败”
	IPC_NOWAIT	非阻塞模式，等待时立即返回 -1。
返回值：成功返回 消息队列的标识符；	失败返回 -1
---------------------------------------------------------------------------
int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflag);
// 发送消息。
参数：
msqid：消息队列的标识符，由 msgget()获得。
msg_ptr：指向要发送消息结构体的地址。
msg_sz：要发送消息的长度（字节数）。
msgflag：函数行为的控制属性，其取值如下：
	0：调用阻塞直到发送消息成功为止。
	IPC_NOWAIT: 非阻塞，等待时返回错误 -1(errno=EAGAIN)，若消息没有立即发送则调用该函数立即返回。
返回值：	成功返回 0；
 		失败返回 -1: 当消息被删除errno=EIDRM；调用线程被信号中断errno=EINTR
---------------------------------------------------------------------------
int msgrcv(int msqid, void *msg_ptr, size_t msg_sz, long msgtype, int msgflag);
// 读取信息
从标识符为 msqid 的消息队列中接收一个消息。一旦接收消息成功，则消息在消息队列中被删除。
参数：
msqid：消息队列的标识符，由 msgget()获得，代表要从哪个消息列中获取消息。
msg_ptr：指向要存放消息结构体的地址。
msg_sz：要存放消息的长度（字节数）。
msgtype：消息的类型。可以有以下几种类型：
	msgtyp = 0：返回队列中的第一个消息。
	msgtyp > 0：返回队列中消息类型为 msgtyp 的消息（常用）。
	msgtyp < 0：返回队列中消息类型值小于或等于 msgtyp 绝对值的消息，
			如果这种消息有若干个，则取类型值最小的消息。
	注意：若队列中有多条此类型的消息，则获取最先添加的消息，即先进先出原则。
 		msgtype		返回队列中消息类型
			0			100
			100			100
			200			200
			-100		100
			-200		100
 struct msg_data // 自定义数据包时,第一个结构成员 必须 为 __syscall_slong_t 的 msgtype
 {
 	__syscall_slong_t mtype;	// 收到消息的类型 msgtype
 	...
 }
msgflag：函数行为的控制属性，其取值如下：
	0：调用阻塞直到接收消息成功为止。
	IPC_NOWAIT: 非阻塞，等待时返回错误(errno=ENOMSG)，若没有收到消息立即返回 -1。
	MSG_NOERROR: 即使消息太大也没有错误。若返回的消息字节数比 nbytes 字节数多，
 			则消息就会截短到 nbytes 字节，且不通知消息发送的进程。
返回值：	成功返回 读取消息的长度；
 		失败返回 -1: 当消息被删除errno=EIDRM；调用线程被信号中断errno=EINTR
---------------------------------------------------------------------------
int msgctl(int msqid, int cmd, struct msqid_ds *buf); // 控制: 修改、删除 消息队列
对消息队列进行各种控制，如修改消息队列的属性，或删除消息消息队列。
参数：
msqid：消息队列的标识符，由 msgget()获得。
cmd：函数功能的控制。其取值如下：
	IPC_RMID：删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数。
	IPC_STAT：将 msqid 相关的数据结构中的元素的当前值 存入到 由 buf 指向的结构中。
	IPC_SET：将 msqid 相关的数据结构中的元素(msqid_ds{} 中的 msg_perm{})
 				设置为 由 buf 指向的结构中的对应值。
buf：数据结构 msqid_ds 地址，用来存放或更改消息队列的属性。
返回值：成功返回 0；	失败返回 -1
---------------------------------------------------------------------------
	linux内核采用的结构 msqid_ds 管理 消息队列
struct msqid_ds	{	// 消息队列的属性
    struct ipc_perm msg_perm; //消息队列访问权限
    struct msg *msg_first; //指向第一个消息的指针
    struct msg *msg_last;  //指向最后一个消息的指针
	ulong  msg_cbytes;  //消息队列当前的字节数
	ulong  msg_qnum;    //消息队列当前的消息个数
	ulong  msg_qbytes;  //消息队列可容纳的最大字节数
	pid_t  msg_lsqid;   //最后发送消息的进程号ID
	pid_t  msg_lrqid;   //最后接收消息的进程号ID
	time_t msg_stime;   //最后发送消息的时间
	time_t msg_rtime;   //最后接收消息的时间
	time_t msg_ctime;   //最近修改消息队列的时间
};
---------------------------------------------------------------------------
	linux内核采用的结构 msg_queue 来描述 消息队列
struct msg_queue {	  // 消息队列的状态
	struct ipc_perm q_perm;
	time_t q_stime;   // last msgsnd() time
	time_t q_rtime;   // last msgrcv() time
	time_t q_ctime;   // last change time
	unsigned long q_cbytes; // 当前队列中的字节数
	unsigned long q_qnum;   // 当前队列中的消息数
	unsigned long q_qbytes; // 队列上的最大字节数
	pid_t q_lspid;    // 最后一刻 msgsnd()
	pid_t q_lrpid;    // last receive pid
	struct list_head q_messages;
	struct list_head q_receivers;
	struct list_head q_senders;
};
---------------------------------------------------------------------------
	syetem V IPC 为每一个 IPC 结构设置了一个 ipc_perm 结构，该结构规定了许可权和所有者
struct ipc_perm {
   	key_t  key;    //调用shmget()时给出的关键字
   	uid_t  uid;    //共享内存所有者的有效用户ID
  	gid_t  gid;    //共享内存所有者所属组的有效组ID
   	uid_t  cuid;   //共享内存创建 者的有效用户ID
   	gid_t  cgid;   //共享内存创建者所属组的有效组ID
  	unsigned short  mode; //Permissions + SHM_DEST和SHM_LOCKED标志
  	unsigned short  seq;  //序列号
};		 */

#include "msg_test.h"

/******************************************************************************/

/* Generates key for System V style IPC.
key_t ftok(const char *pathname, int id);//生成键值
注意当 pathname 文件不存在，键值会变，建议 自定义 key 键值
参数：pathname: 路径名；		id: 项目 ID，非 0 整数(只有低 8 位有效,1~127)
返回值：成功返回 key 值；	失败返回 -1		*/
key_t Ftok(const char *pathname, const int &proi_id) // 生成键值
{
	using namespace std;
	key_t key; // 键值
	if (pathname == nullptr) {
		key = random();
	}
	else {
		key = ftok(pathname, proi_id);
		err_sys(key, __FILE__, __func__, __LINE__, "ftok()");
//		perror("ftok failed");
//		exit(-1);
	}
	cout << "key = " << key << endl;
	return key;
}

/*
int msgctl(int msqid, int cmd, struct msqid_ds *buf); // 控制: 修改、删除 消息队列
对消息队列进行各种控制，如修改消息队列的属性，或删除消息消息队列。
参数：
msqid：消息队列的标识符，由 msgget()获得。
cmd：函数功能的控制。其取值如下：
	IPC_RMID：删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数。
	IPC_STAT：将 msqid 相关的数据结构中的元素的当前值 存入到 由 buf 指向的结构中。
	IPC_SET：将 msqid 相关的数据结构中的元素(msqid_ds{} 中的 msg_perm{})
 				设置为 由 buf 指向的结构中的对应值。
buf：数据结构 msqid_ds 地址，用来存放或更改消息队列的属性。
返回值：成功返回 0；	失败返回 -1
---------------------------------------------------------------------------
	linux内核采用的结构 msqid_ds 管理 消息队列
struct msqid_ds	{	// 消息队列的属性
    struct ipc_perm msg_perm; //消息队列访问权限
    struct msg *msg_first; //指向第一个消息的指针
    struct msg *msg_last;  //指向最后一个消息的指针
	ulong  msg_cbytes;  //消息队列当前的字节数
	ulong  msg_qnum;    //消息队列当前的消息个数
	ulong  msg_qbytes;  //消息队列可容纳的最大字节数
	pid_t  msg_lsqid;   //最后发送消息的进程号ID
	pid_t  msg_lrqid;   //最后接收消息的进程号ID
	time_t msg_stime;   //最后发送消息的时间
	time_t msg_rtime;   //最后接收消息的时间
	time_t msg_ctime;   //最近修改消息队列的时间
};	*/
void msg_stat(const int &msqid) // 输出消息队列属性
{
	struct msqid_ds msg_info{};
	Msgctl(msqid, IPC_STAT, &msg_info); // 控制: 修改、删除 消息队列

	printf( "消息队列当前的字节数: %ld\n",msg_info.msg_cbytes);
	printf( "number of messages in queue is %lu\n",msg_info.msg_qnum);
	printf( "max number of bytes on queue is %lu\n",msg_info.msg_qbytes);
	//每个消息队列的容量（字节数）都有限制MSGMNB，值的大小因系统而异。在创建新的消息队列时，
	// msg_qbytes的缺省值就是MSGMNB
	printf( "pid of last msgsnd is %d\n",msg_info.msg_lspid);
	printf( "pid of last msgrcv is %d\n",msg_info.msg_lrpid);
	printf( "last msgsnd time is %s", ctime(&(msg_info.msg_stime)));
	printf( "last msgrcv time is %s", ctime(&(msg_info.msg_rtime)));
	printf( "last change time is %s", ctime(&(msg_info.msg_ctime)));
	printf( "msg uid is %d\n",msg_info.msg_perm.uid);
	printf( "msg gid is %d\n",msg_info.msg_perm.gid);
}

/******************************************************************************/

#define MSG_KEY (123450) // 自定义键值

/***************************** 一对一 简单应答 **********************************/

#define OneToOne  11111  /** 从消息队列 只接收 类型为 OneToOne 的消息 **/

void test_msg_service() // 独立 服务器 消息队列
{
	int msqid; // 消息队列的标识符
	msqid = Msgget(MSG_KEY, IPC_CREAT/*若无则创建*/ | FILE_MODE/*文件权限*/);

	pid_t pid = getpid();
	_debug_log_info("test_msg_service(): 进程 ID %d begin; msqid: %d", pid, msqid)

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;
	while (true) {
		recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata), OneToOne, 0);
		// 从消息队列接收消息
		std::cout << "--------------------------------------------\n"
				  << "service recieve[" << recvlen << "]: "
				  << "mtype: " << msgdata.mtype << "\t"
				  << "mtext: " << msgdata.mtext << "\t"
				  << "data: " << msgdata.data << std::endl;

		msg_stat(msqid); // 输出消息队列属性
		if (if_run("quit", msgdata.data)) break;
		if (if_run("time", msgdata.data)) {
			memset(msgdata.data, 0, 64);
			time_t ticks;
/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
			ticks = time(nullptr);
			snprintf(msgdata.data, sizeof(msgdata.data), "%.24s\n", ctime(&ticks));
		}

		msg_data_set(msgdata, OneToOne, 'S', 0);
		Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0); // 将消息发送到消息队列
	}

	struct msqid_ds msqidds{};
	/* 删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数 */
	Msgctl(msqid, IPC_RMID, &msqidds); // 控制: 修改、删除 消息队列

	_debug_log_info("test_msg_service(): 进程 ID %d end; msqid: %d", pid, msqid)
}

void test_msg_client() // 独立 客户端 消息队列
{
	int msqid; // 消息队列的标识符
	msqid = Msgget(MSG_KEY, FILE_MODE/*文件权限*/);

	pid_t pid = getpid();
	_debug_log_info("test_msg_client(): 进程 ID %d begin; msqid: %d", pid, msqid)

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;
	while (true) {
		if (Read(STDIN_FILENO, msgdata.data, sizeof(msgdata.data)) <= 0) break;

		msg_data_set(msgdata, OneToOne, 'C', 0);
		Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0); // 将消息发送到消息队列
		std::cout << "--------------------------------------------\n"
				  << "client send:\t"
				  << "mtype: " << msgdata.mtype << "\t"
				  << "mtext: " << msgdata.mtext << "\t"
				  << "data: " << msgdata.data << std::endl;

		msg_stat(msqid); // 输出消息队列属性
		if (if_run("quit", msgdata.data)) break;

		recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata), OneToOne, 0);
		// 从消息队列接收消息
		std::cout << "client recieve[" << recvlen << "]: "
				  << "mtype: " << msgdata.mtype << "\t"
				  << "mtext: " << msgdata.mtext << "\t"
				  << "data: " << msgdata.data << std::endl;
	}

	_debug_log_info("test_msg_client(): 进程 ID %d end; msqid: %d", pid, msqid)
}

/***************************** 一对多 迭代服务器 *********************************/
/* 						服务器
				服务器应答 ↓ ↑ 客户请求
						队列
					↗  ↙    ↘  ↖
					客户1    客户2             */

void test_msg_service_echo() // 独立 迭代服务器 消息队列
{
	int msqid; // 消息队列的标识符
	msqid = Msgget(MSG_KEY, IPC_CREAT/*若无则创建*/ | FILE_MODE/*文件权限*/);

	pid_t pid = getpid();
	_debug_log_info("test_msg_service_echo(): 进程 ID %d begin; msqid: %d", pid, msqid)

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;
	while (true) {
		recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
						 0/** 0:从消息队列 接收所有类型的消息*/, 0);
		// 从消息队列接收消息
		std::cout << "--------------------------------------------\n"
				  << "service recieve[" << recvlen << "]: "
				  << "mtype: " << msgdata.mtype << "\t"
				  << "mtext: " << msgdata.mtext << "\t"
				  << "data: " << msgdata.data << std::endl;

		if (if_run("quit", msgdata.data)) break;
		if (if_run("time", msgdata.data)) {
			memset(msgdata.data, 0, 64);
			time_t ticks;
/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
			ticks = time(nullptr);
			snprintf(msgdata.data, sizeof(msgdata.data), "%.24s\n", ctime(&ticks));
		}

		msg_data_set(msgdata, msgdata.mtype, 'S', pid);
		Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0); // 将消息发送到消息队列
	}

	struct msqid_ds msqidds{};
	/* 删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数 */
	Msgctl(msqid, IPC_RMID, &msqidds); // 控制: 修改、删除 消息队列

	_debug_log_info("test_msg_service_echo(): 进程 ID %d end; msqid: %d", pid, msqid)
}

void test_msg_client_call() // 独立 客户端 消息队列
{
	int msqid; // 消息队列的标识符
	msqid = Msgget(MSG_KEY, FILE_MODE/*文件权限*/);

	pid_t pid = getpid();
	_debug_log_info("test_msg_client_call(): 进程 ID %d begin; msqid: %d", pid, msqid)

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;
	while (true) {
		if (Read(STDIN_FILENO, msgdata.data, MSG_DATA_size) > 0) {
			msg_data_set(msgdata, pid, 'C', 0);
			Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0); // 将消息发送到消息队列
			std::cout << "--------------------------------------------\n"
					  << "client send:\t"
					  << "mtype: " << msgdata.mtype << "\t"
					  << "mtext: " << msgdata.mtext << "\t"
					  << "data: " << msgdata.data << std::endl;

			recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
							 pid/**从消息队列接收指定 pid 类型的消息*/, 0);
			// 从消息队列接收消息
			std::cout << "client recieve[" << recvlen << "]: "
					  << "mtype: " << msgdata.mtype << "\t"
					  << "mtext: " << msgdata.mtext << "\t"
					  << "peer_id: " << msgdata.peer_id << "\t"
					  << "data: " << msgdata.data << std::endl;
		}
	}
	_debug_log_info("test_msg_client_call(): 进程 ID %d end; msqid: %d", pid, msqid)
}

/*************************** 一对多 fork 迸发服务器 ******************************/
/* 			服务器		←	服务器	→		服务器
			(服务器应答) 		↑ (客户请求) 	(服务器应答)
			↓				队列				↓
			队列			↗  			↖		队列
				客户1 					客户2	    */

void test_msg_service_fork() // 独立 迸发服务器 消息队列
{
	int msqid; // 消息队列的标识符
	msqid = Msgget(MSG_KEY, IPC_CREAT/*若无则创建*/ | FILE_MODE/*文件权限*/);

	pid_t pid_p = getpid(), pid_c;
	_debug_log_info("test_msg_service_fork(): 进程 ID %d begin; msqid: %d", pid_p, msqid)

/*
	SIGCHLD，在一个进程终止或者停止时，将 SIGCHLD 信号发送给其 父进程，
按 系统默认 将忽略 此信号，如果父进程希望被告知其子系统的这种状态，则应捕捉此信号。
	SIGCHLD 属于 unix 以及 类unix 系统的一种信号，
产生原因 siginfo_t 代码值：
	1，子进程已终止 CLD_EXITED
	2，子进程异常终止（无core） CLD_KILLED
	3，子进程异常终止（有core） CLD_DUMPED
	4，被跟踪子进程以陷入 CLD_TRAPPED
	5，子进程已停止 CLD_STOPED
	6，停止的子进程已经继续 CLD_CONTINUED   */
	Signal_fork(SIGCHLD); // 注册 信号处理程序

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;
	while (true) {
		recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
						 0/** 0:从消息队列 接收所有类型的消息*/, 0);
		// 从消息队列接收消息
		std::cout << "--------------------------------------------\n"
				  << "service recieve[" << recvlen << "]: "
				  << "mtype: " << msgdata.mtype << "\t"
				  << "mtext: " << msgdata.mtext << "\t"
				  << "peer_id: " << msgdata.peer_id << "\t"
				  << "data: " << msgdata.data << std::endl;

		if (strcmp(msgdata.mtext, "C") == 0 && (pid_c = Fork()) == 0) {	/* child */
			usleep(1000); // 1 毫秒
			msqid = Msgget(MSG_KEY + msgdata.peer_id, FILE_MODE/*文件权限*/);
			pid_c = getpid();
			_debug_log_info("子进程 ID %d begin; msqid: %d", pid_c, msqid)

			msg_data_set(msgdata, msgdata.peer_id, 's', pid_c);
			bzero(msgdata.data, MSG_DATA_size);
			snprintf(msgdata.data, MSG_DATA_size, "迸发服务器\n");
			Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0);
			// 将消息发送到消息队列

			while (true) {
				recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
								 0/** 0:从消息队列 接收所有类型的消息*/, 0);
				// 从消息队列接收消息
				std::cout << "--------------------------------------------\n"
						  << "service recieve[" << recvlen << "]: "
						  << "mtype: " << msgdata.mtype << "\t"
						  << "mtext: " << msgdata.mtext << "\t"
						  << "peer_id: " << msgdata.peer_id << "\t"
						  << "data: " << msgdata.data << std::endl;

				if (if_run("quit", msgdata.data)) break;
				if (if_run("time", msgdata.data)) {
					memset(msgdata.data, 0, 64);
					time_t ticks;
/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
					ticks = time(nullptr);
					snprintf(msgdata.data, sizeof(msgdata.data),
							 "%.24s\n", ctime(&ticks));
				}

				msg_data_set(msgdata, msgdata.peer_id, 's', pid_c);
				Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0);
				// 将消息发送到消息队列
			}

			_debug_log_info("子进程 ID %d end; msqid: %d", pid_c, msqid)
			exit(0); // child terminates
		}

		_debug_log_info("ID %d 创建 子进程 ID %d", pid_p, pid_c)
	}

	struct msqid_ds msqidds{};
	/* 删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数 */
	Msgctl(msqid, IPC_RMID, &msqidds); // 控制: 修改、删除 消息队列

	_debug_log_info("test_msg_service_fork(): 进程 ID %d end; msqid: %d", pid_p, msqid)
}

void test_msg_client_fcall() // 独立 客户端 消息队列
{
	int msqid_service, msqid; // 消息队列的标识符
	msqid_service = Msgget(MSG_KEY, FILE_MODE/*文件权限*/);

	pid_t pid = getpid();
	_debug_log_info("test_msg_client_fcall(): 进程 ID %d begin; msqid_service: %d",
					pid, msqid_service)

	msqid = Msgget(MSG_KEY + pid,
				   IPC_CREAT/*若无则创建*/ | FILE_MODE/*文件权限*/);
	_debug_log_info("test_msg_client_fcall(): 进程 ID %d begin; msqid: %d", pid, msqid)

	struct msg_data msgdata{}; // 自定义数据包
	ssize_t recvlen;

	msg_data_set(msgdata, pid, 'C', pid);
	bzero(msgdata.data, MSG_DATA_size);
	snprintf(msgdata.data, MSG_DATA_size, "客户端\n");
	Msgsnd(msqid_service, &msgdata, MSG_DATA_sendLEN(msgdata), 0);
	// 将消息发送到消息队列

	recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
					 pid/**从消息队列接收指定 pid 类型的消息*/, 0);
	// 从消息队列接收消息
	std::cout << "client recieve[" << recvlen << "]: "
			  << "mtype: " << msgdata.mtype << "\t"
			  << "mtext: " << msgdata.mtext << "\t"
			  << "peer_id: " << msgdata.peer_id << "\t"
			  << "data: " << msgdata.data << std::endl;

	while (true) {
		if (Read(STDIN_FILENO, msgdata.data, MSG_DATA_size) > 0) {
			msg_data_set(msgdata, pid, 'C', pid);
			Msgsnd(msqid, &msgdata, MSG_DATA_sendLEN(msgdata), 0);
			// 将消息发送到消息队列
			std::cout << "--------------------------------------------\n"
					  << "client send:\t"
					  << "mtype: " << msgdata.mtype << "\t"
					  << "mtext: " << msgdata.mtext << "\t"
					  << "my_id: " << msgdata.peer_id << "\t"
					  << "data: " << msgdata.data << std::endl;

			if (if_run("quit", msgdata.data)) break;

			recvlen = Msgrcv(msqid, &msgdata, sizeof(msgdata),
							 pid/**从消息队列接收指定 pid 类型的消息*/, 0);
			// 从消息队列接收消息
			std::cout << "client recieve[" << recvlen << "]: "
					  << "mtype: " << msgdata.mtype << "\t"
					  << "mtext: " << msgdata.mtext << "\t"
					  << "peer_id: " << msgdata.peer_id << "\t"
					  << "data: " << msgdata.data << std::endl;
		}
	}

	struct msqid_ds msqidds{};
	/* 删除由 msqid 指示的消息队列，该队列的任何消息都被丢弃，不使用第3个参数 */
	Msgctl(msqid, IPC_RMID, &msqidds); // 控制: 修改、删除 消息队列

	_debug_log_info("test_msg_client_fcall(): 进程 ID %d end; msqid: %d", pid, msqid)
}

