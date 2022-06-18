//
// Created by zcq on 2021/4/11.
//

#include "get_time_debug.h"

/* 获取时间格式：15:35:28.257981 */
std::string time_HMS()
{
	char buf[16];
	struct timeval tv_s{};
	err_sys(gettimeofday(&tv_s, nullptr),
			__FILE__, __func__, __LINE__, "gettimeofday()");
	time_t time_t_val = tv_s.tv_sec;
	char *ptr = ctime(&time_t_val);
	/* ctime(): Fri Sep 13 00:00:00 1986\n\0 */
	/*          012345678901234567890123 4 5 */
	strcpy(buf, &ptr[11]); // buf[]: 00:00:00 1986\n\0
	/*        00:00:00.123456\0          */
	/*        012345678901234 5          */
	snprintf(buf + 8, sizeof(buf) - 8, ".%06ld", tv_s.tv_usec);
	return std::string(buf); /* 15:35:28.257981 */
}

/*************************************************************************/

/*	#include <time.h>   时间间隔/时间差 计算函数 clock() 与 time () 用法分析
	在很多情况下，为了评判某个算法，函数的优劣， 比较同一个功能的两个函数的效率高低时，
 我们经常通过计算耗费时间作为判断标准。C语言中有两个相关的函数用来计算时间差，分别是：
 time_t time (time_t *timer); 	// 返回的时间单位为 秒
 	参数: 若指针 timer 非空，则保存返回的时间。
 clock_t clock (void);			// 返回的时间单位为 微秒
返回值分析：
 time():  返回从公元1970年1月1号0时0分0秒的UTC时间算起到现在所经过的秒数。
 clock(): 返回从程序启动到函数调用的CPU占用时间的微秒数。     */

/* clock(): 返回从程序启动到函数调用的CPU占用时间的微秒数 */
void time_clock(const uint &millisecond) // 间隔 ?微秒 触发
{
	static clock_t start = 0;
	clock_t tmp = clock();
	if ((tmp - start) >= millisecond) {
		std::cout << "****** 间隔 " << millisecond << " 微秒 触发" << std::endl;
		start = tmp;
	}
}

/* time(): 返回从公元1970年1月1号0时0分0秒的UTC时间算起到现在所经过的秒数 */
void time_time(const uint &second) // 间隔 ?秒 触发
{
	static time_t start = 0;
	time_t tmp = time(nullptr);
	if ( (tmp - start) >= second) {
		std::cout << "****** 间隔 " << second << " 秒 触发" << std::endl;
		start = tmp;
	}
}

/*************************************************************************/

/* 星期几 */
std::string weekday(const int &weday) // 星期几
{
	std::string weekday;
	switch (weday) {
		case 0: weekday = "星期日"; break;
		case 1: weekday = "星期一"; break;
		case 2: weekday = "星期二"; break;
		case 3: weekday = "星期三"; break;
		case 4: weekday = "星期四"; break;
		case 5: weekday = "星期五"; break;
		case 6: weekday = "星期六"; break;
		default:
			break;
	}
	return weekday;
}

/* 	#include <time.h>
 localtime()   获取系统时间，不是线程安全，精度为秒。
 localtime_r() 获取系统时间，是线程安全，运行于 linux 平台下！
 localtime_s() 获取系统时间，是线程安全，运行于 windows 平台下！
---------------------------------------------------------------------------
 localtime() 不是线程安全：在使用时，只需定义一个指针，并不需要为指针申请空间，
 而指针必须要指向内存空间才可以使用，其实申请空间的动作由函数自己完成，
 这样在多线程的情况下，如果有另一个线程调用了这个函数，那么指针指向的 tm{} 结构体的数据就会改变。
 在 localtime_s() 与 localtime_r() 调用时，定义的是 tm{} 的结构体，
 获取到的时间已经保存在 本地线程的 tm{} 中，并不会受其他线程的影响。
---------------------------------------------------------------------------
 struct tm *localtime (const time_t *timer)
 struct tm *localtime_r (const time_t *timer, struct tm *tp);
 	ISO C 破碎时间结构
	struct tm
	{
		int tm_sec;		秒		[0-60]
		int tm_min;		分钟	[0-59]
		int tm_hour;	小时	[0-23]
		int tm_mday;	日		[1-31]
		int tm_mon;		月		[0-11] 从 0 算起，要 +1
		int tm_year;	年		从 1900年 算起
		int tm_wday;	星期几	[0-6]
		int tm_yday;	1年的天数[0-365]
		int tm_isdst;	DST [-101]
		long int 	tm_gmtoff;	UTC 以东的秒数。
		const char 	*tm_zone;	时区的缩写。
	};        */

/* 获取时间格式：2021-3-11 星期日 16:37:46 */
std::string time_local_r() // 线程安全
{
	std::string timestr;
	struct tm tm_s{};
	time_t second = time(nullptr); // time()返回单位为 秒
	if (localtime_r(&second, &tm_s) == nullptr)
		err_sys(-1, __FILE__, __func__, __LINE__,
				"localtime_r()");
	timestr += std::to_string(tm_s.tm_year + 1900);
	timestr += "-";
	timestr += std::to_string(tm_s.tm_mon + 1);
	timestr += "-";
	timestr += std::to_string(tm_s.tm_mday);
	timestr += " ";
	timestr += weekday(tm_s.tm_wday); // 星期几
	timestr += " ";
	timestr += std::to_string(tm_s.tm_hour);
	timestr += ":";
	timestr += std::to_string(tm_s.tm_min);
	timestr += ":";
	timestr += std::to_string(tm_s.tm_sec);
#if 0
	printf("%d-%d-%d %d:%d:%d\n",
		   tm_s.tm_year + 1900, tm_s.tm_mon + 1, tm_s.tm_mday,
		   tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec);
#endif
	return timestr; /* 2021-3-11 星期日 16:37:46 */
}

/* 获取时间格式：2021-3-11 星期日 16:37:46 */
std::string time_local() // 不是线程安全
{
	std::string timestr;
	time_t second = time(nullptr); // time()返回单位为 秒
	struct tm *tm_s = localtime(&second);
	if (tm_s == nullptr)
		err_sys(-1, __FILE__, __func__, __LINE__,
				"localtime()");
	timestr += std::to_string(tm_s->tm_year + 1900);
	timestr += "-";
	timestr += std::to_string(tm_s->tm_mon + 1);
	timestr += "-";
	timestr += std::to_string(tm_s->tm_mday);
	timestr += " ";
	timestr += weekday(tm_s->tm_wday); // 星期几
	timestr += " ";
	timestr += std::to_string(tm_s->tm_hour);
	timestr += ":";
	timestr += std::to_string(tm_s->tm_min);
	timestr += ":";
	timestr += std::to_string(tm_s->tm_sec);
#if 0
	printf("%d-%d-%d %d:%d:%d\n",
		   tm_s.tm_year + 1900, tm_s.tm_mon + 1, tm_s.tm_mday,
		   tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec);
#endif
	return timestr; /* 2021-3-11 星期日 16:37:46 */
}

/*************************************************************************/

