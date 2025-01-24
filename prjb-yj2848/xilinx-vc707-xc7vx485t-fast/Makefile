# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0
##########################
### ESP Local Makefile ###
##########################

all: help

### Global design variables ###
DESIGN_PATH = $(PWD)
ESP_ROOT = $(realpath /opt/esp/)
TECHLIB  = virtex7
BOARD    = xilinx-vc707-xc7vx485t
DESIGN   = esp-$(BOARD)
SMP = 0
BASE_FREQ_MHZ ?= 50
LEON3_STACK ?= 0x5bfffff0
USE_OPENSBI ?= 1
ESP_DEFCONFIG = $(DESIGN_PATH)/esp_xilinx-vc707-xc7vx485t_defconfig

### Design top level and testbench ###
TOP    = top
SIMTOP = testbench


### Modelsim Simulation Options ###

# Compile flags
VCOMOPT +=
VLOGOPT +=

XCOMOPT +=
XLOGOPT +=

VSIMOPT +=


### Additional design files ###
TOP_VHDL_RTL_PKGS +=
TOP_VHDL_RTL_SRCS +=
TOP_VLOG_RTL_SRCS +=
TOP_VHDL_SIM_PKGS +=
TOP_VHDL_SIM_SRCS +=
TOP_VLOG_SIM_SRCS +=


### Xilinx Vivado hw_server ###
FPGA_HOST ?= espdev.cs.columbia.edu
XIL_HW_SERVER_PORT ?= 3131


### Network configuration ###

# IP address or host name of the host connected to the FPGA
UART_IP ?= espdev.cs.columbia.edu
UART_PORT ?= 4330

# SSH IP address or host name of the ESP Linux instance or gateway
SSH_IP ?=
SSH_PORT ?= 22

# ESPLink IP address or gateway (DO NOT USE HOST NAME)
ESPLINK_IP ?= 128.59.22.75
ESPLINK_PORT ?= 46313

# MAC address for Linux if using IP address reservation (e.g. 00aabb33cc77)
# LINUX_MAC ?=


### Include global Makefile ###
include $(ESP_ROOT)/utils/Makefile
