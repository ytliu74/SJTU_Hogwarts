#include <arm_neon.h>

#include "neon_rearrange.h"


void NeonInputRearrange_2(int8_t* din, int8_t* dout, const int c, const int h,
        const int w, const int pad){
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for(int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for(int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
        }

        int dout_array_len = (h + 2 * pad) * (w + 2 * pad);

        int offset_i = dout_array_len * 16 * i;

        int num_8x8 = UpRound(dout_array_len, 8);

        int8x8_t pixel_array_0;
        int8x8_t pixel_array_1;
        int8x8_t pixel_array_2;
        int8x8_t pixel_array_3;
        int8x8_t pixel_array_4;
        int8x8_t pixel_array_5;
        int8x8_t pixel_array_6;
        int8x8_t pixel_array_7;
        int8x8_t pixel_array_8;
        int8x8_t pixel_array_9;
        int8x8_t pixel_array_10;
        int8x8_t pixel_array_11;
        int8x8_t pixel_array_12;
        int8x8_t pixel_array_13;
        int8x8_t pixel_array_14;
        int8x8_t pixel_array_15;

        for (int k = 0; k < num_8x8; k ++) {
            int offset = k * 16 * 8 + offset_i;

            pixel_array_0 = vld1_s8(dout_array[0] + 8 * k);
            pixel_array_1 = vld1_s8(dout_array[1] + 8 * k);
            pixel_array_2 = vld1_s8(dout_array[2] + 8 * k);
            pixel_array_3 = vld1_s8(dout_array[3] + 8 * k);
            pixel_array_4 = vld1_s8(dout_array[4] + 8 * k);
            pixel_array_5 = vld1_s8(dout_array[5] + 8 * k);
            pixel_array_6 = vld1_s8(dout_array[6] + 8 * k);
            pixel_array_7 = vld1_s8(dout_array[7] + 8 * k);
            pixel_array_8 = vld1_s8(dout_array[8] + 8 * k);
            pixel_array_9 = vld1_s8(dout_array[9] + 8 * k);
            pixel_array_10 = vld1_s8(dout_array[10] + 8 * k);
            pixel_array_11 = vld1_s8(dout_array[11] + 8 * k);
            pixel_array_12 = vld1_s8(dout_array[12] + 8 * k);
            pixel_array_13 = vld1_s8(dout_array[13] + 8 * k);
            pixel_array_14 = vld1_s8(dout_array[14] + 8 * k);
            pixel_array_15 = vld1_s8(dout_array[15] + 8 * k);

            dout[offset + 0] = vget_lane_s8(pixel_array_0, 0);
            dout[offset + 1] = vget_lane_s8(pixel_array_1, 0);
            dout[offset + 2] = vget_lane_s8(pixel_array_2, 0);
            dout[offset + 3] = vget_lane_s8(pixel_array_3, 0);
            dout[offset + 4] = vget_lane_s8(pixel_array_4, 0);
            dout[offset + 5] = vget_lane_s8(pixel_array_5, 0);
            dout[offset + 6] = vget_lane_s8(pixel_array_6, 0);
            dout[offset + 7] = vget_lane_s8(pixel_array_7, 0);
            dout[offset + 8] = vget_lane_s8(pixel_array_8, 0);
            dout[offset + 9] = vget_lane_s8(pixel_array_9, 0);
            dout[offset + 10] = vget_lane_s8(pixel_array_10, 0);
            dout[offset + 11] = vget_lane_s8(pixel_array_11, 0);
            dout[offset + 12] = vget_lane_s8(pixel_array_12, 0);
            dout[offset + 13] = vget_lane_s8(pixel_array_13, 0);
            dout[offset + 14] = vget_lane_s8(pixel_array_14, 0);
            dout[offset + 15] = vget_lane_s8(pixel_array_15, 0);
            dout[offset + 16] = vget_lane_s8(pixel_array_0, 1);
            dout[offset + 17] = vget_lane_s8(pixel_array_1, 1);
            dout[offset + 18] = vget_lane_s8(pixel_array_2, 1);
            dout[offset + 19] = vget_lane_s8(pixel_array_3, 1);
            dout[offset + 20] = vget_lane_s8(pixel_array_4, 1);
            dout[offset + 21] = vget_lane_s8(pixel_array_5, 1);
            dout[offset + 22] = vget_lane_s8(pixel_array_6, 1);
            dout[offset + 23] = vget_lane_s8(pixel_array_7, 1);
            dout[offset + 24] = vget_lane_s8(pixel_array_8, 1);
            dout[offset + 25] = vget_lane_s8(pixel_array_9, 1);
            dout[offset + 26] = vget_lane_s8(pixel_array_10, 1);
            dout[offset + 27] = vget_lane_s8(pixel_array_11, 1);
            dout[offset + 28] = vget_lane_s8(pixel_array_12, 1);
            dout[offset + 29] = vget_lane_s8(pixel_array_13, 1);
            dout[offset + 30] = vget_lane_s8(pixel_array_14, 1);
            dout[offset + 31] = vget_lane_s8(pixel_array_15, 1);
            dout[offset + 32] = vget_lane_s8(pixel_array_0, 2);
            dout[offset + 33] = vget_lane_s8(pixel_array_1, 2);
            dout[offset + 34] = vget_lane_s8(pixel_array_2, 2);
            dout[offset + 35] = vget_lane_s8(pixel_array_3, 2);
            dout[offset + 36] = vget_lane_s8(pixel_array_4, 2);
            dout[offset + 37] = vget_lane_s8(pixel_array_5, 2);
            dout[offset + 38] = vget_lane_s8(pixel_array_6, 2);
            dout[offset + 39] = vget_lane_s8(pixel_array_7, 2);
            dout[offset + 40] = vget_lane_s8(pixel_array_8, 2);
            dout[offset + 41] = vget_lane_s8(pixel_array_9, 2);
            dout[offset + 42] = vget_lane_s8(pixel_array_10, 2);
            dout[offset + 43] = vget_lane_s8(pixel_array_11, 2);
            dout[offset + 44] = vget_lane_s8(pixel_array_12, 2);
            dout[offset + 45] = vget_lane_s8(pixel_array_13, 2);
            dout[offset + 46] = vget_lane_s8(pixel_array_14, 2);
            dout[offset + 47] = vget_lane_s8(pixel_array_15, 2);
            dout[offset + 48] = vget_lane_s8(pixel_array_0, 3);
            dout[offset + 49] = vget_lane_s8(pixel_array_1, 3);
            dout[offset + 50] = vget_lane_s8(pixel_array_2, 3);
            dout[offset + 51] = vget_lane_s8(pixel_array_3, 3);
            dout[offset + 52] = vget_lane_s8(pixel_array_4, 3);
            dout[offset + 53] = vget_lane_s8(pixel_array_5, 3);
            dout[offset + 54] = vget_lane_s8(pixel_array_6, 3);
            dout[offset + 55] = vget_lane_s8(pixel_array_7, 3);
            dout[offset + 56] = vget_lane_s8(pixel_array_8, 3);
            dout[offset + 57] = vget_lane_s8(pixel_array_9, 3);
            dout[offset + 58] = vget_lane_s8(pixel_array_10, 3);
            dout[offset + 59] = vget_lane_s8(pixel_array_11, 3);
            dout[offset + 60] = vget_lane_s8(pixel_array_12, 3);
            dout[offset + 61] = vget_lane_s8(pixel_array_13, 3);
            dout[offset + 62] = vget_lane_s8(pixel_array_14, 3);
            dout[offset + 63] = vget_lane_s8(pixel_array_15, 3);
            dout[offset + 64] = vget_lane_s8(pixel_array_0, 4);
            dout[offset + 65] = vget_lane_s8(pixel_array_1, 4);
            dout[offset + 66] = vget_lane_s8(pixel_array_2, 4);
            dout[offset + 67] = vget_lane_s8(pixel_array_3, 4);
            dout[offset + 68] = vget_lane_s8(pixel_array_4, 4);
            dout[offset + 69] = vget_lane_s8(pixel_array_5, 4);
            dout[offset + 70] = vget_lane_s8(pixel_array_6, 4);
            dout[offset + 71] = vget_lane_s8(pixel_array_7, 4);
            dout[offset + 72] = vget_lane_s8(pixel_array_8, 4);
            dout[offset + 73] = vget_lane_s8(pixel_array_9, 4);
            dout[offset + 74] = vget_lane_s8(pixel_array_10, 4);
            dout[offset + 75] = vget_lane_s8(pixel_array_11, 4);
            dout[offset + 76] = vget_lane_s8(pixel_array_12, 4);
            dout[offset + 77] = vget_lane_s8(pixel_array_13, 4);
            dout[offset + 78] = vget_lane_s8(pixel_array_14, 4);
            dout[offset + 79] = vget_lane_s8(pixel_array_15, 4);
            dout[offset + 80] = vget_lane_s8(pixel_array_0, 5);
            dout[offset + 81] = vget_lane_s8(pixel_array_1, 5);
            dout[offset + 82] = vget_lane_s8(pixel_array_2, 5);
            dout[offset + 83] = vget_lane_s8(pixel_array_3, 5);
            dout[offset + 84] = vget_lane_s8(pixel_array_4, 5);
            dout[offset + 85] = vget_lane_s8(pixel_array_5, 5);
            dout[offset + 86] = vget_lane_s8(pixel_array_6, 5);
            dout[offset + 87] = vget_lane_s8(pixel_array_7, 5);
            dout[offset + 88] = vget_lane_s8(pixel_array_8, 5);
            dout[offset + 89] = vget_lane_s8(pixel_array_9, 5);
            dout[offset + 90] = vget_lane_s8(pixel_array_10, 5);
            dout[offset + 91] = vget_lane_s8(pixel_array_11, 5);
            dout[offset + 92] = vget_lane_s8(pixel_array_12, 5);
            dout[offset + 93] = vget_lane_s8(pixel_array_13, 5);
            dout[offset + 94] = vget_lane_s8(pixel_array_14, 5);
            dout[offset + 95] = vget_lane_s8(pixel_array_15, 5);
            dout[offset + 96] = vget_lane_s8(pixel_array_0, 6);
            dout[offset + 97] = vget_lane_s8(pixel_array_1, 6);
            dout[offset + 98] = vget_lane_s8(pixel_array_2, 6);
            dout[offset + 99] = vget_lane_s8(pixel_array_3, 6);
            dout[offset + 100] = vget_lane_s8(pixel_array_4, 6);
            dout[offset + 101] = vget_lane_s8(pixel_array_5, 6);
            dout[offset + 102] = vget_lane_s8(pixel_array_6, 6);
            dout[offset + 103] = vget_lane_s8(pixel_array_7, 6);
            dout[offset + 104] = vget_lane_s8(pixel_array_8, 6);
            dout[offset + 105] = vget_lane_s8(pixel_array_9, 6);
            dout[offset + 106] = vget_lane_s8(pixel_array_10, 6);
            dout[offset + 107] = vget_lane_s8(pixel_array_11, 6);
            dout[offset + 108] = vget_lane_s8(pixel_array_12, 6);
            dout[offset + 109] = vget_lane_s8(pixel_array_13, 6);
            dout[offset + 110] = vget_lane_s8(pixel_array_14, 6);
            dout[offset + 111] = vget_lane_s8(pixel_array_15, 6);
            dout[offset + 112] = vget_lane_s8(pixel_array_0, 7);
            dout[offset + 113] = vget_lane_s8(pixel_array_1, 7);
            dout[offset + 114] = vget_lane_s8(pixel_array_2, 7);
            dout[offset + 115] = vget_lane_s8(pixel_array_3, 7);
            dout[offset + 116] = vget_lane_s8(pixel_array_4, 7);
            dout[offset + 117] = vget_lane_s8(pixel_array_5, 7);
            dout[offset + 118] = vget_lane_s8(pixel_array_6, 7);
            dout[offset + 119] = vget_lane_s8(pixel_array_7, 7);
            dout[offset + 120] = vget_lane_s8(pixel_array_8, 7);
            dout[offset + 121] = vget_lane_s8(pixel_array_9, 7);
            dout[offset + 122] = vget_lane_s8(pixel_array_10, 7);
            dout[offset + 123] = vget_lane_s8(pixel_array_11, 7);
            dout[offset + 124] = vget_lane_s8(pixel_array_12, 7);
            dout[offset + 125] = vget_lane_s8(pixel_array_13, 7);
            dout[offset + 126] = vget_lane_s8(pixel_array_14, 7);
            dout[offset + 127] = vget_lane_s8(pixel_array_15, 7);
        }
    }
}