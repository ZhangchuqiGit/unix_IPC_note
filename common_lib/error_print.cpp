//#pragma once

/******** 错误打印消息 **********/

#include "error_print.h" // 错误打印消息

/*	ANSI C标准中有几个标准预定义宏（也是常用的）：
__LINE__：在源代码中插入当前源代码行号；
__FILE__：在源文件中插入当前源文件名；
__FUNC__：在源文件中插入当前函数名；
__DATE__：在源文件中插入当前的编译日期
__TIME__：在源文件中插入当前编译时间；
__STDC__：当要求程序严格遵循ANSI C标准时该标识被赋值为1；
__cplusplus：当编写C++程序时该标识符被定义。                      */

//myperr(retval, "pthread_getattr_np", __LINE__);// 输出自定义错误信息
void myperr(const int &retval, std::string_view ptr, const int &code_line)
//输出自定义错误信息
{
//	code_line--;
	if (retval != 0) {
		std::cerr << "Error:";
		if (!ptr.empty()) {
			std::cerr << "theme[ " << ptr.data() << " ];";
		}
		if (code_line != 0) {
			std::cerr << "line[" << code_line << "];";
		}
		std::cerr << "value[" << retval << "];"
				  << "errno[" << errno << "]" << std::endl;

		if (errno != 0) {
			std::cerr << "strerror(errno): "
					  << std::strerror(errno)
					  << std::endl; //返回描述 errno 代码含义的字符串
			std::cerr << "gai_strerror(errno): "
					  << gai_strerror(errno);
			std::cerr << "hstrerror(errno): " << hstrerror(errno);
		}
	}
}
// myperr(-1, "pthread_getattr_np", __LINE__);// 输出自定义错误信息
//void myperr (const int &&retval, std::string_view ptr, const int &&code_line)
//{
//	myperr (retval, ptr, code_line);
//}

/* 守护进程 daemon *//* set nonzero by daemon_init() */
volatile sig_atomic_t daemon_proc = 0; // 可以原子修改的整数类型，而不会在操作过程中到达信号

/***** 不建议直接调用，通过 function(const char *str, ...) ******/
static void err_doit(const int &ret_value,
					 const char *code_file,		/*源文件名*/
					 const char *code_func, 	/*函数名*/
					 const int &code_line, 		/*源代码行号*/
					 const int &system_log_level,
					 const char *fmt, va_list ap ) /* 不建议直接调用 */
{
	if (ret_value < 0) {
		char buf[MAXsize_error + 1]; // 1 个 '\n'

		/* 将 *fmt 和 ... 内容 复制到 buf */
		vsnprintf(buf, MAXsize_error/* 要复制的大小 */, fmt, ap);    /* safe */

		size_t buf_len; // strlen(buf) 实际长度
//	size_t retval = std::size(buf)-1; // 最大大小 = MAXLINE + 1
		strcat(buf, "\n"); // 结尾附加 '\n'

		if (errno != 0) {
			buf_len = strlen(buf); // 实际长度
			// strerror(errno) 返回描述 errno 代码含义的字符串
			snprintf(buf + buf_len, MAXsize_error - buf_len,
					 "\t{ strerror( errno: %d ): %s }", errno,
					 strerror(errno));
			strcat(buf, "\n"); // 结尾附加 '\n'
		}
		if (h_errno != 0) {
			/* 非可重入查找功能的错误状态。特定于线程的 h_errno 变量 */
			buf_len = strlen(buf); // 实际长度
			snprintf(buf + buf_len, MAXsize_error - buf_len,
					 "\t{ <netdb.h>; hstrerror( h_errno: %d ): %s }",
					 h_errno, hstrerror(h_errno)/* <netdb.h> */ );
			strcat(buf, "\n"); // 结尾附加 '\n'
		}
		if (ret_value < -1) {
			/* Error values for `getaddrinfo' function.  */
			buf_len = strlen(buf); // 实际长度
			snprintf(buf + buf_len, MAXsize_error - buf_len,
					 "\t{ <netdb.h>; gai_strerror( ret_value: %d ): %s }",
					 ret_value, gai_strerror(ret_value)/* <netdb.h> */ );
			strcat(buf, "\n"); // 结尾附加 '\n'
		}

		std::string strbuf;
		if (code_file != nullptr) {
			strbuf += "File path: ";
			strbuf += code_file;
			strbuf += "\n";
		}
		strbuf += "[ ";
		strbuf += time_HMS();
		strbuf += " ]  ";
		if (code_func != nullptr) {
			strbuf += code_func;
			strbuf += "()  ";
		}

		buf_len = strlen(buf); // 实际长度
		if (buf[buf_len-1] == '\n') buf[buf_len-1] = '\0';
		/** 守护进程 **/
		if (daemon_proc) {		/* 生成日志消息 */
			syslog(LOG_LOCAL1 | system_log_level,
				   "%s%d;\treturn value: %d\nINFO: %s\n",
				   strbuf.c_str(), code_line, ret_value, buf);
		}
		else {
#if 1 /* c++ */
			std::cout.flush();
			std::cerr << strbuf.c_str()
					  << "Line: "  << code_line
					  << " ;  return value: " << ret_value  << "\n"
					  << "INFO: " << buf << std::endl;
			std::cerr.flush();
#else /* c */
			fflush(stdout);		/* in case stdout and stderr are the same */
			fputs(buf, stderr);
			fflush(stderr);
#endif
		}
		strbuf.clear();
	}
}

void err_doit(const int &ret_value, const int &code_line,
			  const int &system_log_level ,
			  const char *fmt, va_list ap ) /* 不建议直接调用 */
{
	err_doit(ret_value, nullptr, nullptr, code_line,
			 system_log_level, fmt, ap );
}

/********************* 可直接调用 ****************************/

//与系统调用 有关 的 非 致命错误打印消息 并 返回
void err_ret(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
}

//与系统调用 有关 的 非 致命错误打印消息 并 返回
void err_ret(const int &ret_value,
			 const char *code_file,		/*源文件名*/
			 const char *code_func, 	/*函数名*/
			 const int &code_line, 		/*源代码行号*/
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
}

//与系统调用 有关 的 致命 错误打印消息 并 终止
void err_sys(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_line, LOG_ERR, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) {
		if (!daemon_proc) std::cerr << __func__ << "():exit" << std::endl;
		exit(ret_value);
	}
}

//与系统调用 有关 的 致命 错误打印消息 并 终止
void err_sys(const int &ret_value,
			 const char *code_file,		/*源文件名*/
			 const char *code_func, 	/*函数名*/
			 const int &code_line, 		/*源代码行号*/
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) exit(ret_value);
}

//系统调用打印消息，转储核心 和 终止 有关的致命错误
void err_dump(const int &ret_value, const int &code_line,
			  const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_line, LOG_ERR, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) {
		if (!daemon_proc) std::cerr << __func__ << "():abort" << std::endl;
		abort();        /* dump core and terminate */
		exit(ret_value);/* shouldn't get here */
	}
}

//系统调用打印消息，转储核心 和 终止 有关的致命错误
void err_dump(const int &ret_value,
			  const char *code_file,		/*源文件名*/
			  const char *code_func, 		/*函数名*/
			  const int &code_line, 		/*源代码行号*/
			  const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) {
		abort();        /* dump core and terminate */
		exit(ret_value);/* shouldn't get here */
	}
}

//与系统调用 无关 的 非 致命错误打印消息并返回
void err_msg(const int &ret_value, const int &code_line,
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
}

//与系统调用 无关 的 非 致命错误打印消息并返回
void err_msg(const int &ret_value,
			 const char *code_file,		/*源文件名*/
			 const char *code_func, 	/*函数名*/
			 const int &code_line, 		/*源代码行号*/
			 const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
}


//与系统调用 无关 的 致命 错误打印消息并终止
void err_quit(const int &ret_value, const int &code_line,
			  const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_line, LOG_ERR, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) {
		if (!daemon_proc) std::cerr << __func__ << "():exit" << std::endl;
		exit(ret_value);
	}
}

//与系统调用 无关 的 致命 错误打印消息并终止
void err_quit(const int &ret_value,
			  const char *code_file,		/*源文件名*/
			  const char *code_func, 		/*函数名*/
			  const int &code_line, 		/*源代码行号*/
			  const char *fmt, ... /* 可直接调用 */ ) {
	va_list ap;
	va_start(ap, fmt); // <stdarg.h>
	err_doit(ret_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
	va_end(ap); // <stdarg.h>
	if (ret_value < 0) exit(ret_value);
}

/* Error print for getaddrinfo() */
void err_addrinfo(const int &ret_value, const int &code_line,
				  const char *fmt, ... /* 可直接调用 */ ) {
	if (ret_value != 0) {
		int err_value;
		if (ret_value > 0) err_value = -ret_value;
		else err_value = ret_value;

		va_list ap;
		va_start(ap, fmt); // <stdarg.h>
		err_doit(err_value, code_line, LOG_ERR, fmt, ap);
		va_end(ap); // <stdarg.h>

		if (!daemon_proc) std::cerr << __func__ << "():exit" << std::endl;
		exit(ret_value);
	}
}

/* Error print for getaddrinfo() */
void err_addrinfo(const int &ret_value,
				  const char *code_file,		/*源文件名*/
				  const char *code_func, 		/*函数名*/
				  const int &code_line, 		/*源代码行号*/
				  const char *fmt, ... /* 可直接调用 */ ) {
	if (ret_value != 0) {
		int err_value;
		if (ret_value > 0) err_value = -ret_value;
		else err_value = ret_value;

		va_list ap;
		va_start(ap, fmt); // <stdarg.h>
		err_doit(err_value, code_file, code_func, code_line, LOG_INFO, fmt, ap);
		va_end(ap); // <stdarg.h>

		exit(ret_value);
	}
}

void err_h_errno(const int &ret_value, const int &h_errno_val,
				 const int &code_line)
{
	if (ret_value != 0) {
		std::cerr << "Line: " << code_line
				  << " ; h_errno: " << h_errno_val
				  << " ; gai_strerror(h_errno): "
				  << gai_strerror(h_errno_val)/* <netdb.h> */ << std::endl;
		exit(ret_value);
	}
}

void err_h_errno(const int &ret_value,
				 const int &h_errno_val,
				 const char *code_file,		/*源文件名*/
				 const char *code_func, 	/*函数名*/
				 const int &code_line 		/*源代码行号*/ ) {
	if (ret_value != 0) {
		std::string strbuf;
		if (code_file != nullptr) {
			strbuf += "File: ";
			strbuf += code_file;
			strbuf += ";\t";
		}
		if (code_func != nullptr) {
			strbuf += "Func: ";
			strbuf += code_func;
			strbuf += ";\t";
		}
		strbuf += "";

		std::cerr << strbuf.c_str() << "Line: " << code_line
				  << ";\th_errno: " << h_errno_val
				  << ";\ngai_strerror(h_errno): "
				  << gai_strerror(h_errno_val)/* <netdb.h> */ << std::endl;
		exit(ret_value);
	}
}


#if 0
int main()
{
	myperr(-1, "pthread_getattr_np", __LINE__ );// 输出自定义错误信息
	err_msg(-1, __LINE__,"d%ddddd%sddddd%s",123,"---","+-*/");
	err_doit(-1, __LINE__, 0, "fmt%s", "00000");

	return 0;
}
#endif

