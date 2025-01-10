#Copyright (c) 2011-2022 Columbia University, System Level Design Group
#SPDX-License-Identifier: Apache-2.0

set DMA_WIDTH $::env(DMA_WIDTH)
set arch $::env(arch)

project new -name conv_layer_$arch\_sysc_catapult_dma$DMA_WIDTH

set CSIM_RESULTS "./tb_data/catapult_csim_results.log"
set RTL_COSIM_RESULTS "./tb_data/catapult_rtl_cosim_results.log"
set sfd [file dir [info script]]

solution new -state initial
options defaults

options set /Input/CppStandard c++11

options set /Input/CompilerFlags {-DBANKED_MULTI_PROCESS -DSC_INCLUDE_DYNAMIC_PROCESSES }

options set /Input/CompilerFlags " -DBANKED_MULTI_PROCESS -DSC_INCLUDE_DYNAMIC_PROCESSES -DCONNECTIONS_ACCURATE_SIM -DCONNECTIONS_NAMING_ORIGINAL -DSEGMENT_BURST_SIZE=16 -DHLS_CATAPULT -DHLS_READY -D${arch}"

options set /Input/SearchPath {/opt/matchlib_examples_kit_fall_2024/matchlib_toolkit/include} -append
options set /Input/SearchPath {/opt/matchlib_examples_kit_fall_2024/matchlib_toolkit/examples/boost_home} -append
options set /Input/SearchPath {/opt/matchlib_examples_kit_fall_2024/matchlib_toolkit/examples/boost_home} -append
# options set /Input/SearchPath {/opt/matchlib_examples_kit_fall_2024/matchlib_toolkit/examples/matchlib/cmod/include } -append
# options set /Input/SearchPath {/opt/cad/catapult/shared/pkgs/matchlib/cmod/include} -append
options set /Input/SearchPath {/opt/Catapult_2024.1_2/Mgc_home/shared/pkgs/matchlib/cmod/include} -append


options set /Input/SearchPath {.} -append
options set /Input/SearchPath { \
				   ../src/ \
				   ../../inc/ \
				   ../../inc/core/systems } -append
options set /Input/SearchPath {../../pv/mojo} -append

# flow package require /DesignWrapper
flow package require /SCVerify
flow package require /QuestaSIM
flow package option set /QuestaSIM/ENABLE_CODE_COVERAGE true

#Input

solution file add "../tb/system.hpp" -exclude true
solution file add "../tb/system.cpp" -exclude true
solution file add "../tb/sc_main.cpp" -exclude true
solution file add "../tb/driver.cpp" -exclude true
solution file add "../tb/driver.hpp" -exclude true
solution file add "../tb/memory.hpp" -exclude true
solution file add "../src/utils.hpp"
solution file add "../src/accelerated_sim.hpp"
solution file add "../src/conv_layer_conf_info.hpp"
solution file add "../src/conv_layer_functions.hpp"
solution file add "../../inc/esp_dma_info_sysc.hpp"
solution file add "../src/conv_layer.hpp"
solution file add "../src/conv_layer.cpp"
solution file add "../src/conv_layer_specs.hpp"
solution file add "../src/conv_layer_utils.hpp"

solution file set ../src/conv_layer_specs.hpp -args -DDMA_WIDTH=$DMA_WIDTH

#
# Output
#

# Verilog only
solution option set Output/OutputVHDL false
solution option set Output/OutputVerilog true

# Package output in Solution dir
solution option set Output/PackageOutput true
solution option set Output/PackageStaticFiles true

# Add Prefix to library and generated sub-blocks
solution option set Output/PrefixStaticFiles true
solution options set Output/SubBlockNamePrefix "esp_acc_conv_layer_dma$DMA_WIDTH"

# Do not modify names
solution option set Output/DoNotModifyNames true

go new

go analyze


directive set -DESIGN_HIERARCHY conv_layer_sysc_catapult




go compile

# solution library add mgc_Xilinx-VIRTEX-u-2_beh -- -rtlsyntool Vivado -manufacturer Xilinx -family VIRTEX-u -speed -2 -part xcvu065-ffvc1517-2-e

solution library \
    add mgc_Xilinx-$FPGA_FAMILY$FPGA_SPEED_GRADE\_beh -- \
    -rtlsyntool Vivado \
    -manufacturer Xilinx \
    -family $FPGA_FAMILY \
    -speed $FPGA_SPEED_GRADE \
    -part $FPGA_PART_NUM

solution library add Xilinx_RAMS

go libraries
if { $arch == "SMALL"} {
    directive set -CLOCKS {clk {-CLOCK_PERIOD 20.0}}
} elseif { $arch == "MEDIUM"} {
    directive set -CLOCKS {clk {-CLOCK_PERIOD 8.0}}
} elseif { $arch == "FAST"} {
    directive set -CLOCKS {clk {-CLOCK_PERIOD 3.0}}
}

go assembly
go architect
go allocate

go extract



flow run /Vivado/synthesize -shell vivado_v/conv_layer_sysc_catapult.v.xv
project save
