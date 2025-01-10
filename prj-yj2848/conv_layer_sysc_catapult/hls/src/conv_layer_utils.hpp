/* Copyright 2022 Columbia University SLD Group */

#ifndef __CONV_LAYER_UTILS_HPP__
#define __CONV_LAYER_UTILS_HPP__

#include "conv_layer.hpp"

void conv_layer_sysc_catapult::load_d(bool ping, uint32_t base_addr, uint32_t size)
{
    uint32_t index = 0;
    uint32_t mem_index = 0;
    uint32_t bank_index = 0;
    uint32_t mem_off = base_addr;
	#pragma hls_resource core=add limit=1
    #if defined(FAST)
        #pragma unroll yes
    #endif 
    for (index = size; index > 0; )
    {
        uint32_t j = 0;
        uint32_t beats = (index < 256) ? index : 256;
        uint32_t len = beats;

        dma_info_t dma_info(mem_off, len, DMA_SIZE);
        dma_read_ctrl.Push(dma_info);

        const uint32_t data_mask = (((uint32_t) 1) << FPDATA_WL) - 1;
		#pragma hls_unroll factor=2
		#pragma hls_logic_opt
		#pragma hls_resource core=add limit=1
        for (; j < beats; ++j)
        {
            DMA_WORD r = dma_read_chnl.Pop();

            FPDATA t;
            FPDATA_WORD r1 = data_mask & r;
            int2f(r1, t);
#if defined(FAST)
            if (ping)
                input_ping[0][mem_index++] = data_mask & r;
            else
                input_pong[0][mem_index++] = data_mask & r;
#endif

#if defined(SMALL)
			if (ping)
                input_ping[0][mem_index++] = data_mask & r;
            else
                input_pong[0][mem_index++] = data_mask & r;
                //input_ping[0][mem_index++] = data_mask & r;
#endif
#if defined(MEDIUM)
                //input_ping[0][mem_index++] = data_mask & r;
			if (ping)
                input_ping[0][mem_index++] = data_mask & r;
            else
                input_pong[0][mem_index++] = data_mask & r;
#endif             
        }

        mem_off += beats;
        index -= beats;

        wait();
    }
}

void conv_layer_sysc_catapult::load_b(uint32_t base_addr, uint32_t size)
{
    uint32_t index = 0;
    uint32_t mem_index = 0;
    uint32_t mem_off = base_addr;
	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
	#pragma hls_resource core=add limit=1
    for (index = size; index > 0; )
    {
        uint32_t j = 0;
        uint32_t beats = (index < 256) ? index : 256;
        uint32_t len = beats;

        dma_info_t dma_info(mem_off, len, DMA_SIZE);
        dma_read_ctrl.Push(dma_info);
        const uint32_t data_mask = (((uint32_t) 1) << FPDATA_WL) - 1;
		#pragma hls_unroll factor=2
		#pragma hls_logic_opt
		#pragma hls_resource core=add limit=1
        for (j = 0; j < beats; ++j)
        {
            DMA_WORD r = dma_read_chnl.Pop();
            output_b[0][mem_index++] = data_mask & r;
        }

        mem_off += beats;
        index -= beats;

        wait();
    }
}

void conv_layer_sysc_catapult::load_w(bool ping, uint32_t base_addr, uint32_t size, uint32_t chan)
{
    uint32_t index = 0;
    uint32_t bank_index = 0;
    uint32_t mem_index = 0;
    uint32_t mem_off = base_addr;
	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
	#pragma hls_resource core=add limit=1
    for (index = size; index > 0; )
    {
        uint32_t j = 0;
        uint32_t beats = (index < 256) ? index : 256;
        uint32_t len = beats;

        dma_info_t dma_info(mem_off, len, DMA_SIZE);
        dma_read_ctrl.Push(dma_info);

        const uint32_t data_mask = (((uint32_t) 1) << FPDATA_WL) - 1;
		#pragma hls_unroll factor=2
		#pragma hls_logic_opt
		#pragma hls_resource core=add limit=1
        for (; j < beats; ++j)
        {
            DMA_WORD r = dma_read_chnl.Pop();
#if defined(FAST)
            if (ping)
                input_w_ping[0][mem_index++] = data_mask & r;
            else
                input_w_pong[0][mem_index++] = data_mask & r;
#endif
#if defined(SMALL)
                //input_w_ping[0][mem_index++] = data_mask & r;
			if (ping)
                input_w_ping[0][mem_index++] = data_mask & r;
            else
                input_w_pong[0][mem_index++] = data_mask & r;
#endif
#if defined(MEDIUM)
                //input_w_ping[0][mem_index++] = data_mask & r;
			if (ping)
                input_w_ping[0][mem_index++] = data_mask & r;
            else
                input_w_pong[0][mem_index++] = data_mask & r;
#endif           
        }

        mem_off += beats;
        index -= beats;

        wait();
    }
}

void conv_layer_sysc_catapult::store_data(bool ping, uint32_t base_addr, uint32_t size)
{
    uint32_t index = 0;
    uint32_t mem_index = 0;
    uint32_t mem_off = base_addr;
	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
	#pragma hls_resource core=add limit=1
    for (index = size; index > 0; )
    {
        uint32_t j = 0;
        uint32_t beats = (index < 256) ? index : 256;
        uint32_t len = beats;

        dma_info_t dma_info(mem_off, len, DMA_SIZE);
        dma_write_ctrl.Push(dma_info);
		#pragma hls_unroll factor=2
		#pragma hls_logic_opt
		#pragma hls_resource core=add limit=1
		#pragma hls_pipeline off
        for (; j < beats; ++j)
        {
            FPDATA_WORD w;
#if defined(FAST)
            if (ping)
                w = output_ping[0][mem_index++];
            else
                w = output_pong[0][mem_index++];
#endif
#if defined(SMALL)
                //w = output_ping[0][mem_index++];
			if (ping)
                w = output_ping[0][mem_index++];
            else
                w = output_pong[0][mem_index++];
#endif
#if defined(MEDIUM)
                //w = output_ping[0][mem_index++];
			if (ping)
                w = output_ping[0][mem_index++];
            else
                w = output_pong[0][mem_index++];
#endif          

            dma_write_chnl.Push(w);

            wait();
        }

        mem_off += beats;
        index -= beats;

        wait();
    }
}

 #endif// __CONV_LAYER_UTILS_HPP__
