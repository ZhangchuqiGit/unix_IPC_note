//
// Created by zcq on 2021/4/1.
//

#include "stdin_input.h"

/*************************************************************************/
int std_cin(std::string &buf_str)
{
	using namespace std;
	while (true) {
		buf_str.clear();
		cerr.flush();
		cout.flush();
		cin >> buf_str;
		if (!cin.good()) {
			cin.clear();  // 清除错误标志
			if (errno == EINTR) return 0; // Interrupted system call
			buf_str.clear();
			return -1;
		} else break;
	}
	return (int)buf_str.size();
}

/*************************************************************************/
ssize_t get_Line(FILE *fp, const void *vptr, const ssize_t &size_val)
{
	std::cerr.flush();
	std::cout.flush();
	char *ptr = (char *)vptr;
	memset(ptr, 0, size_val);
/*	ptr[num]; num = strlen( ptr ) = sizeof( ptr ) / sizeof( T ) */
#if 1
	char cc = '\n';
	std::cin.get(cc); // 从终端捕获 '\n'，丢弃
	if (cc == '\n' || cc == '\r') {
		std::cin.clear();
		std::cout.clear();
	}
	else {
		std::cin.putback(cc);
		std::cin.clear();
	}
#endif
	if ( fp == nullptr ) { 	/* 适合终端 */
		std::string cmdbuf;
//		std::getline(std::cin, cmdbuf);	// std::getline()从输入流中读取字符串
		if ( !std::getline(std::cin, cmdbuf).good() ) // 读取一行，不含'\n'
		{
			std::cin.clear(); // 清除错误标志
			return -1;
		}
	}
	else {
		/* 适合文件 */
		if ( fgets(ptr, MAXLINE, fp) == nullptr && ferror(fp) )// 读取一行，包含'\n'
			return -1;
	}
	return 0;
}

/*************************************************************************/
std::string name_host_serv(std::string_view name)
{
	using namespace std;
	string name_str;
	while (true) {
		std::cout << "connect to the " << name << " name: ";
		cin >> name_str;
		if (!cout.good()) cout.clear();  // 清除错误标志
		if (!cin.good()) {
			cin.clear();  // 清除错误标志
			name_str.clear();
		}else if (name_str == "\n" || name_str == " " ||
				  name_str == "\r" || name_str == "\t" ) {
			name_str.clear();
			continue;
		}else break;
	}
	return name_str;
}

/*************************************************************************/
int port_host_serv(std::string_view name)
{
	using namespace std;
	string portnum;
	while (true) {
		std::cout << "connect to the " << name << " name: ";
		cin >> portnum;
		if (!cout.good()) cout.clear();  // 清除错误标志
		if (!cin.good()) {
			cin.clear();  // 清除错误标志
			portnum.clear();
		}else if (portnum == "\n" || portnum == " " ||
				  portnum == "\r" || portnum == "\t" ) {
			portnum.clear();
			continue;
		}else break;
	}
	return std::stoi(portnum);
}

/*************************************************************************/
/* 	输入列表内容，如：111 222 333  zzz    gsfg
	获取每一个 std::vector<string> 子内容
---------------------------------------------------------------------------
 	std::string hints_str;
	hints_str += "usage: web <#paramax> <hostname> <pagename1> <pagename2> ...\n";
	hints_str += "usage: web      3    www.baidu.com    /       image1.gif ...\n";
	hints_str += "Now input please...\n";
	hints_str += "usage: web ";
	std::vector<std::string> vec = user_string(hints_str);
	for (const auto &i: vec) cout << i << endl;           */
std::vector<std::string> hints_strtovec(std::string_view hints_str)
{
	using namespace std;
	if (!hints_str.empty()) cout << hints_str << endl; // 自定义提示
	else cout << "hints_strtovec() -----------------" << endl;

	string cmdbuf;
#if 1
	char buf[MAXLINE] = {'\0'};
	cin.clear();
	cout.flush();
	int retval = (int)read(STDIN_FILENO, buf, sizeof(buf));
	err_sys(retval, __LINE__, "read()");
	if (retval == 1 && buf[0] == '\n') buf[0] = ' ';
	cmdbuf = buf;
#else
	cout.flush();
	char input = cin.get();
	if (input == '\n' || input == '\r') cin.clear();
	else cin.putback(input);
	/* std::getline()从输入流中读取字符串 */
	if (!std::getline(std::cin, cmdbuf).good()) {
		cin.clear();
		cout << endl;
		err_quit(-1, __LINE__, "std::getline().good");
	}
#endif
//	_debug_log_info("")
#if 0
	/* 	输入列表内容，如：111 222     333  zzz    gsfg
	获取每一个 std::vector<string> 子内容 */
	cout << "输入列表内容: " << cmdbuf << endl;
	vector <string> cmdarg = strtovec(cmdbuf);
	cout << "输入列表个数: " << cmdarg.size() << endl;
	uint num = 0;
	for (const auto &i: cmdarg) {
		++num;
		cout << "NO " << setw(2) << setfill('0') << num
			 << ": " << i.data() << endl;
	}
	return cmdarg;
#else
	return strtovec(cmdbuf);
#endif
}

/*************************************************************************/
/* 	输入列表内容，如：111 222     333  zzz    gsfg
	获取每一个 std::vector<string> 子内容 */
std::vector<std::string> strtovec(std::string_view hints_str)
{
	if (hints_str.empty()) err_quit(-1, __LINE__, "strtovec()");
	std::vector <std::string> vecstr;
//	vecstr.clear();
	long strsize = (long)hints_str.size();
	for (long i=0, j=0, pos; i < strsize; ++i, ++j) {
		pos = (long)hints_str.find(' ', i);
		if (pos < 0) pos = strsize;
		if (pos > i && hints_str.substr(i, pos-i) != "\n" )
			vecstr.push_back((std::string)hints_str.substr(i, pos-i));
		i = pos;
	}
#if 0
	for (const auto &i : vecstr) std::cout << i << "  ";
	std::cout << std::endl;
#endif
	return vecstr;
}

/*************************************************************************/


