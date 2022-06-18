#include <iostream>
using namespace std;

#include <cstdio>

// environ[] 接收 envp[] 传递 <unistd.h>
#include <unistd.h> // environ[]

/*	bash shell 	------------------------
	$ g++ test.cpp -o test
  	$ ./test myfile.txt youfile.txt one.txt
	c++ 		------------------------
		int main(int argc, char **argv)
----------------------------------------
  	c++    		bash shell
  	argc	=	${#argv[@]} # 参数个数
  	argv[0]	=	myfile.txt 	# 参数 : ./当前文件名
  	argv[1]	=	myfile.txt 	# 参数 1
  	argv[2]	=	youfile.txt # 参数 2
  	argv[3]	=	one.txt 	# 参数 3	  	*/
int main(int argc, char **argv)
{
	cout << "数据 ：" << endl;
	for (int i=0; environ[i] != nullptr; ++i) {
		printf("environ[%d] :\t%s\n", i, environ[i]);
	}
	cout << "=========================" << endl
		 << "文件 参数 数量 argc = " << argc << endl;
	for (int i=0 ; i < argc ; ++i) {
		printf("argv[%d] :\t%s\n", i, argv[i]);
	}

	return 0;
}
