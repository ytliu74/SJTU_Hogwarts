#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <arm_neon.h>

#define INPUT_EXTEND_SCALE 16

int UpRound(int a, int b){
    return (a - 1) / b + 1;
}

void InputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
        const int w, const int pad){
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for(int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        // dout_array: (1, h_pad_ w_pad)
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for(int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
        }
        for(int r = 0; r < (h + 2 * pad); r++) {
            for(int c = 0; c < (w + 2 * pad); c++) {
                for(int k = 0; k < 16; k++) {
                    dout[idx_fpga_idata++] = *(dout_array[k]++);
                }
            }
        }
    }
}

void NeonInputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
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

int main()
{
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};

    int8_t din[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

    int8_t dout_1[256];
    int8_t dout_2[256];

    int c = 32;
    int h = 2;
    int w = 4;
    int pad = 0;

    clock_gettime(CLOCK_REALTIME, &time1);
    for (int n = 0; n < 1000; n ++)
        InputRearrange(din, dout_1, c, h, w, pad);
    clock_gettime(CLOCK_REALTIME, &time2);

    float time_elapsed_1 = (time2.tv_sec - time1.tv_sec)*1000 + (time2.tv_nsec - time1.tv_nsec)/1000000;

    clock_gettime(CLOCK_REALTIME, &time1);
    for (int n = 0; n < 1000; n ++)
        NeonInputRearrange(din, dout_2, c, h, w, pad);
    clock_gettime(CLOCK_REALTIME, &time2);

    float time_elapsed_2 = (time2.tv_sec - time1.tv_sec)*1000 + (time2.tv_nsec - time1.tv_nsec)/1000000;

    bool hasWrong = false;

    for (int i = 0; i < 255; i++)
        if (dout_1[i] != dout_2[i]) {
            printf("Wrong at %d \n", i);
            hasWrong = true;
        }

    if (hasWrong)
        for (int k = 0; k < 255; k++){
            printf("dout_1[%d]: %d, dout_2[%d]: %d \n", k, dout_1[k], k, dout_2[k]);
        }
    else printf("Results match \n");

    printf("Origin: %f ms \n", time_elapsed_1);
    printf("With Neon: %f ms \n", time_elapsed_2);

    return 0;
}