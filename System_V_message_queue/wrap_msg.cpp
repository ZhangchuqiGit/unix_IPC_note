//
// Created by zcq on 2021/5/16.
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
  	unsigned short   seq; //序列号
};		 */

#include "wrap_msg.h"


/**************** 自定义 可变长度 消息体 *****************/

void msg_data_set(struct msg_data &msgdata,
				  const __syscall_slong_t &mtype, const char &mtext,
				  const long &peer_id )
{
	msgdata.mtype = mtype;
	strncpy(msgdata.mtext, &mtext, 1);
	msgdata.peer_id = peer_id;
}

/**********************************************************/

void Msgctl(const int &msqid,
			const int &cmd, struct msqid_ds *buf) // 控制: 修改、删除 消息队列
{
	if (msgctl(msqid, cmd, buf) == -1) // 控制: 修改、删除 消息队列
//		if (errno != EIDRM/*Identifier removed*/)
		err_sys(-1, __FILE__, __func__, __LINE__, "msgctl()");
}

int Msgget(const key_t &key, const int &msgflg) // 创建 新的 或 打开 一个消息队列
{
	int msqid; // 消息队列的标识符
	if ((msqid = msgget(key, msgflg)) < 0) // 创建 新的 或 打开 一个消息队列
		err_sys(msqid, __FILE__, __func__, __LINE__, "msgget()");
	return msqid;
}

ssize_t Msgrcv(const int &msqid,
			   void *msgp, const size_t &msgsz,
			   const long &msgtyp,
			   const int &msgflg,
			   const uint8_t mode) // 从消息队列接收消息
{
	memset(msgp, 0, msgsz);
	ssize_t sizeval;
	if ((sizeval = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg)) < 0) {
		if (errno == EINTR/* Interrupted system call */ ) return 0;
		if (errno == EAGAIN/* O_NONBLOCK : Try again */ ) return -1234;
		if (errno == ENOMSG/* 非阻塞，等待时返回错误 */ ) return -1235;
		err_sys(sizeval, __FILE__, __func__, __LINE__, "msgrcv()");
	}
	if (mode == 1 && sizeval > 1) {
		char *ptr = (char *)msgp;
		ptr = ptr + sizeval - 1;
		/* 去除 换行符: read()读取数据时会将最后的回车(\n)同时读入到buf中 */
		if (*ptr == '\n') {
			*ptr = '\0';
			sizeval--;
		}
	}
	return (sizeval);
}

void Msgsnd(const int &msqid,
			const void *msgp, const size_t &msgsz,
			const int &msgflg) // 将消息发送到消息队列
{
	if (msgsnd(msqid, msgp, msgsz, msgflg) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "msgsnd()");
}

