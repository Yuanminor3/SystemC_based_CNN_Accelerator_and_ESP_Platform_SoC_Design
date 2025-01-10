/* Copyright 2021 Columbia University SLD Group */

#include "utils.hpp"
#include "driver.hpp"
#include "system.hpp"

// -- Processes

void driver_t::driver_thread(void)
{
    // Reset
    conf_info.Reset();

    wait();

    REPORT_INFO("=== TEST BEGIN ===");

    // Configure

    {
        system_ref->load_memory();

        CCS_LOG("sysref loadmem done" );
        conf_info_t config=system_ref->load_regs(false);
        conf_info.Push(config);
        CCS_LOG("info pushed " );

        wait();
    }

    // Computation

    {
        uint32_t rdata;
        do { wait(); } while (!acc_done.read());

        system_ref->dump_memory();
        system_ref->validate();

        system_ref->clean_up();

        CCS_LOG("\n === TEST COMPLETED === \n");
    }

    // Conclude

    {
        sc_stop();
    }
}


// // -- Functions (write)

// bool driver_t::do_write(uint32_t addr, uint32_t data)
// {

//     conf_info_t config;

//     /* <<--params-->> */
//     config.mac_n = mac_n;
//     config.mac_vec = mac_vec;
//     config.mac_len = mac_len;



//     b_payload b = driver_w_initiator.single_write(addr, data);

//     return (b.resp == Enc::XRESP::OKAY);
// }
