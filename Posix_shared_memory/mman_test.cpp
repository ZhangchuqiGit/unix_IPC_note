//
// Created by zcq on 2021/5/22.
//

#include "mman_test.h"

/** 内核空间和用户空间   // cat /proc/进程ID/maps #查看具体内存分配
 	通常32位Linux内核地址空间划分 3G 为用户空间，1G 为内核空间
 	Linux 的虚拟地址空间范围为 0～4G，Linux 内核将这4G字节的空间分为两部分，
 将最高的 1G字节（从虚拟地址0xC0000000到0xFFFFFFFF）供内核使用，称为 内核空间。
 将较低的 3G字节（从虚拟地址0x00000000到0xBFFFFFFF）供各个进程使用，称为 用户空间。
 因为每个进程可以通过系统调用进入内核，因此，Linux内核由系统内的所有进程共享。
--------------------------------------------------------------------------------
    Linux 使用两级保护机制：0级供内核使用，3级供用户程序使用，
 每个进程有 各自的 私有用户空间（0～3G），这个空间对系统中的其他进程是不可见的，
 最高的 1GB 字节虚拟 内核空间 则为所有进程以及内核所 共享。
--------------------------------------------------------------------------------
    内核空间中存放的是内核代码和数据，进程的用户空间中存放的是用户程序的代码和数据。
 不管是内核空间还是用户空间，它们都处于虚拟空间中。
 虽然内核空间占据了每个虚拟空间中的最高 1GB字节，但映射到物理内存却总是最低地址（0x00000000），
 另外，使用虚拟地址可以很好地保护 内核空间 不被 用户空间 破坏，
 虚拟地址到物理地址转换过程有操作系统和CPU共同完成
 (操作系统为CPU设置好页表，CPU通过MMU单元进行地址转换)。
--------------------------------------------------------------------------------
 注：多任务操作系统中的每一个进程都运行在一个属于它自己的内存沙盒中，
 这个沙盒就是虚拟地址空间（virtual address space），
 在32位模式下，它总是一个4GB的内存地址块。
 这些虚拟地址通过页表（page table）映射到物理内存，页表由操作系统维护并被处理器引用。
 每个进程都拥有一套属于它自己的页表。
--------------------------------------------------------------------------------
 	共享内存可以说是最有用的进程间通信方式，也是最快的IPC形式。两个不同进程A、B共享内存的意思是，
 同一块物理内存被映射到进程A、B各自的进程地址空间。进程A可以即时看到进程B对共享内存中数据的更新，
 反之亦然。由于多个进程共享同一块内存区域，必然需要某种同步机制，互斥锁和信号量都可以。
 	采用共享内存通信的一个显而易见的好处是效率高，因为进程可以直接读写内存，而不需要任何数据的拷贝。
 对于像管道和消息队列等通信方式，则需要在内核和用户空间进行 4次的数据拷贝，
 而共享内存则只拷贝 2次数据：一次从输入文件到共享内存区，另一次从共享内存区到输出文件。
 	实际上，进程之间在共享内存时，并不总是读写少量数据后就解除映射，
 不总是有新的通信时再重新建立共享内存区域。而是保持共享区域，直到通信完毕为止，
 这样，数据内容一直保存在共享内存中，并没有写回文件。
 共享内存中的内容往往是在解除映射时才写回文件的。因此，采用共享内存的通信方式效率是非常高的。
--------------------------------------------------------------------------   **/

/** 系统调用 mmap()用于共享内存时有下面两种常用的方式：
 * 通常挂在 /dev/shm 目录中，可以直接 ls /dev/shm 查看。
 * 非亲缘关系的进程间通信要考虑同步问题。
 内存映射文件：
	1)使用普通文件提供的内存映射：适用于任何进程之间。
		1.打开或创建一个文件（open()）
		2.调用 mmap()将这个文件映射到调用进程的空间，这种方式有许多特点和要注意的地方。
	2)使用特殊文件提供匿名内存映射：适用于具有亲缘关系的进程之间。
		由于父子进程特殊的亲缘关系，fork()后，子进程继承父进程匿名映射后的地址空间，
		同样也继承 mmap()返回的地址，父子进程就可以通过映射区进行通信了。
		注意，mmap()返回的地址，需要由父子进程共同维护。
 共享内存区对象：
	3)使用 无亲缘关系的进程之间的 Posix 共享内存区。CMake 编译 -lrt
 		一般步骤为：
 		1.创建或打开一个新的共享内存区对象（shm_open()）
 		2.调用 mmmp()将这个文件映射到调用进程的空间。 **/

/******************************* 内存映射文件 ***********************************/
/* #include <sys/mman.h>
--------------------------------------------------------------------------------
 	mmap()系统调用：通过映射同一个普通文件到调用进程的地址空间，实现共享内存。
 普通文件被映射到进程的地址空间后，进程可以向像访问普通内存一样对文件进行访问（如指针），
 不必再调用 read()、write()等操作。与 mmap()系统调用配合使用的系统调用有 munmap()、msync()等。
 	实际上，mmap()系统调用并不是完全为了用于共享内存而设计的。
 它本身提供了不同于一般对普通文件的访问方式，是进程可以像读写内存一样对普通文件操作。
 而 Posix 或 System V 的共享内存则是纯粹用于共享内存的，当然 mmap()实现共享内存也是主要应用之一。
-----------------------
 // 系统调用：返回映射区的起始地址
 void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
 addr:	映射区起始地址，设置为 NULL 时表示由系统决定映射区的起始地址。
 len:	映射空间的大小/映射区的长度。长度单位是以内存页（如 4096，可自定义 4000）为单位。
 fd:	文件描述符。一般是由 open()函数返回，表明进行的是 普通文件到内存的映射。
 	其值也可以设置为 -1，此时需要指定 flags 参数中的 MAP_ANON，表明进行的是 匿名映射。
 offset:被映射对象内容的起点（偏移量），一般设为0，表示从文件头开始映射。
-----------------------
 prot:	内存保护标志，不能与文件的打开模式冲突。
 是以下的某个值，可以通过 or 运算组合：
	PROT_EXEC 	页内容/数据 可执行
	PROT_READ 	页内容/数据 可读
    PROT_WRITE 	页内容/数据 可写
    PROT_NONE 	页内容/数据 不可访问
-----------------------
 flags:	指定映射对象的类型，以下位的组合体.
 MAP_FIXED 	使用指定的 映射区起始地址 *addr，
    	如果由 *addr 和 len 参数指定的内存区重叠于现存的映射空间，重叠部分将会被丢弃。
    	如果指定的 *addr 不可用，操作将会失败。起始地址必须落在页的边界上，如 0、4096、8192。
 MAP_SHARED 共享模式，与其他进程共享此映射空间。
    	对共享区的写入，直到 msync()或者 munmap()被调用，(文件到内存映射模式)硬盘文件才会被更新。
    	MAP_SHARED 和 MAP_PRIVATE 是互斥的，二选一。
 MAP_PRIVATE 私有模式，不与其他进程共享此映射空间。
    	内存区域的写入不会影响到原文件。
    	MAP_SHARED 和 MAP_PRIVATE 是互斥的，二选一。
 MAP_NORESERVE 不要为这个映射保留交换空间。当交换空间被保留，对映射区修改的可能会得到保证。
 		当交换空间不被保留，同时内存不足，对映射区的修改会引起页违例阻塞。
 MAP_LOCKED 锁定映射区的页面，从而防止页面被交换出内存。
 MAP_GROWSDOWN 用于堆栈，告诉内核VM系统，映射区可以向下扩展。
 MAP_ANONYMOUS 匿名映射，映射区不与任何文件关联。
 MAP_32BIT 	只提供32位地址。将映射区放在进程地址空间的 低 2GB，MAP_FIXED 指定时会被忽略。
 		当前这个标志只在 x86-64 平台上得到支持。
 MAP_POPULATE 为文件映射通过预读的方式准备好页表，随后对映射区的访问不会被页违例阻塞。
 MAP_NONBLOCK 仅和 MAP_POPULATE 一起使用。不执行预读，只为已存在于内存中的页面建立页表入口。
--------------------------------------------------------------------------------
 当映射关系解除后，对原来映射地址的访问将导致页错误发生
 // 系统调用：在进程地址空间中解除一个映射关系
int munmap(void *addr, size_t len);
 addr： 是调用 mmap()时返回的地址
 len:	映射空间的大小/映射区的长度。长度单位是以内存页（如 4096，可自定义 4000）为单位。
--------------------------------------------------------------------------------
 进程在映射空间对共享内容的改变一般不直接写回到磁盘文件中，往往在调用 munmap()后才执行该操作。
 可以通过调用 msync()实现共享内存区的内容同步到磁盘文件。
 // 系统调用：共享内存区的内容同步到磁盘文件
int msync(void * addr, size_t len, int flags);
 addr:	映射区起始地址
 len:	映射空间的大小/映射区的长度。长度单位是以内存页（如 4096，可自定义 4000）为单位。
-----------------------
 flags:	取值如 MS_ASYNC/MS_SYNC/MS_INVALIDATE：
    MS_ASYNC	异步写，调用会立即返回，不等到更新的完成;
    MS_SYNC		同步写，调用会等到更新完成之后再返回;
    MS_INVALIDATE 通知 使用该共享区域的进程，在本进程的 数据/共享内容 更改之后（作为终稿），
    	使得文件的其他映射失效，从而使得共享该文件的其他进程去重新获取最新值。   */

void *Mmap(void *addr, const size_t &len,
		   const int &prot, const int &flags, const int &fd, const off_t &offset)
{
	void *ptr;
	if ( (ptr = mmap(addr, len, prot, flags, fd, offset)) == MAP_FAILED)
		err_sys(-1, __FILE__, __func__, __LINE__, "mmap()");
	return(ptr);
}

void Munmap(void *addr, const size_t &len) // 系统调用：在进程地址空间中解除一个映射关系
{
	if (munmap(addr, len) == -1)
		err_sys(-1, __FILE__, __func__, __LINE__, "munmap()");
}

/******************************************************************************/

void cat_proc_pid_maps() // 查看具体内存分配
{
	sleep(1);
	pid_t pid = getpid();
	_debug_log_info("父进程 ID: %d", pid) // cat /proc/进程ID/maps # 查看具体内存分配
	std::string cmdstr = "cat /proc/";
	cmdstr += std::to_string(pid);
	cmdstr += "/maps";
	FILE *fp = popen(cmdstr.c_str(), "r"); // 等待 子进程 执行 shell 命令
// "r" 则返回值(文件指针)连接到 shell 的 标准输出；
// "w" 则返回值(文件指针)连接到 shell 的 标准输入。
/* "r" 标准输出 */
	std::cout << "fileno(fp): " << fileno(fp) /*转化文件描述符*/ << std::endl;
	std::cout << "-------------------------------- 父进程" << std::endl;
	char buf[MAXLINE];
	memset(buf, 0, sizeof(buf)); // 清空
	while( fgets(buf, std::size(buf), fp) != nullptr ) // 父进程 获取 子进程 shell 终端输出
	{
		printf("%s", buf); // 打印终端输出的每一行内容
//		memset(buf, 0, sizeof(buf)); // 清空
	}
}

#define FileName  	"/tmp/zcq.file.mman"
#define	SEM_name  	"mman" 	// 基于路径名 /dev/shm/sem.mman
#define Count_Num  	10000

/* 内存映射文件：1）使用普通文件提供的内存映射 + 同步（Posix 有名信号量 实现 互斥锁） */
void test_mman_1_1() // 共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)
{
	/* open file, initialize to 0, map into memory */
#if 0 // 使用普通文件提供的内存映射
	int zero=0;
	int fd = Open(FileName, O_RDWR | O_CREAT, FILE_MODE);
	Write(fd, &zero, sizeof(zero));
#else
	int fd = Open("/dev/zero", O_RDWR); // 保证内存映射区被初始化为 0
#endif
	int *data_addr = (int *)Mmap(nullptr, sizeof(int),
								 PROT_READ | PROT_WRITE,
								 MAP_SHARED, fd, 0);
	Close(fd);

	/* create, initialize, and unlink semaphore */
	sem_t *mutex = Sem_open(SEM_name, O_CREAT, FILE_MODE, 1);
	Sem_unlink(SEM_name);

	/* 如果 buf=NULL，则使 stream 变为无缓冲。否则，它使用大小为 buffsize 的缓冲区 buf */
	setbuf(stdout, nullptr);

	if (Fork() == 0) {		/* child */
		for (int i = 0; i < Count_Num; ++i) {
			Sem_wait(mutex);
			printf("child: %d\n", (*data_addr)++);
			Sem_post(mutex);
		}
		exit(0);
	}

	/* parent */
	for (int i = 0; i < Count_Num; ++i) {
		Sem_wait(mutex);
		printf("parent: %d\n", (*data_addr)++);
		Sem_post(mutex);
	}

	cat_proc_pid_maps(); // 查看具体内存分配
	Munmap(data_addr, sizeof(int)); // 系统调用：在进程地址空间中解除一个映射关系
	unlink(FileName);
}

/* 内存映射文件：1）使用普通文件提供的内存映射 + 同步（Posix 匿名信号量 实现 互斥锁） */
void test_mman_1_2() // 共享内存区：计数器：父子进程 + Posix匿名信号量(互斥锁)
{
	struct _shared{
		sem_t mutex;
		int	count;
	} shared_data{}, *shared_ptr;

	/* open file, initialize to 0, map into memory */
#if 0 // 使用普通文件提供的内存映射
	int zero=0;
	int fd = Open(FileName, O_RDWR | O_CREAT, FILE_MODE);
	Write(fd, &shared_data, sizeof(shared_data));
#else
	int fd = Open("/dev/zero", O_RDWR); // 保证内存映射区被初始化为 0
#endif
	shared_ptr = (struct _shared *)Mmap(nullptr, sizeof(struct _shared),
										PROT_READ | PROT_WRITE,
										MAP_SHARED, fd, 0);
	Close(fd);

	/* initialize semaphore that is shared between processes */
	Sem_init(&shared_ptr->mutex, 1/*1:不同进程间*/, 1);

	/* 如果 buf=NULL，则使 stream 变为无缓冲。否则，它使用大小为 buffsize 的缓冲区 buf */
	setbuf(stdout, nullptr);

	if (Fork() == 0) {		/* child */
		for (int i = 0; i < Count_Num; ++i) {
			Sem_wait(&shared_ptr->mutex);
			printf("child: %d\n", shared_ptr->count++);
			Sem_post(&shared_ptr->mutex);
		}
		exit(0);
	}

	/* parent */
	for (int i = 0; i < Count_Num; ++i) {
		Sem_wait(&shared_ptr->mutex);
		printf("parent: %d\n", shared_ptr->count++);
		Sem_post(&shared_ptr->mutex);
	}

	cat_proc_pid_maps(); // 查看具体内存分配
	Munmap(shared_ptr, sizeof(struct _shared)); // 系统调用：在进程地址空间中解除一个映射关系
	unlink(FileName);
//	sleep(1);
//	Sem_destroy(&shared_ptr->mutex); // 销毁信号量
}

/* 内存映射文件：2）使用特殊文件提供匿名内存映射 + 同步（Posix 有名信号量 实现 互斥锁） */
void test_mman_2_1() // (匿名)共享内存区：计数器：父子进程 + Posix有名信号量(互斥锁)
{
	int *data_addr = (int *)Mmap(nullptr, sizeof(int),
								 PROT_READ | PROT_WRITE,
								 MAP_SHARED | MAP_ANON/*匿名内存映射*/,
								 -1/*匿名*/, 0);

	/* create, initialize, and unlink semaphore */
	sem_t *mutex = Sem_open(SEM_name, O_CREAT, FILE_MODE, 1);
	Sem_unlink(SEM_name);

	/* 如果 buf=NULL，则使 stream 变为无缓冲。否则，它使用大小为 buffsize 的缓冲区 buf */
	setbuf(stdout, nullptr);

	if (Fork() == 0) {		/* child */
		for (int i = 0; i < Count_Num; ++i) {
			Sem_wait(mutex);
			printf("child: %d\n", (*data_addr)++);
			Sem_post(mutex);
		}
		exit(0);
	}

	/* parent */
	for (int i = 0; i < Count_Num; ++i) {
		Sem_wait(mutex);
		printf("parent: %d\n", (*data_addr)++);
		Sem_post(mutex);
	}

	cat_proc_pid_maps(); // 查看具体内存分配
//	Munmap(data_addr, sizeof(int)); // 系统调用：在进程地址空间中解除一个映射关系
}

/******************************************************************************/

/* 内存映射文件：1）使用普通文件提供的内存映射
 * mmap()开辟的 内存映射区的大小 可能不同于 被映射文件的大小 */
void test_mman_01() // 测试 mmap()开辟的 内存映射区的大小
{
	std::string hints_str = "usage: <file_size> <mmap_size> ";
	std::vector<std::string> vec_str = hints_strtovec(hints_str);
	if (vec_str.size() != 2) err_quit(-1, __LINE__,
									  "%s", hints_str.c_str());
	long filesize = strtoimax(vec_str[0].c_str(), nullptr, 10);
	long mmapsize = strtoimax(vec_str[1].c_str(), nullptr, 10);

	/**------------ 打开文件：创建或截断；设定文件大小 ---------------**/
	int fd = Open(FileName, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
	int ret = (int)lseek(fd, filesize-1, SEEK_SET); // 定位文件指针
	err_sys(ret, __LINE__, "lseek()");
	Write(fd, "", 1); // 填充 1字节，此处代表文件尾，相当设定文件大小

	char *ptr = (char *)Mmap(nullptr, mmapsize,
							 PROT_READ | PROT_WRITE,
							 MAP_SHARED, fd, 0);
	Close(fd);

	long pagesize = sysconf(_SC_PAGESIZE/*页大小*/);
	printf("页大小: %ld\n", pagesize);

	long i;
	for (i = 0; i < std::max(filesize, mmapsize); i+=pagesize) {
		printf("ptr[%ld] = %d\n", i, ptr[i]);
		ptr[i] = 1; // 标示页 头
		printf("ptr[%ld] = %d\n", i+pagesize-1, ptr[i+pagesize-1]);
		ptr[i+pagesize-1] = 1; // 标示页 尾
	}
	printf("ptr[%ld] = %d\n", i, ptr[i]);

	Munmap(ptr, mmapsize); // 系统调用：在进程地址空间中解除一个映射关系
	err_sys(unlink(FileName), __LINE__, "unlink()");
}

/******************************************************************************/

/** 系统调用 mmap()用于共享内存时有下面两种常用的方式：
 * 通常挂在 /dev/shm 目录中，可以直接 ls /dev/shm 查看。
 * 非亲缘关系的进程间通信要考虑同步问题。
 内存映射文件：
	1)使用普通文件提供的内存映射：适用于任何进程之间。
		1.打开或创建一个文件（open()）
		2.调用 mmap()将这个文件映射到调用进程的空间，这种方式有许多特点和要注意的地方。
	2)使用特殊文件提供匿名内存映射：适用于具有亲缘关系的进程之间。
		由于父子进程特殊的亲缘关系，fork()后，子进程继承父进程匿名映射后的地址空间，
		同样也继承 mmap()返回的地址，父子进程就可以通过映射区进行通信了。
		注意，mmap()返回的地址，需要由父子进程共同维护。
 共享内存区对象：
	3)使用 无亲缘关系的进程之间的 Posix 共享内存区。CMake 编译 -lrt
 		一般步骤为：
 		1.创建或打开一个新的共享内存区对象（shm_open()）
 		2.调用 mmmp()将这个文件映射到调用进程的空间。 **/

/****************** 共享内存区对象 Posix 共享内存区 /dev/shm/zcq ******************/
/* #include <sys/mman.h> <unistd.h> <sys/types.h>
 	Posix 共享内存编程 涉及的函数不多，但是非亲缘关系的进程间进行通信，则要考虑同步问题。
--------------------------------------------------------------------------------
 // 创建或打开一个共享内存区对象 /dev/shm/zcq
 int shm_open (const char *name, int oflag, mode_t mode);
	oflag：	参考 open()参数，O_CREAT|O_EXCL|O_RDWR（这个套路已经很多次了）。
	mode： 	如 0644，0777
 返回 文件描述符 fd
--------------------------------------------------------------------------------
 // 删除共享内存段
 int shm_unlink (const char *name);
--------------------------------------------------------------------------------
 // 调整 文件的大小/共享内存区的大小： 截断文件 fd，打开长度为 length 个字节
 int ftruncate(int fd, off_t length);
 	fd 文件描述符；  length 要修改的大小，尽量为 文件的大小=页大小 sysconf(_SC_PAGESIZE)=4096
----------------------------------
 [文件的大小]				: 0 -      	- 4999	(5000)
 [映射文件的大小]			: 0 - 	- 4095 		(4096)
 [mmap()/共享内存区大小]	: 0 - 1023			(1024)
 [文件的大小]				: 0 -   - 4095		(4096) //调整成 页大小
 [mmap()/共享内存区大小]	: 0 - 	- 4095		(4096) //调整成 页大小
-------------------------------------------------------------------------  	*/

/* 创建或打开一个共享内存区对象 /dev/shm/shm.zcq */
int Shm_open (const char *shm_name, const int &oflag, const mode_t &mode)
{
	int fd;
	if ( (fd = shm_open(shm_name, oflag, mode)) == -1)
		err_sys(-1, __LINE__, "shm_open()");
	return fd;
}

void Shm_unlink (const char *shm_name) // 删除共享内存段
{
	if ( shm_unlink(shm_name) == -1)
		err_sys(-1, __LINE__, "shm_unlink()");
}

/* 调整 文件的大小/共享内存区的大小： 截断文件 fd，打开长度为 length 个字节 */
void Ftruncate(const int &fd, const off_t &length)
{
	if (ftruncate(fd, length) == -1)
		err_sys(-1, __LINE__, "ftruncate()");
}

/****************** 共享内存区对象 Posix 共享内存区 /dev/shm/zcq ******************/

#define	SHM_name  	"shm_mman" 	// 基于路径名 /dev/shm/shm_mman

struct shm_s {
	int	count;
};

/* 共享内存区对象：3）使用 Posix 共享内存区 + 同步（Posix 有名信号量 实现 互斥锁） */
void test_mman_3_service() // 共享内存区：计数器：Posix有名信号量(互斥锁)
{
	pid_t pid = getpid(); // cat /proc/进程ID/maps # 查看具体内存分配
	_debug_log_info("test_mman_3_service 进程 ID: %d", pid)

	shm_unlink(SHM_name); // OK if this fails

	/* create shm, set its size, mmap it, close descriptor */
	int fd = Shm_open(SHM_name, O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
	Ftruncate(fd, sizeof(struct shm_s));
	struct shm_s *data_addr = (struct shm_s *)Mmap(nullptr, sizeof(struct shm_s),
												   PROT_READ | PROT_WRITE,
												   MAP_SHARED, fd, 0);
	Close(fd);
	data_addr->count = 0;

	sem_unlink(SEM_name); // OK if this fails

	/* create, initialize semaphore */
	sem_t *mutex = Sem_open(SEM_name, O_CREAT | O_EXCL, FILE_MODE, 1);

	int tmp=data_addr->count;
	while (true) {
		usleep(10000);
		Sem_wait(mutex);
		if (data_addr->count == -1) { // 结束标志
			Sem_post(mutex);
			break;
		}
		if (data_addr->count != tmp) {
			printf("service[%d] client: %d\n", pid, data_addr->count);
			tmp = data_addr->count;
		}
		Sem_post(mutex);
	}
//	Sem_close(mutex);
	Shm_unlink(SHM_name); // 删除共享内存段
	Sem_unlink(SEM_name);
}

/* 共享内存区对象：3）使用 Posix 共享内存区 + 同步（Posix 有名信号量 实现 互斥锁） */
void test_mman_3_client() // 共享内存区：计数器：Posix有名信号量(互斥锁)
{
	int fd = Shm_open(SHM_name, O_RDWR, FILE_MODE);
	struct shm_s *data_addr = (struct shm_s *)Mmap(nullptr, sizeof(struct shm_s),
												   PROT_READ | PROT_WRITE,
												   MAP_SHARED, fd, 0);
	Close(fd);

	sem_t *mutex = Sem_open(SEM_name, 0);

	pid_t pid = getpid();
	for (int i = 0; i < 100000; i++) {
		Sem_wait(mutex);
		printf("pid %d: %d\n", pid, data_addr->count++);
		Sem_post(mutex);
	}
}

#define	MESSAGE_num  	16 	// max #numbers of messages
#define	MESSAGE_size  	256 // max #bytes per message. null at end

struct shm_messages {
	/** 模拟：当(多个)发送者发送过多的消息时，可能造成接收者忙不过来的情况，
	用 overflow 标记 消息丢弃或数据溢出 的次数 **/
	int overflow; 					// #overflows by senders
	sem_t overflowmutex; 			// mutex for overflow counter

	sem_t mutex, empty_num, stored_num;

	long msg_offset[MESSAGE_num]; 	// offsets in shared memory of each message
	char msg_data[MESSAGE_num * MESSAGE_size]; // the actual messages
};

/* 共享内存区对象：3）使用 Posix 共享内存区 + 同步（Posix 匿名信号量 实现 互斥锁） */
void test_mman_3_consumer_service() // 共享内存区：消息：Posix匿名信号量(互斥锁)
{
	pid_t pid = getpid(); // cat /proc/进程ID/maps # 查看具体内存分配
	_debug_log_info("test_mman_3_consumer_service 进程 ID: %d", pid)

	shm_unlink(SHM_name); // OK if this fails

	/* create shm, set its size, mmap it, close descriptor */
	int fd = Shm_open(SHM_name, O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
	Ftruncate(fd, sizeof(struct shm_messages));
	struct shm_messages *data_addr;
	data_addr = (struct shm_messages *)Mmap(nullptr, sizeof(struct shm_messages),
											PROT_READ | PROT_WRITE,
											MAP_SHARED, fd, 0);
	Close(fd);
	bzero(data_addr, sizeof(struct shm_messages));

	/* initialize the array of offsets */
	for (int i=0; i < MESSAGE_num; ++i)
		data_addr->msg_offset[i] = i * MESSAGE_size;

	/* initialize the semaphores in shared memory */
	Sem_init(&data_addr->mutex, 1/*1:不同进程间*/, 1);
	Sem_init(&data_addr->empty_num, 1/*1:不同进程间*/, MESSAGE_num);
	Sem_init(&data_addr->stored_num, 1/*1:不同进程间*/, 0);
	Sem_init(&data_addr->overflowmutex, 1/*1:不同进程间*/, 1);

	int index=0, overflow_last=0, overflow;
	long offset=0;
	while (true) {
		Sem_wait(&data_addr->stored_num);	// 等待至少1个存储项目
		Sem_wait(&data_addr->mutex);
		offset = data_addr->msg_offset[index];
		printf("PID:%d\tindex:%d\toffset[index]:%ld\tdata: %s\n",
			   pid, index, offset, &data_addr->msg_data[offset]);
		Sem_post(&data_addr->mutex);
		Sem_post(&data_addr->empty_num); 	// 已有1个空插槽

		Sem_wait(&data_addr->overflowmutex);
		overflow = data_addr->overflow;
		Sem_post(&data_addr->overflowmutex);

		if (++index >= MESSAGE_num) index = 0;
		if (overflow != overflow_last) {
			_debug_log_info("overflow number:%d", overflow)
			overflow_last = overflow;
		}
	}
}

#define LOOP_num  (MESSAGE_num * 100) // 发送次数

/* 共享内存区对象：3）使用 Posix 共享内存区 + 同步（Posix 匿名信号量 实现 互斥锁） */
void test_mman_3_producer_client() // 共享内存区：消息：Posix匿名信号量(互斥锁)
{
	int fd = Shm_open(SHM_name, O_RDWR, FILE_MODE);
	struct shm_messages *data_addr;
	data_addr = (struct shm_messages *)Mmap(nullptr, sizeof(struct shm_messages),
											PROT_READ | PROT_WRITE,
											MAP_SHARED, fd, 0);
	Close(fd);

	pid_t pid=getpid();
	char sendbuf[MESSAGE_size];
	bzero(sendbuf, MESSAGE_size);
	for (int i=0, index=0; i < LOOP_num; ++i) {
		snprintf(sendbuf, MESSAGE_size, "client  pid:%d  i:%d", pid, i);

		if (Sem_trywait(&data_addr->empty_num) == -1
			&& errno == EAGAIN) { 			// 等待至少1个空插槽
			Sem_wait(&data_addr->overflowmutex);
			_debug_log_info("overflow number:%d", data_addr->overflow++)
			Sem_post(&data_addr->overflowmutex);
			continue;
		}
		Sem_wait(&data_addr->mutex);

		strcpy(&data_addr->msg_data[data_addr->msg_offset[index]], sendbuf);

		Sem_post(&data_addr->mutex);
		Sem_post(&data_addr->stored_num);	// 已有1个存储项目

		if (++index >= MESSAGE_num) index = 0;
		std::cout << sendbuf << std::endl;
	}
}
