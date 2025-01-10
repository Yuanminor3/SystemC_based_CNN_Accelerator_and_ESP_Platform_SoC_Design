#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>

#include "mojo.h"
#include "dwarf.h"
#include "system.hpp"

#include "accelerated_sim.hpp"

std::ofstream ofs;

// Mojo network
mojo::network *cnn;

#include "conv_layer_specs.hpp"

#define IMAGE_SIZE 3 * 32 * 32

// -- Utilities

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)


template<typename T, size_t FP_WL>
void system_t::load_fpdata(float *in, uint32_t base_address,
                           uint32_t size, uint32_t actual_size)
{
    for (uint32_t i = 0; i < size ; i++)
    {
        ac_int<DMA_WIDTH> data_dma;

        for (int word = 0; word < 1; word++)
        {
            const uint32_t index = i + word;
            if (index >= actual_size)
            { break; }

            ac_ieee_float32 data = in[index];

#ifdef FL_POINT
            FPDATA fpdata=data;
#else
            FPDATA fpdata=data.convert_to_ac_fixed<FPDATA_WL,FPDATA_IL,true,AC_TRN, AC_WRAP>();
#endif
            FPDATA_WORD fpdata_word;
            f2int(fpdata, fpdata_word);

            data_dma.set_slc(word*FPDATA_WL,fpdata_word);
        }
        memory[base_address  + i] = data_dma;

    }
}

// sc_bv -> FPDATA -> float
void system_t::convert_output(int target_layer)
{
    float *output = cnn->layer_sets[target_layer]->node.x;

#ifdef RUN_ACCELERATED_SIM
    std::string acc_file_name = "accelerated_test_syn_" + std::to_string(TARGET_LAYER) + ".txt";
    ofs.open(acc_file_name, std::ofstream::out);
#elif  FL_POINT
    std::string acc_fp_file_name = "test" + std::to_string(TARGET_LAYER) + ".txt";
    ofs.open(acc_fp_file_name, std::ofstream::out);
#endif

    for (uint32_t i = 0; i < o_size ; i++)
    {
        ac_int<DMA_WIDTH> data_dma = memory[o_addr + i];

        for (uint32_t word = 0; word < 1; word++)
        {
            const uint32_t index =  i + word;

            if (index >= actual_o_size)
            { break; }

            FPDATA_WORD data_bv = data_dma.slc<FPDATA_WL>(word * FPDATA_WL);
            FPDATA data_fp;
            bv2fp(data_bv, data_fp);

#ifdef FL_POINT
            float data_float=data_fp;
#else
            double data_double=data_fp.to_double();
            float data_float=(float) data_double;
#endif
            output[index] = data_float;



#ifdef RUN_ACCELERATED_SIM
            // Print meaningful partial results for validation of
            // accelerated_test
            // Compare Fixed-point behavioral (accelerated_test_syn.txt)
            // with floating point behavioral (accelerated_test.txt)
            if (index < pool_size * pool_size * reduced_chans)
            { ofs << index << ": " << output[index] << std::endl; };
#elif FL_POINT
            if (index < pool_size * pool_size * dst_chans)
            { ofs << index << ": " << std::round(output[index]*100000.0)/100000.0 << std::endl; };
#endif

        }
    }

#ifdef RUN_ACCELERATED_SIM
    ofs.close();
#elif FL_POINT
    ofs.close();
#endif

}

void system_t::run_pv(int layer, bool fully_connected = false)
{
    if (fully_connected)
        fc_compute(cnn->layer_sets[layer]->node.x,
                   cnn->layer_sets[layer - 1]->node.x,
                   cnn->W[layer - 1]->x,
                   cnn->layer_sets[layer]->bias.x,
                   cnn->W[layer - 1]->cols,
                   cnn->W[layer - 1]->rows,
                   cnn->layer_sets[layer]->relu);
    else
        convolution_compute(cnn->layer_sets[layer]->node.x,
                            cnn->layer_sets[layer]->bias.x,
                            cnn->layer_sets[layer - 1]->node.x,
                            cnn->W[layer - 1]->x,
                            cnn->layer_sets[layer]->node.cols,
                            cnn->layer_sets[layer]->node.rows,
                            cnn->layer_sets[layer - 1]->node.chans,
                            cnn->layer_sets[layer]->node.chans,
                            get_pool_size(cnn->layer_sets[layer]->node.cols,
                                          cnn->layer_sets[layer]->node.rows,
                                          cnn->layer_sets[layer]->do_pool,
                                          cnn->layer_sets[layer]->do_pad),
                            get_pool_stride(cnn->layer_sets[layer]->node.cols,
                                            cnn->layer_sets[layer]->node.rows,
                                            cnn->layer_sets[layer]->do_pool,
                                            cnn->layer_sets[layer]->do_pad),
                            cnn->layer_sets[layer]->do_pool,
                            cnn->layer_sets[layer]->do_pad);
}

void system_t::move_weights(int target_layer, bool fully_connected)
{
    float *w = cnn->W[target_layer - 1]->x;
    int src_chans;
    int dst_chans;
    int r = reduced_chans;

    if (fully_connected)
    {
        src_chans = cnn->W[target_layer - 1]->cols;
        dst_chans = cnn->W[target_layer - 1]->rows;

        for (int i = 1; i < r; i++)
            for (int j = 1; j < r; j++)
            { w[r * i + j] = w[src_chans * i + j]; }
    }
    else
    {
        src_chans = cnn->layer_sets[target_layer - 1]->node.chans;
        dst_chans = cnn->layer_sets[target_layer]->node.chans;

        int index = 0;
        for (int src_chan = 0; src_chan < src_chans; src_chan++) {
            for (int dst_chan = 0; dst_chan < r; dst_chan++) {
                for (int k = 0; k < 9; k++) {
                    w[index * 9 + k] = w[(src_chan * dst_chans + dst_chan) * 9 + k];
                }
                index++;
            }
        }
    }
}

void system_t::set_configuration_param(int target_layer, bool fully_connected)
{
    // ======================   ^  <------------- i_addr
    // |       input        |   | i_size
    // ======================   -  <------------- w_addr
    // |    input weights   |   | w_size
    // ======================   -  <------------- b_addr
    // |     output bias    |   | o_size
    // ======================   -  <------------- o_addr
    // |       output       |   | o_size
    // ======================   v

    if (fully_connected)
    {
        w_cols = cnn->W[target_layer - 1]->cols;
        w_rows = cnn->W[target_layer - 1]->rows;

        actual_i_size = w_cols;
        i_size = round_up(actual_i_size, DMA_ADJ);

        actual_w_size = w_cols * w_rows;
        w_size = round_up(actual_w_size, DMA_ADJ);

        actual_b_size = w_rows;
        b_size = round_up(actual_b_size, DMA_ADJ);

        actual_o_size = w_rows;
        o_size = round_up(actual_o_size, DMA_ADJ);
    }
    else
    {
        cols = cnn->layer_sets[target_layer]->node.cols;
        rows = cnn->layer_sets[target_layer]->node.rows;

        src_chans = cnn->layer_sets[target_layer - 1]->node.chans;
        dst_chans = cnn->layer_sets[target_layer]->node.chans;

        do_pool = cnn->layer_sets[target_layer]->do_pool;
        do_pad = cnn->layer_sets[target_layer]->do_pad;
        pool_size = get_pool_size(cols, rows, do_pool, do_pad);
        pool_stride = get_pool_stride(cols, rows, do_pool, do_pad);

        actual_i_size = cols * rows * src_chans;
        i_size = round_up(actual_i_size, DMA_ADJ);

        actual_w_size = 9 * src_chans * dst_chans;
        w_size = round_up(actual_w_size, DMA_ADJ);

        actual_b_size = dst_chans;
        b_size = round_up(actual_b_size, DMA_ADJ);

        //
        // WARNING: the convolution requires an output memory as large as the input memory
        //          to place temporary results. After performing max_pooling, however, the
        //          actual output is much smaller (pool_size), thus we can save time on
        //          DMA transactions by only transfering useful data.
        //
        actual_o_size = cols * rows * dst_chans;
        o_size = round_up(actual_o_size, DMA_ADJ);
    }

    // Compute memory mapping
    i_addr = 0;
    w_addr = i_size;
    b_addr = i_size + w_size;
    o_addr = i_size + w_size + b_size;
}

// -- Functions

conf_info_t system_t::load_regs(bool fully_connected)
{

    conf_info_t config;

    /* <<--params-->> */
    config.in_base_addr_sig = i_addr ;
    config.in_w_base_addr_sig = w_addr ;
    config.out_b_base_addr_sig = b_addr ;
    config.out_base_addr_sig = o_addr ;
    config.in_size_sig= i_size;
    config.in_w_size_sig= w_size;
    config.out_size_sig= o_size;
    config.out_b_size_sig= b_size;
    config.num_cols_sig=cols;
    config.num_rows_sig=rows;
#ifdef RUN_ACCELERATED_SIM
    config.src_chans_sig=reduced_chans;
    config.dst_chans_sig=reduced_chans;
#else
    config.src_chans_sig=src_chans;
    config.dst_chans_sig=dst_chans;
#endif
    config.do_pool_sig=do_pool;
    config.do_pad_sig=do_pad;
    config.pool_size_sig=pool_size;
    config.pool_stride_sig=pool_stride;

    return config;
}


void system_t::setup_memory(int target_layer, bool fully_connected)
{
    wait();

    CCS_LOG("Software inference completed");

    CCS_LOG("Testing target layer "<< target_layer);

    // Compact weights in memory if running accelerated RTL test
#ifdef RUN_ACCELERATED_SIM
    reduced_chans = REDUCED_CHANS(target_layer);
    move_weights(target_layer, fully_connected);
#endif

    // Determine configuration parameters from CNN model
    set_configuration_param(target_layer, fully_connected);


    CCS_LOG("config_param ");

    // ======================   ^  <------------- i_addr
    // |       input        |   | i_size
    // ======================   -  <------------- w_addr
    // |    input weights   |   | w_size
    // ======================   -  <------------- b_addr
    // |     output bias    |   | o_size
    // ======================   -  <------------- o_addr
    // |       output       |   | o_size
    // ======================   v

    // memory->data = new ac_int<DATA_WIDTH>[
     // (i_size + w_size + o_size + b_size)];

#ifdef FL_POINT
    load_fpdata<FPDATA, 32>(cnn->layer_sets[target_layer - 1]->node.x,
                            i_addr, i_size, actual_i_size);
    load_fpdata<FPDATA, 32>(cnn->W[target_layer - 1]->x,
                            w_addr, w_size, actual_w_size);
    load_fpdata<FPDATA, 32>(cnn->layer_sets[target_layer]->bias.x,
                            b_addr, b_size, actual_b_size);
#else
    load_fpdata<FPDATA, FPDATA_WL>(cnn->layer_sets[target_layer - 1]->node.x,
                                   i_addr, i_size, actual_i_size);
    load_fpdata<FPDATA, FPDATA_WL>(cnn->W[target_layer - 1]->x,
                                   w_addr, w_size, actual_w_size);
    load_fpdata<FPDATA, FPDATA_WL>(cnn->layer_sets[target_layer]->bias.x,
                                   b_addr, b_size, actual_b_size);
#endif

    CCS_LOG("Load memory completed" << target_layer );

    wait();
}

void system_t::load_memory(void)
{
    // -- Load model

    CCS_LOG("Configuration");

    cnn = new mojo::network();
    assert(cnn->read(model_path));

    CCS_LOG("DWARF7 model loaded");

    // -- Read image

    float *image = new float[IMAGE_SIZE];
    std::ifstream infile(image_path.c_str(), std::ifstream::binary);
    assert(infile.read((char *) image, IMAGE_SIZE * sizeof(float)));
    cnn->forward_setup(image);
    delete[] image;

    CCS_LOG("DWARF7 image: " << image_path.c_str());

    //
    // Run inference
    //

    // Input layer -> nothing to be done
    // input 34x34 (including pad of 2 pixels), 3 channels

    //
    // Convolution type 1
    //
// #ifndef TARGET_LAYER_1
#if TARGET_LAYER != 1
    // convolution 34x34 (including pad of 2 pixels), kernel
    //  size 3x3, 32 output channels, relu
    run_pv(1);

    //
    // Convolution type 2
    //
// #ifndef TARGET_LAYER_2
#if TARGET_LAYER != 2
    // convolution 34x34 (including pad of 2 pixels), kernel
    // size 3x3, 32 output channels, relu, max pooling 2x2 (pad on output -> 18x18)
    run_pv(2);

    //
    // Convolution type 3
    //
// #ifndef TARGET_LAYER_3
#if TARGET_LAYER != 3
    // convolution 18x18 (including pad of 2 pixels), kernel
    //  size 3x3, 64 output channels, relu, max pooling 2x2 (pad on output -> 10x10)
    run_pv(3);

    //
    // Convolution type 4
    //
// #ifndef TARGET_LAYER_4
#ifndef TARGET_LAYER != 4
    // convolution 10x10 (including pad of 2 pixels), kernel
    //  size 3x3, 128 output channels, relu, max pooling 2x2 (no pad on output -> 4x4)
    run_pv(4);

    //
    // Fully Connected
    //
// #ifndef TARGET_LAYER_5
#if TARGET_LAYER != 5
    // fully_connected 1x1, 2048 to 64 channels, relu
    run_pv(5, true);

// #ifndef TARGET_LAYER_6
#if TARGET_LAYER != 6
    // fully_connected 1x1, 64 to 10 channels, brokemax
    run_pv(6, true);

#else
    setup_memory(6, true);
#endif // TARGET_LAYER_6
#else
    setup_memory(5, true);
#endif // TARGET_LAYER_5
#else
    setup_memory(4, false);
#endif // TARGET_LAYER_4
#else
    setup_memory(3, false);
#endif // TARGET_LAYER_3
#else
    setup_memory(2, false);
#endif // TARGET_LAYER_2
#else
    setup_memory(1, false);
#endif // TARGET_LAYER_1

}

void system_t::dump_memory(void)
{
// #ifdef TARGET_LAYER_6
#if TARGET_LAYER == 6
    convert_output(6);
#else
// #ifdef TARGET_LAYER_5
#if TARGET_LAYER == 5
    convert_output(5);
#else
// #ifdef TARGET_LAYER_4
#if TARGET_LAYER == 4
    convert_output(4);
#else
// #ifdef TARGET_LAYER_3
#if TARGET_LAYER == 3
    convert_output(3);
#else
// #ifdef TARGET_LAYER_2
#if TARGET_LAYER == 2
    convert_output(2);
#else
// #ifdef TARGET_LAYER_1
#if TARGET_LAYER == 1
    convert_output(1);
#else
    run_pv(1, false);
#endif // TARGET_LAYER_1
    run_pv(2, false);
#endif // TARGET_LAYER_2
    run_pv(3, false);
#endif // TARGET_LAYER_3
    run_pv(4, false);
#endif // TARGET_LAYER_4
    run_pv(5, true);
#endif // TARGET_LAYER_5
    run_pv(6, true);
#endif // TARGET_LAYER_6

    //
    // Inference Complete
    //

    CCS_LOG("Dump memory completed");
}

#define ERROR_THRESHOLD 0.01

void system_t::validate(void)
{
    int first, second;
    int tot_errors = 0;
    std::stringstream stm;

    // -- This is the final output of the DWARF7
    float *output = cnn->layer_sets[6]->node.x;

    CCS_LOG("Inference completed");

    first = mojo::arg_max(output, cnn->out_size());
    CCS_LOG("#1: "<< labels[first]<<" "<< output[first]);

    output[first] = -1; // find the next best
    second = mojo::arg_max(output, cnn->out_size());
    CCS_LOG("#2: "<< labels[second]<<" "<< output[second]);

    size_t last_dot_pos = image_path.find_last_of('.');
    std::string file_name = (last_dot_pos == std::string::npos)
                            ? image_path
                            : image_path.substr(0, last_dot_pos);
    if (file_name == labels[first])
    {
        CCS_LOG("------------------------------------");
        CCS_LOG("  Validation succeeded!  ");
        CCS_LOG("------------------------------------");
    }
    else
    {
        CCS_LOG("------------------------------------");
        CCS_LOG("  Validation failed!  ");
        CCS_LOG("------------------------------------");
    }
}

void system_t::clean_up(void)
{
    delete cnn;
}