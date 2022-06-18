//
// Created by zcq on 2021/5/24.
//

/*
五、共享内存
1.什么是共享内存
共享存储允许两个或多个进程共享一给定的存储区。因为数据不需要在客户机和服务器之间复制，所以这是最快的一种 I P C（读写同一块物理内存）。
2.如何实现
使用共享存储的唯一窍门是多个进程之间对一给定存储区的同步存取。若服务器将数据放入共享存储区，则在服务器做完这一操作之前，客户机不应当去取这些数据。通常，信号量被用来实现对共享存储存取的同步。(记录锁/读写锁也可用于这种场合。)
3.内核中共享内存的属性结构
struct shmid_ds {
	 struct ipc_perm shm_perm;  	ipc_perm 结构
	 struct anon_map *shm_amp; 		pointer in kernel
	 int shm_segsz;  				size of segment in bytes
	 ushort shm_lkcnt;  			number of times segment is being locked
	 pid_t shm_lpid;  				pid of last shmop()
	 pid_t shm_cpid;  				pid of creator
	 ulong shm_nattch;  			number of current attaches
	 ulong shm_cnattach;  			used only for shminfo
	 time_t shm_atime;  			last-attach time
	 time_t shm_dtime;  			last-detach time
	 time_t shm_ctime;  			last-change time
	 ...
	};
4.shmget 函数创建共享内存或获得一个共享内存标识符
#include<sys/shm.h>  int shmget(key_t key,int size,int flag);  //成功返回共享内存ID，出错返回-1
		key 关键字，可以通过函数 ftok 创建，也可以自己指定
		size 是该共享存储段的最小值。如果正在创建一个新段(一般在服务器中)，则必须指定其size。如果正在存访一个现存的段(一个客户机)，则将size指定为0。
oflag 读写权限值的组合
5.shmctl 函数对共享内存执行多种操作
#include <sys/shm.h>  int shmctl(int shmid, int cmd, struct shmid_ds * buf); //成功为0，出错为-1
		shmid 标识符 shmget 函数返回的共享内存标识符
		cmd 指定下列 5 种命令中的一种，使其在 shmid 指定的段上执行
IPC_STAT    取此段的 shmid_ds 结构，并将它存储在由 buf 指向的结构中
IPC_SET    将 buf 指向的结构中的值设置到此共享存储段的 shmid_ds 结构
IPC_RMID   从系统中删除该共享存储段
IPC_LOCK   在内存中对共享存储段加锁
IPC_UNLOCK 解锁共享存储段
6.shmat 函数将共享内存连接到调用进程的地址空间中
#include <sys/shm.h>
void * shmat(int shmid, const void * addr, int flag); //成功则返回执行共享内存段的指针，出错则为-1
shmid 标识符 shmget 函数返回的共享内存标识符
addr 共享存储段连接到调用进程的哪个地址上与 addr 参数以及 flag 中是否指定 SHM_RND 位有关。
SHM_RND 命令的意思是 “取整”；
如果 addr 为 0，则此段连接到由内核选择的第一个可用地址上（一般应指定 addr为0,以便由内核选择地址。）
如果 addr 非 0，并且没有指定 SHM_RND ，则此段连接到 addr 所指定的地址上；
如果 addr 非 0，并且指定了 SHM_RND，则此段连接到 (addr - (addr mod SHMLBA)) 所表示的地址上，该算式是将地址向下取最近 1 个 SHMLBA 的倍数。
SHMLBA 的意思是 “低边界地址倍数”，它总是 2 的乘方。
参数 flag 如果指定了 SHM_RDONLY 位，则以只读方式连接此段，否则以读写方式连接此段；
如果 shmat 执行成功，那么内将与该共享存储段相关的 shmid_ds 结构中的 shm_nattch 计数器加 1。
7.函数 shmdt 使共享存储段与该进程分离
#include <sys/shm.h>  int shmdt(void * addr);      //成功返回0，出错返回-1
addr调用 shmat 函数的返回值，共享内存在进程中的实际地址；
如果执行成功，shmdt 将使相关的 shmid_ds 结构中的 shm_nattch 计数器减1。
注意，这并不从系统中删除其标识符以及其数据结构。该标识符仍然存在,直至某个进程(一般是服务器)调用 shmctl
		(带命令IPC_RMID)特地删除它。
8.共享内存操作步骤：  1.创建共享内存  2.映射共享内存到虚拟地址空间
3.多线程同步读写（一般任务记得进行同步和互斥操作）  4.通信结束（解除映射关系，删除共享内存）
9.关于删除共享内存的问题
在删除共享内存的时候，并不是直接删的。
如果有进程依然与共享内存保持映射连接关系，
那么共享内存将不会被立即删除，
而是等最后一个映射断开后删除，
在这期间，将拒绝其他进程使用shmat连接此共享内存！
*/


#include "shm_test.h"



