#Copyright (c) 2011-2022 Columbia University, System Level Design Group
#SPDX-License-Identifier: Apache-2.0


# ACCELERATOR = conv_layer_sysc_catapult
# ESP_ROOT = ../../../../..
TECH=virtex7

# include ../../../common/common.mk
include ./dma.mk

CXXFLAGS += -g -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-label -Wno-maybe-uninitialized

ifndef SYSTEMC_HOME
$(error - Environment variable SYSTEMC_HOME must be defined)
endif
ifndef CONNECTIONS_HOME
$(error - Environment variable CONNECTIONS_HOME must be defined)
endif
ifndef MATCHLIB_HOME
$(error - Environment variable MATCHLIB_HOME must be defined)
endif
ifndef BOOST_HOME
$(error - Environment variable BOOST_HOME must be defined)
endif
ifndef RAPIDJSON_HOME
$(error - Environment variable RAPIDJSON_HOME must be defined)
endif
ifndef AC_TYPES
$(error - Environment variable AC_TYPES must be defined)
endif
ifndef AC_SIMUTILS
$(error - Environment variable AC_SIMUTILS must be defined)
endif

# Default to the compiler installed on the machine
CXX ?= g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64,$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64)
export LD_LIBRARY_PATH
LIBDIRS += -L$(SYSTEMC_HOME)/lib -L$(SYSTEMC_HOME)/lib-linux64

# endif

# ---------------------------------------------------------------------

# Check: $(SYSTEMC_HOME)/include/systemc.h must exist
checkvar_SYSTEMC_HOME: $(SYSTEMC_HOME)/include/systemc.h

# Check: $(CONNECTIONS_HOME)/include/connections/connections.h must exist
checkvar_CONNECTIONS_HOME: $(CONNECTIONS_HOME)/include/connections/connections.h

# Check: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h
checkvar_MATCHLIB_HOME: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h

# Check: $(BOOST_HOME)/include/boost/preprocessor/arithmetic/add.hpp
checkvar_BOOST_HOME: $(BOOST_HOME)/include/boost/preprocessor/arithmetic/add.hpp

# Check: $(RAPIDJSON_HOME)/include/rapidjson/document.h
checkvar_RAPIDJSON_HOME: $(RAPIDJSON_HOME)/include/rapidjson/document.h

# Check: $(AC_TYPES)/include/ac_int.h
checkvar_AC_TYPES: $(AC_TYPES)/include/ac_int.h

# Check: $(AC_SIMUTILS)/include/mc_scverify.h
checkvar_AC_SIMUTILS: $(AC_SIMUTILS)/include/mc_scverify.h

# Rule to check that environment variables are set correctly
checkvars: checkvar_SYSTEMC_HOME checkvar_CONNECTIONS_HOME checkvar_MATCHLIB_HOME checkvar_BOOST_HOME checkvar_RAPIDJSON_HOME checkvar_AC_TYPES checkvar_AC_SIMUTILS
# =====================================================================

export CATAPULT_HOME SYSTEMC_HOME CONNECTIONS_HOME MATCHLIB_HOME BOOST_HOME RAPIDJSON_HOME AC_TYPES AC_SIMUTILS

# Determine the director containing the source files from the path to this Makefile
PWD := $(shell pwd)
SOURCE_DIR = $(subst syn,src/,$(PWD))
TB_DIR = $(subst syn,tb/,$(PWD))

SRCS := $(wildcard $(SOURCE_DIR)*.cpp)
SRCS += $(wildcard $(TB_DIR)*.cpp)

HDRS := $(wildcard $(SOURCE_DIR)*.hpp)
HDRS += $(wildcard $(TB_DIR)*.hpp)

SYS_FILE_SMALL ?= ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL)/conv_layer_sysc_catapult.v1/scverify/concat_sim_conv_layer_sysc_catapult_v_msim/system.cpp.cxxts
SYS_FILE_MEDIUM ?= ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM)/conv_layer_sysc_catapult.v1/scverify/concat_sim_conv_layer_sysc_catapult_v_msim/system.cpp.cxxts
SYS_FILE_FAST ?= ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST)/conv_layer_sysc_catapult.v1/scverify/concat_sim_conv_layer_sysc_catapult_v_msim/system.cpp.cxxts


INCDIRS += -I$(SOURCE_DIR) -I$(TB_DIR) -I$(PWD)/../../pv/mojo -I/opt/opencv/include

INCDIRS += -I$(SYSTEMC_HOME)/include -I$(SYSTEMC_HOME)/src
INCDIRS += -I$(MATCHLIB_TOOL)/../include
# INCDIRS += -I$(MATCHLIB_TOOL)/matchlib_examples/include
INCDIRS += -I$(CONNECTIONS_HOME)/include
INCDIRS += -I$(MATCHLIB_HOME)/cmod/include
INCDIRS += -I$(BOOST_HOME)/include
INCDIRS += -I$(RAPIDJSON_HOME)/include
INCDIRS += -I$(AC_TYPES)/include
INCDIRS += -I$(AC_MATH)/include
INCDIRS += -I$(AC_SIMUTILS)/include

INCDIRS += -I../../inc
INCDIRS += -I../../inc/core/systems

TEST_IMAGE ?= cat

CPPFLAGS += $(INCDIRS)
CPPFLAGS += -DSC_INCLUDE_DYNAMIC_PROCESSES 
CPPFLAGS += -DSEGMENT_BURST_SIZE=16
CXXFLAGS += -D__CUSTOM_SIM__
CXXFLAGS += -D__MATCHLIB_CONNECTIONS__
CXXFLAGS += -DHLS_CATAPULT

LIBS += -lsystemc -lpthread

CATAPULT_PRODUCT = ultra

HW_ROOT ?= $(shell realpath ../..)
INPUT_PATH ?= $(HW_ROOT)/scripts/preprocess
MODELS ?= $(HW_ROOT)/models
MOJO_PRETRAINED ?= $(MODELS)/dwarf7.mojo

IMGS = $(wildcard $(INPUT_PATH)/*.bmp)
BMPS = $(foreach j, $(IMGS), $(shell basename $(j)))
BINS = $(BMPS:.bmp=.bin)

IMAGE_PATH ?= $(HW_ROOT)/data
IMAGE = $(shell ls $(IMAGE_PATH) | sed -e 's/.bmp//' | awk -F/ '{print $$(NF-1)}')
IMAGE-exe-syn = $(addsuffix -exe-syn, $(IMAGE))
IMAGE-exe-syn-tlm = $(addsuffix -exe-syn-tlm, $(IMAGE))
IMAGE-exe = $(addsuffix -exe, $(IMAGE))



IMAGE-small-exe-syn = $(addsuffix -small-exe-syn, $(IMAGE))
IMAGE-small-exe-syn-tlm = $(addsuffix -small-exe-syn-tlm, $(IMAGE))
IMAGE-accelerated-small-exe-syn = $(addsuffix -accelerated-small-exe-syn, $(IMAGE))
IMAGE-small-exe-syn-fp = $(addsuffix -small-exe-syn-fp, $(IMAGE))

IMAGE-medium-exe-syn = $(addsuffix -medium-exe-syn, $(IMAGE))
IMAGE-medium-exe-syn-tlm = $(addsuffix -medium-exe-syn-tlm, $(IMAGE))
IMAGE-accelerated-medium-exe-syn = $(addsuffix -accelerated-medium-exe-syn, $(IMAGE))
IMAGE-medium-exe-syn-fp = $(addsuffix -medium-exe-syn-fp, $(IMAGE))

IMAGE-fast-exe-syn = $(addsuffix -fast-exe-syn, $(IMAGE))
IMAGE-fast-exe-syn-tlm = $(addsuffix -fast-exe-syn-tlm, $(IMAGE))
IMAGE-accelerated-fast-exe-syn = $(addsuffix -accelerated-fast-exe-syn, $(IMAGE))
IMAGE-fast-exe-syn-fp = $(addsuffix -fast-exe-syn-fp, $(IMAGE))




IMAGE-view_wave = $(addsuffix -view_wave, $(IMAGE))
IMAGE-trace.vcd = $(addsuffix -trace.vcd, $(IMAGE))
IMAGE-trace.wlf = $(addsuffix -trace.wlf, $(IMAGE))

IMAGE-sim = $(addsuffix -sim, $(IMAGE))
IMAGE-sim-gui = $(addsuffix -sim-gui, $(IMAGE))
IMAGE-accelerated-sim = $(addsuffix -accelerated-sim, $(IMAGE))
IMAGE-accelerated-sim-gui = $(addsuffix -accelerated-sim-gui, $(IMAGE))

IMAGE-small-sim = $(addsuffix -small-sim, $(IMAGE))
IMAGE-small-sim-gui = $(addsuffix -small-sim-gui, $(IMAGE))
IMAGE-medium-sim = $(addsuffix -medium-sim, $(IMAGE))
IMAGE-medium-sim-gui = $(addsuffix -medium-sim-gui, $(IMAGE))
IMAGE-fast-sim = $(addsuffix -fast-sim, $(IMAGE))
IMAGE-fast-sim-gui = $(addsuffix -fast-sim-gui, $(IMAGE))

IMAGE-accelerated-small-sim = $(addsuffix -accelerated-small-sim, $(IMAGE))
IMAGE-accelerated-small-sim-gui = $(addsuffix -accelerated-small-sim-gui, $(IMAGE))
IMAGE-accelerated-medium-sim = $(addsuffix -accelerated-medium-sim, $(IMAGE))
IMAGE-accelerated-medium-sim-gui = $(addsuffix -accelerated-medium-sim-gui, $(IMAGE))
IMAGE-accelerated-fast-sim = $(addsuffix -accelerated-fast-sim, $(IMAGE))
IMAGE-accelerated-fast-sim-gui = $(addsuffix -accelerated-fast-sim-gui, $(IMAGE))


print_variable:
	@echo $(info $(TARGET_NAME))

.PHONY: all build clean sim_clean sim hls

RTL_OUT_SMALL=../../../../tech/virtex7/acc/conv_layer_sysc_catapult/conv_layer_sysc_catapult_dma$(DMA_WIDTH_SMALL)
RTL_OUT_MEDIUM=../../../../tech/virtex7/acc/conv_layer_sysc_catapult/conv_layer_sysc_catapult_dma$(DMA_WIDTH_MEDIUM)
RTL_OUT_FAST=../../../../tech/virtex7/acc/conv_layer_sysc_catapult/conv_layer_sysc_catapult_dma$(DMA_WIDTH_FAST)

SHELL = /bin/bash
install-small:
	@mkdir -p $(RTL_OUT_SMALL)
	@PRJ_NAME=conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); \
	indir=$$PRJ_NAME/conv_layer_sysc_catapult.v1; \
	infile=$$indir/concat_conv_layer_sysc_catapult.v; \
	outfile=$(RTL_OUT_SMALL)/conv_layer_sysc_catapult_dma$(DMA_WIDTH_SMALL).v; \
	cp $$infile $$outfile; \
	sed -i "s/module conv_layer_sysc_catapult (/module conv_layer_sysc_catapult_dma$(DMA_WIDTH_SMALL) (/g" $$outfile; \


install-medium:
	@mkdir -p $(RTL_OUT_MEDIUM)
	@PRJ_NAME=conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); \
	indir=$$PRJ_NAME/conv_layer_sysc_catapult.v1; \
	infile=$$indir/concat_conv_layer_sysc_catapult.v; \
	outfile=$(RTL_OUT_MEDIUM)/conv_layer_sysc_catapult_dma$(DMA_WIDTH_MEDIUM).v; \
	cp $$infile $$outfile; \
	sed -i "s/module conv_layer_sysc_catapult (/module conv_layer_sysc_catapult_dma$(DMA_WIDTH_MEDIUM) (/g" $$outfile; \

install-fast:
	@mkdir -p $(RTL_OUT_FAST)
	@PRJ_NAME=conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_FAST); \
	indir=$$PRJ_NAME/conv_layer_sysc_catapult.v1; \
	infile=$$indir/concat_conv_layer_sysc_catapult.v; \
	outfile=$(RTL_OUT_FAST)/conv_layer_sysc_catapult_dma$(DMA_WIDTH_FAST).v; \
	cp $$infile $$outfile; \
	sed -i "s/module conv_layer_sysc_catapult (/module conv_layer_sysc_catapult_dma$(DMA_WIDTH_FAST) (/g" $$outfile; \


build: sim_sc_syn

all: hls sim

trace.vcd: sim_sc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^

$(MOJO_PRETRAINED):
	@mkdir -p $(MODELS)
	@cp -f /opt/dwarf_model/dwarf7.mojo $@

$(INPUT_PATH)/preprocess:
	@$(MAKE) -C $(INPUT_PATH)

$(BINS): $(INPUT_PATH)/preprocess
	@ln -fs $(INPUT_PATH)/$@

########################

test-SMALL:
	./test4.sh $(TEST_IMAGE) small

test-MEDIUM:
	./test4.sh $(TEST_IMAGE) medium

test-FAST:
	./test4.sh $(TEST_IMAGE) fast

## BEHAVIORAL SIMULATION

########################
########################
########################

sim_sc_small_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_SMALL) -DSMALL -DCONNECTIONS_ACCURATE_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_small_syn_tlm: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_SMALL) -DSMALL -DCONNECTIONS_FAST_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_small_acc_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_SMALL) -DSMALL -DRUN_ACCELERATED_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_small_syn_fp: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_SMALL) -DSMALL -DFL_POINT $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)


sim_sc_medium_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_MEDIUM) -DMEDIUM -DCONNECTIONS_ACCURATE_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_medium_syn_tlm: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_MEDIUM) -DMEDIUM -DCONNECTIONS_FAST_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_medium_acc_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_MEDIUM) -DMEDIUM -DRUN_ACCELERATED_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_medium_syn_fp: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_MEDIUM) -DMEDIUM -DFL_POINT $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)


sim_sc_fast_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_FAST) -DFAST -DCONNECTIONS_ACCURATE_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_fast_syn_tlm: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_FAST) -DFAST -DCONNECTIONS_FAST_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_fast_acc_syn: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_FAST) -DFAST -DRUN_ACCELERATED_SIM $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_fast_syn_fp: $(SRCS) $(HDRS) $(BINS) $(MOJO_PRETRAINED)
	$(CXX) $(CXXFLAGS) -DDMA_WIDTH=$(DMA_WIDTH_FAST) -DFAST -DFL_POINT $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)


##############

$(IMAGE-small-exe-syn): sim_sc_small_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-small-exe-syn=)
$(IMAGE-small-exe-syn-tlm): sim_sc_small_syn_tlm
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-small-exe-syn-tlm=)
$(IMAGE-accelerated-small-exe-syn): sim_sc_small_acc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-accelerated-small-exe-syn=)
$(IMAGE-small-exe-syn-fp): sim_sc_small_syn_fp
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-small-exe-syn-fp=)

$(IMAGE-medium-exe-syn): sim_sc_medium_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-medium-exe-syn=)
$(IMAGE-medium-exe-syn-tlm): sim_sc_medium_syn_tlm
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-medium-exe-syn-tlm=)
$(IMAGE-accelerated-medium-exe-syn): sim_sc_medium_acc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-accelerated-medium-exe-syn=)
$(IMAGE-medium-exe-syn-fp): sim_sc_medium_syn_fp
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-medium-exe-syn-fp=)

$(IMAGE-fast-exe-syn): sim_sc_fast_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-fast-exe-syn=)
$(IMAGE-fast-exe-syn-tlm): sim_sc_fast_syn_tlm
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-fast-exe-syn-tlm=)
$(IMAGE-accelerated-fast-exe-syn): sim_sc_fast_acc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-accelerated-fast-exe-syn=)
$(IMAGE-fast-exe-syn-fp): sim_sc_fast_syn_fp
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-fast-exe-syn-fp=)


##############

$(IMAGE-trace.vcd): sim_sc_acc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^ $(@:-trace.vcd=)

$(IMAGE-trace.wlf): %.wlf : %.vcd
	vcd2wlf trace.vcd trace.wlf

$(IMAGE-view_wave): %-view_wave : %-trace.wlf
	vsim trace.wlf -nolog -do "add wave -r trace:/SystemC/*" -do "wave zoom full"

##############

## HLS

#This target is used to run hls

hls-small:
	@echo '';if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		rm -r conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); \
	fi;
	@echo '';export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL); \
	catapult -product $(CATAPULT_PRODUCT) -shell -f ./build_prj_top.tcl &

hls-small-gui:
	@echo '';if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		rm -r conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); \
	fi;
	@echo '';export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL); \
	catapult -product $(CATAPULT_PRODUCT) -f ./build_prj_top.tcl &

hls-medium:
	@echo '';if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		rm -r conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); \
	fi;
	@echo '';export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM); \
	catapult -product $(CATAPULT_PRODUCT) -shell -f ./build_prj_top.tcl &

hls-medium-gui:
	@echo '';if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		rm -r conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); \
	fi;
	@echo '';export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM); \
	catapult -product $(CATAPULT_PRODUCT) -f ./build_prj_top.tcl &

hls-fast:
	@echo '';if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		rm -r conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); \
	fi;
	@echo '';export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST); \
	catapult -product $(CATAPULT_PRODUCT) -shell -f ./build_prj_top.tcl &

hls-fast-gui:
	@echo '';if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		rm -r conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); \
	fi;
	@echo '  ';export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST); \
	catapult -product $(CATAPULT_PRODUCT) -f ./build_prj_top.tcl &

##############

## RTL SIMULATION

$(IMAGE-small-sim):$(BINS)
	if test -e $(SYS_FILE_SMALL); then \
		rm $(SYS_FILE_SMALL) & \
	fi;
	@echo '  ';export IMM=$(@:-small-sim=); export MD=RUN_NORMAL_SIM; export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL);\
	if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS SMALL before RTL-simulation"; \
	fi;

$(IMAGE-small-sim-gui):$(BINS)
	if test -e $(SYS_FILE_SMALL); then \
		rm $(SYS_FILE_SMALL) & \
	fi;
	@echo '  ';export IMM=$(@:-small-sim-gui=); export MD=RUN_NORMAL_SIM; export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL);\
	if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;

$(IMAGE-medium-sim):$(BINS)
	if test -e $(SYS_FILE_MEDIUM); then \
		rm $(SYS_FILE_MEDIUM) & \
	fi;
	@echo '  ';export IMM=$(@:-medium-sim=); export MD=RUN_NORMAL_SIM; export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM);\
	if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;

$(IMAGE-medium-sim-gui):$(BINS)
	if test -e $(SYS_FILE_MEDIUM); then \
		rm $(SYS_FILE_MEDIUM) & \
	fi;
	@echo '  ';export IMM=$(@:-medium-sim-gui=); export MD=RUN_NORMAL_SIM; export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM);\
	if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;

$(IMAGE-fast-sim):$(BINS)
	if test -e $(SYS_FILE_FAST); then \
		rm $(SYS_FILE_FAST) & \
	fi;
	@echo '  ';export IMM=$(@:-fast-sim=); export MD=RUN_NORMAL_SIM; export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST);\
	if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS FAST before RTL-simulation"; \
	fi;

$(IMAGE-fast-sim-gui):$(BINS)
	if test -e $(SYS_FILE_FAST); then \
		rm $(SYS_FILE_FAST) & \
	fi;
	@echo '  ';export IMM=$(@:-fast-sim-gui=); export MD=RUN_NORMAL_SIM; export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST);\
	if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;


#This target is used to run accelerated RTL-simulation
#for any input image

$(IMAGE-accelerated-small-sim):$(BINS)
	if test -e $(SYS_FILE_SMALL); then \
		rm $(SYS_FILE_SMALL) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-small-sim=); export MD=RUN_ACCELERATED_SIM; export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL);\
	if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS SMALL before RTL-simulation"; \
	fi;

$(IMAGE-accelerated-small-sim-gui):$(BINS)
	if test -e $(SYS_FILE_SMALL); then \
		rm $(SYS_FILE_SMALL) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-small-sim-gui=); export MD=RUN_ACCELERATED_SIM; export arch=SMALL; export DMA_WIDTH=$(DMA_WIDTH_SMALL);\
	if test -e ./conv_layer_SMALL_sysc_catapult_dma$(DMA_WIDTH_SMALL); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS SMALL before RTL-simulation"; \
	fi;


$(IMAGE-accelerated-medium-sim):$(BINS)
	if test -e $(SYS_FILE_MEDIUM); then \
		rm $(SYS_FILE_MEDIUM) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-medium-sim=); export MD=RUN_ACCELERATED_SIM; export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM);\
	if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;

$(IMAGE-accelerated-medium-sim-gui):$(BINS)
	if test -e $(SYS_FILE_MEDIUM); then \
		rm $(SYS_FILE_MEDIUM) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-medium-sim-gui=); export MD=RUN_ACCELERATED_SIM; export arch=MEDIUM; export DMA_WIDTH=$(DMA_WIDTH_MEDIUM);\
	if test -e ./conv_layer_MEDIUM_sysc_catapult_dma$(DMA_WIDTH_MEDIUM); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS MEDIUM before RTL-simulation"; \
	fi;

$(IMAGE-accelerated-fast-sim):$(BINS)
	if test -e $(SYS_FILE_FAST); then \
		rm $(SYS_FILE_FAST) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-fast-sim=); export MD=RUN_ACCELERATED_SIM; export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST);\
	if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
	echo "Run HLS FAST before RTL-simulation"; \
	fi;


$(IMAGE-accelerated-fast-sim-gui):$(BINS)
	if test -e $(SYS_FILE_FAST); then \
		rm $(SYS_FILE_FAST) & \
	fi;
	@echo '  ';export IMM=$(@:-accelerated-fast-sim-gui=); export MD=RUN_ACCELERATED_SIM; export arch=FAST; export DMA_WIDTH=$(DMA_WIDTH_FAST);\
	if test -e ./conv_layer_FAST_sysc_catapult_dma$(DMA_WIDTH_FAST); then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
	echo "Run HLS FAST before RTL-simulation"; \
	fi;


############

accuracy_comp_small:
	make accuracy_comp_SMALL -C ../../accuracy

accuracy_comp_medium:
	make accuracy_comp_MEDIUM -C ../../accuracy

accuracy_comp_fast:
	make accuracy_comp_FAST -C ../../accuracy

######

sim_clean:
	@rm -rf sim_* *_data.txt *_names.txt

help: checkvars
	-@echo "Environment/Makefile Variables:"
	-@echo "  CATAPULT_HOME      = $(CATAPULT_HOME)"
	-@echo "  SYSTEMC_HOME       = $(SYSTEMC_HOME)"
	-@echo "  CONNECTIONS_HOME   = $(CONNECTIONS_HOME)"
	-@echo "  MATCHLIB_HOME      = $(MATCHLIB_HOME)"
	-@echo "  BOOST_HOME         = $(BOOST_HOME)"
	-@echo "  RAPIDJSON_HOME     = $(RAPIDJSON_HOME)"
	-@echo "  AC_TYPES           = $(AC_TYPES)"
	-@echo "  AC_SIMUTILS        = $(AC_SIMUTILS)"
	-@echo "  CXX                = $(CXX)"
	-@echo "  LIBDIRS            = $(LIBDIRS)"
	-@echo "  LD_LIBRARY_PATH    = $(LD_LIBRARY_PATH)"
	-@echo ""

clean: sim_clean
	@rm -rf *.vcd *.wlf *atap* wrapDir
