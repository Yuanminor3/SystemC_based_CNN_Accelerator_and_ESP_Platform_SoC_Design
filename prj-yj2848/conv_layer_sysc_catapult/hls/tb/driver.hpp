/* Copyright 2021 Columbia University SLD Group */

#ifndef __DRIVER_HPP__
#define __DRIVER_HPP__

// Forward declaration
class system_t;

#include "conv_layer_specs.hpp"

class driver_t : public sc_module
{
    public:

        // -- Input ports

        // Clock signal
        sc_in<bool> clk;

        // Reset signal
        sc_in<bool> resetn;

        // Interrupt signals
        sc_in<bool> acc_done;

        // -- Communication channels

        // // // To program the accelerators
        // // reg_initiator_t reg_initiator;
        Connections::Out<conf_info_t >  CCS_INIT_S1(conf_info);

        // -- References to other modules

        // To call the system functions
        system_t *system_ref;

        // -- Module constructor

        SC_HAS_PROCESS(driver_t);
        driver_t(sc_module_name name)
            : sc_module(name)
            , clk("clk")
            , resetn("resetn")
            , acc_done("acc_done")
        {

            SC_THREAD(driver_thread);
            sensitive << clk.pos();
            async_reset_signal_is(resetn, false);

        }

        // -- Processes

        // To handle read and write requests
        void driver_thread(void);

        // -- Functions (read)

        // To read a particular register


        // bool do_read(uint32_t addr, uint32_t &data);

        // // -- Functions (write)

        // // To write a particular register
        // bool do_write(uint32_t addr, uint32_t data);
};

#endif /* __DRIVER_HPP__ */
