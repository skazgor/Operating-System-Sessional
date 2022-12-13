arr=()
boolarr=(0 0 0 0 0)
while IFS=" " read -r var
do
stringarray=($var)
#echo ${stringarray[1]}
    if [ ${stringarray[0]} = "open" ]; then
        touch ${stringarray[1]}
        j=0
        for j in $(seq 0 4) ; do
            if [ ${boolarr[$j]} = 0 ];then 
                boolarr[$j]=1
                break
            fi
        done
        arr[$j]=${stringarray[1]}

    elif [ ${stringarray[0]} = "close" ]; then

        for i in "${!arr[@]}"; do
             if [[ "${arr[$i]}" = "${stringarray[1]}" ]]; then
                boolarr[$i]=0
            fi
        done

        # boolarr[${arr[${stringarray[1]}]}]=0
    else 
             for i in "${!arr[@]}"; do
             if [[ "${arr[$i]}" = "${stringarray[1]}" ]]; then
                echo ${stringarray[2]} >> ${arr[$i]}
            fi
        done
        
    fi
    echo ${arr[@]}
done 