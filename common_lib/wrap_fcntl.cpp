//
// Created by zcq on 2021/5/11.
//

#include "wrap_fcntl.h"

//int flags = Fcntl(fd, F_GETFL, 0); 		// 获取 I/O 状态
//Fcntl(fd, F_SETFL, flags | O_NONBLOCK); 	// 设置 I/O 非阻塞

int Fcntl(const int & fd, const int & cmd, const int & arg,
		  const int &line, const char *func)
{
	int	retval = fcntl(fd, cmd, arg);
	if (retval < 0) {
		err_sys(retval, __FILE__, __func__, __LINE__,
				"fcntl()\t调用地方: %s(): %d", func, line);
	}
	return retval;
}


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

/*  文件创建 等价形式
形式 1: int fd = open  (filename, O_CREAT|O_WRONLY|O_TRUNC, 0777); // 创建文件 <fcntl.h>
形式 2: int fd = creat (filename, 0555); // 创建文件 <fcntl.h>
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

参数 mode : 指定 文件访问权限, 仅当创建 "新" 文件时才使用 !!!
 */
int Open(const char *pathname, int oflag, ... /* 可直接调用 */ ) // 打开/创建 指定文件
{
	int fd;
	if (oflag & O_CREAT) {
		mode_t mode;
		va_list ap;
		va_start(ap, oflag); // <stdarg.h>
		mode = va_arg(ap, mode_t);
		fd = open(pathname, oflag, mode);
		va_end(ap); // <stdarg.h>
	}
	else {
		fd = open(pathname, oflag);
	}
	if (fd == -1) {
		err_sys(-1, __FILE__, __func__, __LINE__,
				"open(): %s", pathname);
	}
	return(fd);
}

