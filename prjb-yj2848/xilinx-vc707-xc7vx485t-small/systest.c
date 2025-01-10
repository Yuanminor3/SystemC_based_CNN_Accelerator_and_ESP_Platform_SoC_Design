/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include "conv_layer.h"
#include "monitors.h"


// TODO: integrate the device number and name for the target layers you want
// Accelerator for target layer
#define TARGET_LAYER1 0
#define SLD_CONV1 0x04a
#define DEV_NAME1 "sld,conv_layer1_sysc_catapult"
#define TARGET_LAYER2 1
#define SLD_CONV2 0x04b
#define DEV_NAME2 "sld,conv_layer2_sysc_catapult"
// TODO:
// Accelerator for the other layer
#define TARGET_LAYER3 2
#define SLD_CONV3 0x04c
#define DEV_NAME3 "sld,conv_layer3_sysc_catapult"
#define TARGET_LAYER4 3
#define SLD_CONV4 0x04d
#define DEV_NAME4 "sld,conv_layer4_sysc_catapult"

#define N_CONV_LAYERS 4
#define N_FC_LAYERS 2

#define FX_WL 14
#define FX_IL 6

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?      \
            (_sz / CHUNK_SIZE) :        \
            (_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define IN_BASE_ADDR_SIG_REG 0x7c
#define IN_W_BASE_ADDR_SIG_REG 0x78
#define OUT_B_BASE_ADDR_SIG_REG 0x74
#define OUT_BASE_ADDR_SIG_REG 0x70
#define IN_SIZE_SIG_REG 0x6c
#define IN_W_SIZE_SIG_REG 0x68
#define OUT_SIZE_SIG_REG 0x64
#define OUT_B_SIZE_SIG_REG 0x60
#define NUM_COLS_SIG_REG 0x5c
#define NUM_ROWS_SIG_REG 0x58
#define SRC_CHANS_SIG_REG 0x54
#define DST_CHANS_SIG_REG 0x50
#define DO_POOL_SIG_REG 0x4c
#define DO_PAD_SIG_REG 0x48
#define POOL_SIZE_SIG_REG 0x44
#define POOL_STRIDE_SIG_REG 0x40
#define DMA_RATIO 1


// Constants for the Entire Conv Layer
const int32_t in_base_addr_sig      [N_CONV_LAYERS] = {    0, 41324,  87564, 116428};
const int32_t in_w_base_addr_sig    [N_CONV_LAYERS] = { 3468, 78316,  97932, 122828};
const int32_t out_b_base_addr_sig   [N_CONV_LAYERS] = { 4332, 87532, 116364, 196556};
const int32_t out_base_addr_sig     [N_CONV_LAYERS] = {41324, 87564, 116428, 196684};

const int32_t in_size_sig           [N_CONV_LAYERS] = { 3468, 36992,  10368,   6400};
const int32_t in_w_size_sig         [N_CONV_LAYERS] = {  864,  9216,  18432,  73728};
const int32_t out_size_sig          [N_CONV_LAYERS] = {36992, 10368,   6400,   2048};
const int32_t out_b_size_sig        [N_CONV_LAYERS] = {   32,    32,     64,    128};
const int32_t num_cols_sig          [N_CONV_LAYERS] = {   34,    34,     18,     10};
const int32_t num_rows_sig          [N_CONV_LAYERS] = {   34,    34,     18,     10};
const int32_t src_chans_sig         [N_CONV_LAYERS] = {    3,    32,     32,     64};
const int32_t dst_chans_sig         [N_CONV_LAYERS] = {   32,    32,     64,    128};
const int32_t do_pool_sig           [N_CONV_LAYERS] = {    0,     1,      1,      1};
const int32_t do_pad_sig            [N_CONV_LAYERS] = {    1,     1,      1,      0};
const int32_t pool_size_sig         [N_CONV_LAYERS] = {   34,    18,     10,      4};
const int32_t pool_stride_sig       [N_CONV_LAYERS] = { 1156,   324,    100,     16};
const int32_t src_offset            [N_CONV_LAYERS] = {    0,    0,       0,      0};
const int32_t dst_offset            [N_CONV_LAYERS] = {    0,    0,       0,      0};

int main(int argc, char * argv[])
{

    int n;
    int ndev1, ndev2, ndev3, ndev4;

    //TODO: make device per each accelerator
    struct esp_device *espdevs1;
    struct esp_device *dev1;
    struct esp_device *espdevs2;
    struct esp_device *dev2;
    struct esp_device *espdevs3;
    struct esp_device *dev3;
    struct esp_device *espdevs4;
    struct esp_device *dev4;

    unsigned done;
    unsigned **ptable;
    token_t *mem;
    unsigned coherence;
    int num_correct = 0;

    //TODO: change this to match the tile index of the CPU tile
    const int CPU_TILE_IDX = 1;
    //ESP monitors variables
    esp_monitor_args_t mon_args;
    mon_args.read_mode = ESP_MON_READ_SINGLE;
    mon_args.tile_index = CPU_TILE_IDX;
    mon_args.mon_index = MON_DVFS_BASE_INDEX + 3;
    unsigned int cycle_start, cycle_end, cycle_diff;

    const char *labels[] = { "airplane",
                 "automobile",
                 "bird",
                 "cat",
                 "deer",
                 "dog",
                 "frog",
                 "horse",
                 "ship",
                 "truck" };

    const int gold_out[10]={3,5,2,6,9,0,1,4,8,7};

    unsigned mem_size = 200000;

    // Search for the device
    printf("Scanning device tree... \n");

    // TODO: Copy the below based on the target layer you want to integrate
    // Accelerator for target layer
    ndev1 = probe(&espdevs1, VENDOR_SLD, SLD_CONV4, DEV_NAME4);
    if (ndev1 == 0) {
        printf("conv accelerator not found\n");
        return 0;
    }
    printf("**************** %s.%d ****************\n", DEV_NAME1, 0);
    dev1 = &espdevs1[0];
    // Check DMA capabilities
    if (ioread32(dev1, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev1, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }

    //TODO:
    //Accelerator for the other layer
   /* ndev2 = probe(&espdevs2, VENDOR_SLD, SLD_CONV2, DEV_NAME2);
    if (ndev2 == 0) {
        printf("conv accelerator not found\n");
        return 0;
    }
    printf("**************** %s.%d ****************\n", DEV_NAME2, 0);
    dev2 = &espdevs2[0];
    // Check DMA capabilities
    if (ioread32(dev2, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev2, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }


    ndev3 = probe(&espdevs3, VENDOR_SLD, SLD_CONV3, DEV_NAME3);
    if (ndev3 == 0) {
        printf("conv accelerator not found\n");
        return 0;
    }
    printf("**************** %s.%d ****************\n", DEV_NAME3, 0);
    dev3 = &espdevs3[0];
    // Check DMA capabilities
    if (ioread32(dev3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }

    ndev4 = probe(&espdevs4, VENDOR_SLD, SLD_CONV4, DEV_NAME4);
    if (ndev4 == 0) {
        printf("conv accelerator not found\n");
        return 0;
    }
    printf("**************** %s.%d ****************\n", DEV_NAME4, 0);
    dev4 = &espdevs4[0];
    // Check DMA capabilities
    if (ioread32(dev4, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev4, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }*/


    // Allocate memory
    mem = aligned_malloc(sizeof(token_t) * mem_size);
    printf("  memory buffer base-address = %p\n", mem);

    // Alocate and populate page table
    ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
    for (int i = 0; i < NCHUNK(mem_size); i++)
        ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];

    printf("  ptable = %p\n", ptable);
    printf("  nchunk = %lu\n", NCHUNK(mem_size));

    /*  TODO: customize coherence mode, available options:
        *ACC_COH_NONE - non-coherent, always available
        *ACC_COH_LLC - llc-coherent, requires caches enabled
        *ACC_COH_RECALL - coherent DMA, requires caches enabled
        *ACC_COH_FULL - fully coherent, requires cachens enabled and L2 cache in acc tile
    */
    coherence = ACC_COH_NONE;

    printf("  --------------------\n");

//Dwarf-7 network description
    // Convolutional Layers
    int32_t n_channels         [N_CONV_LAYERS] = {    3,  32,  32,  64};
    int32_t feature_map_height [N_CONV_LAYERS] = {   34,  34,  18,  10};
    int32_t feature_map_width  [N_CONV_LAYERS] = {   34,  34,  18,  10};
    int32_t n_filters          [N_CONV_LAYERS] = {   32,  32,  64, 128};
    int32_t filter_dim         [N_CONV_LAYERS] = {    3,   3,   3,   3};
    int32_t is_padded          [N_CONV_LAYERS] = {    1,   1,   1,   0};
    int32_t stride             [N_CONV_LAYERS] = {    1,   1,   1,   1};
    int32_t dilation           [N_CONV_LAYERS] = {    1,   1,   1,   1};
    int32_t do_relu_conv       [N_CONV_LAYERS] = {    1,   1,   1,   1};
    int32_t pool_type          [N_CONV_LAYERS] = {    0,   1,   1,   1};
    int32_t pool_size          [N_CONV_LAYERS] = {   34,  18,  10,   4};
    int32_t pool_stride        [N_CONV_LAYERS] = { 1156, 324, 100,  16};
    int32_t batch_size         [N_CONV_LAYERS] = {    1,   1,   1,   1};

    // Fully Connected Layers
    int32_t do_relu_fc         [N_FC_LAYERS] = { 1, 0};
    int32_t soft               [N_FC_LAYERS] = { 0, 0};
    int32_t transpose          [N_FC_LAYERS] = { 1, 1};
    int32_t ninputs            [N_FC_LAYERS] = { 1, 1};
    int32_t d3                 [N_FC_LAYERS] = { 1, 1};
    int32_t d2                 [N_FC_LAYERS] = { 2048, 64};
    int32_t d1                 [N_FC_LAYERS] = { 64, 10};

    // Model and intermediate features allocation
    // Allocate the buffers to store layers input/output features, weights and biases.

    unsigned INPUT_SIZE = n_channels[0] * feature_map_height[0] * feature_map_width[0];

    float* input_conv;
    float** input_im = (float**) aligned_malloc(sizeof(float) * 10);
    float** w_conv = (float**) aligned_malloc(sizeof(float *) * N_CONV_LAYERS);
    float** bias_conv = (float**) aligned_malloc(sizeof(float *) * N_CONV_LAYERS);
    float** w_fc = (float**) aligned_malloc(sizeof(float *) * N_FC_LAYERS);
    float** bias_fc = (float**) aligned_malloc(sizeof(float *) * N_FC_LAYERS);
    float** output_conv = (float**) aligned_malloc(sizeof(float) * N_CONV_LAYERS);
    float** output_fc = (float**) aligned_malloc(sizeof(float) * N_FC_LAYERS);

    for (int i = 0;i < 10; i++)
        input_im[i] = (float*) aligned_malloc(sizeof(float) * INPUT_SIZE);

    for(int i = 0; i< N_CONV_LAYERS; i++){
        w_conv[i] = (float *) aligned_malloc(sizeof(float) * n_channels[i] * n_filters[i] * filter_dim[i] * filter_dim[i]);
        bias_conv[i] = (float *) aligned_malloc(sizeof(float) * n_filters[i]);
        output_conv[i] = (float *) aligned_malloc(sizeof(float)* n_filters[i] * feature_map_height[i] * feature_map_width[i]);
    }

    for (int i = 0; i< N_FC_LAYERS; i++) {
        w_fc[i] = (float *) aligned_malloc(sizeof(float) * d1[i] * d2[i]);
        bias_fc[i] = (float *) aligned_malloc(sizeof(float) * d1[i]);
        output_fc[i]= (float*) aligned_malloc(sizeof(float) * d1[i]);
    }

    // Model Load:
    // Load the model into the allocated buffers

    load_model(input_im, w_conv, bias_conv, w_fc, bias_fc);

    //Start the inference task for 10 input images
    for (int k=0; k<10; k++)
    {

        printf("######################################################\n");
        printf("########## Start Inference task for image %d ##########\n",k+1);
        printf("######################################################\n");

        if (k==0)
            cycle_start = esp_monitor(mon_args, NULL);

        for (int i=0; i<N_CONV_LAYERS; i++)
        {
            if (i==0){
                input_conv = input_im[k];
            }
            else {
                input_conv = output_conv[i-1];
            }

            // TODO: Copy the accelerator invocation code for each target layer
            if (i == TARGET_LAYER4){
                //convert floating point data to accelerator fixed point format
                //TODO: change FX_WL and FX_IL to match your accelerator datatype
                for (int j = 0; j < in_size_sig[i]; j++){
                    mem[in_base_addr_sig[i] + j] = float2fixed(input_conv[j], FX_WL, FX_IL);
                }

                for (int j = 0; j < in_w_size_sig[i]; j++){
                    mem[in_w_base_addr_sig[i] + j] = float2fixed(w_conv[i][j], FX_WL, FX_IL);
                }

                for (int j = 0; j < out_b_size_sig[i]; j++){
                    mem[out_b_base_addr_sig[i] + j] = float2fixed(bias_conv[i][j], FX_WL, FX_IL);
                }

                iowrite32(dev1, SELECT_REG, ioread32(dev1, DEVID_REG));
                iowrite32(dev1, COHERENCE_REG, coherence);
    #ifndef __sparc
                iowrite32(dev1, PT_ADDRESS_REG, (unsigned long long) ptable);
    #else
                iowrite32(dev1, PT_ADDRESS_REG, (unsigned) ptable);
    #endif
                iowrite32(dev1, PT_NCHUNK_REG, NCHUNK(mem_size));
                iowrite32(dev1, PT_SHIFT_REG, CHUNK_SHIFT);

                // Use the following if input and output data are not allocated at the default offsets
                iowrite32(dev1, SRC_OFFSET_REG, 0x0);
                iowrite32(dev1, DST_OFFSET_REG, 0x0);

                // Write accelerator-specific configuration parameters
                /* <<--regs-config-->> */
                iowrite32(dev1, IN_BASE_ADDR_SIG_REG, in_base_addr_sig[i]);
                iowrite32(dev1, IN_W_BASE_ADDR_SIG_REG, in_w_base_addr_sig[i]);
                iowrite32(dev1, OUT_B_BASE_ADDR_SIG_REG, out_b_base_addr_sig[i]);
                iowrite32(dev1, OUT_BASE_ADDR_SIG_REG, out_base_addr_sig[i]);
                iowrite32(dev1, IN_SIZE_SIG_REG, in_size_sig[i]);
                iowrite32(dev1, IN_W_SIZE_SIG_REG, in_w_size_sig[i]);
                iowrite32(dev1, OUT_SIZE_SIG_REG, out_size_sig[i]);
                iowrite32(dev1, OUT_B_SIZE_SIG_REG, out_b_size_sig[i]);
                iowrite32(dev1, NUM_COLS_SIG_REG, num_cols_sig[i]);
                iowrite32(dev1, NUM_ROWS_SIG_REG, num_rows_sig[i]);
                iowrite32(dev1, SRC_CHANS_SIG_REG, src_chans_sig[i]);
                iowrite32(dev1, DST_CHANS_SIG_REG, dst_chans_sig[i]);
                iowrite32(dev1, DO_POOL_SIG_REG, do_pool_sig[i]);
                iowrite32(dev1, DO_PAD_SIG_REG, do_pad_sig[i]);
                iowrite32(dev1, POOL_SIZE_SIG_REG, pool_size_sig[i]);
                iowrite32(dev1, POOL_STRIDE_SIG_REG, pool_stride_sig[i]);

                /*Flush caches depending on coherence mode, do not change here
                    *flushes triggered by mode:
                    *ACC_COH_NONE - flushes L2 and LLC caches
                    *ACC_COH_LLC - flushes L2 caches
                    *ACC_COH_RECALL - no flush required
                    *ACC_COH_FULL - no flush required
                */
                esp_flush(coherence);

                // Start accelerators
                printf(" ######## CONV LAYER %d : Run Accelerator ########", i+1);
                iowrite32(dev1, CMD_REG, CMD_MASK_START);

                // Wait for completion
                done = 0;
                while (!done) {
                    done = ioread32(dev1, STATUS_REG);
                    done &= STATUS_MASK_DONE;
                }
                iowrite32(dev1, CMD_REG, 0x0);
                for (int j = 0; j < out_size_sig[i]; j++){
                    output_conv[i][j] = fixed2float(mem[out_base_addr_sig[i]+j], FX_WL, FX_IL);
                }
            }
            else {
                printf("\n\n ######## CONV LAYER %d : SW Programmer view ########", i+1);

                // Initialize output_conv to 0 for the convolution_compute pv
                int32_t out_len = n_filters[i]*feature_map_height[i]*feature_map_width[i];
                for (int j=0; j<out_len; j++){
                    output_conv[i][j] = 0;
                }

                convolution_compute(output_conv[i], bias_conv[i], input_conv, w_conv[i],
                                    feature_map_height[i], feature_map_width[i], n_channels[i],
                                    n_filters[i], pool_size[i], pool_stride[i], pool_type[i], is_padded[i]);
                printf("\n\n ######## CONV LAYER %d : SW Programmer view done ########", i+1);
            }
        }

        for (int i=0; i<N_FC_LAYERS; i++) {
            float* input_fc;
            if (i==0){
                input_fc = output_conv[3];
            }
            else{
                input_fc = output_fc[i-1];
            }

            printf("\n\n ######## FC LAYER %d : SW Programmer view ########",i+1);
            fc_compute(output_fc[i], input_fc, w_fc[i], bias_fc[i], d2[i], d1[i], do_relu_fc[i]);
        }

        if (k == 0)
            cycle_end = esp_monitor(mon_args, NULL);

        //Compute classication results
        printf("\n \n Output results: \n\n");
        for (int i = 0; i < d1[N_FC_LAYERS-1]; i++ ) {
            printf(" Class %d (%s): %d \n", i, labels[i], (int) ((output_fc[N_FC_LAYERS-1][i])*100)); 
        }

        int max=-500;
        int max2=-500;
        int id_max,id_max2=0;
        for (int i = 0; i < d1[N_FC_LAYERS-1]; i++ ) {
            if ((int) ((output_fc[N_FC_LAYERS-1][i])*100) > max) {
                max2=max;
                id_max2=id_max;
                max=(int) ((output_fc[N_FC_LAYERS-1][i])*100);
                id_max=i;
            }
        }

        printf("---------------------------------");

        printf("\n Classification result #1: %s \n",labels[id_max]);
        printf(" Classification result #2: %s \n",labels[id_max2]);

        printf("---------------------------------\n");

        if (id_max == gold_out[k])
            num_correct+=1;
    }

    printf("\n\n -> RESULTS:");
    printf("\n---------------------------------");
    printf("\n TEST ACCURACY: %d/10", num_correct);

    cycle_diff = sub_monitor_vals(cycle_start, cycle_end);
    printf("\n Single Inference execution time: %d \n", cycle_diff);
    printf("---------------------------------");

    aligned_free(ptable);
    aligned_free(mem);

    return 0;
}