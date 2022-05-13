if ((NOT LITE_ON_MODEL_OPTIMIZE_TOOL) AND (NOT LITE_WITH_PYTHON) AND (NOT LITE_WITH_INTEL_FPGA))
    return()
endif()

add_subdirectory(bridges) # added by hc 20220416

set(intel_fpga_deps ${lite_kernel_deps} ${intel_fpga_runtime_libs})

# commented by hc 20220416
# add_kernel(conv_depthwise_intel_fpga INTEL_FPGA basic SRCS conv_depthwise.cc DEPS ${intel_fpga_deps})
# add_kernel(conv_gemmlike_intel_fpga INTEL_FPGA basic SRCS conv_gemmlike.cc DEPS ${intel_fpga_deps})
# add_kernel(conv_compute_intel_fpga INTEL_FPGA basic SRCS conv_compute.cc DEPS ${intel_fpga_deps} conv_depthwise_intel_fpga conv_gemmlike_intel_fpga)
# end commented

# added by hc 20220416
add_kernel(conv_intelfpga INTEL_FPGA basic SRCS conv_intelfpga.cc DEPS ${intel_fpga_deps})
add_kernel(conv_compute_intel_fpga INTEL_FPGA basic SRCS conv_compute.cc DEPS ${intel_fpga_deps} conv_intelfpga)
add_kernel(subgraph_compute_intel_fpga INTEL_FPGA basic SRCS subgraph_compute.cc DEPS ${intel_fpga_deps} subgraph_engine_base conv_intelfpga ${intel_fpga_subgraph_bridges})
# end added
