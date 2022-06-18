//
// Created by zcq on 2021/5/12.
//

#ifndef __WRAP_STDIO_H
#define __WRAP_STDIO_H

#include <cstdio>
#include <cstdlib>

#include "zcq_header.h"


FILE *Fopen(const char *pathname, const char *mode);

FILE *Fdopen(const int &fd, const char *type);

void Fclose(FILE *fp);

/* fgets()负责从 stream（通常是文件或者标准输入stdin）中读取 num 个字符到str中，
 * 在读取数据时会将最后的回车(\n)同时读入到buf中，并且会在后面加上字符串结束符(\0)。
 * 成功返回非空指针，失败返回空指针 */
char *Fgets(char *ptr, const int &numc, FILE *stream);

void Fputs(const char *ptr, FILE *stream);

#endif //__WRAP_STDIO_H
