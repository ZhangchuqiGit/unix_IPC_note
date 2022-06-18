#! /bin/bash

## 设置  景色
F_black="\033[30m"
F_red="\033[31m"
F_gold="\033[32m"
F_yellor="\033[33m"
F_blue="\033[34m"
F_light_blue="\033[35m"
F_green="\033[36m"
F_cyan="\033[37m"
F_exit="\e[0m"

#    ANSI控制码的说明:
#    \e[0m  关闭所有属性
#    \033[0m 关闭所有属性
#    \033[1m 设置高亮度
#    \033[4m 下划线
#    \033[5m 闪烁
#    \033[7m 反显
#    \033[8m 消隐

echo -e "${F_black} 0123456789"
echo -e -n "\033[1m"
echo -e "${F_red} 0123456789"
echo -e -n "${F_exit}"
echo -e "${F_gold} 0123456789"
echo -e "${F_yellor} 0123456789"
echo -e -n "\033[5m"
echo -e "${F_blue} 0123456789"
echo -e -n "\033[7m"
echo -e "${F_light_blue} 0123456789"
echo -e -n "\033[8m"
echo -e "${F_green} 0123456789"
echo -e -n "\033[4m"
echo -e "${F_cyan} 0123456789"
echo -e -n "${F_exit}"
echo -e " 0123456789"

echo -e "文件名\t\${0}\t${0}"
echo -e "所有参数\t\${@}\t${@}"
echo -e "参数个数\t\${#}\t${#}"
echo -e "\${1}\t${1}"
echo -e "\${2}\t${2}"
echo -e "\${3}\t${3}"

exit 0
