#!/bin/bash

declare -a IMAGES=("bird" "frog" "cat" "airplane" "automobile"
    "deer" "dog" "horse" "ship" "truck")

declare -a CLASSIFICATION=("bird" "frog" "cat" "airplane" "automobile"
    "deer" "dog" "horse" "ship" "truck")


let accuracy=0

rm -rf accuracy_sys.log

make clean

make

for i in `seq 0 9`; do

    img=${IMAGES[$i]}
    class=${IMAGES[$i]}

    echo "Processing the image of a ${img}"

    LAYER=$LAYER IMAGE=$img ARCH=$1 make dwarf-run > accuracy_sys.log

    grep "1: label|score" accuracy_sys.log | grep -i -q $class
    r=$?
    if [ $r -eq 0 ]; then
	echo "Correct classification"
	let accuracy=accuracy+10
    else
	echo "Wrong classification"
    fi

done

echo "The system accuracy is ${accuracy}%."
if [ ${accuracy} -ge 10 ] ; then
    echo "Accuracy test PASSED"
else
    echo "Accuracy test FAILED"
fi 

echo "The system accuracy is ${accuracy}%." > accuracy_sys_res.log
if [ ${accuracy} -ge 10 ] ; then
    echo "System accuracy test PASSED" >> accuracy_sys_res.log
else
    echo "System accuracy test FAILED" >> accuracy_sys_res.log
fi 

