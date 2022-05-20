#include "neon_rearrange.h"

void asm_rearrange_16_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int8_t* out, int iter, int l_len) {

    // l_len is the length of the dout_array[]
    // len means jump 8 dout_array[]
    int len = l_len * 8;

    asm volatile (
        // r2 -> jump back length
        "sub      r2, %[len], #8   \n"

        // main loop
        "loop:                     \n"

        // load l_0 - l_7
        "vld1.8   d0, [%[l_0]]     \n"
        "vld1.8   d1, [%[l_1]]     \n"
        "vld1.8   d2, [%[l_2]]     \n"
        "vld1.8   d3, [%[l_3]]     \n"
        "vld1.8   d4, [%[l_4]]     \n"
        "vld1.8   d5, [%[l_5]]     \n"
        "vld1.8   d6, [%[l_6]]     \n"
        "vld1.8   d7, [%[l_7]]     \n"

        // calculate l_8 - l_15
        // l_{x} -> l_{x+8}
        "add   %[l_0], %[l_0], %[len]   \n"
        "add   %[l_1], %[l_1], %[len]   \n"
        "add   %[l_2], %[l_2], %[len]   \n"
        "add   %[l_3], %[l_3], %[len]   \n"
        "add   %[l_4], %[l_4], %[len]   \n"
        "add   %[l_5], %[l_5], %[len]   \n"
        "add   %[l_6], %[l_6], %[len]   \n"
        "add   %[l_7], %[l_7], %[len]   \n"

        // load l_8 - l_15
        "vld1.8   d20, [%[l_0]]     \n"
        "vld1.8   d21, [%[l_1]]     \n"
        "vld1.8   d22, [%[l_2]]     \n"
        "vld1.8   d23, [%[l_3]]     \n"
        "vld1.8   d24, [%[l_4]]     \n"
        "vld1.8   d25, [%[l_5]]     \n"
        "vld1.8   d26, [%[l_6]]     \n"
        "vld1.8   d27, [%[l_7]]     \n"

        // calculate next l_0 - l_7
        // use r2 to jump back
        "sub   %[l_0], %[l_0], r2   \n"
        "sub   %[l_1], %[l_1], r2   \n"
        "sub   %[l_2], %[l_2], r2   \n"
        "sub   %[l_3], %[l_3], r2   \n"
        "sub   %[l_4], %[l_4], r2   \n"
        "sub   %[l_5], %[l_5], r2   \n"
        "sub   %[l_6], %[l_6], r2   \n"
        "sub   %[l_7], %[l_7], r2   \n"

        // calculate store base addr
        // "add r0, %[out], %[offset] \n"

        // zip single pixel
        "vzip.8   d0, d1           \n"
        "vzip.8   d2, d3           \n"
        "vzip.8   d4, d5           \n"
        "vzip.8   d6, d7           \n"

        "vzip.8   d20, d21           \n"
        "vzip.8   d22, d23           \n"
        "vzip.8   d24, d25           \n"
        "vzip.8   d26, d27           \n"

        // zip two pixels
        "vzip.16  d0, d2           \n"
        "vzip.16  d1, d3           \n"
        "vzip.16  d4, d6           \n"
        "vzip.16  d5, d7           \n"

        "vzip.16  d20, d22           \n"
        "vzip.16  d21, d23           \n"
        "vzip.16  d24, d26           \n"
        "vzip.16  d25, d27           \n"

        // zip four pixels
        "vzip.32  d0, d4           \n"
        "vzip.32  d2, d6           \n"
        "vzip.32  d1, d5           \n"
        "vzip.32  d3, d7           \n"

        "vzip.32  d20, d24           \n"
        "vzip.32  d22, d26           \n"
        "vzip.32  d21, d25           \n"
        "vzip.32  d23, d27           \n"

        // store to memory
        "vst1.8   d0 , [%[out]]!    \n"
        "vst1.8   d20, [%[out]]!    \n"
        "vst1.8   d4 , [%[out]]!    \n"
        "vst1.8   d24, [%[out]]!    \n"
        "vst1.8   d2 , [%[out]]!    \n"
        "vst1.8   d22, [%[out]]!    \n"
        "vst1.8   d6 , [%[out]]!    \n"
        "vst1.8   d26, [%[out]]!    \n"
        "vst1.8   d1 , [%[out]]!    \n"
        "vst1.8   d21, [%[out]]!    \n"
        "vst1.8   d5 , [%[out]]!    \n"
        "vst1.8   d25, [%[out]]!    \n"
        "vst1.8   d3 , [%[out]]!    \n"
        "vst1.8   d23, [%[out]]!    \n"
        "vst1.8   d7 , [%[out]]!    \n"
        "vst1.8   d27, [%[out]]!    \n"

        "subs     %[iter], %[iter], #1  \n"
        "bne      loop                  \n"

        //restore l_0 - l_7
        "sub   %[l_0], %[l_0], #8   \n"
        "sub   %[l_1], %[l_1], #8   \n"
        "sub   %[l_2], %[l_2], #8   \n"
        "sub   %[l_3], %[l_3], #8   \n"
        "sub   %[l_4], %[l_4], #8   \n"
        "sub   %[l_5], %[l_5], #8   \n"
        "sub   %[l_6], %[l_6], #8   \n"
        "sub   %[l_7], %[l_7], #8   \n"

        // output
        : [out] "+r"(out),
        [l_0] "+r"(l_0),
        [l_1] "+r"(l_1),
        [l_2] "+r"(l_2),
        [l_3] "+r"(l_3),
        [l_4] "+r"(l_4),
        [l_5] "+r"(l_5),
        [l_6] "+r"(l_6),
        [l_7] "+r"(l_7)
        // input
        : [len] "r"(len),
        [iter] "r"(iter)

        : "cc", "memory", "r2",
        "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7","d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27"
        );
}