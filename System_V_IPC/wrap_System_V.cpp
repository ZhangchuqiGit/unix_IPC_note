//
// Created by zcq on 2021/5/10.
//

#include "wrap_System_V.h"

/*	syetem V IPC 为每一个 IPC 结构设置了一个 ipc_perm 结构，该结构规定了许可权和所有者
struct ipc_perm {
   	key_t  key;    //调用shmget()时给出的关键字
   	uid_t  uid;    //共享内存所有者的有效用户ID
  	gid_t  gid;    //共享内存所有者所属组的有效组ID
   	uid_t  cuid;   //共享内存创建 者的有效用户ID
   	gid_t  cgid;   //共享内存创建者所属组的有效组ID
  	unsigned short  mode; //Permissions + SHM_DEST和SHM_LOCKED标志
  	unsigned short   seq; //序列号
};		 */


