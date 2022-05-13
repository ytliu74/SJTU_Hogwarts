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
#include <utility>
#include <vector>
#include <intelfpga.h>
#include "lite/core/op_lite.h"
#include "lite/core/tensor.h"

namespace paddle {
namespace lite {
namespace subgraph {
namespace intel_fpga {

// Graph and node is defined to collect all of converted HiAI IR nodes
//  struct Node {
//   public:
//  	Node* parent_{nullptr}; // The op which it's input tensor belong to.
//  	Node* next_{nullptr};  // Next op to be executed in topological order.
//    nna_conv_s* device_param_{nullptr};
//    int outpu_tensor_ref_{0}; // Output tensor reference count.
//  };

using Node = DeviceGraphNode;

class Graph {
 public:
  bool ExecuteDeviceGraph();

  // For device graph, each node's output should be set.
  // And for input and output node, allocate space for device input and output.
  bool BuildDeviceModel();
  
  // The node in subgraph whose output ref count is larger than 1 is constricted 
  // to no more than 10. Also, the input, filter and output size should be check.
  bool DeviceModelValidCheck();

  ~Graph() {
    auto node = root_;
     // Delete root node's input space which size is extent_input_size.
    if(root_) {
      delete root_->device_param_->ia;
    }
    while(node) {
      // Delete device_param_ and it's children.
      auto device_param = node->device_param_;
      if(device_param) {
        if(device_param->ip) {
          delete device_param->ip;
        }
        if(device_param->ba) {
          delete device_param->ba;
        }
        if(device_param->ka) {
          delete device_param->ka;
        }
        // Delete output node's output space. 
        if(node->output_ref_count_ == 0) {
          delete device_param->oa;
        }
        delete node->device_param_;
      }
      // Delete node itself.
      auto node_delete = node;
      node = node->next_;
      delete node_delete;
    }
    delete data_buff_;
  }

  void set_input_names(const std::vector<std::string> input_names) {
    input_names_ = input_names;
  }

  bool IsInput(const std::string& name) {
    for (int i = 0; i < input_names_.size(); i++) {
      if (input_names_[i] == name) return true;
    }
    return false;
  }

  bool IsOutput(const std::string& name) {
    for (int i = 0; i < output_names_.size(); i++) {
      if (output_names_[i] == name) return true;
    }
    return false;
  }

  void set_output_names(const std::vector<std::string> output_names) {
    output_names_ = output_names;
  }

  void setTensor2Node(std::string name, Node* node) {
    if(tensor2node_.find(name) == tensor2node_.end()) {
      tensor2node_[name] = node;
    } else {
      LOG(FATAL) << "[IntelFPGA] Node" << name << " is redefined.";
    }
  }
  
  Node* GetNodeByTensorName(std::string name) {
    if(tensor2node_.find(name) != tensor2node_.end()) {
      return tensor2node_[name];
    } else {
      return nullptr;
    }
  }

  Node* getGraphRootNode() {
    return root_;
  }
  Node* getGraphTailNode() {
    return tail_;
  }

  void setGraphRootNode(Node* node) {
    root_ = node;
  }
  void setGraphTailNode(Node* node) {
    tail_ = node;
  }

  
 private:
  Node* root_{nullptr};
  Node* tail_{nullptr};
  std::vector<std::string> input_names_;
  std::vector<std::string> output_names_;
  // std::map<std::string, int> tensor_ref_count_; // Referencing count of tensor.
  std::map<std::string, Node*> tensor2node_; // Map tensor to node which output this tensor.
  // Subgraph input.
  int8_t* din_{nullptr};
  // Subgraph output.
  std::map<Node*, int8_t*> output_nodes_;
  // Max device output size;
  int max_device_output_size_{0};
  // std::shared_ptr<FpgaEngine> fpga_exec_engine_;
  int8_t* data_buff_;
};

}  // namespace apu
}  // namespace subgraph
}  // namespace lite
}  // namespace paddle

