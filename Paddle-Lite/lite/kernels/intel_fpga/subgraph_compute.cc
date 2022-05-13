// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// new file by hc 20220416

#include "lite/kernels/intel_fpga/subgraph_compute.h"
#include "lite/kernels/intel_fpga/conv_intelfpga.h"
#include <dlfcn.h>
#include <sys/time.h>
#include <time.h>
#include <utility>
#include "lite/core/op_registry.h"
//#include "lite/kernels/intel_fpga/bridges/graph.h"
#include "lite/kernels/intel_fpga/bridges/paddle_use_bridges.h"

namespace paddle {
namespace lite {
namespace kernels {
namespace intel_fpga {

bool SubgraphEngine::BuildDeviceProgram() {
  if (!origin_program_) {
    //LOG(INFO) << "Build intelfpga origin pragram.";
    BuildOriginProgram();
  }
  if (!device_programs_.count(origin_idims_)) {
    //LOG(INFO) << "Build intelfpga subgraph pragram.";
    int status = 0;
    auto graph = std::make_shared<subgraph::intel_fpga::Graph>();
    graph->set_input_names(input_names_);
    graph->set_output_names(output_names_);

    const auto& bridges = subgraph::SubgraphBridgeRegistry::Instance();
    const auto& insts = origin_program_->instructions(kRootBlockIdx);
    for (auto& inst : insts) {
      auto op = const_cast<OpLite*>(inst.op());
      CHECK(op);
      op->CheckShape();
      op->InferShape();
      std::string op_type = op->op_info()->Type();
      auto kernel = inst.kernel();
      status |=
          bridges.Select(op_type, TARGET(kIntelFPGA))(reinterpret_cast<void*>(graph.get()),
                                                const_cast<OpLite*>(op),
                                                const_cast<KernelBase*>(kernel));
      if (subgraph::CHECK_FAILED(status)) {
        return false;
      }
    }
    graph->BuildDeviceModel();
    device_programs_[origin_idims_] = graph;
  }
  return true;
}

bool SubgraphEngine::LaunchDeviceProgram() {
  auto GetCurrentUS = []() -> double {
    struct timeval time;
    gettimeofday(&time, NULL);
    return 1e+6 * time.tv_sec + time.tv_usec;
  };

  if (device_programs_.count(origin_idims_) == 0) {
    //LOG(INFO) << "Launch intelfpga origin pragram.";
    return LaunchOriginProgram();
  }

//  LOG(INFO) << "Launch intelfpga subgraph pragram.";
  return device_programs_[origin_idims_]->ExecuteDeviceGraph();
}

SubgraphEngine::~SubgraphEngine() {
}

void SubgraphCompute::PrepareForRun() {
  auto& param = this->Param<param_t>();
  engine_.reset(new SubgraphEngine(ctx_.get(),
                                   param.block_idx,
                                   param.program_desc,
                                   param.exec_scope,
                                   param.input_data_names,
                                   param.output_data_names));
  CHECK(engine_);
}

void SubgraphCompute::Run() {
  CHECK(engine_);
  engine_->Run();
}

}  // namespace apu
}  // namespace kernels
}  // namespace lite
}  // namespace paddle

REGISTER_LITE_KERNEL(subgraph,
                     kIntelFPGA,
                     kInt8,
                     kNCHW,
                     paddle::lite::kernels::intel_fpga::SubgraphCompute,
                     def)
    .BindInput("Inputs",
               {LiteType::GetTensorTy(TARGET(kHost),
                                      PRECISION(kInt8),
                                      DATALAYOUT(kNCHW))})
    .BindOutput("Outputs",
                {LiteType::GetTensorTy(TARGET(kHost),
                                       PRECISION(kInt8),
                                       DATALAYOUT(kNCHW))})
    .Finalize();

