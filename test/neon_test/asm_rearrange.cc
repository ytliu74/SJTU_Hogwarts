#include "neon_rearrange.h"

void asm_rearrange_8_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int batch_i, int64_t* out, int offset, int pos) {

    int out_offset = (offset + pos) * 8;
    int batch_offset = 8 * batch_i;

    asm volatile (
        "add  %[l_0], %[l_0], %[batch_offset] \n"
        "add  %[l_1], %[l_1], %[batch_offset] \n"
        "add  %[l_2], %[l_2], %[batch_offset] \n"
        "add  %[l_3], %[l_3], %[batch_offset] \n"
        "add  %[l_4], %[l_4], %[batch_offset] \n"
        "add  %[l_5], %[l_5], %[batch_offset] \n"
        "add  %[l_6], %[l_6], %[batch_offset] \n"
        "add  %[l_7], %[l_7], %[batch_offset] \n"

        "vld1.8   d0, [%[l_0]]     \n"
        "vld1.8   d1, [%[l_1]]     \n"
        "vld1.8   d2, [%[l_2]]     \n"
        "vld1.8   d3, [%[l_3]]     \n"
        "vld1.8   d4, [%[l_4]]     \n"
        "vld1.8   d5, [%[l_5]]     \n"
        "vld1.8   d6, [%[l_6]]     \n"
        "vld1.8   d7, [%[l_7]]     \n"

        // calculate store base addr
        "add r0, %[out], %[offset] \n"

        // zip single pixel
        "vzip.8   d0, d1           \n"
        "vzip.8   d2, d3           \n"
        "vzip.8   d4, d5           \n"
        "vzip.8   d6, d7           \n"

        // calculate the second addr
        "add r1, r0, #16           \n"

        // zip two pixels
        "vzip.16  d0, d2           \n"
        "vzip.16  d1, d3           \n"
        "vzip.16  d4, d6           \n"
        "vzip.16  d5, d7           \n"

        // zip four pixels
        "vzip.32  d0, d4           \n"
        "vzip.32  d2, d6           \n"
        "vzip.32  d1, d5           \n"
        "vzip.32  d3, d7           \n"

        // store to memory
        "vst1.8   d0, [r0]         \n"
        "add      r0, r1, #16      \n"
        "vst1.8   d4, [r1]         \n"
        "add      r1, r0, #16      \n"
        "vst1.8   d2, [r0]         \n"
        "add      r0, r1, #16      \n"
        "vst1.8   d6, [r1]         \n"
        "add      r1, r0, #16      \n"
        "vst1.8   d1, [r0]         \n"
        "add      r0, r1, #16      \n"
        "vst1.8   d5, [r1]         \n"
        "add      r1, r0, #16      \n"
        "vst1.8   d3, [r0]         \n"
        "add      r0, r1, #16      \n"
        "vst1.8   d7, [r1]         \n"

        :
        : [out] "r"(out),
        [l_0] "r"(l_0),
        [l_1] "r"(l_1),
        [l_2] "r"(l_2),
        [l_3] "r"(l_3),
        [l_4] "r"(l_4),
        [l_5] "r"(l_5),
        [l_6] "r"(l_6),
        [l_7] "r"(l_7),
        [offset] "r"(out_offset),
        [batch_offset] "r"(batch_offset)

        : "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "r0", "r1"
        );
}