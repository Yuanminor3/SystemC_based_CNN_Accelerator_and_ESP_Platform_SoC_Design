#!/bin/bash

# Usage:
# ./test.sh <image> <arch> <layer>
#   e.g. ./test.sh cat SMALL TARGET_LAYER_1


if [[ $# < 2 ]] ; then
    echo 'usage: ./test4.sh <image> <arch> '
    echo ' e.g.: ./test4.sh cat SMALL'
    exit 1
fi

# Animal (cat, etc.)
image=$1

# Arch (SMALL, MEDIUM, FAST)
arch=$2

mkdir -p test
mkdir -p native_results

LOGFILE=test/test_${arch}_${image}.log

if test -e $LOGFILE; then
    mv $LOGFILE $LOGFILE.bak
fi

echo "======================" >> $LOGFILE
echo "=== Automated test ===" >> $LOGFILE
echo "======================" >> $LOGFILE
echo "" >> $LOGFILE

echo "=== BEHAV sim with Floating Point Datatype ===" >> $LOGFILE

# Create programmer view results file

# echo -n "  - PV $image ... " >> $LOGFILE

echo "" >> $LOGFILE

# Native floating point
echo -n "native float behavioral $image $arch ... " >> $LOGFILE
timeout --signal=KILL 120m make ${image}-${arch}-exe-syn-fp
mv test4.txt test/test_native_${arch}_${image}.txt

/usr/bin/diff -q test/test_native_${arch}_${image}.txt golden_fp_tests/test4.txt > /dev/null

r=$?
if [ $r -eq 0 ]; then
    echo "Native simulation PASSED" >> $LOGFILE
else
    echo "Native simulation NOT PASSED" >> $LOGFILE
fi

echo "" >> $LOGFILE


# # Component accuracy in fixed-point software
echo "=== Test component fixed-point accuracy in software ===" >> $LOGFILE

rm -f ../../accuracy/accuracy_comp_res.log
timeout --signal=KILL 240m make accuracy_comp_${arch}
if [ -e "../../accuracy/accuracy_comp_res.log" ]; then
    echo "" >> $LOGFILE
    cat ../../accuracy/accuracy_comp_res.log >> $LOGFILE
else
    echo -n "Layer accuracy test ERROR" >> $LOGFILE
fi



#######

# Fixed-point accelerated
timeout --signal=KILL 120m make ${image}-accelerated-${arch}-exe-syn
mv accelerated_test_syn_4.txt test/accelerated_test_beh_${arch}_${image}.txt

echo "" >> $LOGFILE

# HLS
echo "=== ${arch} ===" >> $LOGFILE
echo -n "  - HLS ${arch} ... " >> $LOGFILE
touch hls_output.txt
timeout --signal=KILL 2h make hls-${arch} | tee hls_output.txt
hls_log=conv_layer_${arch^^}_sysc_catapult_dma*/conv_layer_sysc_catapult.v1/messages.txt 
grep -i "error" $hls_log
r=$?
if [ $r -eq 1 ]; then
    echo "PASSED" >> $LOGFILE
else
    echo "FAILED (skip simulation)" >> $LOGFILE
    echo "" >> $LOGFILE
    exit
fi
rm hls_output.txt

echo "    == RTL sim ==" >> $LOGFILE

# RTL accelerted
echo -n "- RTL ${arch} ... " >> $LOGFILE
timeout --signal=KILL 120m make ${image}-accelerated-${arch}-sim | tee -a test_ext.log
mv accelerated_test_syn_4.txt test/accelerated_test_rtl_${arch}_${image}.txt
/usr/bin/diff -q test/accelerated_test_rtl_${arch}_${image}.txt test/accelerated_test_beh_${arch}_${image}.txt > /dev/null
r=$?
if [ $r -eq 0 ]; then
    echo "PASSED" >> $LOGFILE
else
    echo "FAILED (Aborting)" >> $LOGFILE
    exit
fi

echo "" >> $LOGFILE
