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
 		后来进程又企图在同一区间再加一把锁，则新锁将替换老锁。
 	(2) 加读锁（共享锁）文件必须是读打开的，加写锁（排他锁）文件必须是写打开。
 	(3) 进程不能使用 F_GETLK 命令来测试它自己是否在文件的某一部分持有一把锁。
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


#ifndef __FILE_LOCK_H
#define __FILE_LOCK_H

#include <iostream>

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/types.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "wrap_fcntl.h"

#include "zcq_header.h"


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
    多个进程访问同一文件，只有读锁+读锁可以兼容。
    单个进程对同一区间重复加锁的结果是后面的锁覆盖前面的锁。
【规则】：
    如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁；
    如果某进程申请了 写锁，其它进程 可以 申请  读锁，不能申请 写锁。
	写锁 优先级 高于 读锁              */

int lock_fcntl_reg(const int &fd, const int &cmd, const short &type,
				   const short &whence, const off_t &offset,
				   const off_t &len); // 获取/释放 锁

#define	lock_fcntl_read(fd, whence, offset, len) \
			lock_fcntl_reg(fd, F_SETLK, F_RDLCK, whence, offset, len)
#define	lock_fcntl_readw(fd, whence, offset, len) \
			lock_fcntl_reg(fd, F_SETLKW, F_RDLCK, whence, offset, len)
#define	lock_fcntl_write(fd, whence, offset, len) \
			lock_fcntl_reg(fd, F_SETLK, F_WRLCK, whence, offset, len)
#define	lock_fcntl_writew(fd, whence, offset, len) \
			lock_fcntl_reg(fd, F_SETLKW, F_WRLCK, whence, offset, len)
#define	lock_fcntl_un(fd, whence, offset, len) \
			lock_fcntl_reg(fd, F_SETLK, F_UNLCK, whence, offset, len)

void Lock_fcntl_reg(const int &fd, const int &cmd, const short &type,
					const short &whence, const off_t &offset,
					const off_t &len); // 获取/释放 锁

#define	lock_fcntl_Read(fd, whence, offset, len) \
			Lock_fcntl_reg(fd, F_SETLK, F_RDLCK, whence, offset, len)
#define	lock_fcntl_Readw(fd, whence, offset, len) \
			Lock_fcntl_reg(fd, F_SETLKW, F_RDLCK, whence, offset, len)
#define	lock_fcntl_Write(fd, whence, offset, len) \
			Lock_fcntl_reg(fd, F_SETLK, F_WRLCK, whence, offset, len)
#define	lock_fcntl_Writew(fd, whence, offset, len) \
			Lock_fcntl_reg(fd, F_SETLKW, F_WRLCK, whence, offset, len)
#define	lock_fcntl_Un(fd, whence, offset, len) \
			Lock_fcntl_reg(fd, F_SETLK, F_UNLCK, whence, offset, len)

pid_t lock_fcntl_test(const int &fd, const short &type,
					  const short &whence, const off_t &offset,
					  const off_t &len); // 测试 锁

#define	lock_fcntl_is_readable(fd, whence, offset, len) \
			lock_fcntl_test(fd, F_RDLCK, whence, offset, len)
#define	lock_fcntl_is_writeable(fd, whence, offset, len) \
			lock_fcntl_test(fd, F_WRLCK, whence, offset, len)

pid_t Lock_fcntl_test(const int &fd, const short &type,
					  const short &whence, const off_t &offset,
					  const off_t &len); // 测试 锁

#define	lock_fcntl_Is_readable(fd, whence, offset, len) \
			Lock_fcntl_test(fd, F_RDLCK, whence, offset, len)
#define	lock_fcntl_Is_writeable(fd, whence, offset, len) \
			Lock_fcntl_test(fd, F_WRLCK, whence, offset, len)

/**【规则】：
    如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁。
    如果某进程申请了 写锁，其它进程 可以 申请  读锁，不能申请 写锁。
	写锁 优先级 高于 读锁              */

// 如果某进程申请了 读锁，其它进程 可以 再申请 读锁，不能申请 写锁
void test_file_lock_fcntl_01();

// 如果某进程申请了 写锁，其它进程 可以 申请 读锁，不能申请 写锁。
void test_file_lock_fcntl_02();

void test_file_lock_fcntl_03(); // 测试 优先级

/** 最好用 fcntl()记录上锁的方法模拟软件只启动 1次(/var/run/xxx.pid) **/
void test_file_lock_fcntl_04(); // 模拟软件只启动 1次/1个 进程


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
 int flock(int fd, int operation); // 建议性锁
 fd 文件描述符；
-----------------------------
 operation 参数：
 	LOCK_SH ：共享锁
 	LOCK_EX ：排他锁/独占锁
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
void test_file_flock_01(); // 建议性锁：对 fd1 上锁，并不影响程序 dup() 复制为 fd2 上锁
void test_file_flock_02(); // 建议性锁：子进程持有锁，并不影响父进程通过相同的fd获取锁，反之亦然



#endif //__FILE_LOCK_H
