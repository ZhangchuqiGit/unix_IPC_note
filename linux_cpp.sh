#! /bin/bash

declare -a fileString #声明数组变量
i=0
for file in $(ls)
do 
	if [ -d "${file}" ]; then 
		fileString[$i]=${file}
		let '++i'
	fi
done

unset file

for ((i=0,j=1 ; i < ${#fileString[@]} ; ++i, ++j ))
do
	echo -e "file ${j}\t: ${fileString[$i]}"
done

for ((i=0 ; i < ${#fileString[@]} ; ++i ))
do
	fileString[ $i ]="./${fileString[ $i ]}/test/*.cpp"
done

max_size=${#fileString[@]}
let '--max_size'

echo -n "---------------------- 
'q' : exit
'0' : all open
'1' : open from NO:1 \"${fileString[0]}\" to NO:${#fileString[@]} \"${fileString[${max_size}]}\"
'c 1 3': open \"${fileString[0]}\" and open \"${fileString[2]}\"
now input : "
read begin

function beginfunc01()
{
	local fileString_clion=" "	
	for ((i=${begin}-1 ; i < ${#fileString[@]} ; ++i ))
	do
		fileString_clion="${fileString_clion}${fileString[$i]} "
	done
	gedit ${fileString_clion} &>/dev/null
	exit 0
}

function beginfunc02()
{
	declare -a ary_num #声明数组变量
	local i=0
	while [ $# -gt 0 ]
	do
		ary_num[ $i ]=$[ $1 - 1 ]
		let '++i'
		shift
	done

	local tmp=0
	declare -i tmp #声明数组变量

#	for ((i=0, j=${#ary_num[@]}-1 ; i < ${#ary_num[@]} ; ++i ,--j ))
#	do
#		if [ "${i}" -ge "${j}" ]; then 
#			break
#		fi
#		tmp=${ary_num[ $i ]}
#		ary_num[ $i ]=${ary_num[ $j ]}
#		ary_num[ $j ]=${tmp}
#	done

	local fileString_clion=" "
	for ((i=0 ; i < ${#ary_num[@]} ; ++i ))
	do
		tmp=${ary_num[ $i ]}
		fileString_clion="${fileString_clion}${fileString[ $tmp ]} "
	done
	
	gedit ${fileString_clion} &>/dev/null
	exit 0 
}

function beginfunc()
{
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
	;;
	esac
	beginfunc01
}
beginfunc $begin

exit 0


