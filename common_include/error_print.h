//
// Created by zcq on 2021/3/16.
//

/******** 错误打印消息 **********/

#ifndef __ZCQ_ERROR_H
#define __ZCQ_ERROR_H

#include "zcq_header.h"

#include <string>
#include <netdb.h>
#include <syslog.h> // 守护进程
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg> // don't delete
#include <string_view>
#include <iostream>

#define MAXsize_error 4096    /* max text line length */

/*************************************************************************/
/*	ANSI C标准中有几个标准预定义宏（也是常用的）：
 __LINE__：在源代码中插入当前源代码行号；
 __FILE__：在源文件中插入当前源文件名；
 __func__、__FUNCTION__：在源文件中插入当前函数名；
 __DATE__：在源文件中插入当前的编译日期
 __TIME__：在源文件中插入当前编译时间；
 __STDC__：当要求程序严格遵循ANSI C标准时该标识被赋值为1；
 __cplusplus：当编写C++程序时该标识符被定义。
---------------------------------------------------------------------------
	在单一的宏定义中，只能使用 “#” 或 “##” 中的 一种 操作符。除非必要，否则尽量都不使用。
 	#: 	把参数转换成字符串
例子：		#define P(A) 	printf( "%s=%d\n", #A, A);
        	int a=1;	int b=2;	P(a+b);		结果：a+b=123
	##:	粘合剂，将前后两部分粘合起来。
例子：		#define name(n) 	zcq ## n
 			name(666) 	结果：zcq666
---------------------------------------------------------------------------
 __VA_ARGS__: 可变参数的宏
例子：			#define str(...) printf(__VA_ARGS__)
				str("0123456") 	结果：0123456
例子：			#define str(...) printf(__VA_ARGS__)
				str("%d", 123) 	结果：123
---------------------------------------------------------------------------
 ##__VA_ARGS__: 当可变参数的个数为0时，这里##起到把前面多余的","去掉的作用,否则编译出错
例子：			#define myprint(fmt,...) 	printf(fmt,##__VA_ARGS__)
				myprint("1+2") 			结果：1+2
				myprint("1+2","=3") 	结果：1+2=3       */

#ifndef Debug_line
#define Debug_line

// debug_line(__LINE__,__func__,"");
#define debug_line [](const int &line, const char *name , const char * str="") \
{ std::cout << ">—>—>— Line: " << line << " --- " << name << "(): " << str << std::endl;}

#if 0
// _debug_log_info(" ")
#define _debug_log_info(fmt, ...)  { \
printf(">—>—> %d  %s()  INFO: ", __LINE__, __func__); \
printf(fmt, ##__VA_ARGS__); printf("\n");   }
#elif 1
// _debug_log_info(" ")
#define _debug_log_info(fmt, ...)  { printf(">—>—> %d\tINFO: ", __LINE__); \
printf(fmt, ##__VA_ARGS__); printf("\n");   }
#else
// _debug_log_info(" ")
#define _debug_log_info(fmt, ...)  ;
#endif

#endif

/*************************************************************************/

/* 守护进程 daemon *//* set nonzero by daemon_init() */
extern volatile sig_atomic_t daemon_proc; // 可以原子修改的整数类型，而不会在操作过程中到达信号

/*************************** 可直接调用 ***********************************/

/** err_sys(-1, __LINE__, "%s()", __func__); */
/** err_sys(-1, __FILE__, __func__, __LINE__, "%s()", __func__); */
/** err_sys(-1, (char *)__FILE__, __func__, (int)__LINE__, "func():%s", "error"); */

//myperr(retval, "xxx", __LINE__); // 输出自定义错误信息
// 输出自定义错误信息
void myperr(const int &retval, std::string_view ptr, const int &code_line = 0);

//与系统调用 有关 的 非 致命错误打印消息 并 返回
void err_ret(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ );
//与系统调用 有关 的 非 致命错误打印消息 并 返回
void err_ret(const int &ret_value,
			 const char *code_file,	/*源文件名*/
			 const char *code_func,	/*函数名*/
			 const int &code_line, 	/*源代码行号*/
			 const char *fmt, ...	/* 可直接调用 */ );

//与系统调用 有关 的 致命 错误打印消息 并 终止
void err_sys(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ );
//与系统调用 有关 的 致命 错误打印消息 并 终止
void err_sys(const int &ret_value,
			 const char *code_file,	/*源文件名*/
			 const char *code_func,	/*函数名*/
			 const int &code_line,	/*源代码行号*/
			 const char *fmt, ...	/* 可直接调用 */ );

//系统调用打印消息，转储核心 和 终止 有关的致命错误
void err_dump(const int &ret_value, const int &code_line,
			  const char *fmt, ... /* 可直接调用 */ );
//系统调用打印消息，转储核心 和 终止 有关的致命错误
void err_dump(const int &ret_value,
			  const char *code_file,/*源文件名*/
			  const char *code_func,/*函数名*/
			  const int &code_line, /*源代码行号*/
			  const char *fmt, ...	/* 可直接调用 */ );

//与系统调用 无关 的 非 致命错误打印消息并返回
void err_msg(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ );
//与系统调用 无关 的 非 致命错误打印消息并返回
void err_msg(const int &ret_value,
			 const char *code_file,	/*源文件名*/
			 const char *code_func,	/*函数名*/
			 const int &code_line, 	/*源代码行号*/
			 const char *fmt, ...	/* 可直接调用 */ );

//与系统调用 无关 的 致命 错误打印消息并终止
void err_quit(const int &ret_value, const int &code_line,
			  const char *fmt, ... /* 可直接调用 */ );
//与系统调用 无关 的 致命 错误打印消息并终止
void err_quit(const int &ret_value,
			  const char *code_file,/*源文件名*/
			  const char *code_func,/*函数名*/
			  const int &code_line, /*源代码行号*/
			  const char *fmt, ...	/* 可直接调用 */ );

/* Error print for getaddrinfo() */
void err_addrinfo(const int &ret_value, const int &code_line,
				  const char *fmt, ... /* 可直接调用 */ );
/* Error print for getaddrinfo() */
void err_addrinfo(const int &ret_value,
				  const char *code_file,/*源文件名*/
				  const char *code_func,/*函数名*/
				  const int &code_line, /*源代码行号*/
				  const char *fmt, ...	/* 可直接调用 */ );

void err_h_errno(const int &ret_value, const int &h_errno_val,
				 const int &code_line);
void err_h_errno(const int &ret_value,
				 const int &h_errno_val,
				 const char *code_file,		/*源文件名*/
				 const char *code_func, 	/*函数名*/
				 const int &code_line = 0 	/*源代码行号*/ );

#endif //CP_ZCQ_ERROR_H



// errno 	错误对照表
/*	 		错误值			内容		*/

//#define	EPERM		 1	/* Operation not permitted */
//#define	ENOENT		 2	/* No such file or directory */
//#define	ESRCH		 3	/* No such process */
//#define	EINTR		 4	/* Interrupted system call */
//#define	EIO		 5	/* I/O error */
//#define	ENXIO		 6	/* No such device or address */
//#define	E2BIG		 7	/* Argument list too long */
//#define	ENOEXEC		 8	/* Exec format error */
//#define	EBADF		 9	/* Bad file number */
//#define	ECHILD		10	/* No child processes */
//#define	EAGAIN		11	/* Try again */
//#define	ENOMEM		12	/* Out of memory */
//#define	EACCES		13	/* Permission denied */
//#define	EFAULT		14	/* Bad address */
//#define	ENOTBLK		15	/* Block device required */
//#define	EBUSY		16	/* Device or resource busy */
//#define	EEXIST		17	/* File exists */
//#define	EXDEV		18	/* Cross-device link */
//#define	ENODEV		19	/* No such device */
//#define	ENOTDIR		20	/* Not a directory */
//#define	EISDIR		21	/* Is a directory */
//#define	EINVAL		22	/* Invalid argument */
//#define	ENFILE		23	/* File table overflow */
//#define	EMFILE		24	/* Too many open files */
//#define	ENOTTY		25	/* Not a typewriter */
//#define	ETXTBSY		26	/* Text file busy */
//#define	EFBIG		27	/* File too large */
//#define	ENOSPC		28	/* No space left on device */
//#define	ESPIPE		29	/* Illegal seek */
//#define	EROFS		30	/* Read-only file system */
//#define	EMLINK		31	/* Too many links */
//#define	EPIPE		32	/* Broken pipe */
//#define	EDOM		33	/* Math argument out of domain of func */
//#define	ERANGE		34	/* Math result not representable */

//#define	EDEADLK		35	/* Resource deadlock would occur */
//#define	ENAMETOOLONG	36	/* File name too long */
//#define	ENOLCK		37	/* No record locks available */

/* This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.  */
//#define	ENOSYS		38	/* Invalid system call number */

//#define	ENOTEMPTY	39	/* Directory not empty */
//#define	ELOOP		40	/* Too many symbolic links encountered */
//#define	EWOULDBLOCK	EAGAIN	/* Operation would block */
//#define	ENOMSG		42	/* No message of desired type */
//#define	EIDRM		43	/* Identifier removed */
//#define	ECHRNG		44	/* Channel number out of range */
//#define	EL2NSYNC	45	/* Level 2 not synchronized */
//#define	EL3HLT		46	/* Level 3 halted */
//#define	EL3RST		47	/* Level 3 reset */
//#define	ELNRNG		48	/* Link number out of range */
//#define	EUNATCH		49	/* Protocol driver not attached */
//#define	ENOCSI		50	/* No CSI structure available */
//#define	EL2HLT		51	/* Level 2 halted */
//#define	EBADE		52	/* Invalid exchange */
//#define	EBADR		53	/* Invalid request descriptor */
//#define	EXFULL		54	/* Exchange full */
//#define	ENOANO		55	/* No anode */
//#define	EBADRQC		56	/* Invalid request code */
//#define	EBADSLT		57	/* Invalid slot */

//#define	EDEADLOCK	EDEADLK //35	/* Resource deadlock would occur */

//#define	EBFONT		59	/* Bad font file format */
//#define	ENOSTR		60	/* Device not a stream */
//#define	ENODATA		61	/* No data available */
//#define	ETIME		62	/* Timer expired */
//#define	ENOSR		63	/* Out of streams resources */
//#define	ENONET		64	/* Machine is not on the network */
//#define	ENOPKG		65	/* Package not installed */
//#define	EREMOTE		66	/* Object is remote */
//#define	ENOLINK		67	/* Link has been severed */
//#define	EADV		68	/* Advertise error */
//#define	ESRMNT		69	/* Srmount error */
//#define	ECOMM		70	/* Communication error on send */
//#define	EPROTO		71	/* Protocol error */
//#define	EMULTIHOP	72	/* Multihop attempted */
//#define	EDOTDOT		73	/* RFS specific error */
//#define	EBADMSG		74	/* Not a data message */
//#define	EOVERFLOW	75	/* Value too large for defined data type */
//#define	ENOTUNIQ	76	/* Name not unique on network */
//#define	EBADFD		77	/* File descriptor in bad state */
//#define	EREMCHG		78	/* Remote address changed */
//#define	ELIBACC		79	/* Can not access a needed shared library */
//#define	ELIBBAD		80	/* Accessing a corrupted shared library */
//#define	ELIBSCN		81	/* .lib section in a.out corrupted */
//#define	ELIBMAX		82	/* Attempting to link in too many shared libraries */
//#define	ELIBEXEC	83	/* Cannot exec a shared library directly */
//#define	EILSEQ		84	/* Illegal byte sequence */
//#define	ERESTART	85	/* Interrupted system call should be restarted */
//#define	ESTRPIPE	86	/* Streams pipe error */
//#define	EUSERS		87	/* Too many users */
//#define	ENOTSOCK	88	/* Socket operation on non-socket */
//#define	EDESTADDRREQ	89	/* Destination address required */
//#define	EMSGSIZE	90	/* Message too long */
//#define	EPROTOTYPE	91	/* Protocol wrong type for socket */
//#define	ENOPROTOOPT	92	/* Protocol not available */
//#define	EPROTONOSUPPORT	93	/* Protocol not supported */
//#define	ESOCKTNOSUPPORT	94	/* Socket type not supported */
//#define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */
//#define	EPFNOSUPPORT	96	/* Protocol family not supported */
//#define	EAFNOSUPPORT	97	/* Address family not supported by protocol */
//#define	EADDRINUSE	98	/* Address already in use */
//#define	EADDRNOTAVAIL	99	/* Cannot assign requested address */
//#define	ENETDOWN	100	/* Network is down */
//#define	ENETUNREACH	101	/* Network is unreachable */
//#define	ENETRESET	102	/* Network dropped connection because of reset */
//#define	ECONNABORTED	103	/* Software caused connection abort */
//#define	ECONNRESET	104	/* Connection reset by peer */
//#define	ENOBUFS		105	/* No buffer space available */
//#define	EISCONN		106	/* Transport endpoint is already connected */
//#define	ENOTCONN	107	/* Transport endpoint is not connected */
//#define	ESHUTDOWN	108	/* Cannot send after transport endpoint shutdown */
//#define	ETOOMANYREFS	109	/* Too many references: cannot splice */
//#define	ETIMEDOUT	110	/* Connection timed out */
//#define	ECONNREFUSED	111	/* Connection refused */
//#define	EHOSTDOWN	112	/* Host is down */
//#define	EHOSTUNREACH	113	/* No route to host */
//#define	EALREADY	114	/* Operation already in progress */
//#define	EINPROGRESS	115	/* Operation now in progress */
//#define	ESTALE		116	/* Stale file handle */
//#define	EUCLEAN		117	/* Structure needs cleaning */
//#define	ENOTNAM		118	/* Not a XENIX named type file */
//#define	ENAVAIL		119	/* No XENIX semaphores available */
//#define	EISNAM		120	/* Is a named type file */
//#define	EREMOTEIO	121	/* Remote I/O error */
//#define	EDQUOT		122	/* Quota exceeded */

//#define	ENOMEDIUM	123	/* No medium found */
//#define	EMEDIUMTYPE	124	/* Wrong medium type */
//#define	ECANCELED	125	/* Operation Canceled */
//#define	ENOKEY		126	/* Required key not available */
//#define	EKEYEXPIRED	127	/* Key has expired */
//#define	EKEYREVOKED	128	/* Key has been revoked */
//#define	EKEYREJECTED	129	/* Key was rejected by service */

/* for robust mutexes */
//#define	EOWNERDEAD	130	/* Owner died */

//#define	ENOTRECOVERABLE	131	/* State not recoverable */

//#define ERFKILL		132	/* Operation not possible due to RF-kill */
//#define EHWPOISON	133	/* Memory page has hardware error */

