/* Copyright 2021 Columbia University SLD Group */

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#pragma once

#include "driver.hpp"
#include "conv_layer_specs.hpp"


class memory_t : public sc_module
{

    public:

    // -- Input ports

    // Cloc signal
    sc_in<bool> CCS_INIT_S1(clk);

    // Reset signal
    sc_in<bool> CCS_INIT_S1(resetn);

    r_slave<AUTO_PORT>     CCS_INIT_S1(r_target);
    w_slave<AUTO_PORT>     CCS_INIT_S1(w_target);

    static const int sz = 0x1000000;//0x1000000

    SC_CTOR(memory_t) {

        SC_THREAD(target_r_process);
        sensitive << clk.pos();
        async_reset_signal_is(resetn, false);

        SC_THREAD(target_w_process);
        sensitive << clk.pos();
        async_reset_signal_is(resetn, false);

        data = new FPDATA_WORD[sz];

    }

    FPDATA_WORD *data;

    void target_r_process() {
        r_target.reset();
        wait();

        while (1) {
            ar_payload ar;

            r_target.start_multi_read(ar);

            while (1) {
                r_payload r;

                if (ar.addr >= (sz * bytesPerBeat)) {
                    SC_REPORT_ERROR("ram", "invalid addr");
                    r.resp = Enc::XRESP::SLVERR;
                } else {
                    r.data = data[ar.addr / bytesPerBeat];
                }

                if (!r_target.next_multi_read(ar, r)) { break; }
            }
        }

    }


    void target_w_process() {
        w_target.reset();
        wait();

        while (1) {
            aw_payload aw;
            b_payload b;

            w_target.start_multi_write(aw, b);

            while (1) {

                w_payload w = w_target.w.Pop();

                if (aw.addr >= (sz * bytesPerBeat)) {
                    SC_REPORT_ERROR("ram", "invalid addr");
                    b.resp = Enc::XRESP::SLVERR;
                } else {
                    decltype(w.wstrb) all_on{~0};

                    if (w.wstrb == all_on) {
                        data[aw.addr / bytesPerBeat] = w.data.to_uint64();
                    } else {
                        FPDATA_WORD orig  = data[aw.addr / bytesPerBeat];
                        FPDATA_WORD wdata = w.data.to_uint64();
                        #pragma unroll
                        for (int i=0; i<WSTRB_WIDTH; i++)
                            if (w.wstrb[i]) {
                                orig = nvhls::set_slc(orig, nvhls::get_slc<8>(wdata, (i*8)), (i*8));
                            }

                        data[aw.addr / bytesPerBeat] = orig;
                    }
                }

                if (!w_target.next_multi_write(aw)) { break; }
            }

            w_target.b.Push(b);
        }
    }


};


#endif /* __MEMORY_HPP__ */
