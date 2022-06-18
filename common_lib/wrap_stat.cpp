//
// Created by zcq on 2021/5/11.
//

#include "wrap_stat.h"

/******************************************************************************/

/*
二、FIFO（命名管道）
1.什么是FIFO
FIFO有时被称为命名管道。管道只能由具有血缘关系的进程使用，它们共同的祖先进程创建了管道。但是，通过FIFO，不具有血缘关系的进程也能交换数据。
FIFO是一种文件类型，创建FIFO类似于创建文件，而且FIFO的路径名存在与文件系统中，通过stat结构成员st_mode的编码指明文件是否为FIFO类型，可以用S_ISFIFO宏对此进行测试。
2.创建FIFO
(1) 在控制台shell，通过mkfifo命令直接创建 $ mkfifo test.fifo
(2) 在程序中通过mkfifo函数创建
#include<sys/types.h>  #include<sys/stat.h>
int mkfifo(const char *pathname,mode_t mode);  // 成功为0，失败为-1
特别说明：mkfifo只创建管道，不打开，一旦已经用mkfifo创建了一个FIFO，就可用open打开它，而且一般的文件 I / O函数(close、read、write、unlink等)都可用于FIFO;
path 参数 指明路径   		mode 参数 这里要注意一下，创建的管道文件的权限是 mode & ~umask
		创建屏蔽字umask可以通过 umask 函数调整，例如 umask(0000)
mode与打开普通文件的 open() 函数中的 mode 参数相同
3.FIFO的注意事项
当 open 一个 FIFO 时，非阻塞标志（O_NONBLOCK）会产生下列影响(阻塞与非阻塞模式)
(1) 没有指定 O_NONBLOCK ，只读 open 要阻塞到某个其他进程为写而打开这个 FIFO 为止；只写 open 要阻塞到某个其他进程为读而打开它为止。
(2) 如果指定了 O_NONBLOCK，则只读 open 立即返回。但是，如果没有进程为读read而打开这个 FIFO，那么write只写 open 将返回 -1，并将 errno 设置成 ENXIO。
类似于管道，若 write 一个尚无进程为读而打开的 FIFO ，则产生 SIGPIPE 信号，若忽略此信号则进程终止；若某个 FIFO 的最后一个写进程关闭了该 FIFO，则将为该 FIFO 的读进程产生一个文件结束标志。
一个给定的 FIFO 有多个写进程是常见的。如果不希望多个进程所写的数据交叉，则必须考虑原子写操作：常数PIPE_BUF说明了可被原子写到FIFO的最大数据量，每次写入的长度要小于PIPE_BUF字节，这样就能避免多个进程各次写之间的穿插。
4.FIFO的用途
(1) shell 命令使用 FIFO 将数据从一条管道传送到另一条时，无需创建临时文件;
(2) 客户进程-服务器进程应用程序中，FIFO 用作汇聚点，在客户进程和服务器进程之间传递数据(如图)。 */
/* Create a new FIFO named PATH, with permission bits MODE.  */
int Mkfifo (const char *path, const mode_t &mode) // 创建 FIFO（命名管道）
{
	int ret = mkfifo(path, mode);
	if (ret < 0 && errno != EEXIST/*File exists*/ ) {
		err_sys(ret, __FILE__, __func__, __LINE__,
				"can't create %s", path);
	}
	return ret;
}

/******************************************************************************/

bool file_stat_isexit(const char *filename, const int8_t &mode) // 获取文件信息；判断文件是否存在
{
//	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)

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
#if 1 		/* 判断文件是否存在 */
	if ( ! value) // 如果文件不存在，stat() 就会返回 非０
	{
		if (st.st_size >= 0) // 加了一层验证保证
		{
			if (mode) {
				std::cout << "Exist file : " << filename << std::endl;
				file_stat_print(st); // 获取 文件属性信息
			}
			return true;
		}
	}
//	else if(ENOENT == errno) {
//		std::cerr << "NOT exist file : " << filename << std::endl;
//	}
#else
	if ( value == 0 ) // 如果文件不存在，stat() 就会返回 非０
	{
		FILE *fd = fopen(filename, "r");
		if ( fd != nullptr )
//		if ( fd )
		{
			if (mode) {
				std::cout << "Exist file : " << filename << std::endl;
				file_stat_print(st); // 获取 文件属性信息
			}
			return true;
		}
	}
#endif
	return false;
}

/******************************************************************************/


/******************************************************************************/


