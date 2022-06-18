//
// Created by zcq on 2021/3/19.
//

#ifndef __IF_QUIT_OR_RUN_H
#define __IF_QUIT_OR_RUN_H

#include "zcq_header.h"

#include <iostream>


bool if_quit_echo(const void *vptr);

struct args {
	long	arg1;
	long	arg2;
};
struct result {
	long	sum;
};
int if_quit_struct(const void *vptr);

int sctp_quit( std::string_view cmd, std::string_view buf, const int &sockfd,
			   struct sockaddr *to_addr) ;

int if_quit_0(std::string_view cmd, std::string_view buf );

/*************************************************************************/

bool if_run( std::string_view cmd, std::string_view buf);


#endif //__IF_QUIT_OR_RUN_H
