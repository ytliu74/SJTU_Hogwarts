#include "output_rearrange.h"

void asm_output_rearrange_16_layers(int8_t* din, int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int iter, int l_len) {

    int len = l_len * 8;

    asm volatile (
        // r2 -> jump back length
        "sub    r2, %[len], #8       \n"

        // main loop
        "loop:                       \n"

        // load din
        "vld1.8   d0 , [%[din]]!     \n"
        "vld1.8   d20, [%[din]]!     \n"
        "vld1.8   d1 , [%[din]]!     \n"
        "vld1.8   d21, [%[din]]!     \n"
        "vld1.8   d2 , [%[din]]!     \n"
        "vld1.8   d22, [%[din]]!     \n"
        "vld1.8   d3 , [%[din]]!     \n"
        "vld1.8   d23, [%[din]]!     \n"
        "vld1.8   d4 , [%[din]]!     \n"
        "vld1.8   d24, [%[din]]!     \n"
        "vld1.8   d5 , [%[din]]!     \n"
        "vld1.8   d25, [%[din]]!     \n"
        "vld1.8   d6 , [%[din]]!     \n"
        "vld1.8   d26, [%[din]]!     \n"
        "vld1.8   d7 , [%[din]]!     \n"
        "vld1.8   d27, [%[din]]!     \n"

        // unzip 4 pixels
        "vuzp.32  d0, d1  \n"
        "vuzp.32  d2, d3  \n"
        "vuzp.32  d4, d5  \n"
        "vuzp.32  d6, d7  \n"

        "vuzp.32  d20, d21  \n"
        "vuzp.32  d22, d23  \n"
        "vuzp.32  d24, d25  \n"
        "vuzp.32  d26, d27  \n"

        // unzip 2 pixels
        "vuzp.16  d0, d2  \n"
        "vuzp.16  d1, d3  \n"
        "vuzp.16  d4, d6  \n"
        "vuzp.16  d5, d7  \n"

        "vuzp.16  d20, d22  \n"
        "vuzp.16  d21, d23  \n"
        "vuzp.16  d24, d26  \n"
        "vuzp.16  d25, d27  \n"

        // unzip single pixel
        "vuzp.8   d0, d4  \n"
        "vuzp.8   d2, d6  \n"
        "vuzp.8   d1, d5  \n"
        "vuzp.8   d3, d7  \n"

        "vuzp.8   d20, d24  \n"
        "vuzp.8   d22, d26  \n"
        "vuzp.8   d21, d25  \n"
        "vuzp.8   d23, d27  \n"

        // store to dout_array[0] - dout_array[7]
        "vst1.8   d0, [%[l_0]]  \n"
        "vst1.8   d4, [%[l_1]]  \n"
        "vst1.8   d2, [%[l_2]]  \n"
        "vst1.8   d6, [%[l_3]]  \n"
        "vst1.8   d1, [%[l_4]]  \n"
        "vst1.8   d5, [%[l_5]]  \n"
        "vst1.8   d3, [%[l_6]]  \n"
        "vst1.8   d7, [%[l_7]]  \n"

        // calculate dout_array[8] - dout_array[15]
        "add   %[l_0], %[l_0], %[len]  \n"
        "add   %[l_1], %[l_1], %[len]  \n"
        "add   %[l_2], %[l_2], %[len]  \n"
        "add   %[l_3], %[l_3], %[len]  \n"
        "add   %[l_4], %[l_4], %[len]  \n"
        "add   %[l_5], %[l_5], %[len]  \n"
        "add   %[l_6], %[l_6], %[len]  \n"
        "add   %[l_7], %[l_7], %[len]  \n"

        // store to dout_array[8] - dout_array[15]
        "vst1.8   d20, [%[l_0]]  \n"
        "vst1.8   d24, [%[l_1]]  \n"
        "vst1.8   d22, [%[l_2]]  \n"
        "vst1.8   d26, [%[l_3]]  \n"
        "vst1.8   d21, [%[l_4]]  \n"
        "vst1.8   d25, [%[l_5]]  \n"
        "vst1.8   d23, [%[l_6]]  \n"
        "vst1.8   d27, [%[l_7]]  \n"

        // calculate next dout_array[0] - dout_array[7]
        "sub   %[l_0], %[l_0], r2   \n"
        "sub   %[l_1], %[l_1], r2   \n"
        "sub   %[l_2], %[l_2], r2   \n"
        "sub   %[l_3], %[l_3], r2   \n"
        "sub   %[l_4], %[l_4], r2   \n"
        "sub   %[l_5], %[l_5], r2   \n"
        "sub   %[l_6], %[l_6], r2   \n"
        "sub   %[l_7], %[l_7], r2   \n"

        "subs   %[iter], %[iter], #1  \n"
        "bne    loop                  \n"


        : [din] "+r"(din),
        [l_0] "+r"(l_0),
        [l_1] "+r"(l_1),
        [l_2] "+r"(l_2),
        [l_3] "+r"(l_3),
        [l_4] "+r"(l_4),
        [l_5] "+r"(l_5),
        [l_6] "+r"(l_6),
        [l_7] "+r"(l_7)

        : [len] "r"(len),
        [iter] "r"(iter)

        : "cc", "memory", "r2",
        "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
        "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27"
    );
}