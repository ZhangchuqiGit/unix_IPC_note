
#ifndef __IO_FILE_H
#define __IO_FILE_H

#include "zcq_header.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>  //for read
#include <sys/uio.h>
#include <fstream> // 文件流 <istream> <ostream>
#include <iostream> // 标准流 <istream> <ostream>
#include <sys/stat.h> //  POSIX Standard: File Characteristics

/**************************** Standard file descriptors.
	STDIN_FILENO	0	标准输入。
	STDOUT_FILENO	1	标准输出。
	STDERR_FILENO	2	标准错误输出。
	FILE *stdin;		标准输入流。
	FILE *stdout;	标准输出流。
	FILE *stderr;	标准错误输出流。
	int fp = fileno(stdin); 	转化文件描述符 标准输入
	int fp = fileno(stdout); 	转化文件描述符 标准输出
	int fp = fileno(stderr); 	转化文件描述符 标准错误输出        **/

/***************************** I/O File **********************************/

void file_creat_close(); // 创建文件, 文件创建或打开后，不使用时要关闭
void file_open_read(); // 读取 已存在文件
void file_open_write(); // 写 已存在文件
void file_location_write(); // 定位文件
void file_stat(); // 获取 文件信息
void file_lock(); // Linux 文件锁 避免共享的(文件)资源产生竞争状态
void file_map(); // mmap() 文件映射到内存, 像访问内存一样不必再调用 read 或 write 等操作

/**----------------------------------------------------------------**/

void file_stat_print(const struct stat &st); // 文件属性信息

//	char filename[] = "../file_test-01.txt"; // 文件名，相对路径(../)
void file_stat(std::string_view filename); // 获取 文件属性信息

/* 获取文件类型 */
int file_stat_if(const struct stat &st); // 获取文件类型

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int if_file_name(std::string_view filename); // 获取 文件属性信息

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int is_file_fp(FILE *fp); // 获取 文件属性信息

/* 判断文件是否存在 及 文件类型（文件、设备、管道等等） */
int if_file_fd(const int &fd); // 获取 文件属性信息



#endif //__IO_FILE_H
