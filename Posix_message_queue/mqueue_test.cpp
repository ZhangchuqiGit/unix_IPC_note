//
// Created by zcq on 2021/5/13.
//

/** set( CMAKE_C_CXX_FLAGS_behind "-lrt" )   # 修改 后面 编译选项 ！ **/
/** linux 命令：$ ipcs #显示所有 消息队列/共享内存段/信号量数组
 ipcs -q  显示所有 消息队列
 ipcs -m  显示所有 共享内存段
 ipcs -s  显示所有 信号量数组
 ipcrm -h 查看删除选项    **/

/* 	Linux进程间通信之POSIX消息队列  #include <mqueue.h>
	消息队列可认为是一个消息链表，它允许进程之间以消息的形式交换数据。
 有足够写权限的进程或线程可往队列中放置消息，有足够读权限的进程或线程可从队列中取走消息。
 每个消息都是一个记录，它由发送者赋予一个优先级。
 与管道不同，管道是字节流模型，没有消息边界。
 POSIX 消息队列 与 System V 消息队列的 相似之处 在于数据的交换单位是整个消息，
 但它们之间仍然存在一些显著的差异：
 1.POSIX消息队列是引用计数的。
 	只有当所有当前使用队列的进程都关闭了队列之后才会对队列进行标记以便删除。
 2.每个System V消息都有一个整数类型，并且通过msgrcv()可以以各种方式类选择消息。
 与之形成鲜明对比的是，POSIX消息有一个关联的优先级，并且消息之间是严格按照优先级顺序排队的(以及接收)。
 3.POSIX消息队列提供了一个特性允许在队列中的一条消息可用时异步地通知进程。
--------------------------------------------------------------------------------
 消息队列 和 管道、FIFO有很大的区别，主要有以下两点：
 1.一个进程向消息队列写入消息之前，并不需要某个进程在该队列上等待该消息的到达，而管道和FIFO是相反的，
 进程向其中写消息时，管道和FIFO必需已经打开来读(否则open阻塞)，那么内核会产生 SIGPIPE信号。
 2.IPC的持续性不同。
 	管道和FIFO是随进程的持续性，当管道和FIFO最后一次关闭发生时，仍在管道和FIFO中的数据会被丢弃。
 	消息队列是随内核的持续性，即一个进程向消息队列写入消息后，然后终止，
 另外一个进程可以在以后某个时刻打开该队列读取消息。只要内核没有重新自举，消息队列没有被删除。
--------------------------------------------------------------------------------
消息队列中的每条消息通常具有以下属性：
 一个表示优先级的整数；消息的数据部分的长度(可为0)；消息数据本身(若大于0)；
--------------------------------------------------------------------------------
 消息队列可以在进程之间进行共享数据，Posix消息队列有几个主要的特性：
 1.消息队列的每个消息都有自己的优先级，而且每次从中获取消息，总是得到优先级最高的一个
 2.消息队列通过msg_open进行创建，而且每个消息队列通过/somename的方式进行唯一标识。
 3.进程通过mq_close关闭本进程的队列的引用，通过mq_unlink删除内核中的消息队列
 4.使用fork()函数后，子进程复制父进程的文件描述符，那么共享同一个消息队列，而且也共享mq_flags
 5.一个消息队列描述符可以通过select、poll和epoll进行监听
 6.与Posix相对的还有一个更古老的System V消息队列。Posix消息队列接口更好用，但是相对的，
 System V的接口更灵活。根据需要合理选取即可。如果没有特殊需求，使用Posix即可
--------------------------------------------------------------------------------
mq_open()函数创建一个新消息队列或打开一个既有队列，返回后续调用中会用到的消息队列描述符。
mq_setattr()设置消息队列的属性
mq_getattr()获取消息队列的属性
mq_close()函数关闭进程之前打开的一个消息队列。
mq_unlink()函数删除一个消息队列名并当所有进程关闭该队列时对队列进行标记以便删除。
mq_send()函数向队列写入一条消息。
mq_timedsend()
mq_recevie()函数从队列中读取一条消息。
mq_timedreceive()
mq_notify()注册消息异步通知
--------------------------------------------------------------------------------
 // 创建或打开消息队列，成功时返回消息队列的描述符 mqd_t mqdes
 mqd_t mq_open (const char *name, int oflag, ...)
 // mqd_t mq_open(const char *name, int oflag);
 // mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
	 name	标识出了消息队列，消息队列对象的名字的最大长度为 NAME_MAX(255)个字符。
	 oflag	位掩码，消息队列的权限（同 open() ）：
			O_CREAT		队列不存在时创建队列
			O_EXCL		与 O_CREAT 一起使用，若消息队列已存在，则错误返回
			O_RDONLY	只读打开
			O_WRONLY	只写打开
			O_RDWR		读写打开
			O_NONBLOCK	以非阻塞模式打开
		如果使用了 O_CREAT | O_EXCL，若消息队列已经存在，则 errno 置为 EEXIST
	 mode 	消息队列的模式
	 attr 	设置的消息队列的属性，是一个结构体，结构如下：
			struct mq_attr {
				long mq_flags;	//阻塞标志，只有2个值 0 或 O_NONBLOCK
				long mq_maxmsg;	//最大消息数：消息队列中最多能容纳的消息个数
				long mq_msgsize;//每个消息的最大字节数
				long mq_curmsgs;//当前消息队列中正在排队的消息个数
				long pad[4];
			};
	    注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。
--------------------------------------------------------------------------------
 mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性。
 mq_setattr() 只设置 mq_flags，用来 设置或清除 消息队列的 非阻塞标志，忽略另外3个属性。
--------------------------------------------------------------------------------
 // 设置消息队列的属性
 int mq_setattr (mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);
 // 获取消息队列的属性
 int mq_getattr (mqd_t mqdes, struct mq_attr *mqstat);
--------------------------------------------------------------------------------
 // 关闭消息队列
 int mq_close(mqd_t mqdes);  // 类似 close
 // 删除消息队列引用
 int mq_unlink(const char *name);
--------------------------------------------------------------------------------
 // 向消息队列写入一条消息
 int mq_send ( 	mqd_t mqdes, const char *msg_ptr,
            	size_t msg_len, unsigned int msg_prio );
 int mq_timedsend (	mqd_t mqdes, const char *msg_ptr,
 					size_t msg_len, unsigned int msg_prio,
                 	const struct timespec *abs_timeout );
	mqdes：		消息队列的描述符
    msg_ptr：	需要发送的消息
    msg_len：	发送消息的长度
    msg_prio：	发送消息的优先级，不使用时设为 0
    abs_timeout：超时参数
 成功返回0，失败返回-1
-------------------------------
 mq_send()函数向消息队列发送消息，如果消息队列满了，那么该函数阻塞；
 如果队列满了，且消息队列设置为 O_NONBLOCK 模式，那么函数返回 -1，同时 errno 设置为 EAGAIN。
 mq_timedsend() 基本同上，唯一区别在于如果阻塞情况下超时，则立刻返回 -1。
--------------------------------------------------------------------------------
 // 从消息队列中读取一条消息
 ssize_t mq_receive ( 	mqd_t mqdes, char *msg_ptr,
                   		size_t msg_len, unsigned int *msg_prio );
 ssize_t mq_timedreceive ( 	mqd_t mqdes, char *msg_ptr,
                        	size_t msg_len, unsigned int *msg_prio,
                         	const struct timespec *abs_timeout );
    mqdes：		消息队列描述符
    msg_ptr：	指向放置消息的缓冲区
    msg_len：	放置消息缓冲区的长度
    msg_prio：	返回消息的优先级，不使用时设为 nullptr
    abs_timeout：超时参数
 成功返回消息长度，失败返回-1
-------------------------------
 mq_receive函数接受消息队列消息，如果队列是阻塞模式，而且队列空，那么该函数阻塞；
 如果队列空，但是设置为 O_NONBLOCK 模式，则返回 -1，同时 errno 设置为 EAGAIN。
 ma_timedreceive() 唯一的区别在于，如果队列空且超时，则立刻返回 -1。
--------------------------------------------------------------------------------
 // 注册消息异步通知
 int mq_notify ( mqd_t mqdes, const struct sigevent *notification );
	mqdes：		消息队列的标识符
    notification：参数的意义参照 sigevent 结构体：
-------------------------------
 作用：如果有新消息到达一个空的队列，允许调用线程注册一个异步通知，该通知会在消息到达时触发。
 核心的思想在异步这两个字上，也就是说，我们希望从消息队列中获取消息，
 但是不想让当前进程阻塞在一个空的消息队列上，那么通过该函数可以注册异步通知，来处理将来到达的事件。
-------------------------------
 几个注意事项：
    1.一个消息队列只能注册在一个进程。
    2.如果当前进程想要移除注册的事件，只需要把 notification 设置为 NULL注册 mq_notify()即可。
    3.如果进程注册已经存在的注册事件，那么其他注册该事件的进程不会收到信号（见 1）。
    4.只有空消息队列在接收到第一个消息才会触发通知。
-------------------------------
 要明确 sigevent{} 的作用：
 	sigevent{}的作用是为Linux/Unix系统调用函数API提供一个调用接口的通用参数；
	这类API的作用以某种特定的方式来通知进程到达的事件。
 	API根据sigevent提供的结构，来具体处理有关信息。
-------------------------------
union sigval {
  int sival_int; 	// 信号处理(线程)函数 参数
  void *sival_ptr; 	// 信号处理(线程)函数 参数
};
typedef union sigval __sigval_t;
-------------------------------
typedef struct sigevent
{
	__sigval_t sigev_value;
	int sigev_signo; 	// 通知信号
	int sigev_notify; 	// 通知方式
	union {
		int _pad[__SIGEV_PAD_SIZE];
		__pid_t _tid; 	// 线程 ID
		struct {
			void (*_function) (__sigval_t);	// 指定 信号处理(线程)函数。
			pthread_attr_t *_attribute;		// 线程属性。
		} _sigev_thread;
	} _sigev_un;
} sigevent_t;
#define sigev_notify_function   _sigev_un._sigev_thread._function // 信号处理(线程)函数。
#define sigev_notify_attributes _sigev_un._sigev_thread._attribute // 线程属性
-------------------------------
 sigev_notify 通知方式 参数取值和作用如下：
    SIGEV_NONE：	空的提醒，事件发生时不做任何事情
    SIGEV_SIGNAL：	向进程发送 sigev_signo 中指定的信号，这涉及到 sigaction() 的使用
    SIGEV_THREAD：	通知进程在一个新的线程中启动 sigev_notify_function 函数，
    	函数的实参是 sigev_value，系统API自动启动一个线程，我们不用显式启动。
-------------------------------------------------------------------------------- */

#include "mqueue_test.h"

/******************************************************************************/

void test_mq_create(const int &argc, char **argv) // 只创建 消息队列
{
	struct mq_attr attr{};
	bzero(&attr, sizeof(attr));
	int opt_c, flags = O_RDWR | O_CREAT; // 读写 + 队列不存在时创建队列
//	std::cout << "usage: mqcreate [ -e ] [ -m maxmsg -z msgsize ] <name>" << std::endl;
	while (true) {
		/* "ab:cd:e"，这就是一个选项字符串。对应到命令行就是 -a ,-b *,-c ,-d *, -e 。
		冒号又是什么呢？冒号表示参数，一个冒号就表示这个选项后面必须带有参数（没有带参数会报错哦），
		但是这个参数可以和选项连在一起写，也可以用空格隔开，
		比如 -a123 和-a 123（中间有空格）都表示123是-a的参数 **/
		opt_c = getopt(argc, argv, "em:z:");

		if (opt_c == -1) break;
		else if (opt_c == '?') {
			err_quit(-1, __LINE__, "getopt(): c == '?'");
		}
		switch (opt_c) {
			case 'e':
				flags |= O_EXCL; // 与 O_CREAT 一起使用，若消息队列已存在，则错误返回
				break;
			case 'm':
				// 消息队列中最多能容纳的消息个数
				attr.mq_maxmsg = strtol(optarg, nullptr, 10);
				break;
			case 'z':
				// 每个消息的最大字节数
				attr.mq_msgsize = strtol(optarg, nullptr, 10);
				break;
			default: break;
		}
	}
	if (optind != argc - 1)
		err_quit(-1, __LINE__,
				 "usage: mqcreate [ -e ] [ -m maxmsg -z msgsize ] <name>");
	if ((attr.mq_maxmsg != 0 && attr.mq_msgsize == 0) ||
		(attr.mq_maxmsg == 0 && attr.mq_msgsize != 0))
		err_quit(-1, __LINE__,
				 "must specify both -m maxmsg and -z msgsize");

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(argv[optind], flags, FILE_MODE,
						(attr.mq_maxmsg != 0) ? &attr : nullptr);
	Mq_close(mqd);
	exit(0);
}

mqd_t mq_create(const mode_t &mode) // 只创建 消息队列
{
	using namespace std;
	string bufstr;
	cout << "usage: [ -e ] [ -m maxmsg -z msgsize ] <name>" << endl;
	cin >> bufstr;
	vector<string> bufvec = strtovec(bufstr);
	bufstr.clear();
	size_t num_vec = bufvec.size();
	if (num_vec < 1)
		err_quit(-1, __LINE__,
				 "usage: [ -e ] [ -m maxmsg -z msgsize ] <name>");

	struct mq_attr attr{};
	bzero(&attr, sizeof(attr));
	int flags = O_RDWR | O_CREAT; // 读写 + 队列不存在时创建队列

	for (int i=0; i < (int)num_vec-1; ++i) {
		if (bufvec[i] == "-e") {
			flags |= O_EXCL; // 与 O_CREAT 一起使用，若消息队列已存在，则错误返回
			continue;
		}
		if (bufvec[i] == "-m") {
			++i;
			// 消息队列中最多能容纳的消息个数
			attr.mq_maxmsg = strtol(bufvec[i].c_str(), nullptr, 10);
			continue;
		}
		if (bufvec[i] == "-z") {
			++i;
			// 每个消息的最大字节数
			attr.mq_msgsize = strtol(bufvec[i].c_str(), nullptr, 10);
			continue;
		}
	}
	if ((attr.mq_maxmsg > 0 && attr.mq_msgsize == 0) ||
		(attr.mq_maxmsg == 0 && attr.mq_msgsize > 0) ||
		(attr.mq_maxmsg < 0 || attr.mq_msgsize < 0) )
		err_quit(-1, __LINE__,
				 "must specify both -m maxmsg:%d and -z msgsize:%d",
				 attr.mq_maxmsg, attr.mq_msgsize);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	return Mq_open(bufvec.back().c_str(), flags, mode,
				   (attr.mq_maxmsg > 0) ? &attr : nullptr);
}

/******************************************************************************/

void mq_attr_info(const mqd_t &mqd, struct mq_attr &attr)
{
	Mq_getattr(mqd, &attr);
	using namespace std;
	cout << "mq_attr_info()" << "\n"
		 << "一个进程能够同时打开消息队列的最大数目 " << sysconf(_SC_MQ_OPEN_MAX) << "\n"
		 << "消息队列的最大优先级 " << sysconf(_SC_MQ_PRIO_MAX) << "\n"
		 << "struct mq_attr{}" << "\n"
		 << "mq_flags\t" << ((attr.mq_flags==0)? "0":"O_NONBLOCK") << "\n"
		 << "mq_maxmsg\t" << attr.mq_maxmsg << "\n"
		 << "mq_msgsize\t" << attr.mq_msgsize << "\n"
		 << "mq_curmsgs\t" << attr.mq_curmsgs << endl;
}

/******************************** 简单应答 *************************************/

void test_mq_service() // 独立 服务器 消息队列
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_service(): 进程 ID %d begin", pid)

	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	struct mq_attr attr{};
	attr.mq_maxmsg = 10; 		//最大消息数：消息队列中最多能容纳的消息个数
	attr.mq_msgsize = 1024; 	//每个消息的最大字节数

	Mq_unlink(POSIX_mqueue_PATH);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_RDWR/*读写*/ | O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, &attr);

	mq_attr_info(mqd, attr);

	char mq_buf[attr.mq_msgsize];
	ssize_t sizeval = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
	std::cout << "Mq_receive [" << sizeval << "]: " << mq_buf << std::endl;
	Mq_send(mqd, mq_buf, strlen(mq_buf), 0/*无需优先级*/);

	_debug_log_info("test_mq_service(): 进程 ID %d end", pid)
}

void test_mq_client() // 独立 客户端 消息队列
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_client(): 进程 ID %d begin", pid)

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_RDWR/*读写*/ | O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, nullptr);

	struct mq_attr attr{};
	mq_attr_info(mqd, attr);

	char mq_buf[attr.mq_msgsize];
	bzero(mq_buf, sizeof(mq_buf));
	std::cout << "input: ";
	std::cout.flush();
	std::cin >> mq_buf;

	Mq_send(mqd, mq_buf, strlen(mq_buf), 0/*无需优先级*/);
	ssize_t sizeval = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
	std::cout << "Mq_receive [" << sizeval << "]: " << mq_buf << std::endl;

	_debug_log_info("test_mq_client(): 进程 ID %d end", pid)
}

/******************************************************************************/

void test_send_mq_notify() // 周期 发送 消息队列
{
	pid_t pid = getpid();
	_debug_log_info("test_send_mq_notify(): 进程 ID %d begin", pid)

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_NONBLOCK/*非阻塞*/ | O_RDWR/*读写*/
						| O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, nullptr);

	struct mq_attr attr{};
	mq_attr_info(mqd, attr);

	std::string mq_str;
	mq_str.resize(attr.mq_msgsize);
	mq_str = "test_send_mq_notify\nzcq\n";
	int16_t num=0;
	while (true) {
		sleep(3);
		num++;
		_debug_log_info("NO: %d\tTIME: %d S", num, num*3)
		Mq_send(mqd, mq_str.c_str(), mq_str.size(), 0/*无需优先级*/);
	}

	_debug_log_info("test_send_mq_notify(): 进程 ID %d end", pid)
}

/********************** 通知 + 非阻塞 mq_receive 效率低 **************************/

volatile sig_atomic_t mqFlag=0; // volatile 原子类型，不会在操作过程中到达信号

static void sig_usr1(int signo) // 信号处理程序
{
	mqFlag = 1;
}

void test_mq_notify_signal() // 独立 消息队列 通知
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_notify_signal(): 进程 ID %d begin", pid)

	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	struct mq_attr attr{};
	attr.mq_maxmsg = 5; 		//最大消息数：消息队列中最多能容纳的消息个数
	attr.mq_msgsize = 1024; 	//每个消息的最大字节数

	Mq_unlink(POSIX_mqueue_PATH);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_NONBLOCK/*非阻塞*/ | O_RDWR/*读写*/
						| O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, &attr);

	mq_attr_info(mqd, attr);
	char mq_buf[attr.mq_msgsize];

	sigset_t zeromask, newmask, oldmask; // 要阻止，取消阻止或等待的 信号组
	sigemptyset(&zeromask); // 清除 信号组 中的所有信号
	sigemptyset(&newmask); // 清除 信号组 中的所有信号
	sigemptyset(&oldmask); // 清除 信号组 中的所有信号
	sigaddset(&newmask, SIGUSR1); // 添加到 newmask 信号组

	Signal(SIGUSR1, sig_usr1); // 注册 信号处理程序

	/** 注册消息异步通知 **/
	struct sigevent	sigev{};
	sigev.sigev_signo = SIGUSR1; // 通知信号
	sigev.sigev_notify = SIGEV_SIGNAL; // 通知方式: 向进程发送 sigev_signo 中指定的信号
	Mq_notify(mqd, &sigev); // 注册消息异步通知

	for (ssize_t ret; ; ) {

		/** SIG_BLOCK 阻塞 newmask 而不受其信号中断打扰，直到 SIG_UNBLOCK
		阻塞时，信号处理程序 任工作 **/
		ret = sigprocmask(SIG_BLOCK, &newmask, &oldmask); // 单线程:检测或改变信号屏蔽字
		err_sys(ret, __LINE__, "sigprocmask()");

		/** 等待 zeromask(空信号) 即 永久等待，效率低（经内核） **/
		while (mqFlag == 0) sigsuspend(&zeromask); // 等待信号到达
		mqFlag = 0; // reset flag

		Mq_notify(mqd, &sigev); // 注册消息异步通知

		while (true) {
			ret = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
			if (ret <= 0) break;
			std::cout << "Mq_receive [" << ret << "]: " << mq_buf << std::endl;
		}

		/** 解阻塞 newmask **/
		ret = sigprocmask(SIG_UNBLOCK, &newmask, nullptr); // 单线程:检测或改变信号屏蔽字
		err_sys(ret, __LINE__, "sigprocmask()");
	}

	_debug_log_info("test_mq_notify_signal(): 进程 ID %d end", pid)
}

/*************************** 通知 + sigwait 效率较高 ****************************/

void test_mq_notify_sigwait() // 独立 消息队列 通知
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_notify_sigwait(): 进程 ID %d begin", pid)

	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	struct mq_attr attr{};
	attr.mq_maxmsg = 6; 		//最大消息数：消息队列中最多能容纳的消息个数
	attr.mq_msgsize = 512; 	//每个消息的最大字节数

	Mq_unlink(POSIX_mqueue_PATH);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_NONBLOCK/*非阻塞*/ | O_RDWR/*读写*/
						| O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, &attr);

	mq_attr_info(mqd, attr);
	char mq_buf[attr.mq_msgsize];

	sigset_t newmask; // 要阻止，取消阻止或等待的 信号组
	sigemptyset(&newmask); // 清除 信号组 中的所有信号
	sigaddset(&newmask, SIGUSR1); // 添加到 newmask 信号组

	/** 注册消息异步通知 **/
	struct sigevent	sigev{};
	sigev.sigev_signo = SIGUSR1; // 通知信号
	sigev.sigev_notify = SIGEV_SIGNAL; // 通知方式: 向进程发送 sigev_signo 中指定的信号
	Mq_notify(mqd, &sigev); // 注册消息异步通知

	/** SIG_BLOCK 阻塞 newmask 而不受其信号中断打扰，直到 SIG_UNBLOCK
		阻塞时，信号处理程序 任工作 **/
	int ret = sigprocmask(SIG_BLOCK, &newmask, nullptr); // 单线程:检测或改变信号屏蔽字
	err_sys(ret, __LINE__, "sigprocmask()");

	for ( ; ; ) {
		sigwait(&newmask, &ret); // 等待信号到达
		if (ret == SIGUSR1) {
			Mq_notify(mqd, &sigev); // 注册消息异步通知
			while (true) {
				ret = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
				if (ret <= 0) break;
				std::cout << "Mq_receive [" << ret << "]: " << mq_buf
						  << std::endl;
			}
		}
	}

	_debug_log_info("test_mq_notify_sigwait(): 进程 ID %d end", pid)
}

/***************************** 通知 + select 效率高 *****************************/

int pipe_fd[2];

void sig_usr1_02(int signo) // 信号处理程序
{
	write(pipe_fd[1], "", 1); // 通知
}

void test_mq_notify_pipe_select() // 独立 消息队列 通知
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_notify_pipe_select(): 进程 ID %d begin", pid)

	int ret = pipe(pipe_fd);
	err_quit(ret, __LINE__, "pipe()");

	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	struct mq_attr attr{};
	attr.mq_maxmsg = 7; 		//最大消息数：消息队列中最多能容纳的消息个数
	attr.mq_msgsize = 128; 	//每个消息的最大字节数

	Mq_unlink(POSIX_mqueue_PATH);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
	 	Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
	 	可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
	 	mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
	 	注意坑：
	 	1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
	 		Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
	 	2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd_t mqd = Mq_open(POSIX_mqueue_PATH,
						O_NONBLOCK/*非阻塞*/ | O_RDWR/*读写*/
						| O_CREAT/*队列不存在时创建队列*/,
						FILE_MODE, &attr);

	mq_attr_info(mqd, attr);
	char mq_buf[attr.mq_msgsize];

	sigset_t newmask; // 要阻止，取消阻止或等待的 信号组
	sigemptyset(&newmask); // 清除 信号组 中的所有信号
	sigaddset(&newmask, SIGUSR1); // 添加到 newmask 信号组

	Signal(SIGUSR1, sig_usr1_02); // 注册 信号处理程序

	/** 注册消息异步通知 **/
	struct sigevent	sigev{};
	sigev.sigev_signo = SIGUSR1; // 通知信号
	sigev.sigev_notify = SIGEV_SIGNAL; // 通知方式: 向进程发送 sigev_signo 中指定的信号
	Mq_notify(mqd, &sigev); // 注册消息异步通知

//	ret = Fcntl(pipe_fd[0], F_GETFL, 0); 			// 获取 I/O 状态
//	Fcntl(pipe_fd[0], F_SETFL, ret | O_NONBLOCK); 	// 设置 I/O 非阻塞

	fd_set rset; // 鉴别事件的标志位集合
	FD_ZERO(&rset); // 初始化集合:所有位均关闭
	while (true) {
		_debug_log_info(" ")

		FD_SET(pipe_fd[0], &rset); // 设置 sock_fd 位
		ret = select(pipe_fd[0]+1, &rset,
					 nullptr, nullptr, nullptr);
		if (ret < 0) {
			if (errno == EINTR) continue;
			err_sys(ret, __LINE__, "select()");
		}

		_debug_log_info(" ")

		if (FD_ISSET(pipe_fd[0], &rset)) {
			_debug_log_info(" ")
			read(pipe_fd[0], &mq_buf, 1);
			_debug_log_info(" ")
			Mq_notify(mqd, &sigev); // 注册消息异步通知
			while (true) {
				ret = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
				if (ret <= 0) break;
				std::cout << "Mq_receive [" << ret << "]: " << mq_buf
						  << std::endl;
			}
		}
	}

	_debug_log_info("test_mq_notify_pipe_select(): 进程 ID %d end", pid)
}

/******************************* 通知 + 启动线程 ********************************/

void mypthread_state(const pthread_t &pid) // 获取线程属性
{
	int retval; // 返回值
	int pth_state; // 线程 可分离状态
	pthread_attr_t attr_pth; // 线程 属性值

	/* 获取 线程 属性值 */
	retval=pthread_getattr_np(pid, &attr_pth); // 获取线程属性值
	err_quit(retval, __LINE__, "pthread_getattr_np()");

	retval=pthread_attr_getdetachstate(&attr_pth, &pth_state);//从属性值获取线程可分离状态
	err_quit(retval, __LINE__, "pthread_attr_getdetachstate()");
	if (retval == 0) {
		std::cout << "线程可分离状态: "
				  << ((pth_state == PTHREAD_CREATE_JOINABLE) ?
					  "PTHREAD_CREATE_JOINABLE" : "PTHREAD_CREATE_DETACHED")
				  << std::endl;
	}

	size_t stack_size; // 栈大小
	retval=pthread_attr_getstacksize(&attr_pth, &stack_size); // 获得 当前线程 栈大小
	err_quit(retval, __LINE__, "pthread_attr_getstacksize()");
	std::cout << "栈大小 stack size: " << stack_size << " byte"<<std::endl;

	/* 销毁 线程 属性值 */
	retval=pthread_attr_destroy(&attr_pth); // 销毁线程属性值
	err_quit(retval, __LINE__, "pthread_attr_destroy()");
}

struct mq_attr attr{}; // 消息队列 属性
struct sigevent	sigev{}; // 注册消息异步通知
mqd_t mqd; // 消息队列描述符

void notify_thread(union sigval arg) // 信号处理(线程)函数
{
	pthread_t pid = pthread_self(); // 获取当前线程 ID 值
	_debug_log_info("notify_thread(): 处理(线程)函数 ID %ld begin: %d", pid, arg.sival_int)
	mypthread_state(pid); // 获取线程属性
	std::this_thread::sleep_for(std::chrono::milliseconds(1));//线程休眠 1ms
	int ret;
	char mq_buf[attr.mq_msgsize];
	Mq_notify(mqd, &sigev); // 注册消息异步通知
	while (true) {
		ret = Mq_receive(mqd, mq_buf, sizeof(mq_buf), nullptr/*无需优先级*/);
		if (ret <= 0) break;
		std::cout << "Mq_receive [" << ret << "]: " << mq_buf
				  << std::endl;
	}
	_debug_log_info("notify_thread(): 处理(线程)函数 ID %ld end: %d", pid, arg.sival_int)
	pthread_exit(nullptr); // 当前线程退出，不影响进程
}

void test_mq_notify_thread() // 独立 消息队列 通知
{
	pid_t pid = getpid();
	_debug_log_info("test_mq_notify_pipe_select(): 进程 ID %d begin", pid)

	int ret = pipe(pipe_fd);
	err_quit(ret, __LINE__, "pipe()");

	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
//	struct mq_attr attr{};
	attr.mq_maxmsg = 8; 		//最大消息数：消息队列中最多能容纳的消息个数
	attr.mq_msgsize = 256; 		//每个消息的最大字节数

	Mq_unlink(POSIX_mqueue_PATH);

	/** 注意一个坑：消息队列的名称只能有开头一个 / 符号！！！！
		 Linux下的Posix消息队列是在vfs中创建的，
		drwxrwxrwt   2 root root        40 5月  15  2021 mqueue/
		 可以用 ll /dev 查看
		sudo mount -t mqueue none /dev/mqueue # 挂载虚拟设备
		 mkdir -m 777 -p -v /dev/mqueue # 创建文件夹
		 注意坑：
		 1.消息队列的名称必须加上斜杆且只能有开头一个 / 符号！！！！
			 Mq_open /zcq.1234 后 出现在 /dev/mqueue/zcq.1234
		 2.attr.mq_maxmsg 不能超过 /proc/sys/fs/mqueue/msg_max 的数值。
		3.attr.mq_msgsize不能超过 /proc/sys/fs/mqueue/msgsize_max 的数值。  */
	/* mq_open() 只会设置 mq_maxmsg 和 mq_msgsize，忽略另外2个属性 */
	mqd = Mq_open(POSIX_mqueue_PATH,
				  O_NONBLOCK/*非阻塞*/ | O_RDWR/*读写*/
				  | O_CREAT/*队列不存在时创建队列*/,
				  FILE_MODE, &attr);

	mq_attr_info(mqd, attr);

	/** 注册消息异步通知 **/
//	struct sigevent	sigev{};
//	sigev.sigev_signo = SIGUSR1; 		// 通知信号
	sigev.sigev_notify = SIGEV_THREAD; 	// 通知方式: 通过线程创建进行传递
#if 1
	sigev.sigev_value.sival_int = 123456; // 信号处理(线程)函数 参数
#else
	sigev.sigev_value.sival_ptr = nullptr; // 信号处理(线程)函数 参数
#endif
	sigev.sigev_notify_function = notify_thread; 	// 指定 信号处理(线程)函数。
	sigev.sigev_notify_attributes = nullptr; 		// 线程属性：默认可分离
	Mq_notify(mqd, &sigev); // 注册消息异步通知

	while (true) {
		pause(); // 暂停过程，直到出现信号为止。总返回-1并将 errno 置 EINTR
	}

	_debug_log_info("test_mq_notify_pipe_select(): 进程 ID %d end", pid)
}

/******************************************************************************/

