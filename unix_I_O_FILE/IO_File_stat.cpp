
#include "IO_File.h"

#include <fstream> // 文件流 <istream> <ostream>
#include <iostream> // 标准流 <istream> <ostream>
#include <cstdio>
#include <cstring>
#include <unistd.h> // POSIX Standard: Symbolic Constants
#include <sys/types.h> // POSIX Standard: Primitive System Data Types

#include <sys/stat.h> //  POSIX Standard: File Characteristics
#include <fcntl.h> // POSIX Standard : File Control Operations

void file_stat_print(const struct stat &st) // 文件属性信息
{
	std::cout << "------------------" << std::endl
			  << "Device : "<< st.st_dev << std::endl
			  << "User ID : "<< st.st_uid << std::endl
			  << "Group ID : "<< st.st_gid << std::endl
			  << "Device number : "<< st.st_rdev << std::endl
			  << "Optimal block size for I/O : "<< st.st_blksize << std::endl
			  << "文件大小 : "<< st.st_size << " byte" << std::endl
			  << "Time of last access : " << st.st_mtime << std::endl
			  << "Time of last modification : " << st.st_mtime << std::endl
			  << "Time of last status change : " << st.st_ctime << std::endl
			  << "serial number : " << st.st_ino << std::endl
			  << "mode : " << st.st_mode << std::endl;

	std::cout << "----------------- POSIX 中定义了检查这些类型的宏定义" << std::endl
			  << " 判断是否为符号连接 \t" << S_ISLNK(st.st_mode) << std::endl
			  << " 是否为常规文件 \t" << S_ISREG(st.st_mode)  << std::endl
			  << " 是否为目录 \t" << S_ISDIR(st.st_mode) << std::endl
			  << " 是否为字符设备 \t" << S_ISCHR(st.st_mode) << std::endl
			  << " 是否为块设备 \t" << S_ISBLK(st.st_mode) << std::endl
			  << " 是否为先进先出 \t" << S_ISFIFO(st.st_mode) << std::endl
			  << " 是否为socket文件 \t" << S_ISSOCK(st.st_mode) << std::endl ;
}

//	char filename[] = "../file_test-01.txt"; // 文件名，可相对路径(../)
void file_stat(std::string_view filename) // 获取 文件属性信息
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

/*	stat() 通过 文件名字，获取（文件、设备、管道）对应的属性。文件 没有打开 的操作
 	int stat (const char * file_name, struct stat * buf); // 文件 没有打开 的操作
-----------------------------------
 	fstat() 通过 文件描述符 获取（文件、设备、管道）对应的属性。文件 打开后 的操作
 	int fstat (int fd, struct stat * buf); // 文件 打开后 的操作
-----------------------------------
	lstat() 通过 符号连接，获取 符号连接（文件、设备、管道）对应的属性。
 			但是 命名的文件 是一个符号连接时，返回该符号连接的有关信息，
 			而不是由 该符号连接 引用的 文件的信息 !!!
	int lstat (const char * file_name, struct stat * buf);
-----------------------------------
通过 文件名(file_name) 或 文件描述符(fd) 获取文件信息，并保存在 buf 所指的结构体 stat 中。
返回值: 执行成功则返回 0，失败返回 -1，错误代码存于 errno  	*/

//	char filename[] = "../file_test-01.txt"; // 文件名，可相对路径(../)
	int value = stat(filename.data(), &st); // 文件 没有打开 的操作
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
		FILE *fd = std::fopen(filename.data(), "r");
		if ( fd != nullptr ) // 加了一层验证保证
//		if ( fd )
		{
			std::cout << "Exist file : " << filename << std::endl;
			file_stat_print(st);
		}
	}
#endif
}


/* 创建和删除文件目录项 */
void file_touch_rm() // 创建和删除文件目录项
{
	const char filename[] = "../file_test-01.txt"; // 文件名，可相对路径(../)
	const char filename02[] = "../file_test-02.txt"; // 文件名，可相对路径(../)
	int fd = -1; // 文件描述符 ; 错误(-1)

	struct stat st{}; // 存放文件属性信息

	/*	void *memset(void *str, int c, size_t n)
	填充 n 个字符 c 到参数 str 所指向的内存块。返回值是一个指向存储区 str 的指针。	 */
	memset(&st, 0, sizeof(st)); // 清零

	fd = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0777); // 创建文件 最高权限 <fcntl.h>
	err_sys(fd, __LINE__, "open():%s", filename);

	fstat(fd, &st); // 文件 打开后 的操作
	std::cout << "filename 创建文件 open\tlink = " << st.st_nlink << std::endl;//测试链接数

	fstat(fd, &st); // 文件 打开后 的操作
	std::cout << "filename\t\t\t\tlink = " << st.st_nlink << std::endl;//测试链接数

	close(fd);//3.关闭文件
	stat(filename, &st); // 文件 没有打开 的操作
	std::cout << "filename 关闭文件 close\tlink = " << st.st_nlink << std::endl;//测试链接数

	link(filename, filename02);//4.创建硬链接
	stat(filename, &st); // 文件 没有打开 的操作
	std::cout << "filename02 创建硬链接 link\tlink = " << st.st_nlink << std::endl;//测试链接数

	unlink(filename02);//5.删除 文件
	stat(filename, &st); // 文件 没有打开 的操作
	std::cout << "filename02 删除 unlink\tlink = " << st.st_nlink << std::endl;//测试链接数

	//6.重复步骤2
	fd = open(filename, O_RDONLY);//打开已存在文件
	fstat(fd, &st); // 文件 打开后 的操作
	std::cout << "filename 打开已存在文件 open\tlink = " << st.st_nlink << std::endl;//测试链接数

	unlink(filename);//7.删除
	fstat(fd, &st); // 文件 打开后 的操作
	std::cout << "filename 删除 unlink\tlink = " << st.st_nlink << std::endl;//测试链接数

	close(fd);//8.此步骤可以不显示写出，因为进程结束时，打开的文件自动被关闭。

}

/*************************************************************************/
/*************************************************************************/
/*	stat() 通过 文件名字，获取（文件、设备、管道）对应的属性。文件 没有打开 的操作
 	int stat (const char * file_name, struct stat * buf); // 文件 没有打开 的操作
-----------------------------------
 	fstat() 通过 文件描述符 获取（文件、设备、管道）对应的属性。文件 打开后 的操作
 	int fstat (int fd, struct stat * buf); // 文件 打开后 的操作
-----------------------------------
	lstat() 通过 符号连接，获取 符号连接（文件、设备、管道）对应的属性。
 			但是 命名的文件 是一个符号连接时，返回该符号连接的有关信息，
 			而不是由 该符号连接 引用的 文件的信息 !!!
	int lstat (const char * file_name, struct stat * buf);
-----------------------------------
通过 文件名(file_name) 或 文件描述符(fd) 获取文件信息，并保存在 buf 所指的结构体 stat 中。
返回值: 执行成功则返回 0，失败返回 -1，错误代码存于 errno  	*/

/* 获取文件类型 */
int file_stat_if(const struct stat &st) // 获取文件类型
{
	switch (st.st_mode & S_IFMT)
	{
		case S_IFDIR: 	return 1;// 是否是一个 目录
		case S_IFCHR: 	return 2;// 是否是一个 字符 设备
		case S_IFBLK: 	return 3;// 是否是一个 块 设备
		case S_IFREG: 	return 4;// 是否是一个 一般 文件
		case S_IFIFO: 	return 5;// 是否是一个 先进先出 文件
		case S_IFLNK: 	return 6;// 是否是一个 链接 文件
		case S_IFSOCK: 	return 7;// 是否是一个 socket 文件
		default:
			break;
	}
	return 0;
}

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int if_file_name(std::string_view filename) // 获取 文件属性信息
{
//	char filename[] = "../file_test-01.txt"; // 文件名，可相对路径(../)
	struct stat st{}; // 存放文件属性信息
	bzero(&st, sizeof(st)); // 清 0
//	memset(&st, 0, sizeof(st)); // 清 0

	int value = stat(filename.data(), &st); // 文件 没有打开 的操作
#if 1
	/** 判断文件是否存在 **/
	if ( ! value) // 如果文件不存在，stat() 就会返回 非０
	{
		if (st.st_size >= 0) // 加了一层验证保证
		{
			return file_stat_if(st); // 获取文件类型
		}
	}
	else if(ENOENT == errno) { /* No such file or directory */
		return -1;
	}
#else
	/** 判断文件是否存在 **/
	if ( value == 0 ) // 如果文件不存在，stat() 就会返回 非０
	{
		FILE *fd = std::fopen(filename.data(), "r");
		if ( fd != nullptr ) // 加了一层验证保证
//		if ( fd )
		{
			return file_stat_if(st); // 获取文件类型
		}
		else if(ENOENT == errno) { /* No such file or directory */
			return -1;
		}
	}
#endif
	return -2;
}

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int is_file_fp(FILE *fp) // 获取 文件属性信息
{
	return if_file_fd(fileno(fp)); // 获取 文件属性信息
}

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int if_file_fd(const int &fd) // 获取 文件属性信息
{
//	char filename[] = "../file_test-01.txt"; // 文件名，可相对路径(../)
	struct stat st{}; // 存放文件属性信息
	bzero(&st, sizeof(st)); // 清 0
//	memset(&st, 0, sizeof(st)); // 清 0

	int value = fstat(fd, &st); // 文件 打开后 的操作
#if 1
	/** 判断文件是否存在 **/
	if ( ! value) // 如果文件不存在，stat() 就会返回 非０
	{
		if (st.st_size >= 0) // 加了一层验证保证
		{
			return file_stat_if(st); // 获取文件类型
		}
	}
	else if(ENOENT == errno) { /* No such file or directory */
		return -1;
	}
#else
	/** 判断文件是否存在 **/
	if ( value == 0 ) // 如果文件不存在，stat() 就会返回 非０
	{
		FILE *fd = std::fopen(filename.data(), "r");
		if ( fd != nullptr ) // 加了一层验证保证
//		if ( fd )
		{
			return file_stat_if(st); // 获取文件类型
		}
		else if(ENOENT == errno) { /* No such file or directory */
			return -1;
		}
	}
#endif
	return -2;
}

