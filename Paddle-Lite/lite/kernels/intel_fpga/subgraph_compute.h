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

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <intelfpga.h>
#include "lite/core/kernel.h"
#include "lite/core/subgraph_bridge_registry.h"
#include "lite/kernels/intel_fpga/bridges/graph.h"
#include "lite/core/subgraph_engine_base.h"

namespace paddle {
namespace lite {
namespace kernels {
namespace intel_fpga {


class SubgraphEngine : public subgraph::SubgraphEngineBase {
 public:
  SubgraphEngine(KernelContext* ctx,
                 int block_idx,
                 const std::shared_ptr<const cpp::ProgramDesc>& program_desc,
                 Scope* exec_scope,
                 const std::vector<std::string>& input_names,
                 const std::vector<std::string>& output_names)
      : subgraph::SubgraphEngineBase(ctx,
                                     block_idx,
                                     program_desc,
                                     exec_scope,
                                     input_names,
                                     output_names) {}

  ~SubgraphEngine();

 protected:
  bool BuildDeviceProgram() override;
  bool LaunchDeviceProgram() override;

  std::map<std::vector<std::vector<int64_t>>, std::shared_ptr<subgraph::intel_fpga::Graph>>
      device_programs_;
};

class SubgraphCompute
    : public KernelLite<TARGET(kIntelFPGA), PRECISION(kInt8), DATALAYOUT(kNCHW)> {
 public:
  using param_t = operators::SubgraphParam;

  void PrepareForRun() override;

  void Run() override;

  virtual ~SubgraphCompute() = default;

 private:
  std::unique_ptr<SubgraphEngine> engine_;
};

}  // namespace apu
}  // namespace kernels
}  // namespace lite
}  // namespace paddle

