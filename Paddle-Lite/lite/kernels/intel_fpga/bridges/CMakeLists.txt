# new file by hc 20220416
if ((NOT LITE_ON_MODEL_OPTIMIZE_TOOL) AND (NOT LITE_WITH_PYTHON) AND (NOT LITE_WITH_INTEL_FPGA))
    return()
endif()

lite_cc_library(subgraph_bridge_graph_intel_fpga SRCS graph.cc DEPS)
set(intel_fpga_subgraph_bridge_deps subgraph_bridge_registry  subgraph_bridge_graph_intel_fpga ${intel_fpga_runtime_libs})

lite_cc_library(subgraph_bridge_conv_op_intel_fpga SRCS conv_op.cc DEPS ${intel_fpga_subgraph_bridge_deps})

set(intel_fpga_subgraph_bridges
        subgraph_bridge_registry
        subgraph_bridge_conv_op_intel_fpga
        CACHE INTERNAL "intel_fpga_subgraph_bridges")

message(STATUS "+++++ intel_fpga_subgraph_bridges: ${intel_fpga_subgraph_bridges}")
