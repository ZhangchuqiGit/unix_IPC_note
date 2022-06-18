//
// Created by zcq on 2021/5/17.
//

/** 锁 以 进程 为基 **/

/** ----------------------------------------------------------------------------
 建议锁
 	如果某一个进程对一个文件持有一把锁之后，其他进程仍然可以直接对文件进行操作(open, read, write)
 而不会被系统禁止，即使这个进程没有持有锁。只是一种编程上的约定。
 建议锁只对遵守建议锁准则的进程生效(程序在操作前应该自觉的检查所的状态之后才能进行后续操作)。
--------------------------------------------------------------------------------
 强制锁
 	试图实现一套内核级的锁操作。当有进程对某个文件上锁之后，
 其他进程会在open、read或write等文件操作时发生错误。
--------------------------------------------------------------------------------
 记录锁
 	只对文件中自己所关心的那一部分加锁。记录锁是更细粒度的文件锁。
 记录锁存在于文件结构体中，并不与文件描述符相关联，会在进程退出时候被释放掉。
--------------------------------------------------------------------------------
 记录锁(record locking)的功能是：
	 当第一个进程正在读或修改文件的某个部分(区域)时，使用记录锁可以阻止其他进程修改该文件部分(区域)。
 “记录”这个词是一种误用，因为 UNIX系统内核根本没有使用文件记录这种概念。
 一个更适合的术语可能是 字节范围锁 (byte-range locking)，
 因为它锁定的只是文件中的一个区域(也可已是整个文件，相当文件锁)。
--------------------------------------------------------------------------------
 						fcntl()、lockf() 和 flock() 的区别
-----------------------------
	fcntl()和 flock()是系统调用，而 lockf()是库函数。
 	flock()只能对整个文件上锁，不能对文件的某一部分上锁（不是记录锁），
 这是和 fcntl()/lockf() 的重要区别，fcntl()/lockf()可以对文件的某个区域上锁（记录锁）。
----------------------------- fcntl()、lockf()
	lockf()实际上是 fcntl()的封装，所以 lockf()和 fcntl()的底层实现是一样的。
 	lockf()只支持 互斥锁/排他锁，不支持 共享锁。
 	fcntl()是功能最强大的，它既 支持 共享锁 又支持 互斥锁/排他锁，
 即可以 锁住 整个文件，又能 只锁 文件的 某一部分（记录锁）。
 	fcntl()、lockf()记录锁的自动继承和释放：
		锁与进程的联系：当一个进程终止时，它所建立的锁全部被释放。
		锁与文件的联系：无论一个描述符何时关闭，该进程通过这一描述符引用的文件上的任何一把锁都会被释放。
		由 fork()产生的子进程 不继承 父进程的锁 !
----------------------
 fcntl()/lockf()的特性：
 	(1) 上锁可递归，如果一个进程对一个文件区间已经有一把锁，
 		后来进程又企图在同一区间再加一把锁，新锁 将替换 老锁。
 	(2) 加读锁（共享锁）文件必须是读打开的，加写锁（排他锁）文件必须是写打开。
 	(3) 进程 不能 使用 F_GETLK 命令来测试 它自己 是否在文件的某一部分持有一把锁。
 	F_GETLK 命令定义说明，返回信息指示是否现存的锁阻止调用进程设置它自己的锁。
 	因为，F_SETLK 和 F_SETLKW 命令总是替换进程的现有锁，所以调用进程绝不会阻塞在自己持有的锁上，
 	于是 F_GETLK 命令绝不会报告 调用进程自己持有的锁。
 	(4) 进程终止时，他所建立的所有文件锁都会被释放，flock()也是一样的。
 	(5) 任何时候关闭一个 描述符 时，
 	则该进程通过这一描述符可以引用的文件上的任何一把锁都被释放（这些锁都是该进程设置的）。
 	这一点与 flock()不同，只有关闭所有 (fork()或 dup())复制出的 fd，锁才会释放。
 	(6) 由 fork()产生的子进程 不继承 父进程的锁 ! 这一点与 flock()不同。
 	(7) 在执行 exec()后，新程序可以 继承 原程序的锁，这一点与 flock()是相同的。
 	（如果在 exec()前 close(fd)，相应文件的锁也会被释放）。
 	(8) 支持 强制性锁：对一个特定文件打开其设置 组位(S_ISGID=02000，UGOrwxrwxrwx)，
 	并关闭其 组执行位(S_IXGRP=0010，UGO --- --x ---)，则对该文件开启了强制性锁机制。
 	在 Linux中如果要使用强制性锁，则要在文件系统 mount 时，使用 -o mand 打开该机制。
----------------------------- flock()
	flock()是建议性锁，linux存在 强制性锁(mandatory lock)和 劝告/建议性锁(advisory lock)。
    flock()创建的锁是和 文件打开表项（struct file）相关联的，而不是 fd。
 	子进程会继承父进程的锁。这就意味着复制文件fd（通过 fork()或者 dup()）后，
 那么通过这两个fd都可以操作这把锁（例如通过一个fd加锁，通过另一个fd可以释放锁）。
 但是上锁过程中关闭其中一个fd，锁并不会释放（因为file结构并没有释放），
 只有关闭所有 (fork()或 dup())复制出的 fd，锁才会释放。这一点与 fcntl()/lockf()不同。
 	通过 flock()对其中 fd 加锁，通过另一个无法解锁、也无法上锁，除非复制文件 fd(fork/dup）
--------------------------------------------------------------------------------
  						fcntl()/lockf() 和 flock() 的关系 - 互不影响
 flock()加锁，并不影响 fcntl()/lockf()的加锁，两种调用产生的锁的类型是不同的。
 可以通过 /proc/locks 查看进程获取锁的状态。
 $ sudo cat /proc/locks | grep 18849
1: POSIX  ADVISORY  WRITE 18849 08:02:852674 10737400 10737420 (有区间是 记录锁)
2: FLOCK  ADVISORY  WRITE 18849 08:02:852674 0 EOF (文件锁)
 ADVISORY	表明是劝告锁。
 WRITE		顾名思义，是写锁；还有读锁。
 18849		是持有锁的进程ID。
 08:02:852674	表示的对应磁盘文件的所在设备的 主设备号、次设备号，文件对应的 inode number。
 0 		表示的是锁的起始位置
 EOF 	表示的是锁的结束位置。对 flock()来是总是 0 和 EOF(只是文件锁，不是记录锁)。  **/


#include "file_lock.h"


/**-------------------- fcntl() 系统调用 --------------------**/
/* #include <fcntl.h>
 int fcntl(int fd, int cmd, struct flock *flockptr); // 若成功，依赖于 cmd，否则返回 -1
 fd 文件描述符；
-----------------------------
 cmd 参数：
 F_SETLK：	(不阻塞)设置锁类型：F_RDLCK(读锁), F_WRLCK(写锁), F_UNLCK(释放锁)
		如果内核无法将锁授予本进程（被其他进程抢先占了锁），立即返回 -1，设置 errno=EAGAIN/EACCES
 F_SETLKW：	(阻塞)设置锁类型，和 F_SETLK 的唯一区别是申请不到锁就傻等 wait。
 F_GETLK：	获取锁的相关信息：会修改传入的 struct flock{}。可判断是否可以对文件加锁。
 注意 F_GETLK 与 FSETLK 不是原子操作，在两个函数调用之间可能有新的进程进行了加锁操作。
 在设置或释放文件上的一把锁时，系统会按要求组合或分裂相邻区。
 记录锁在使用过程中也可能会造成死锁，需要当心。
-----------------------------
struct flock {
	short l_type; 	// 锁类型：F_RDLCK(读锁), F_WRLCK(写锁), F_UNLCK(释放锁)
	short l_whence; // 定位： SEEK_SET(文件头), SEEK_CUR(文件当前位置), SEEK_END(文件尾)
	off_t l_start; 	// 相对于 l_whence 字段的偏移量：+ 增加，- 减少
	off_t l_len; 	// 需要锁定的长度/区域大小(字节)；0 表示到文件尾(都处于锁的范围内)
	pid_t l_pid; 	// 当前获得文件锁的进程ID（F_GETLK）
};
 关于该结构需要注意的是：
	锁可以在当前文件尾端处开始或者越过尾端处开始，但是不能在文件起始位置之前开始。
    对整个文件加锁，需指定 l_whence=SEEK_SET，l_start=0，l_len=0。
    多个进程访问同一文件/区间，可以有 读锁 + 读锁 + ... + 读锁。
    单个进程对同一区间重复加锁的结果是后面的锁覆盖前面的锁。
【规则】：
    如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁；
    如果某进程申请了 写锁，其它进程 可以 申请  读锁，不能申请 写锁。
	写锁 优先级 高于 读锁              */

int lock_fcntl_reg(const int &fd, const int &cmd, const short &type,
				   const short &whence, const off_t &offset,
				   const off_t &len) // 获取/释放 锁
{
	struct flock lock{};
	lock.l_type = type;	// 锁类型：F_RDLCK(读锁), F_WRLCK(写锁), F_UNLCK(释放锁)
	lock.l_whence = whence;//定位：SEEK_SET(文件头), SEEK_CUR(文件当前位置), SEEK_END(文件尾)
	lock.l_start = offset; // 相对于 l_whence 字段的偏移量：+ 增加，- 减少
	lock.l_len = len; // 需要锁定的长度/区域大小(字节)；0 表示到文件尾(都处于锁的范围内)
	return( fcntl(fd, cmd, &lock) );	/* -1 upon error */
}

void Lock_fcntl_reg(const int &fd, const int &cmd, const short &type,
					const short &whence, const off_t &offset,
					const off_t &len) // 获取/释放 锁
{
	if (lock_fcntl_reg(fd, cmd, type, whence, offset, len) == -1) {
		err_sys(-1, __FILE__, __func__, __LINE__,
				"lock_fcntl_reg()");
	}
}

pid_t lock_fcntl_test(const int &fd, const short &type,
					  const short &whence, const off_t &offset,
					  const off_t &len) // 测试 锁
{
	struct flock lock{};
	lock.l_type = type;	// 锁类型：F_RDLCK(读锁), F_WRLCK(写锁), F_UNLCK(释放锁)
	lock.l_whence = whence;//定位：SEEK_SET(文件头), SEEK_CUR(文件当前位置), SEEK_END(文件尾)
	lock.l_start = offset; // 相对于 l_whence 字段的偏移量：+ 增加，- 减少
	lock.l_len = len; // 需要锁定的长度/区域大小(字节)；0 表示到文件尾(都处于锁的范围内)
	if (fcntl(fd, F_GETLK, &lock) == -1) return(-1);
	if (lock.l_type == F_UNLCK) return(0);
	return(lock.l_pid);	// 返回 获得文件锁的进程ID
}

pid_t Lock_fcntl_test(const int &fd, const short &type,
					  const short &whence, const off_t &offset,
					  const off_t &len) // 测试 锁
{
	pid_t pid;
	if ( (pid = lock_fcntl_test(fd, type, whence, offset, len)) == -1) {
		err_sys(-1, __FILE__, __func__, __LINE__,
				"lock_fcntl_test()");
	}
	return(pid);
}

/**【规则】：
    如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁。
    如果某进程申请了 写锁，其它进程 可以 申请  读锁，不能申请 写锁。
	写锁 优先级 高于 读锁              */

/* 	时间	  	父进程   子进程01	子进程02
	0     	取得读锁
 	1		|		申请写锁
 	2		|		.		取得读锁
	3	  	释放读锁	.		.
 	4				取得写锁	释放读锁
 	5				释放写锁                  */

// 如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁
void test_file_lock_fcntl_01()
{
	using namespace std;
	pid_t pid_parent = getpid();
	_debug_log_info("test_file_lock_fcntl_01(): 进程 ID %d begin", pid_parent)

	int fd = Open("/tmp/zcq_tmp.txt",
				  O_CREAT/*不存则创*/|O_RDWR, FILE_MODE);
	lock_fcntl_Read(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] parent 取得读锁" << endl;

	pid_t pid_child01 = Fork();
	if (pid_child01 == 0) {    // 创建 子进程
		sleep(1);
		pid_child01 = getpid();
//		_debug_log_info("子进程01 ID %d begin", pid_child01)

		cout << "[" << time_HMS()
			 << "] child01 申请写锁" << endl;
		lock_fcntl_Writew(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child01 取得写锁" << endl;

		sleep(1);
		lock_fcntl_Un(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child01 释放写锁" << endl;

//		_debug_log_info("子进程01 ID %d end", pid_child01)
		exit(0);
	}

	pid_t pid_child02 = Fork();
	if (pid_child02 == 0) { 	// 创建 子进程
		sleep(2);
		pid_child02 = getpid();
//		_debug_log_info("子进程02 ID %d begin", pid_child02)

		cout << "[" << time_HMS()
			 << "] child02 申请读锁" << endl;
		lock_fcntl_Read(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child02 取得读锁" << endl;

		sleep(2);
		lock_fcntl_Un(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child02 释放读锁" << endl;

//		_debug_log_info("子进程02 ID %d end", pid_child02)
		exit(0);
	}

	_debug_log_info("创建 子进程01 ID %d\t创建 子进程02 ID %d", pid_child01, pid_child02)

	sleep(3);
	lock_fcntl_Un(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] parent 释放读锁" << endl;

//	sleep(3);
	_debug_log_info("test_file_lock_fcntl_01(): 进程 ID %d end", pid_parent)
}

/* 	时间	  	父进程   子进程01	子进程02
	0     	取得写锁
 	1		|		申请写锁	取得读锁
 	2		|		.		释放读锁
	3	  	释放写锁	取得写锁
 	4				释放写锁		     	             */

// 如果某进程申请了 写锁，其它进程 可以 申请 读锁，不能申请 写锁。
void test_file_lock_fcntl_02()
{
	using namespace std;
	pid_t pid_parent = getpid();
	_debug_log_info("test_file_lock_fcntl_02(): 进程 ID %d begin", pid_parent)

	int fd = Open("/tmp/zcq_tmp.txt",
				  O_CREAT/*不存则创*/|O_RDWR, FILE_MODE);
	lock_fcntl_Readw(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] parent 取得写锁" << endl;

	pid_t pid_child01 = Fork();
	if (pid_child01 == 0) {    // 创建 子进程
		sleep(1);
		pid_child01 = getpid();
//		_debug_log_info("子进程01 ID %d begin", pid_child01)

		cout << "[" << time_HMS()
			 << "] child01 申请写锁" << endl;
		lock_fcntl_Writew(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child01 取得写锁" << endl;

		sleep(1);
		lock_fcntl_Un(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child01 释放写锁" << endl;

//		_debug_log_info("子进程01 ID %d end", pid_child01)
		exit(0);
	}

	pid_t pid_child02 = Fork();
	if (pid_child02 == 0) { 	// 创建 子进程
		sleep(1);
		pid_child02 = getpid();
//		_debug_log_info("子进程02 ID %d begin", pid_child02)

		cout << "[" << time_HMS()
			 << "] child02 申请读锁" << endl;
		lock_fcntl_Readw(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child02 取得读锁" << endl;

		sleep(1);
		lock_fcntl_Un(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS()
			 << "] child02 释放读锁" << endl;

//		_debug_log_info("子进程02 ID %d end", pid_child02)
		exit(0);
	}

	_debug_log_info("创建 子进程01 ID %d\t创建 子进程02 ID %d", pid_child01, pid_child02)

	sleep(3);
	lock_fcntl_Un(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] parent 释放写锁" << endl;

//	sleep(3);
	_debug_log_info("test_file_lock_fcntl_02(): 进程 ID %d end", pid_parent)
}

/* 	时间	  	父进程   子进程01	子进程02	子进程03
	0
 	1		申请读锁	申请读锁	申请写锁	申请写锁
 	2		.		.		.		.
	3	  	取得读锁	取得读锁	取得写锁	取得写锁
 	4		释放读锁	释放读锁	释放写锁	释放写锁        */

void test_file_lock_fcntl_03() // 测试 优先级
{
	using namespace std;

	int fd = Open("/tmp/zcq_tmp.txt",
				  O_CREAT/*不存则创*/| O_RDWR, FILE_MODE);

	auto lambda_write = [&fd](const uint8_t &num) {
		sleep(1);
		cout << "[" << time_HMS() << "] child0" << (int)num << " 申请写锁" << endl;
		lock_fcntl_Writew(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS() << "] child0" << (int)num << " 取得写锁" << endl;
		lock_fcntl_Un(fd, SEEK_SET, 0, 0);
		cout << "[" << time_HMS() << "] child0" << (int)num << " 释放写锁" << endl;
		exit(0);
	};

	pid_t pid_child02 = Fork(); // 创建 子进程
	if (pid_child02 == 0) lambda_write(2);

	pid_t pid_child03 = Fork(); // 创建 子进程
	if (pid_child03 == 0) lambda_write(3);

	pid_t pid_child01 = Fork(); // 创建 子进程
	sleep(1);

	cout << "[" << time_HMS() << "] "
		 << ((pid_child01==0)? "child01":"parent") << " 申请读锁" << endl;
	lock_fcntl_Readw(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] "
		 << ((pid_child01==0)? "child01":"parent") << " 取得读锁" << endl;
	lock_fcntl_Un(fd, SEEK_SET, 0, 0);
	cout << "[" << time_HMS() << "] "
		 << ((pid_child01==0)? "child01":"parent") << " 释放读锁" << endl;
}

/** 最好用 fcntl()记录上锁的方法模拟软件只启动 1次(/var/run/xxx.pid) **/
//#define	PATH_RUN_pid	"/var/run/zcq.pid"
#define	PATH_RUN_pid	"/tmp/zcq.pid"

void test_file_lock_fcntl_04() // 模拟软件只启动 1次/1个 进程
{
	int fd = Open(PATH_RUN_pid, O_CREAT/*不存则创*/| O_RDWR, FILE_MODE);

	auto lambda_write = [&fd](const pid_t &pid) {
		if (lock_fcntl_write(fd, SEEK_SET, 0, 0) < 0) {
			if (errno == EACCES || errno == EAGAIN) {
				err_quit(-1, __LINE__,
						 "PID:%d, Unable to lock %s, is %s already running ?",
						 pid, PATH_RUN_pid, __FILE__);
			}
			err_sys(-1, __LINE__, "lock_fcntl_write(): PID %d", pid);
		}
	};

	pid_t pid = getpid();
	_debug_log_info("进程 ID %d", pid)

	lambda_write(pid);
	std::string strbuf = std::to_string(pid);
	ftruncate(fd, 0); // 截断文件 fd，打开长度为 length 个字节
	Write(fd, strbuf.c_str(), strbuf.size());

	if (Fork()==0) { // 创建 子进程
		sleep(1);
		pid = getpid();
		_debug_log_info("子进程 ID %d", pid)

		lambda_write(pid);
		strbuf = std::to_string(pid);
		ftruncate(fd, 0); // 截断文件 fd，打开长度为 length 个字节
		Write(fd, strbuf.c_str(), strbuf.size());

		exit(0);
	}
	sleep(10);
}


/**-------------------- lockf() 库函数(是 fcntl()的封装) --------------------**/
/* #include <fcntl.h> / <unistd.h>
 int lockf(int fd, int cmd, off_t len); // 若成功，依赖于 cmd，否则返回 -1
 len：为从文件当前位置的起始要锁住的长度。
 cmd 参数：
 	F_LOCK：给文件(区域)加互斥锁；若文件已被加锁，则阻塞。
	F_TLOCK：同 F_LOCK；但不会阻塞，返回错误。
	F_ULOCK：解锁先前锁定的区域。        */



/**-------------------- flock() 系统调用(劝告/建议性锁) --------------------**/
/* #include <sys/file.h>
 int flock(int fd, int operation); // 建议性锁，只对整个文件上锁
 fd 文件描述符；
-----------------------------
 operation 参数：
 	LOCK_SH ：共享锁/读锁
 	LOCK_EX ：互斥锁/排他锁/独占锁/写锁
 	LOCK_UN : 解锁。
 	LOCK_NB ：锁定时不要阻塞（与以上三种操作一起使用）         */

/* flock()创建的锁是和文件打开表项(struct file)相关联的，而不是fd。
 * 这就意味着复制文件fd（通过 fork()或 dup()）后，
 * 那么通过这两个fd都可以操作这把锁（例如通过一个fd加锁，通过另一个fd可以释放锁），
 * 也就是说子进程继承父进程的锁。
 * 但是上锁过程中关闭其中一个fd，锁并不会释放（因为file结构并没有释放），
 * 只有关闭所有复制出的fd，锁才会释放。
 * 使用 open()打开同一个文件 2次，得到的 2个 fd 是独立的（因为底层对应 2个 file对象），
 * 通过其中一个加锁，通过另一个无法解锁、也无法上锁。    */
void test_file_flock_01() // 建议性锁：对 fd1 上锁，并不影响程序 dup() 复制为 fd2 上锁
{
	_debug_log_info("test_file_flock_01()")
	int ret;
	int fd1 = Open("../process_file_lock/tmp.txt",
				   O_CREAT/*不存则创*/|O_RDWR, 0777);

	int fd2 = dup(fd1); // 复制 fd，在同一文件上返回 新的 文件描述符。
	err_sys(fd2, __LINE__, "dup()");

	printf("fd1: %d, fd2: %d\n", fd1, fd2);

#if 0
	ret = flock(fd1, LOCK_EX/*互斥锁*/);
#else
	ret = flock(fd1, LOCK_SH/*共享锁*/);
#endif
	err_sys(ret, __LINE__, "flock()");
	printf("get lock1, ret: %d\n", ret);

	/** fd2 是 dup() 后 新的 文件描述符，可上锁(互斥锁+共享锁) **/
#if 0
	ret = flock(fd2, LOCK_EX/*互斥锁*/);
#else
	ret = flock(fd2, LOCK_SH/*共享锁*/);
#endif
	err_sys(ret, __LINE__, "flock()");
	printf("get lock2, ret: %d\n", ret);

	/* 使用 open()打开同一个文件 ？次，得到的 ？个 fd 是独立的（因为底层对应 ？个 file对象），
	 * 通过其中一个加锁，通过另一个无法解锁、也无法上锁。   */

	int fd3 = Open("../process_file_lock/tmp.txt", O_RDONLY, DIR_MODE);

	int fd4 = Open("../process_file_lock/tmp.txt", O_WRONLY, FILE_MODE);

	printf("fd3: %d, fd4: %d\n", fd3, fd4);

	/** 该进程已上锁(互斥锁)，则不能再上锁 errno=11: Resource temporarily unavailable **/
	/** 该进程已上锁(共享锁)，可以再上锁(共享锁) **/
	ret = flock(fd3, LOCK_NB/*不阻塞*/|LOCK_SH/*共享锁*/);
	err_sys(ret, __LINE__, "flock()");
	printf("get lock3, ret: %d\n", ret);

	/** 该进程已上锁(互斥锁)，则不能再上锁 errno=11: Resource temporarily unavailable **/
	/** 该进程已上锁(共享锁)，则不能再上锁(互斥锁) errno=11: xxx **/
//	ret = flock(fd4, LOCK_NB/*不阻塞*/|LOCK_EX/*互斥锁*/);
//	err_sys(ret, __LINE__, "flock()");
//	printf("get lock4, ret: %d\n", ret);
}

/* flock()创建的锁是和文件打开表项(struct file)相关联的，而不是fd。
 * 这就意味着复制文件fd（通过 fork()或 dup()）后，
 * 那么通过这两个fd都可以操作这把锁（例如通过一个fd加锁，通过另一个fd可以释放锁），
 * 也就是说子进程继承父进程的锁。
 * 但是上锁过程中关闭其中一个fd，锁并不会释放（因为file结构并没有释放），
 * 只有关闭所有复制出的fd，锁才会释放。
 * 使用 open()打开同一个文件 2次，得到的 2个 fd 是独立的（因为底层对应 2个 file对象），
 * 通过其中一个加锁，通过另一个无法解锁、也无法上锁。    */
void test_file_flock_02() // 建议性锁：子进程持有锁，并不影响父进程通过相同的fd获取锁，反之亦然
{
	_debug_log_info("test_file_flock_02()")
	int ret;
	int fd = Open("../process_file_lock/tmp.txt",
				  O_CREAT/*不存则创*/|O_RDWR, 0777);
	if ((Fork()) == 0){
		ret = flock(fd, LOCK_EX/*互斥锁*/);
		printf("chile get lock, fd: %d, ret: %d\n", fd, ret);
		sleep(2);
		printf("chile exit\n");
		exit(0);
	}
	sleep(1);
	ret = flock(fd, LOCK_EX/*互斥锁*/);
	printf("parent get lock, fd: %d, ret: %d\n", fd, ret);
	printf("parent exit\n");
}


