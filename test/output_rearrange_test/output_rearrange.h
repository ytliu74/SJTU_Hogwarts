#ifndef NEON_REARRANGE_H
#define NEON_REARRANGE_H

#include <stdint.h>
#include <arm_neon.h>
#include <iostream>
#endif

#define INPUT_EXTEND_SCALE 16

int UpRound(int a, int b);
void OutputRearrange(int8_t* din, int8_t* dout, const int c, const int h, const int w);

void asm_output_rearrange_16_layers(int8_t* din, int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int iter, int l_len);

void NeonRearrange_1(int8_t* din, int8_t* dout, const int c, const int h, const int w);