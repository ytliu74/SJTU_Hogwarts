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

#include "lite/kernels/intel_fpga/bridges/graph.h"
#include <utility>

namespace paddle {
namespace lite {
namespace subgraph {
namespace intel_fpga {

bool Graph::ExecuteDeviceGraph() {
  return IntelFpgaConvDepthwise(root_, din_, output_nodes_, max_device_output_size_, data_buff_);
  // return fpga_exec_engine_->FpgaExecute();
}

bool Graph::BuildDeviceModel() {
  // Fill output padding of each node by next node's input padding.
  int ddr_block = 0;
  int idx = 0;
  auto cur_node = root_;
  while(cur_node) {
    if(cur_node->output_ref_count_ == 1 && cur_node->next_ && cur_node->is_output == false) {
      cur_node->device_param_->ip->out_pad = cur_node->next_->device_param_->ip->in_pad;
    }
     
    // Special nodes use individual ddr block instead of double buffer.
    // Such nodes are SIME nodes.
    if((cur_node->is_output && cur_node->output_ref_count_ == 1) || (cur_node->output_ref_count_ > 1)) {
      cur_node->ddr_block_idx = ddr_block++;
    }

    auto fpga_param = cur_node->device_param_;
    int extent_input_size = ((fpga_param->ip->in_c - 1) / INPUT_EXTEND_SCALE + 1) * 
      INPUT_EXTEND_SCALE *
      (fpga_param->ip->in_h + 2 * fpga_param->ip->in_pad) *
      (fpga_param->ip->in_w + 2 * fpga_param->ip->in_pad);
    fpga_param->extent_in_size = extent_input_size;
    CHECK_LE(extent_input_size, NNA_IDM_SIZE);

    // Convert origin output to device output'space.
    int extent_output_size = ((fpga_param->ip->output_c - 1) / INPUT_EXTEND_SCALE + 1) *
         INPUT_EXTEND_SCALE *
         (fpga_param->ip->output_h + 2 * fpga_param->ip->out_pad) *
         (fpga_param->ip->output_w + 2 * fpga_param->ip->out_pad);
    fpga_param->extent_out_size = extent_output_size;
    CHECK_LE(extent_output_size, NNA_ODM_SIZE);

    // For output node, set extent_out_size in device param.
    if(cur_node->is_output) {
      // Store origin outptut address in graph's output_nodes.
      output_nodes_[cur_node] = fpga_param->oa; 
      max_device_output_size_ = std::max(max_device_output_size_, fpga_param->extent_out_size);
      // Allocate space for extent-output.
      fpga_param->oa = new int8_t[fpga_param->extent_out_size];
    }
    cur_node->id = idx++;
    cur_node = cur_node->next_;
  }

  if(root_) {
    din_ = root_->device_param_->ia;
    // Allocate space for root node.
    root_->device_param_->ia = new int8_t[root_->device_param_->extent_in_size];
  }

  // fpga_exec_engine_ = std::make_shared<FpgaEngine>(root_, din_, output_nodes_, max_device_output_size_);
  data_buff_ = new int8_t [max_device_output_size_ + root_->device_param_->extent_in_size];
  memset(data_buff_, 0, root_->device_param_->extent_in_size);
  return DeviceModelValidCheck();
}

bool Graph::DeviceModelValidCheck() {
  return true;
}

}  // namespace apu
}  // namespace subgraph
}  // namespace lite
}  // namespace paddle

