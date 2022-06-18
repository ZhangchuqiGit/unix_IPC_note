//
// Created by zcq on 2021/5/11.
//

#ifndef __WRAP_STAT_H
#define __WRAP_STAT_H

#include <sys/stat.h>

#include "zcq_header.h"

int Mkfifo (const char *path, const mode_t &mode); // 创建 FIFO（命名管道）

bool file_stat_isexit(const char *filename, const int8_t &mode = 0);
// 获取文件信息；判断文件是否存在




#endif //__WRAP_STAT_H
