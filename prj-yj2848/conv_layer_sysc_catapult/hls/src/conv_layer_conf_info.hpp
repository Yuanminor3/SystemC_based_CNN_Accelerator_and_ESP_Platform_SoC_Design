// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __CONF_INFO_T_HPP__
#define __CONF_INFO_T_HPP__

#pragma once

#include <sstream>
#include <ac_int.h>
#include <ac_fixed.h>
#include "conv_layer_specs.hpp"
#include "auto_gen_port_info.h"

//
// Configuration parameters for the accelerator.
//


struct conf_info_t
{
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
        uint32_t do_pool_sig;
        uint32_t do_pad_sig;
        uint32_t pool_size_sig;
        uint32_t pool_stride_sig;
//这段访问方法，方便读取
    AUTO_GEN_FIELD_METHODS(conf_info_t, ( \
                               in_base_addr_sig \
                               , in_w_base_addr_sig \
                               , out_b_base_addr_sig \
                               , out_base_addr_sig \
                               , in_size_sig \
                               , in_w_size_sig \
                               , out_size_sig \
                               , out_b_size_sig \
                               , num_cols_sig \
                               , num_rows_sig \
                               , src_chans_sig \
                               , dst_chans_sig \
                               , do_pool_sig \
                               , do_pad_sig \
                               , pool_size_sig \
                               , pool_stride_sig \

                               ) )

};

#endif // __MAC_CONF_INFO_T_HPP__
