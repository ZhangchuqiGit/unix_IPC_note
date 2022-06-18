//
// Created by zcq on 2021/4/11.
//

#ifndef __GET_TIME_DEBUG_H
#define __GET_TIME_DEBUG_H

#include "zcq_header.h"

/* 获取时间格式：15:35:28.257981 */
std::string time_HMS();

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
 struct tm *localtime_r (const time_t *timer, struct tm *tp); 	 */

/* 获取时间格式：2021-3-11 星期日 16:37:46 */
std::string time_local_r(); // 线程安全

/* 获取时间格式：2021-3-11 星期日 16:37:46 */
std::string time_local(); // 不是线程安全

/* clock(): 返回从程序启动到函数调用的CPU占用时间的微秒数 */
void time_clock(const uint &millisecond); // 间隔 ?微秒 触发

/* time(): 返回从公元1970年1月1号0时0分0秒的UTC时间算起到现在所经过的秒数 */
void time_time(const uint &second); // 间隔 ?秒 触发


#endif //__GET_TIME_DEBUG_H
