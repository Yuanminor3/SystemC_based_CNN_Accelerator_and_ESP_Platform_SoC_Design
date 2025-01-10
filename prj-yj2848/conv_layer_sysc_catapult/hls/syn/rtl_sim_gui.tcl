set IM $::env(IMM)
set MODE_SIM $::env(MD)
set arch $::env(arch)
set dma_width $::env(DMA_WIDTH)

# project load conv_layer_SMALL_sysc_catapult_dma32.ccs
project load conv_layer_$arch\_sysc_catapult_dma$dma_width.ccs

flow package option set /SCVerify/INVOKE_ARGS $IM

flow run /SCVerify/launch_make ./scverify/Verify_concat_sim_conv_layer_sysc_catapult_v_msim.mk {} SIMTOOL=msim simgui CXX_OPTS=-D$MODE_SIM

