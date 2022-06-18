#! /bin/bash

#declare -i value #声明整数型变量 
#declare -a array='([0]="a" [1]="b" [2]="c")' #声明数组变量

declare -a fileString #声明数组变量
i=0

for file in $(ls)
do 
	if [ -d "${file}" ]; then 
		#fileString[$i]="$(pwd)/${file} "
		fileString[$i]=${file}
		let '++i'
	fi
done

unset file

declare -i max_size jj tt #声明整数型变量
max_size=${#fileString[@]}
let '--max_size'

#echo ${fileString[@]}
for ((i=0,j=1 ; i <= ${max_size} ; ++i, ++j ))
do
	echo -e "file ${j}\t: ${fileString[$i]}"
#	if [ ${jj} -le ${max_size} ]; then 
#		echo -e "\t\t\t${tt} : ${fileString[$jj]}"
#	else
#		echo 
#	fi
done

echo -n "---------------------- 
'q' : exit
'0' : all open
'1' : open from NO:1 \"${fileString[0]}\" to NO:${#fileString[@]} \"${fileString[${max_size}]}\"
'c 1 3': open \"${fileString[0]}\" and open \"${fileString[2]}\"
now input : "
#declare -i begin #声明整数型变量 
read begin
#echo ${begin}

function beginfunc00()
{
	begin=$0
	if [ "${begin}" == 'q' ]; then 
		exit 0
	fi

	if [ ${begin} -eq 0 ]; then 
		begin=1
	fi 
	exit 0
}

function beginfunc01()
{
	local fileString_clion=" "	
	local tmp=$[begin-1]
	for ((i=${max_size} ; i >= $tmp ; --i ))
	do
		fileString_clion="${fileString_clion}./${fileString[$i]} "
	done

	echo -e "\nClion \c"
	echo ${fileString_clion}

	sh /home/${USER}/.config/JetBrains/CLion2020.3/bin/clion.sh ${fileString_clion} &> /dev/null
	exit 0
}

function beginfunc02()
{
	local fileString_clion=" "
	local tmp=0
	#declare -i file_size #声明数组变量
	#local file_size=$#
	#for ((i=0 ; i < file_size ; ++i ))
	while [ $# -gt 0 ]
	do
		tmp=$[$1-1]
		fileString_clion="${fileString_clion}./${fileString[ $tmp ]} "
		shift
	done

	echo -e "\nClion \c"
	echo ${fileString_clion}
	sh /home/${USER}/.config/JetBrains/CLion2020.3/bin/clion.sh ${fileString_clion} &> /dev/null
	exit 0 
}

function beginfunc()
{
	while [ $# -gt 0 ]
	do
		case "$1" in 
		q)
			exit 0
		;;
		0)
			begin=1
			break
		;;
		c)
			shift
			local tmp=$@
			beginfunc02  $tmp
			break
		;;
		*)
			break
		;;
		esac
	done
	beginfunc01
}
beginfunc $begin

exit 0 

