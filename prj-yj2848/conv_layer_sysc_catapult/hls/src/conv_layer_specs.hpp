// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __ACCSPECS__
#define __ACCSPECS__

#pragma once

#include <mc_connections.h>
#include "nvhls_connections.h"
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>
#include "esp_dma_info_sysc.hpp"
#include "conv_layer_conf_info.hpp"
#include <ArbitratedScratchpadDP.h>

#define DATA_WIDTH 32


#if defined(FAST)
    #define DMA_SIZE 256
#else
    #define DMA_SIZE SIZE_WORD
#endif 

#define MEM_SIZE 16777216/(DMA_WIDTH/8)//parameters 16777216

#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 1


//Struct for indexing the configuration registers
struct dma_address_map {
    uint32_t cmd_sig;
    uint32_t in_base_addr_sig;
    uint32_t in_w_base_addr_sig;
    uint32_t out_b_base_addr_sig;
    uint32_t out_base_addr_sig;
    uint32_t in_size_sig;
    uint32_t in_w_size_sig;
    uint32_t out_size_sig;
    uint32_t out_b_size_sig;
    uint32_t num_cols_sig;
    uint32_t num_rows_sig;
    uint32_t src_chans_sig;
    uint32_t dst_chans_sig;
    bool do_pool_sig;
    bool do_pad_sig;
    uint32_t pool_size_sig;
    uint32_t pool_stride_sig;
};

//Custom Datatypes for synthesis

#define DMA_ADJ (DMA_WIDTH / DATA_WIDTH)
// #define FPDATA_WL 32
// #define FPDATA_IL 16
// #define FPDATA_WWL 32
// #define FPDATA_WIL 16

#if defined(SMALL)
#define FPDATA_WL 16
#define FPDATA_IL 8
#define FPDATA_WWL 16
#define FPDATA_WIL 8
#endif
#if defined(MEDIUM)
#define FPDATA_WL 32
#define FPDATA_IL 16
#define FPDATA_WWL 32
#define FPDATA_WIL 16
#endif
#if defined(FAST)
#define FPDATA_WL 32
#define FPDATA_IL 16
#define FPDATA_WWL 32
#define FPDATA_WIL 16
#endif
typedef ac_int<DMA_WIDTH> DMA_WORD;
typedef ac_int<FPDATA_WL> FPDATA_WORD;

#ifdef FL_POINT
typedef float FPDATA;
#else
typedef ac_fixed<FPDATA_WL, FPDATA_IL> FPDATA;
#endif

template<unsigned int kAddressSz>
struct Address{
    typedef NVUINTW(kAddressSz) value;
};

typedef NVUINTW(DATA_WIDTH) DATA_TYPE;

// Functions for converting ac_fixed to ac_int and vice-versa

#ifdef FL_POINT
inline void int2f(const FPDATA_WORD& in, FPDATA& out)
{ uint32_t data = in.to_uint(); float *ptr = (float *) &data; out = *ptr;\
}

inline void f2int(const FPDATA& in, FPDATA_WORD& out)
{ uint32_t *ptr = (uint32_t *) &in; out = *ptr; }

inline void bv2fp(const ac_int<FPDATA_WL>& data_in, FPDATA &data_out)
{ uint32_t data = data_in.to_uint(); FPDATA *ptr = (FPDATA *) &data; data_out = *ptr; }

// Functions for converting ac_fixed to ac_int and vice-versa
#else
inline void int2f(const FPDATA_WORD& in, FPDATA& out)
{ out.set_slc(0,in.slc<FPDATA_WL>(0)); }

inline void f2int(const FPDATA& in, FPDATA_WORD& out)
{ out.set_slc(0,in.slc<FPDATA_WL>(0)); }

inline void bv2fp(const ac_int<FPDATA_WL>& data_in, FPDATA &data_out)// hls_knob
{
    // Copy data
    for (int i = 0; i < FPDATA_WL; i++)
        data_out.set_slc(i,data_in.slc<1>(i));
}
#endif

//Scratchpads parameters ad access datatypes

const unsigned int inbks = 1;
const unsigned int inwbks = 1;
const unsigned int bbks = 1;
const unsigned int outbks = 1;
#if defined(SMALL)
    const unsigned int inebks = 1000; //4000
    const unsigned int inwebks = 1000; //2000
    const unsigned int bebks = 1000; //2000
    const unsigned int outebks = 1000; // 4000
#elif defined(MEDIUM)
    const unsigned int inebks = 1000; //4000
    const unsigned int inwebks = 1000; //2000
    const unsigned int bebks = 1000; //2000
    const unsigned int outebks = 1000; // 4000
#elif defined(FAST)
	const unsigned int inebks = 1000; //4000
    const unsigned int inwebks = 1000; //2000
    const unsigned int bebks = 1000; //2000
    const unsigned int outebks = 1000; // 4000
#endif 

#endif
