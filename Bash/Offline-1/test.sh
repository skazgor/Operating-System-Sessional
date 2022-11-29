#!/bin/bash
if [[ $# < 1 ]] ; then 
    mark="100"
else
    mark=$1
fi
array=()
i=0
chmod 777 Submissions 
for file in $(ls -1 ./Submissions )
do
    chmod 777 Submissions/${file}
    if [ "${file}.sh" = $(ls Submissions/${file}) ];then 
        array[$i]=1;
    else 
        array[$i]=0;
    fi 
    chmod 777 Submissions/${file}/$(ls Submissions/${file})
    ((i=i+1))
done
i=0;
echo "student_id,score">output.csv
for file in $(ls -1 ./Submissions )
do
    j=0
    for file_ in $(ls -1 ./Submissions )
    do
        if [ "${file}" = "${file_}" ]; then
            ((j=j+1))
            continue
        fi
        if [ -z "$(diff --ignore-all-space Submissions/${file}/$(ls Submissions/${file}) Submissions/${file_}/$(ls Submissions/${file_}))" ]; then
            array[$i]="-1"
        fi
        ((j=j+1))
    done
    bash Submissions/${file}/$(ls Submissions/${file}) > temp.txt
    str=$( diff --ignore-all-space temp.txt AcceptedOutput.txt)
    cout=0
    while [ $( expr index "$str" '<>') -gt 0 ]
    do  
        ((count=count+1))
        str=${str:$( expr index "$str" '<>')}
    done
    echo ${array[@]}
    ((val=5*$count))
    if [[ $mark -gt $val ]] ; then 

        ((array[i]= ${array[i]} *($mark - 5* $count) ))
    else
      array[i]=0
    fi
    echo "${file},${array[$i]}" >> output.csv
    count=0
    ((i=i+1))
done
echo ${array[@]}
rm temp.txt