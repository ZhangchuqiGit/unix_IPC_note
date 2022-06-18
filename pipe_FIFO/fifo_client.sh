#! /bin/bash

pid=$$ # 进程 ID
echo -e "\033[31m进程 ID：$pid\e[0m"

my_FIFO="/tmp/FIFO_one_multiple.client.$pid"
echo -e "$my_FIFO\n"
mkfifo $my_FIFO # 创建 FIFO

service_FIFO="/tmp/FIFO_one_multiple.service"
echo -e "$pid /home/zcq/linux_cpp.sh" >> $service_FIFO

echo -e "read /home/zcq/linux_cpp.sh\n\033[32m"
cat < $my_FIFO
echo -e "\e[0m\n"

sudo rm -f $my_FIFO

echo -e "\033[31m进程 ID：$pid 退出\e[0m"
