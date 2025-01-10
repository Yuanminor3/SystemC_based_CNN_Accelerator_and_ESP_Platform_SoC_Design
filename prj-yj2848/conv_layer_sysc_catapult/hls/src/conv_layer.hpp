/* Copyright 2022 Columbia University SLD Group */
#pragma once

#include "conv_layer_specs.hpp"
#include "utils.hpp"
#include "conv_layer_conf_info.hpp"
#include "ac_shared_bank_array.h"

#ifndef __CONV_LAYER_HPP__
#define __CONV_LAYER_HPP__

#pragma once

#pragma hls_design top
SC_MODULE(conv_layer_sysc_catapult)
{
    public:
        // -- Input ports
        sc_in<bool> clk; // Clock signal
        sc_in<bool> rst; // Reset signal

        // -- Output ports
        sc_out<bool> CCS_INIT_S1(acc_done);

        Connections::In<conf_info_t>  CCS_INIT_S1(conf_info);
        Connections::In< ac_int<DMA_WIDTH> >  CCS_INIT_S1(dma_read_chnl);
        Connections::Out< ac_int<DMA_WIDTH> > CCS_INIT_S1(dma_write_chnl);
        Connections::Out<dma_info_t> CCS_INIT_S1(dma_read_ctrl);
        Connections::Out<dma_info_t> CCS_INIT_S1(dma_write_ctrl);

        // -- Constructor
        SC_CTOR(conv_layer_sysc_catapult)
        {
           // Register the threads
           SC_THREAD(config);
           sensitive << clk.pos();
           async_reset_signal_is(rst, false);

           SC_THREAD(load_input);
           sensitive << clk.pos();
           async_reset_signal_is(rst, false);

           SC_THREAD(compute_kernel);
           sensitive << clk.pos();
           async_reset_signal_is(rst, false);

           SC_THREAD(store_output);
           sensitive << clk.pos();
           async_reset_signal_is(rst, false);
        }

    private:
        // -- Processes
        void config(void);
        void load_input(void); // Load input from memory
        void compute_kernel(void); // Perform the computation
        void store_output(void); // Store output in memory

        // -- Functions (kernel)
        inline void convolution_compute(uint32_t num_cols, uint32_t num_rows, uint32_t src_chans, uint32_t dst_chans, int src_chan, int dst_chan, bool do_pool, bool do_pad, uint32_t pool_size, uint32_t pool_stride, bool pingpong, bool out_pingpong);
        inline void poolpad_compute(uint32_t num_cols, uint32_t num_rows, uint32_t src_chans, uint32_t dst_chans, int src_chan, int dst_chan, bool do_pool, bool do_pad, uint32_t pool_size, uint32_t pool_stride, bool pingpong, bool out_pingpong);

        // To read the input data
        inline void load_d(bool ping, uint32_t base_addr, uint32_t size);
        inline void load_w(bool ping, uint32_t base_addr, uint32_t size,uint32_t chan);
        inline void load_b(uint32_t base_addr, uint32_t size);

        // To store the output data
        inline void store_data(bool ping, uint32_t base_addr, uint32_t size);

        // Instantiate SyncChannel for handshakes across threads
        Connections::SyncChannel CCS_INIT_S1(sync12b);
        Connections::SyncChannel CCS_INIT_S1(sync2b3b);

        Connections::Combinational<bool> CCS_INIT_S1(sync01);
        Connections::Combinational<bool> CCS_INIT_S1(sync02b);
        Connections::Combinational<bool> CCS_INIT_S1(sync03b);

        // -- CSRs for configuration information
        conf_info_t cmd;

        // Communication Channels between config and other processes
        Connections::Combinational<conf_info_t> CCS_INIT_S1(conf_info_chan1);
        Connections::Combinational<conf_info_t> CCS_INIT_S1(conf_info_chan2b);
        Connections::Combinational<conf_info_t> CCS_INIT_S1(conf_info_chan3b);

        // PLMs declarations
#if defined(FAST)
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_pong;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_pong;
        ac_shared_bank_array_2D<DATA_TYPE, bbks, bebks> output_b;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_ping;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_pong;
#elif defined(SMALL)
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_pong;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_pong;
        ac_shared_bank_array_2D<DATA_TYPE, bbks, bebks> output_b;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_ping;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_pong;
		
#elif defined(MEDIUM)
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inbks, inebks> input_pong;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_ping;
        ac_shared_bank_array_2D<DATA_TYPE, inwbks, inwebks> input_w_pong;
        ac_shared_bank_array_2D<DATA_TYPE, bbks, bebks> output_b;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_ping;
        ac_shared_bank_array_2D<DATA_TYPE, outbks, outebks> output_pong;

#endif
};

#endif /* __CONV_LAYER_HPP__ */