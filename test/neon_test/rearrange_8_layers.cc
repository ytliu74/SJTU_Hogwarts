#include "neon_rearrange.h"

void rearrange_8_layers(int8_t* l_0, int8_t* l_1, int8_t* l_2, int8_t* l_3,
    int8_t* l_4, int8_t* l_5, int8_t* l_6, int8_t* l_7, int64_t* out, int pos) {

    int8x8_t d_array_0 = vld1_s8(l_0);
    int8x8_t d_array_1 = vld1_s8(l_1);
    int8x8_t d_array_2 = vld1_s8(l_2);
    int8x8_t d_array_3 = vld1_s8(l_3);
    int8x8_t d_array_4 = vld1_s8(l_4);
    int8x8_t d_array_5 = vld1_s8(l_5);
    int8x8_t d_array_6 = vld1_s8(l_6);
    int8x8_t d_array_7 = vld1_s8(l_7);

    int8x8x2_t zip_01 = vzip_s8(d_array_0, d_array_1);
    int8x8x2_t zip_23 = vzip_s8(d_array_2, d_array_3);
    int8x8x2_t zip_45 = vzip_s8(d_array_4, d_array_5);
    int8x8x2_t zip_67 = vzip_s8(d_array_6, d_array_7);

    int16x4_t zip_01_0 = (int16x4_t)zip_01.val[0]; // a1b1 a2b2 a3b3 a4b4
    int16x4_t zip_01_1 = (int16x4_t)zip_01.val[1]; // a5b5 a6b6 a7b7 a8b8
    int16x4_t zip_23_0 = (int16x4_t)zip_23.val[0]; // c1d1 c2d2 c3d3 c4d4
    int16x4_t zip_23_1 = (int16x4_t)zip_23.val[1]; // c5d5 c6d6 c7d7 c8d8
    int16x4_t zip_45_0 = (int16x4_t)zip_45.val[0];
    int16x4_t zip_45_1 = (int16x4_t)zip_45.val[1];
    int16x4_t zip_67_0 = (int16x4_t)zip_67.val[0];
    int16x4_t zip_67_1 = (int16x4_t)zip_67.val[1];

    int16x4x2_t zip_01_23_0 = vzip_s16(zip_01_0, zip_23_0);
    int16x4x2_t zip_01_23_1 = vzip_s16(zip_01_1, zip_23_1);

    int16x4x2_t zip_45_67_0 = vzip_s16(zip_45_0, zip_67_0);
    int16x4x2_t zip_45_67_1 = vzip_s16(zip_45_1, zip_67_1);

    int32x2_t zip_01_23_0_0 = (int32x2_t)zip_01_23_0.val[0]; // a1b1c1d1 a2b2c2d2
    int32x2_t zip_01_23_0_1 = (int32x2_t)zip_01_23_0.val[1]; // a3b3c3d3 a4b4c4d4
    int32x2_t zip_01_23_1_0 = (int32x2_t)zip_01_23_1.val[0]; // a5b5c5d5 a6b6c6d6
    int32x2_t zip_01_23_1_1 = (int32x2_t)zip_01_23_1.val[1]; // a7b7c7d7 a8b8c8d8

    int32x2_t zip_45_67_0_0 = (int32x2_t)zip_45_67_0.val[0]; // e1f1g1h1 e2f2g2h2
    int32x2_t zip_45_67_0_1 = (int32x2_t)zip_45_67_0.val[1]; // e3f3g3h3 e4f4g4h4
    int32x2_t zip_45_67_1_0 = (int32x2_t)zip_45_67_1.val[0]; // e5f5g5h5 e6f6g6h6
    int32x2_t zip_45_67_1_1 = (int32x2_t)zip_45_67_1.val[1]; // e7f7g7h7 e8f8g8h8

    int32x2x2_t zip_01_23_45_67_0 = vzip_s32(zip_01_23_0_0, zip_45_67_0_0);
    int32x2x2_t zip_01_23_45_67_1 = vzip_s32(zip_01_23_0_1, zip_45_67_0_1);
    int32x2x2_t zip_01_23_45_67_2 = vzip_s32(zip_01_23_1_0, zip_45_67_1_0);
    int32x2x2_t zip_01_23_45_67_3 = vzip_s32(zip_01_23_1_1, zip_45_67_1_1);

    int64x1_t zip_0 = (int64x1_t)zip_01_23_45_67_0.val[0];    // a1b1c1d1e1f1g1h1
    int64x1_t zip_1 = (int64x1_t)zip_01_23_45_67_0.val[1];    // a2b2c2d2e2f2g2h2
    int64x1_t zip_2 = (int64x1_t)zip_01_23_45_67_1.val[0];    // a3b3c3d3e3f3g3h3
    int64x1_t zip_3 = (int64x1_t)zip_01_23_45_67_1.val[1];    // a4b4c4d4e4f4g4h4
    int64x1_t zip_4 = (int64x1_t)zip_01_23_45_67_2.val[0];    // a5b5c5d5e5f5g5h5
    int64x1_t zip_5 = (int64x1_t)zip_01_23_45_67_2.val[1];    // a6b6c6d6e6f6g6h6
    int64x1_t zip_6 = (int64x1_t)zip_01_23_45_67_3.val[0];    // a7b7c7d7e7f7g7h7
    int64x1_t zip_7 = (int64x1_t)zip_01_23_45_67_3.val[1];    // a8b8c8d8e8f8g8h8


    vst1_s64(out + pos + 0, zip_0);
    vst1_s64(out + pos + 2, zip_1);
    vst1_s64(out + pos + 4, zip_2);
    vst1_s64(out + pos + 6, zip_3);
    vst1_s64(out + pos + 8, zip_4);
    vst1_s64(out + pos + 10, zip_5);
    vst1_s64(out + pos + 12, zip_6);
    vst1_s64(out + pos + 14, zip_7);
}