//
// Created by zcq on 2021/4/1.
//

#ifndef __STDIN_INPUT_H
#define __STDIN_INPUT_H


#include "zcq_header.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <string_view>
#include <arpa/inet.h> //for inet_addr
#include "unistd.h"  //for read

#include <iostream>
//using namespace std;

int std_cin(std::string &buf_str);

ssize_t get_Line(FILE *fp, const void *vptr, const ssize_t &size_val);

std::string name_host_serv(std::string_view name);

int port_host_serv(std::string_view name);

/* 	输入列表内容，如：111 222 333  zzz    gsfg
	获取每一个 std::vector<string> 子内容        */
std::vector<std::string> hints_strtovec(std::string_view hints_str);

/* 	输入列表内容，如：111 222     333  zzz    gsfg
	获取每一个 std::vector<string> 子内容 */
std::vector<std::string> strtovec(std::string_view hints_str);


#endif //__STDIN_INPUT_H
