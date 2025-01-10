#!/bin/bash

ARCHS="SMALL MEDIUM FAST"

for arc in $ARCHS; do

    # Get area of each architecture
    LATENCY_TRACE="${PWD}/conv_layer_${arc}_sysc_catapult*/conv_layer_sysc_catapult.v1/scverify/concat_sim_conv_layer_sysc_catapult_v_msim/sim.log"
    LATENCY_LOG="${PWD}/latency_${arc}.log"

    if test ! -e $LATENCY_TRACE; then
	echo "--- latency trace log not found ---"
	continue
    fi

    LATENCY=$(cat $LATENCY_TRACE | grep "system_inst #1" |  sed 's/\s\+/ /g'  | cut -d " " -f 2)

    rm -f $LATENCY_LOG
    echo "$LATENCY" >> $LATENCY_LOG

done
