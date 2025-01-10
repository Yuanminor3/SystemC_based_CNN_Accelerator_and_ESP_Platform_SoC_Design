#!/bin/bash

declare -a IMAGES=("bird" "frog" "cat" "airplane" "automobile"
    "deer" "dog" "horse" "ship" "truck")

let LAYER=2

# echo -n "LAYER IS $LAYER"

let accuracy=0

rm -rf accuracy_comp.log

make clean

for i in `seq 0 5`; do

    img=${IMAGES[$i]}
    class=${IMAGES[$i]}

    echo "Processing the image of a ${img}"

    dst=${PWD}
    src=../hls/syn/

    cd $src
    make ${img}-${1}-exe-syn > $dst/accuracy_comp.log
    cd $dst

    grep " system_inst #1:" accuracy_comp.log | grep -i -q $class
    r=$?
    if [ $r -eq 0 ]; then
	echo "Correct classification"
	let accuracy=accuracy+10
    else
	echo "Wrong classification"
    fi

done

tot_accuracy=$(($accuracy*10/6))
echo "Layer ${LAYER} accuracy is ${tot_accuracy}%."
if [ ${accuracy} -ge 30 ] ; then
    echo "Accuracy test PASSED"
else
    echo "Accuracy test FAILED"
fi

echo "The layer ${LAYER} accuracy is ${tot_accuracy}%." > accuracy_comp_res.log
if [ ${accuracy} -ge 30 ] ; then
    echo "Layer accuracy test PASSED" >> accuracy_comp_res.log
else
    echo "Layer accuracy test FAILED" >> accuracy_comp_res.log
fi

