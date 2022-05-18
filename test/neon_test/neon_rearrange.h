#ifndef NEON_REARRANGE_H
#define NEON_REARRANGE_H

#include <stdint.h>
#endif

#define INPUT_EXTEND_SCALE 16

int UpRound(int a, int b);

void NeonInputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void NeonInputRearrange_2(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void InputRearrange(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);
void InputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad);