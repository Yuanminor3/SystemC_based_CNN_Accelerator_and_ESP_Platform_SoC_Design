/* Copyright 2022 Columbia University, SLD Group */

#include "system.hpp"
#include <systemc.h>
#include <mc_scverify.h>

sc_trace_file *trace_file_ptr;

std::string image_path = "cat.bin";

std::string model_path = "../../models/dwarf7.mojo";

int sc_main(int argc, char *argv[])
{
    // Kills various SystemC warnings
    sc_report_handler::set_actions(SC_WARNING, SC_DO_NOTHING);
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    if (argc >= 2)
    { image_path = std::string(argv[1]) + ".bin"; }

    system_t system_inst("system_inst", model_path, image_path);
    trace_hierarchy(&system_inst, trace_file_ptr);

    sc_start();

    return 0;
}
