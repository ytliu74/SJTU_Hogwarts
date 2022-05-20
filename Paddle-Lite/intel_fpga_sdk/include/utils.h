/* Copyright (c) 2020 AWCloud. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <memory.h>
#include <algorithm>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <arm_neon.h>
#include "saturate.h"
#ifdef __cplusplus

extern "C" {
#endif

#include <stdint.h>

/* function declarations */
int64_t intelfpga_gettime(void );
float intelfpga_absmax(const float* din, int size);
void intelfpga_fp32_to_int8(const float* din, int8_t* dout, const float scale, int size);
void intelfpga_fp32_to_int32(const float* din, int32_t* dout, const float scale, int size);

void intelfpga_int32_to_fp32(const int* din, float* dout, float scale, int size);
void intelfpga_fill_bias_act(float* tensor, const float* bias, int channel, int channel_size, int relu, float alpha);
void intelfpga_int8_to_fp32(const int8_t* in,
                  float* out,
                  const float* scale,
                  int axis_size,
                  int64_t outer_size,
                  int64_t inner_size);

int8_t Quantize(float x);

void rearrange_8_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int64_t* out, int pos);

void PrintTensor(std::string, void*, int);

void conv_chw_pad(int8_t* din, int8_t* dout, int ch, int h, int w, int pad);

bool conv_trans_weights_numc(const int8_t* din,
    int8_t* dout, int chout, int chin, int n, int kernel_size);

void InputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
    const int w, const int pad);

void OutputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
    const int w);

#ifdef __cplusplus
}
#endif
