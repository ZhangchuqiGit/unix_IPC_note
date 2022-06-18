//
// Created by zcq on 2021/5/12.
//

#include "wrap_stdio.h"


FILE *Fopen(const char *pathname, const char *mode)
{
	FILE *fp = fopen(pathname, mode);
	if (fp == nullptr)
		err_sys(-1, __FILE__, __func__, __LINE__, "fopen()");
	return(fp);
}

FILE *Fdopen(const int &fd, const char *type)
{
	FILE *fp = fdopen(fd, type);
	if (fp == nullptr)
		err_sys(-1, __FILE__, __func__, __LINE__, "fdopen()");
	return(fp);
}

void Fclose(FILE *fp)
{
	if (fclose(fp) != 0)
		err_sys(-1, __FILE__, __func__, __LINE__, "fclose()");
}

/* fgets()负责从 stream（通常是文件或者标准输入stdin）中读取 num 个字符到str中，
 * 在读取数据时会将最后的回车(\n)同时读入到buf中，并且会在后面加上字符串结束符(\0)。
 * 成功返回非空指针，失败返回空指针 */
char *Fgets(char *ptr, const int &numc, FILE *stream) // 非线程安全
{
	char *rptr = fgets(ptr, numc, stream);
	if (rptr == nullptr && ferror(stream))
		err_sys(-1, __FILE__, __func__, __LINE__, "fgets()");
#if 1
	/* 去除 换行符 */
	if (*(ptr+numc-1) == '\n') *(ptr+numc-1) = '\0';
#endif
	return (rptr);
}

void Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		err_sys(-1, __FILE__, __func__, __LINE__, "fputs()");
}

