

/** I/O control 控制操作   #include <unistd.h> or #include <sys/ioctl.h>
 	网络程序(一般是服务器程序)中 ioctl()常用于在程序启动时获得主机上所有接口的信息：
 接口的地址、接口是否支持广播、是否支持多播 等等。
---------------------------------------------------------------------------
 int ioctl (int sock_fd, unsigned long int request, ...); // I/O 控制操作
 返回：成功返回 0，出错返回 -1
---------------------------------------------------------------------------
 	下表列出了 ioctl() 参数 request 以及 地址 ... 必须指向的数据类型：
 类别		Request				说明						数据类型
- - - -
 套接字		SIOCATMARK			是否位于带外标记				int
			SIOCSPGRP			设置 套接字的进程ID或进程组ID	int
			SIOCGPGRP			获取 套接字的进程ID或进程组ID	int
- - - -
 文件		FIONBIN				设置/清除 非阻塞I/O标志		int
			FIOASYNC			设置/清除 信号驱动异步I/O标志	int
			FIONREAD			获取 接收缓存区中的字节数		int
			FIOSETOWN			设置 文件的进程ID或进程组ID	int
			FIOGETOWN			获取 文件的进程ID或进程组ID	int
- - - -
(网络)接口	SIOCGIFNAME			获取 接口名字(网口名)
			SIOCSIFNAME			设置 接口名称(网口名)
			SIOCSIFLINK			设置 接口频道
			SIOCGIFCONF			获取 接口清单(含网口名)		struct ifconf
			SIOCDIFADDR			删除 接口地址(IP)			struct ifreq
			SIOCSIFADDR			设置 接口地址(IP)			struct ifreq
			SIOCGIFADDR			获取 接口地址(IP)			struct ifreq
			SIOCSIFFLAGS		设置 接口状态标志				struct ifreq
			SIOCGIFFLAGS		获取 接口状态标志				struct ifreq
			SIOCSIFDSTADDR		设置 远程地址(点到点)			struct ifreq
			SIOCGIFDSTADDR		获取 远程地址(点到点)			struct ifreq
			SIOCGIFBRDADDR		获取 广播地址(broadcast)		struct ifreq
			SIOCSIFBRDADDR		设置 广播地址(broadcast)		struct ifreq
			SIOCGIFNETMASK		获取 子网掩码(network mask)	struct ifreq
			SIOCSIFNETMASK		设置 子网掩码(network mask)	struct ifreq
			SIOCGIFMETRIC		获取 接口测度				struct ifreq
			SIOCSIFMETRIC		设置 接口测度				struct ifreq
			SIOCGIFMTU			获取 接口 MTU 最大传输单元	struct ifreq
			SIOCSIFMTU			设置 接口 MTU 最大传输单元	struct ifreq
			SIOCGIFMEM			获取 内存地址（BSD）
			SIOCSIFMEM			设置 内存地址（BSD）
			SIOCSIFHWADDR		设置 以太网/主机硬件地址(MAC机器码)
			SIOCGIFHWADDR		获取 以太网/主机硬件地址(MAC机器码)
			SIOCADDMULTI		增加 组播地址列表
			SIOCDELMULTI		删除 组播地址列表
			SIOCSIFPFLAGS		设置 扩展标志集
			SIOCGIFPFLAGS		获取 扩展标志集
			SIOCSIFHWBROADCAST	设置 硬件广播地址
			SIOCGIFCOUNT		获取 设备数量
			SIOCGIFBR			获取 桥接支持
			SIOCSIFBR			设置 桥接选项
			SIOCGIFTXQLEN		获取 发送队列长度
			SIOCSIFTXQLEN		设置 发送队列长度
			SIOCGIFBR			桥接支持
			SIOCSIFBR			设置 桥接选项
			SIOCxxx				（还有很多取决于系统的实现，见<ioctls.h>）
- - - -
 ARP高速缓存	SIOCSARP			设置 ARP 表项				struct arpreq
			SIOCGARP			获取 ARP 表项				struct arpreq
			SIOCDARP			删除 ARP 表项				struct arpreq
- - - -
 RARP 缓存 	SIOCSRARP			设置 ARP 表项
			SIOCGRARP			获取 ARP 表项
			SIOCDRARP			删除 ARP 表项
- - - -
 路由		SIOCADDRT			增加 路由表条目				struct rtentry
			SIOCDELRT			删除 添加路由表条目			struct rtentry
			SIOCRTMSG			呼叫 路由系统
- - - -
 网卡		SIOCGIFMAP			获取 网卡映射参数
			SIOCSIFMAP			设置 网卡映射参数
 			SIOCGIFINDEX   		获取 网卡序号(接口索引 if_index)
- - - -
DLCI 		SIOCADDDLCI			创建 新 DLCI 设备
			SIOCDELDLCI			删除 DLCI 设备
- - - -
 流			I_xxx				（见书 31.5 节）              **/
/*************************************************************************/

#include "wrap_ioctl.h"


/* 对底层内核的控制 */
int IOctl(const int &fd, const ulong &request, const void *arg,
		  const int &line, const char *func)
{
	int retval = ioctl(fd, request, arg);
	if (retval < 0) {
		err_sys(close(fd),  __FILE__, __func__, __LINE__, "close()");
		err_sys(retval, __FILE__, __func__, __LINE__,
				"ioctl()\t调用地方: %s(): %d", func, line);
	}
	return retval;
}
