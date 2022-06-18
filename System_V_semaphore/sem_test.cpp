//
// Created by zcq on 2021/5/20.
//

/** linux 命令：$ ipcs #显示所有 消息队列/共享内存段/信号量数组
 ipcs -q  显示所有 消息队列
 ipcs -m  显示所有 共享内存段
 ipcs -s  显示所有 信号量数组
 ipcrm -h 查看删除选项    **/

/*1.什么是信号量
进程间通信方式之一，用于实现进程间同步与互斥（不传递数据）。
 多个进程同时操作一个临界资源的时候就需要通过同步与互斥机制来实现对临界资源的安全访问。
信号量与前面介绍的管道、FIFO以及消息队列不同，信号量是具有一个等待队列的计数器
 （0代表现在还有没有资源可以使用），用于为多个进程提供对共享数据的访问。
常用的信号量形式被称之为双态信号量(binary semaphore)。它控制单个资源，其初始值为1。
 但是，一般而言，信号量的初值可以是任一正值，该值说明有多少个共享资源单位可供共享应用。
 SYSTEM V的信号量和POSIX的信号量思想相同，但是前者很复杂，而后者的使用非常简单，
 这里介绍的是SYSTEM V的信号量。
2.实现方法
当信号量没有资源可用时，这时候需要阻塞等待，常用的信号量形式被称之为双态信号量(binary semaphore)。
 它控制单个资源，其初始值为1。但是，一般而言，信号量的初值可以是任一正值，
 该值说明有多少个共享资源单位可供共享应用。
同步：只有信号量资源计数大于0的时候，会通知别人，打断等待，去操作临界资源，也就是说，
 别人释放了资源（+1）之后你才能获取资源（-1）然后进行操作。
互斥：信号量如果要实现互斥，那么它的计数只能是 0/1 (一元信号量，双态信号量)，
 一个进程获取临界资源后，在他没释放临界资源之前，别的进程无法获取该临界资源。
为了获取共享资源，进程需要执行下列操作：
(1)测试控制该资源的信号量。
(2)若此信号量的值为正（大于0），则进程可以使用该资源。在这种情况下，进程会将信号量值减一，
 表示使用了一个资源单位。
(3)若此信号量值为0，则进程进入休眠状态，直至信号量大于0，进程被唤醒后，继续步骤（1）。
当进程不再使用由一个信号量控制的共享资源时，该信号量值增1。如果有进程正在休眠等待此信号量，则唤醒他们。
为了正确地实现信息量，信号量值的测试及减 1 操作是原子操作。为此,信号量通常是在内核中实现的。 */

/* #include <sys/sem.h>  <sys/ipc.h>  <sys/types.h>
 System V 信号量 并不如 Posix 信号量 那样“好用”，
 相比之下它的年代更加久远，SystemV 在老系统中的使用更加广泛。System V 信号随内核持续。
 System V 信号量 有一个新的概念叫做：计数信号量集，其实是把信号量放入数组中，用一些特别的结构封装。
--------------------------------------------------------------------------------
 systemV 信号随内核持续的，semget()调用成功后，
 在内核会维护一个信息结构（semid_ds{}可以理解为信号量集的表头，或者链表的头节点），
 里面包含了一些信息。
------------------------
 // 创建信号量，通过 ftok()函数生成键值 key，返回 信号集的标识符 semid
 int semget (key_t key, int nsems, int semflg);
	 key 	通过 ftok()根据特定 path 获取
	 nsems 	表示 创建的 信号量集中 信号量的个数（可以理解为数组的元素个数）
        若只是访问一个已存在的信号集，必须指定 nsems=0。
	 semflg 权限标志位，如 O_CREAT | O_EXCL | 0644（参考 open()）;
	    会存入 semid_ds{}的 sem_perm{}的成员 mode。
-----------------------
	struct semid_ds {
		struct ipc_perm sem_perm;		操作许可，如 0644，0777。
		__SEM_PAD_TIME (sem_otime, 1);	上次 semop()时间，初始为 0。
		__SEM_PAD_TIME (sem_ctime, 2);	上次由 semctl()更改，初始为 “当前时间”。
		__syscall_ulong_t sem_nsems;	被置为 semget()的参数 nsems 值。
		__syscall_ulong_t __glibc_reserved3;
		__syscall_ulong_t __glibc_reserved4;
		struct sem * sem_base;	//这一项在man page中没有明确表示，
								//但man page提到信号量集中有这样的结构。
	};
    struct sem {
        ushort_t semval;   //信号集每一个的信号量当前值
        short sempid;      //最后一个调用 semop()的进程ID
        ushort semncnt;    //等待 semval 大于当前值的进程数（一有进程释放资源 就被唤醒）
        ushort semzcnt;    //等待 semval=0 的进程数
        ushort semadj;     //信号量调整值
    };
-----------------------
 syetem V IPC 为每一个 IPC 结构设置了一个 ipc_perm 结构，该结构规定了许可权和所有者
	struct ipc_perm {
		key_t  key;    //调用shmget()时给出的关键字
		uid_t  uid;    //共享内存所有者的有效用户ID
		gid_t  gid;    //共享内存所有者所属组的有效组ID
		uid_t  cuid;   //共享内存创建 者的有效用户ID
		gid_t  cgid;   //共享内存创建者所属组的有效组ID
		unsigned short mode; //权限标志位 + SHM_DEST 和 SHM_LOCKED 标志
		unsigned short seq; //序列号
	};
--------------------------------------------------------------------------------
 // 信号量控制
 int semctl (int semid, int semnum, int cmd, ...);
 	semid 信号集的标识符
 	semnum 指定信号量集中的某个成员，类似于数组下标（0, 1, 2, ..., nsems-1）。
-----------------------
	CMD 对应的宏(semnum 值仅仅用于前5个命令)
		命令 		作用
		GETVAL		返回 semval（信号量的当前值）
		SETVAL		把 semval（信号量的当前值） 设定为指定值，会使 semadj=0。
		GETPID		返回 sempid（）
		GETNCNT		返回 semncnt（等待 semval 大于当前值的进程数）
		GETZCNT		返回 semzcnt（等待 semval=0 的进程数）
		------------
		GETALL		返回所有 semval 值，由 *array 指针返回
		SETALL		设置所有 semval
		IPC_RMID	删除指定 semid 信号量集
		IPC_SET		设置 semid_ds *buf (uid，gid 和 mode)
		IPC_STAT	返回 semid_ds *buf
-----------------------
	第四个参数 ...
		union semun {
			int semval; // 信号集每一个的信号量当前值
			struct semid_ds *buf; (使用选项 IPC_SET、IPC_STAT)
			unsigned short *array; (使用选项 SETALL、GETALL)
			struct seminfo *buf; // IPC_INFO 的缓冲区（特定于Linux）
		};
--------------------------------------------------------------------------------
 // 信号量操作：post(+1)/wait(-1)/...
 int semop (int semid, struct sembuf *sops, size_t nsops);
  	semid 信号集的标识符
    nsops 为 sops 指向的数组 sembuf{}的元素个数
-----------------------
	struct sembuf {
		unsigned short sem_num; // 指定信号量集中的某个成员，
								// 类似于数组下标（0, 1, 2, ..., nsems-1）
		short sem_op; // 计数值操作 负数, 0, 正数
		short sem_flg; // 操作标志(默认为 0，表示计数值小于减去的数，便会阻塞)
 	};
-----------------------
 sem_op 值分为3类：
-----------------------
	sem_op > 0：将值添加到 semval 上，对应于释放某个资源。
        如果指定了 SEM_UNDO 标志，则从该进程的 semadj 中减去 sem_op。
-----------------------
	sem_op = 0：表示调用进程希望等待 semval 值变为0，如果已经是0 ，则此函数立即返回。
        如果此信号量值 非 0，则适用于下列条件：
        a. 若指定了 IPC_NAWAIT，则出错返回 EAGAIN
        b. 若未指定 IPC_NOWAIT，则该信号量的 semzcnt 值加 1，
            然后调用进程/线程被挂起，直至下列的一个事件发生。
-----------------------
	sem_op < 0：希望等待到 semval 值变为 大于或等于|sem_op|；
		如果满足条件，则 semval 减去 |sem_op|，否则 semncnt+1 且线程睡眠。
        如若信号量小于 sem_op 的绝对值，则适用于下列条件：
        a. 若指定了 IPC_NAWAIT，则出错返回 EAGAIN
        b. 若未指定 IPC_NOWAIT，则该信号量的 semzcnt 值加 1，
            然后调用进程/线程被挂起，直至下列的一个事件发生。
--------------------------------------------------------------------------------
 信号量超时操作
 int semtimedop (int semid, struct sembuf *sops, size_t nsops,
                 const struct timespec *timeout);
-------------------------------------------------------------------------------- */


#include "sem_test.h"





