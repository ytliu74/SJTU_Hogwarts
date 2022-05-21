#ifndef NEON_REARRANGE_H
#define NEON_REARRANGE_H

#include <stdint.h>
#include <arm_neon.h>
#include <iostream>
#endif

#define INPUT_EXTEND_SCALE 16

int UpRound(int a, int b);

void rearrange_8_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int64_t* out, int pos);
void asm_rearrange_16_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int8_t* out, int iter, int l_len);

void NeonInputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void NeonInputRearrange_2(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void NeonInputRearrange_3(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);

void InputRearrange(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void InputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);