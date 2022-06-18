
#include "IO_File.h"


#include <iostream>

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
//	file_creat_close(); // 创建文件, 文件创建或打开后，不使用时要关闭
//	file_open_read(); // 读取 已存在文件
//	file_open_write(); // 写 已存在文件
//	file_location_write(); // 定位文件
//	file_stat(); // 获取 文件信息
//	file_lock(); // Linux 文件锁 避免共享的(文件)资源产生竞争状态
//	file_map(); // mmap() 文件映射到内存, 像访问内存一样不必再调用 read 或 write 等操作

	return 0;
}
#endif


/*-------------------------------------------------------------------*/


/* if errnum is nonzero, follow it with ": " and 	`strerror (errnum)`.
 * if status is nonzero, terminate the program with `exit (status)`. */

/* 自定义错误打印 */
void file_error_print(const int errnum, const char *format1=nullptr, const char *format2=nullptr)
{
	if ( errnum < 0 ) // exit (errno)
	{
		if ( format2 == nullptr )
			error(errno, errnum, "Fail : %s\n", format1);
		else
			error(errno, errnum, "Fail : %s[%s]\n",format1,format2);
	}
	else
	{
		std::cout << format1 << " (" << errnum << ")" << std::endl;
	}
}


#include <sys/types.h> // POSIX Standard: Primitive System Data Types
#include <fcntl.h> // POSIX Standard : File Control Operations
#include <unistd.h> // POSIX Standard: Symbolic Constants

/* Linux 文件访问权限  (u)rwx  (g)rwx  (o)rwx
 * (用户组  u:所属用户  g:所属组  o:其他用户)  r:读  w:写  x:执行
 * Linux 命令 "ls -l" 可列出文件的权限,
 * 如 drwxrwxrwx 1 root root 4096 3月  3  20:46  cmake-build-debug
 *    s-rwxrwxrwx 1 root root 2847 12月 9  15:04  CMakeLists.txt
 * 第一个字母对应的关系 : “d”目录/文件夹, “-”普通文件, ”l“符号链接,
 *                     ”c“ 字符设备, "b"块设备, "s"套接字, "p"管道
 * 权限			(u)rwx (g)rwx (o)rwx			mode
 * 最高权限	0777 = 111    111    111  读写执行
 * 只读		0444 = 100    100    100
 * 读写		0666 = 110    110    110  */

/* 创建文件, 关闭文件 */
void file_creat_close() // 创建文件, 文件创建或打开后，不使用时要关闭
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)
	char filename02[] = "../file_test-02.txt"; // 文件名，相对路径(../)

/*  文件创建 等价形式
形式 1: int fd = open  (filename,O_CREAT|O_WRONLY|O_TRUNC,mode); // 创建文件 <fcntl.h>
形式 2: int fd = creat (filename,mode); // 创建文件 <fcntl.h>
	返回值：成功返 文件描述符，否则返 -1
------------------------------------------------------
int open(const char *pathname, int oflag);
int open(const char *pathname, int oflag, mode_t mode);
参数 oflag : 用于 指定文件 的 打开/创建 模式
	O_RDONLY   	只读模式
	O_WRONLY  	只写模式
	O_RDWR     	读写模式
	O_APPEND   	每次写操作都写入文件的末尾, 读写位置一直指向文件尾 !!!
	O_CREAT   	如果指定文件不存在，则创建这个 "新" 文件 !!!
    O_EXCL    	如果要创建的文件已存在，则返回 -1，并且修改 errno 的值
    O_TRUNC   	如果文件存在，并且以只写/读写方式打开，则清空文件全部内容
    O_NOCTTY	如果路径名指向终端设备，不要把这个设备用作控制终端。
    O_NONBLOCK	如果路径名指向 FIFO/块文件/字符文件，则把文件的打开和后继 I/O 设置为非阻塞模式（nonblocking mode）

	以下三个常量是三选一，会降低性能, 它们用于同步输入输出 :
  	O_DSYNC   等待物理 I/O 结束后再 write。在不影响读取新写入的数据的前提下，不等待文件属性更新。
	O_RSYNC   read 等待所有写入同一区域的写操作完成后再进行
	O_SYNC    等待物理 I/O 结束后再 write，包括更新文件属性的 I/O open 返回的文件描述符一定是最小的未被使用的描述符。

参数 mode : 指定 文件访问权限, 仅当创建 "新" 文件时才使用 !!!  */
	// 文件创建 形式 open()
	fd = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0777); // 创建文件 最高权限 <fcntl.h>
	file_error_print(fd, "open file", filename);

	// 文件创建 形式 creat()
	int fd02 = creat(filename02,0444); // 创建文件 只读 <fcntl.h>
	file_error_print(fd02, "open file", filename02);

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
	close(fd02);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}


//#include <cstring>
//#include <fcntl.h> // POSIX Standard : File Control Operations
//#include <unistd.h> // POSIX Standard: Symbolic Constants

/* 不创建文件, 打开 已存在文件 并 读取内容 */
void file_open_read() // 读取 已存在文件
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

	fd = open(filename,O_RDONLY); // 只读模式 打开 已存在文件 <fcntl.h>
	file_error_print(fd, "open file", filename);

/*	ssize_t read(int fd, void *buf, size_t nbytes); // 读取文件数据 <unistd.h>
	read 函数从 fd 指定的 已打开 文件中 读取 nbytes 字节到 buf 中。
		返回值：	实际读取到的字节数；
				返回 0（读到 EOF）; 	返回 -1（见 errno 错误对照表）
	以下几种情况会导致读取到的 字节数 小于 nbytes ：
    A. 	读取普通文件时，读到文件末尾还不够 nbytes 字节。
		例如：如果文件只有 30 字节，而我们想读取 100 字节，
		那么实际读到的只有 30 字节，read 函数返回 30 。
		此时再使用 read 函数作用于这个文件会导致 read 返回 0 （读到 EOF）。
    B. 	从 终端设备（terminal device）读取时，一般情况下 每次 只能 读取 一行。
    C. 	从 网络 读取时，网络缓存可能导致读取的字节数小于 nbytes 字节。
    D. 	读取 pipe 或者 FIFO 时，pipe 或 FIFO 里的字节数可能小于 nbytes 。
    E. 	从面向记录（record-oriented）的设备读取时，
    	某些面向记录的设备（如磁带）每次最多 只能 返回一个记录。
    F. 	在读取了部分数据时被信号中断。   */

	ssize_t size=-1;
	char buf[16]; // 存放 读取 的 文件数据
	std::cout << "------------ Read" << std::endl;
	while (size) // 循环读取文件数据，直至文件末尾或者出错
	{
		size=read(fd,buf,std::size(buf)); // 读取文件数据 <unistd.h>
		file_error_print(size, "read file", filename);
		if (size>0)
		{
			for(int i=0;i<size;++i){
				std::cout << buf[i] ;
			}
			std::cout << std::endl;
		}
		else if (size==0)
		{
			std::cout << "------------ Reach the end of file"
					  << std::endl;
		}
		else // 返回 -1（见 errno 错误对照表）
		{
			break;
		}
	}

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}


//#include <fcntl.h> // POSIX Standard : File Control Operations
//#include <unistd.h> // POSIX Standard: Symbolic Constants

/* 不创建文件, 打开 已存在文件 并 写内容 */
void file_open_write() // 写 已存在文件
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

	/* O_APPEND 每次写操作都写入文件的末尾 */
	fd = open(filename,O_WRONLY|O_APPEND); // 只写模式 打开 已存在文件 <fcntl.h>
	file_error_print(fd, "open file", filename);

	char buf[]="\n++++++"; // 存放 写 的 文件数据

/*	ssize_t write(int filedes, const void *buf,size_t nbytes); // 写文件数据 <unistd.h>
write 函数向 filedes 中写入 nbytes [std::size(buf)-1]字节数据，数据来源为 buf 。
	返回值：	实际写入文件的字节数；	返回 -1（见 errno 错误对照表）,
			常见的出错原因是磁盘空间满了或者超过了文件大小限制。      */
	ssize_t size = write(fd,buf,std::size(buf)-1); // 写文件数据 <unistd.h>
	file_error_print(size, "write file", filename);

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}


//#include <fcntl.h> // POSIX Standard : File Control Operations
//#include <unistd.h> // POSIX Standard: Symbolic Constants

/* 不创建文件, 打开 已存在文件 并 写内容
 * lseek() 用来定位文件指针，用户可以定位到指定的位置进行读写等操作 */
void file_location_write() // 定位文件
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

	fd = open(filename,O_WRONLY); // 只写模式 打开 已存在文件 <fcntl.h>
	file_error_print(fd, "open file", filename);

/*	每一个已打开的文件都有一个读写位置, 当打开文件时通常其读写位置是指向文件开头
若以附加方式打开文件(如 O_APPEND 每次写操作都写入文件末尾 ),则读写位置一直指向文件尾 !!!
	off_t lseek(int fd, off_t offset, int whence); // 设置文件指针位移 <unistd.h>
 		参数 whence		参数 offset 值(位移量/字节个数)
		SEEK_SET 		从文件的起始位置开始定位
		SEEK_CUR 		从当前位置开始定位, 参数 offset 可正可负也可为零
		SEEK_END 		从文件末尾位置开始定位, 参数 offset 可正可负也可为零
下列是教特别的使用方式:
	1) 欲将读写位置移到文件开头时:	off_t offset = lseek(int fd, 0, SEEK_SET);
	2) 欲将读写位置移到文件尾时:	off_t offset = lseek(int fd, 0, SEEK_END);
	3) 想要取得目前文件位置时:		off_t offset = lseek(int fd, 0, SEEK_CUR);
返回值：	当调用成功时则返回目前的读写位置, 也就是 距离 文件开头 多少个 offset 字节.
		若有错误则返回 -1（见 errno 错误对照表） */

	/* 获取 文件内容 大小 (字节个数) */
	off_t offset = lseek(fd, 1, SEEK_END); // 设置文件指针位移 <unistd.h>
	file_error_print(offset, "lseek file");

	char buf[]=" () "; // 存放 写 的 文件数据
	ssize_t size = write(fd,buf,std::size(buf)-1); // 写文件数据 <unistd.h>
	file_error_print(size, "write file", filename);

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}


//#include <unistd.h> // POSIX Standard: Symbolic Constants
//#include <sys/types.h> // POSIX Standard: Primitive System Data Types
#include<sys/stat.h> //  POSIX Standard: File Characteristics
#include <cstdio>
#include <cstring>



void file_stat() // 获取 文件信息
{
	/* struct stat {	// 存放文件属性信息
    dev_t         st_dev;       //文件的设备编号
    ino_t         st_ino;       //节点
    mode_t        st_mode;      //文件的 类型 和 存取的 权限
    nlink_t       st_nlink;     //连到该文件的硬连接数目，刚建立的文件值为1
    uid_t         st_uid;       //用户ID 文件所有者
    gid_t         st_gid;       //组ID 文件所有者对应的组
    dev_t         st_rdev;      //(特殊设备号码)若此文件为设备文件，则为其设备编号
    off_t         st_size;      //普通文件 文件大小
    unsigned long st_blksize;   //块大小(文件系统的I/O 缓冲区大小)
    unsigned long st_blocks;    //块数
    time_t        st_atime;     //最后一次访问时间
    time_t        st_mtime;     //最后一次修改时间
    time_t        st_ctime;     //文件状态改变时间(指属性)
}; 	*/
	struct stat st{}; // 存放文件属性信息

	/*	void *memset(void *str, int c, size_t n)
	填充 n 个字符 c 到参数 str 所指向的内存块。返回值是一个指向存储区 str 的指针。	 */
	memset(&st, 0, sizeof(st)); // 清零

	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

/*	stat() 通过 文件名字，获取 文件 对应的属性。文件 没有打开 的操作
 	int stat (const char * file_name, struct stat * buf); // 文件 没有打开 的操作
-----------------------------------
 	fstat() 通过 文件描述符 获取文件对应的属性。文件 打开后 的操作
 	int fstat (int fd, struct stat * buf); // 文件 打开后 的操作
-----------------------------------
	lstat() 通过 符号连接，获取 符号连接 对应的属性。
 			但是 命名的文件 是一个符号连接时，返回该符号连接的有关信息，
 			而不是由 该符号连接 引用的 文件的信息 !!!
	int lstat (const char * file_name, struct stat * buf);
-----------------------------------
通过 文件名(file_name) 或 文件描述符(fd) 获取文件信息，并保存在 buf 所指的结构体 stat 中。
返回值: 执行成功则返回 0，失败返回 -1，错误代码存于 errno  	*/

	int value = stat(filename, &st); // 文件 没有打开 的操作
#if 0 /* 判断文件是否存在 */
	if ( ! value) // 如果文件不存在，stat() 就会返回 非０
	{
		if (st.st_size >= 0) // 加了一层验证保证
		{
			std::cout << "Exist file : " << filename << std::endl;
			file_stat_print(st);
		}
	}
	else if(ENOENT == errno) {
		std::cerr << "NOT exist file : " << filename << std::endl;
	}
#else
	if ( value == 0 ) // 如果文件不存在，stat() 就会返回 非０
	{
		FILE *fd = fopen(filename, "r");
		if ( fd != nullptr )
//		if ( fd )
		{
			std::cout << "Exist file : " << filename << std::endl;
			file_stat_print(st);
		}
	}
#endif
}



//#include <unistd.h> // POSIX Standard: Symbolic Constants
//#include <sys/types.h> // POSIX Standard: Primitive System Data Types

/*	使用两次 fcntl() 函数分别用于 给文件上锁 和 判断文件是否可以上锁	*/
void file_lock_set(int fd, short l_type/* 锁定的状态 */) // fcntl () 默认是 建议锁
{
	std::cout << "**********************" << std::endl;

/*	记录锁的具体状态
	struct flock {
    	short l_type;  // 锁定的状态 : F_RDLCK 读锁，F_WRLCK 写锁，F_UNLCK 解锁
	    off_t l_start; // 锁相对偏移量(字节数)
    	short l_whence;// 锁开始的位置，SEEK_SET, SEEK_CUR, SEEK_END
    	off_t l_len;   // 锁定区域的大小，0 表示从 起始 到 文件末
    	pid_t l_pid;   // 当前的 进程 ID 号, 仅由 F_GETLK（fcntl() 的 cmd 参数) 返回
	};
	参数 l_type		文件锁的状态
  		F_RDLCK 	读取锁，多个进程可同时获取的 共享锁
  		F_WRLCK 	写入锁，任何时刻只能一个进程获取的 独占锁
  		F_UNLCK 	解锁
--------------------------------
	参数 l_whence	参数 l_start 值(位移量/字节个数)
		SEEK_SET 		从文件的起始位置开始定位
		SEEK_CUR 		从当前位置开始定位, 参数 l_whence（offset）可正可负也可为零
		SEEK_END 		从文件末尾位置开始定位, 参数 offset 可正可负也可为零
	加锁整个文件，通常 l_start=0，l_whence=SEEK_SET，l_len=0	 */
	struct flock lock{}; // 记录锁的具体状态
	lock.l_type = l_type; // 根据不同的 l_type 值给文件 上锁 或 解锁
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	lock.l_pid = getpid();

/*	fcntl () 执行 文件描述符(fd) 上 CMD 描述的 文件控制操作, 如果出错返回 -1
 	fcntl () 默认是 建议锁；	在 Linux ROOT 权限下用命令 mount -o mand 实现 强制锁
int fcntl(int fd, int cmd);
int fcntl(int fd, int cmd, long arg);
int fcntl(int fd, int cmd, struct flock *lock);
参数 cmd :
	F_GETLK		获取 参数 lock 描述 信息
	F_SETLK		设置记录锁定信息（非阻塞版本）, 设置 参数 lock 描述的 文件锁
	F_SETLKW	设置记录锁定信息（阻塞版本）, 命令名中的 W 表示等待(wait),
				如果存在其他锁，则调用进程睡眠；如果捕捉到信号则睡眠中断

	F_DUPFD 复制文件描述符
	F_GETFD	返回 文件描述符 close-on-exec 标志,
 			若标志未设置,则文件经过exec函数后仍保持打开状态
	F_SETFD	设置 文件描述符 close-on-exec 标志, 该标志以参数 arg 的 FD_CLOEXEC 位决定
	F_GETFL	获取文件状态标志。
	F_SETFL	设置 open 文件状态标志。
	F_GETOWN：获取 接收到 SIGIO 和 SIGURG 信号的 进程号 或 进程组号
	F_SETOWN：设置 去接收 SIGIO 和 SIGURG 信号的 进程号 或 进程组号		*/

	/* 给文件上锁 */
	if ( ( fcntl(fd, F_SETLK, &lock) ) == 0) // 参数 F_SETLK 非阻塞版本 锁
	{
		switch (lock.l_type) {
			case F_RDLCK : /*读取锁*/
				std::cout << "read lock set by " ;
				break;
			case F_WRLCK : /*写入锁*/
				std::cout << "write lock set by " ;
				break;
			case F_UNLCK : /*解锁*/
				std::cout << "release lock by " ;
				break;
			default :
				break;
		}
		std::cout << getpid() << "\tlock.l_pid : " << lock.l_pid << std::endl;
	}
	else
	{
		/* 判断文件是否可以上锁 */
		fcntl(fd, F_GETLK, &lock); // 参数 F_GETLK 获取 参数 lock 描述 信息
		switch (lock.l_type) {
			case F_RDLCK : /*读取锁*/
				std::cout << "read lock already set by " ;
				break;
			case F_WRLCK : /*写入锁*/
				std::cout << "write lock already set by ";
				break;
			case F_UNLCK : /*解锁*/
				std::cout << "release lock already by " ;
				break;
			default :
				std::cout << "default " ;
				break;
		}
		std::cout << getpid() << "\tlock.l_pid : " << lock.l_pid << std::endl;
	}
}

/* 多个用户共同操作一个文件时，用 Linux 文件锁 避免共享的(文件)资源产生竞争状态 */
void file_lock() // Linux 文件锁 避免共享的(文件)资源产生竞争状态
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

	fd = open(filename,O_RDWR | O_CREAT, 0777); // 打开 已存在文件 <fcntl.h>
	file_error_print(fd, "open file", filename);

	std::cout << "----------------- pid : " << getpid() << std::endl;

	/*给文件上写入锁*/
	file_lock_set(fd, F_WRLCK);// 建议锁
	/*给文件解锁*/
	file_lock_set(fd, F_UNLCK);// 建议锁
	/*给文件上读取锁*/
	file_lock_set(fd, F_RDLCK);// 建议锁
	/*给文件解锁*/
	file_lock_set(fd, F_UNLCK);// 建议锁

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}

//#include <unistd.h> // POSIX Standard: Symbolic Constants
//#include <fcntl.h> // POSIX Standard : File Control Operations
//#include <sys/types.h> // POSIX Standard: Primitive System Data Types
//#include<sys/stat.h> //  POSIX Standard: File Characteristics
//#include <fcntl.h> // POSIX Standard : File Control Operations
#include <cstring> /* for memcpy */
#include <sys/mman.h> /* for mmap and munmap */

/* 	mmap() 将一个 文件 或者 其它对象 映射进内存。? 页 = ? * 1024
	mmap()系统调用使得进程之间通过映射同一个普通文件实现共享内存。
	普通文件被映射到进程地址空间后，进程可以向访问普通内存一样对文件进行访问，
	不必再调用 read()或 write()等操作。
---------------------------------
	munmap() 执行相反的操作，删除特定地址区域的对象映射
---------------------------------
注	实际上，mmap()系统调用并不是完全为了用于共享内存而设计的。
 	它本身提供了不同于一般对普通文件的访问方式，进程可以像读写内存一样对普通文件的操作。  */
void file_map() // mmap() 文件映射到内存, 像访问内存一样不必再调用 read 或 write 等操作
{
	int fd = -1; // 文件描述符 ; 错误(-1)
	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

	fd = open(filename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR); // 打开文件 <fcntl.h>
	file_error_print(fd, "open file", filename);

	/* 获取 文件内容区域 (映射区) 大小 : length 值(字节个数) */
	off_t length = 1024; // 文件内容区域 (映射区) 大小
	length = lseek(fd, 1, SEEK_END); // 设置文件指针位移 <unistd.h>
	std::cout << "length : " << length << std::endl;

/*	mmap() 和 munmap() : Unix 的系统函数，更贴近底层
=====================================================================
void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *start, size_t length);
参数 ：	----------
start	映射区的起始地址；通常设置为 start=0，表示由系统自己决定
length	映射区的长度，即文件映射到内存的数据大小
fd		文件描述符
----------------------------------------
prot	映射区的保护标志(分配内存的权限), 不能与文件的打开模式冲突。
		prot 值可以组合，如 : PROT_READ | PROT_WRITE
	prot 值		标志
	PROT_EXEC 	映射区(页内容) 可以被 执行
	PROT_READ  	映射区(页内容) 可以被 读取
	PROT_WRITE 	映射区(页内容) 可以被 写入
	PROT_NONE  	映射区(页内容) 不 可访问
----------------------------------------
flags	指定 映射对象的类型、映射选项 和 映射页是否可以共享。
		flags 值可以组合，如 :  |
	标志 MAP_SHARED 和 MAP_PRIVATE 是互斥的，只能使用其中一个 !!!
	flags 值	标志
	MAP_SHARED	共享映射空间，允许其它进程共享写入数据（将写入到原文件）。
	MAP_PRIVATE	写入到临时的内存区域，不会影响到原文件，（不写入到原文件）
 				不允许其它进程访问此内存区域，不影响 其它进程 操作原文件
	MAP_FIXED	使用指定的 start 映射起始地址，
 				若由 start 和 len 参数 指定的内存区重叠于现存的映射空间，重叠部分将会被丢弃。
 				如果指定的起始地址不可用，操作将会失败。并且起始地址必须落在页的边界上。
				不推荐此参数，建议设置 start=0, 由系统自己决定。
 	MAP_ANONYMOUS	匿名映射，映射区不与任何文件关联，映射区不能共享
	MAP_LOCKED	锁定映射区的页面，防止页面被交换出内存。
	MAP_NORESERVE	不要为这个映射保留交换空间。
					当交换空间被保留，对映射区修改的可能会得到保证。
					当交换空间不被保留，同时内存不足，对映射区的修改会引起段违例信号。
	MAP_GROWSDOWN	用于堆栈，告诉内核 VM 系统，映射区可以向下扩展。
	MAP_32BIT 	将映射区放在进程地址空间的低 2 GB，指定 MAP_FIXED 时会忽略 MAP_32BIT。
 				当前这个标志只在 x86-64 平台上得到支持。
	MAP_POPULATE 	为文件映射通过预读的方式准备好页表。随后对映射区的访问不会被页违例阻塞。
	MAP_NONBLOCK 	仅和 MAP_POPULATE 一起使用时才有意义。
 					不执行预读，只为已存在于内存中的页面建立页表入口。
----------------------------------------
offset  位置 	标志
	SEEK_SET 	从文件的起始位置开始定位
	SEEK_CUR 	从当前位置开始定位
	SEEK_END 	从文件末尾位置开始定位
----------------------------------------
返回	成功时，mmap()返回 被映射 文件内容区域 的 指针，munmap()返回 0。
		失败时，mmap()返回 MAP_FAILED [ 其值为 (void *)-1 ]，munmap() 返回 -1。
	errno 值	标志
	EACCES		访问出错
	EAGAIN		文件已被锁定，或者太多的内存已被锁定
	EBADF		fd不是有效的文件描述词
	EINVAL		一个或者多个参数无效
	ENFILE		已达到系统对打开文件的限制
	ENODEV		指定文件所在的文件系统不支持内存映射
	ENOMEM		内存不足，或者进程已超出最大内存映射数量
	EPERM		权能不足，操作不允许
	ETXTBSY		已写的方式打开文件，同时指定 MAP_DENYWRITE 标志
	SIGSEGV		试着向只读区写入
	SIGBUS		试着访问不属于进程的内存区   */

	void *start_addr = nullptr; // 映射区的起始地址；通常设置为 start=0，表示由系统自己决定
//	start_addr = (void*)0x80000; // 映射区的起始地址
	char *mapped_mem  = nullptr; // 指向映射内存的指针

/*	建立 文件 与 内存 的 映射	*/
	mapped_mem = (char*) mmap( start_addr,	// 映射区的起始地址
							   length,		// 映射区的长度，即文件映射到内存的数据大小
			/* 分配内存的权限 */ PROT_READ,	// 允许 读取
							   MAP_PRIVATE,	// 写入到临时的内存区域（不写入到原文件）
							   fd,			// 文件描述符
							   SEEK_SET	);	// 从文件的起始位置开始定位

	std::cout << "------------\n" << "打印映射区内容 : \n" << mapped_mem
			  << "\n------------" << std::endl;

/*	建立 文件 与 内存 的 映射, 修改内存映射	*/
	mapped_mem = (char*) mmap( start_addr,	// 映射区的起始地址
							   length,		// 映射区的长度，即文件映射到内存的数据大小
	/* 分配内存的权限,修改内存映射 */ PROT_READ|PROT_WRITE,	// 允许 读取, 写入
							   MAP_SHARED,	// 允许其它进程共享写入数据（将写入到原文件）
							   fd,			// 文件描述符
							   SEEK_SET	);	// 从文件的起始位置开始定位

	std::string str_seek="123";
	std::string str_replace="zcq";
	std::cout << "str_seek.size() : " << str_seek.size() << std::endl;
	std::cout << "str_replace.size() : " << str_replace.size() << std::endl;

/*	char *strstr (char *haystack, 		const char *	needle);
		参数		标志
		haystack	要被检索的 C 字符串。
		needle		在 haystack 字符串内 要搜索的 小字符串。
	strstr() 在 字符串	haystack 中查找 第一次出现 字符串 	needle 的位置，不包含终止符'\0'
返回值	该函数返回在 haystack 中第一次出现 needle 字符串的 位置，如果未找到则返回 null。*/
#if 1	// strstr() 每查找小字符串一次，从下一个位置（地址）开始查找，减小 strstr() 时间
	char *ptr = mapped_mem;
	while (  ( ptr = strstr(ptr, str_seek.c_str()) )  )// 查找 小字符串
	{
		memcpy(ptr, str_replace.c_str(), str_replace.size()); // 此处来 修改 原文件 内容
		ptr += str_replace.size();
	}
#else	// strstr() 每次从地址 mapped_mem 开始查找小字符串
	char *ptr = nullptr;
	while (  ( ptr = strstr(mapped_mem, str_seek.c_str()) )  )// 查找 小字符串
	{
		memcpy(ptr, str_replace.c_str(), str_replace.size()); // 此处来 修改 原文件 内容
	}
#endif

	std::cout << "------------\n" << "打印映射区内容 : \n" << mapped_mem
			  << "\n------------" << std::endl;

	munmap(mapped_mem, length); // 删除 特定地址 映射区 的 对象 mmap()

	close(fd);  // 文件创建或打开后，不使用时要 关闭文件 <unistd.h>
}

