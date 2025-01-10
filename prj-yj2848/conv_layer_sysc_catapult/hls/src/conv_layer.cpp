/* Copyright 2022 Columbia University SLD Group */

#include "conv_layer.hpp"

// -- Functions (utils)

#include "conv_layer_utils.hpp"

// -- Functions (kernel)

#include "conv_layer_functions.hpp"

#define ALIGNMENT_MASK ((DATA_WIDTH / 8) - 1)

// -- Processes
void conv_layer_sysc_catapult::config(void) {  

    conf_info.Reset();
    conf_info_chan1.ResetWrite();

    conf_info_chan2b.ResetWrite();
    conf_info_chan3b.ResetWrite();


    sync01.ResetWrite();
    sync02b.ResetWrite();
    sync03b.ResetWrite();


    wait();

    while (1) {
        conf_info_t cmd = conf_info.Pop();

        conf_info_chan1.Push(cmd);
        conf_info_chan2b.Push(cmd);
        conf_info_chan3b.Push(cmd);

        sync01.Push(1);
        sync02b.Push(1);
        sync03b.Push(1);

    }
}


void conv_layer_sysc_catapult::load_input(void)
{
    bool pingpong = true;
    dma_read_chnl.Reset();
    dma_read_ctrl.Reset();
    conf_info_chan1.ResetRead();
    sync01.ResetRead();
    sync12b.reset_sync_out();

    wait();

    while (true)
    {
        sync01.Pop();

        conf_info_t cmd = conf_info_chan1.Pop();

        uint32_t in_base_addr    = cmd.in_base_addr_sig;
        uint32_t in_w_base_addr  = cmd.in_w_base_addr_sig;
        uint32_t out_b_base_addr = cmd.out_b_base_addr_sig;
        uint32_t num_cols        = cmd.num_cols_sig;
        uint32_t num_rows        = cmd.num_rows_sig;
        uint32_t src_chans       = cmd.src_chans_sig;
        uint32_t dst_chans       = cmd.dst_chans_sig;

        // Load

        {
            int in_index;
            int in_w_index;

            const int in_len = num_cols * num_rows;
            const int in_w_len = 9;

            load_b(out_b_base_addr, dst_chans);
            #if defined(FAST)
                #pragma unroll yes
            #endif
			
            for (int dst_chan = 0; dst_chan < dst_chans; dst_chan++)
            {
                #if defined(FAST)
                    #pragma unroll yes
                #endif
                for (int src_chan = 0; src_chan < src_chans; src_chan++)
                {

                    // Update input offset
                    in_index = in_base_addr + (src_chan * num_cols * num_rows );

                    // Update weights offset
                    in_w_index = in_w_base_addr + (9 * (src_chan * dst_chans + dst_chan) );

                    if (pingpong)
                    {
                        load_d(1, in_index, in_len);
                        load_w(1, in_w_index, in_w_len,src_chan);
                    }
                    else
                    {
                        load_d(0, in_index, in_len);
                        load_w(0, in_w_index, in_w_len,src_chan);
                    }

                    pingpong = !pingpong;
                    sync12b.sync_out();

                }
            }
        }
    }
}


void conv_layer_sysc_catapult::compute_kernel(void)
{
    bool pingpong = true;
    bool out_pingpong = true;

    conf_info_chan2b.ResetRead();
    sync02b.ResetRead();

    sync12b.reset_sync_in();
    sync2b3b.reset_sync_out();

    wait();

    while (true)
    {
        sync02b.Pop();

        conf_info_t cmd = conf_info_chan2b.Pop();

        bool do_pad          = cmd.do_pad_sig;
        bool do_pool         = cmd.do_pool_sig;
        uint32_t num_cols        = cmd.num_cols_sig;
        uint32_t num_rows        = cmd.num_rows_sig;
        uint32_t src_chans       = cmd.src_chans_sig;
        uint32_t dst_chans       = cmd.dst_chans_sig;
        uint32_t pool_size       = cmd.pool_size_sig;
        uint32_t pool_stride     = cmd.pool_stride_sig;

        // Compute
        #if defined(FAST)
            #pragma unroll yes
        #endif
        for (int dst_chan = 0; dst_chan < dst_chans; dst_chan++)
        {
            #if defined(FAST)
                #pragma unroll yes
            #endif
            for (int src_chan = 0; src_chan < src_chans; src_chan++)
            {
                // Handshake with load_input
                sync12b.sync_in();


                // Execute the computational kernel
                convolution_compute(num_cols, num_rows,
                                    src_chans, dst_chans, src_chan, dst_chan,
                                    do_pool, do_pad, pool_size, pool_stride,
                                    pingpong, out_pingpong);

                if (src_chan ==src_chans -1)
                {
                    // Execute pooling and padding
                    poolpad_compute(num_cols, num_rows,
                                     src_chans, dst_chans, src_chan, dst_chan,
                                     do_pool, do_pad, pool_size, pool_stride,
                                     pingpong, out_pingpong);
                }

                // Update pingpong var
                pingpong = !pingpong;
            }

            // Flip out_pingpong variable
            out_pingpong = !out_pingpong;

            sync2b3b.sync_out();

        }
    }
}



void conv_layer_sysc_catapult::store_output(void)
{
    bool out_pingpong = true;

    dma_write_chnl.Reset();
    dma_write_ctrl.Reset();

    sync2b3b.reset_sync_in();
    sync03b.ResetRead();

    conf_info_chan3b.ResetRead();

    acc_done.write(false);
    wait();

    while (true)
    {
        sync03b.Pop();

        conf_info_t cmd = conf_info_chan3b.Pop();

        uint32_t out_base_addr   = cmd.out_base_addr_sig;
        uint32_t out_size        = cmd.out_size_sig;
        uint32_t dst_chans       = cmd.dst_chans_sig;
        uint32_t pool_size       = cmd.pool_size_sig;

        const int out_len = pool_size * pool_size;
        #if defined(FAST)
            #pragma unroll yes
        #endif
        for (int dst_chan = 0; dst_chan < dst_chans; dst_chan++)
        {

            const int out_index_offset = dst_chan * pool_size * pool_size ;
            int out_index = out_base_addr + out_index_offset;

            sync2b3b.sync_in();

            if (out_pingpong)
                store_data(1, out_index, out_len);
            else
                store_data(0, out_index, out_len);

            out_pingpong = !out_pingpong;
        }

        acc_done.write(true); wait();
        acc_done.write(false);
    }
}
