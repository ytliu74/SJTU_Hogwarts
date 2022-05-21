#include <arm_neon.h>

#include "neon_rearrange.h"


void NeonInputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h,
        const int w, const int pad){
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for(int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for(int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
        }

        int len = (h + 2 * pad) * (w + 2 * pad);
        int32x4x4_t dout_array_vec;
        int8x16_t dout_val_vec;
        int32x4_t one;
        one = vdupq_n_s32(1);
        dout_array_vec = vld4q_s32((int32_t *)dout_array);
        for (int j =0; j < len; j++){
            int8_t dout_val[16] = {*(int8_t *)vgetq_lane_s32(dout_array_vec.val[0],0),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[1],0),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[2],0),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[3],0),
                                   *(int8_t *)vgetq_lane_s32(dout_array_vec.val[0],1),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[1],1),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[2],1),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[3],1),
                                   *(int8_t *)vgetq_lane_s32(dout_array_vec.val[0],2),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[1],2),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[2],2),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[3],2),
                                   *(int8_t *)vgetq_lane_s32(dout_array_vec.val[0],3),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[1],3),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[2],3),*(int8_t *)vgetq_lane_s32(dout_array_vec.val[3],3)};
            dout_val_vec = vld1q_s8(dout_val);
            vst1q_s8((dout + 16 * j + i * 16 * len),dout_val_vec) ;
            for(int m = 0; m < 4; m++)
            dout_array_vec.val[m] = vaddq_s32(dout_array_vec.val[m],one);
        }
        vst4q_s32((int32_t *)dout_array,dout_array_vec);
    }
}