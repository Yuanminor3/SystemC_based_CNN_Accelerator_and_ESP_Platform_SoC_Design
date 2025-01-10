/* Copyright 2021 Columbia University SLD Group */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <stdlib.h>
#include <string.h>

#include "driver.hpp"
#include <mc_scverify.h>

#include "ac_int.h"
#include "ac_fixed.h"
#include "ac_float.h"
#include "ac_std_float.h"
#include "ac_math.h"
#include "conv_layer.hpp"
#include "esp_dma_info_sysc.hpp"
#include "esp_dma_controller.hpp"

#define TARGET_LAYER 4

class system_t : public sc_module
{
public:

        // Shared memory buffer model
        ac_int<DMA_WIDTH> *memory;

        // DMA controller instace
        esp_dma_controller<DMA_WIDTH, MEM_SIZE > *dmac;// parameters

        driver_t *driver;

        // -- Modules
        sc_clock clk;

        // Reset signal
        // sc_in<bool> resetn;
		#pragma remove_out_reg
        sc_signal<bool> resetn;

        CCS_DESIGN(conv_layer_sysc_catapult) CCS_INIT_S1(acc);

        // -- Input ports

        // Interrupt signals
		#pragma remove_out_reg
        sc_signal<bool> acc_done;
		#pragma remove_out_reg
        sc_signal<bool> acc_rst;

        // -- Internal Channels

        Connections::Combinational<ac_int<DMA_WIDTH>>        CCS_INIT_S1(dma_read_chnl);
        Connections::Combinational<ac_int<DMA_WIDTH>>        CCS_INIT_S1(dma_write_chnl);
        Connections::Combinational<dma_info_t>        CCS_INIT_S1(dma_read_ctrl);
        Connections::Combinational<dma_info_t>        CCS_INIT_S1(dma_write_ctrl);
        Connections::Combinational<conf_info_t>        CCS_INIT_S1(conf_info);

        SC_HAS_PROCESS(system_t);
        system_t(sc_module_name name,
                 std::string model_path,
                 std::string image_path)
            : sc_module(name)
            , memory(new ac_int<DMA_WIDTH>[MEM_SIZE])
            , dmac(new esp_dma_controller<DMA_WIDTH, MEM_SIZE>("dma-controller", memory))
            , driver(new driver_t("driver"))
#if defined(SMALL)
            , clk("clk", 20.00, SC_NS, 0.5, 0, SC_NS, true)
#elif defined(MEDIUM)
            , clk("clk", 8.00, SC_NS, 0.5, 0, SC_NS, true)
#elif defined(FAST)
            , clk("clk", 3.00, SC_NS, 0.5, 0, SC_NS, true)
#endif 
            , resetn("resetn")
            , acc_done("acc_done")
            , model_path(model_path)
            , image_path(image_path)
    {		
            sc_object_tracer<sc_clock> trace_clk(clk);

            // Binding the acceleratorr
            acc.clk(clk);
            acc.rst(resetn);
            acc.conf_info(conf_info);
            acc.dma_read_chnl(dma_read_chnl);
            acc.dma_write_chnl(dma_write_chnl);
            acc.dma_read_ctrl(dma_read_ctrl);
            acc.dma_write_ctrl(dma_write_ctrl);
            acc.acc_done(acc_done);


            dmac->clk(clk);
            dmac->rst(resetn);
            dmac->dma_read_ctrl(dma_read_ctrl);
            dmac->dma_read_chnl(dma_read_chnl);
            dmac->dma_write_ctrl(dma_write_ctrl);
            dmac->dma_write_chnl(dma_write_chnl);
            dmac->acc_done(acc_done);
            dmac->acc_rst(acc_rst);

            driver->clk(clk);
            driver->resetn(resetn);
            driver->acc_done(acc_done);
            driver->conf_info(conf_info);
            driver->system_ref = this;

            SC_THREAD(reset);

    }

    void reset() {
        resetn.write(0);
        wait(50, SC_NS);
        resetn.write(1);
    }

        // -- Functions

        // Load the value of the registers
        conf_info_t load_regs(bool fully_connected);

        // Load the input values in memory
        void load_memory(void);
        void setup_memory(int target_layer, bool fully_connected);

        // Read the output values from memory
        void dump_memory(void);

        // Verify that the results are correct
        void validate(void);

        // Optionally free resources (arrays)
        void clean_up(void);

        // Load a float vector in memory
        template<typename T, size_t FP_WL>
        void load_fpdata(float *in,
                         uint32_t base_addr,
                         uint32_t size,
                         uint32_t actual_size);

        // Convert output from memory back to float
        void convert_output(int target_layer);

        // Call programmer's view on one layer of the CNN
        void run_pv(int layer, bool fully_connected);

        // Set configuration parameters from CNN model
        void set_configuration_param(int target_layer, bool fully_connected);

        // Rearrange weights in memory for accelerated partial simulation
        void move_weights(int target_layer, bool fully_connected);

        // -- Private data

        // I/O Dimensions
        uint32_t actual_w_size;
        uint32_t actual_i_size;
        uint32_t actual_b_size;
        uint32_t actual_o_size;

        uint32_t w_size;
        uint32_t i_size;
        uint32_t b_size;
        uint32_t o_size;

        // Configuration parameters
        uint32_t w_cols;
        uint32_t w_rows;
        uint32_t cols;
        uint32_t rows;
        uint32_t src_chans;
        uint32_t dst_chans;
        bool do_pool;
        bool do_pad;
        uint32_t pool_size;
        uint32_t pool_stride;

        uint32_t reduced_chans;

        // Base addresses
        uint32_t w_addr;
        uint32_t i_addr;
        uint32_t b_addr;
        uint32_t o_addr;

        // Path for the model
        std::string model_path;

        // Image to be classified
        std::string image_path;
};

#endif /* __SYSTEM_HPP__ */
