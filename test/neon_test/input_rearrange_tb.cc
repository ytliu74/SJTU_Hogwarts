#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <arm_neon.h>
#include "neon_rearrange.h"

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

int main()
{
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    struct timespec time3 = {0, 0};
    struct timespec time4 = {0, 0};

    int data_length = 8192;

    int8_t din[data_length];
    for (int i = 0; i < data_length; i++)
        din[i] = i % 16;


    int8_t dout_1[data_length];
    int8_t dout_2[data_length];

    int c = 16;
    int h = 16;
    int w = 32;
    int pad = 0;

    clock_gettime(CLOCK_REALTIME, &time1);
    for (int n = 0; n < 1000; n ++)
        InputRearrange(din, dout_1, c, h, w, pad);
    clock_gettime(CLOCK_REALTIME, &time2);

    float time_elapsed_1 = (time2.tv_sec - time1.tv_sec)*1000 + (time2.tv_nsec - time1.tv_nsec)/1000000;

    clock_gettime(CLOCK_REALTIME, &time3);
    for (int n = 0; n < 1000; n ++)
        InputRearrange_1(din, dout_2, c, h, w, pad);
    clock_gettime(CLOCK_REALTIME, &time4);

    float time_elapsed_2 = (time4.tv_sec - time3.tv_sec)*1000 + (time4.tv_nsec - time3.tv_nsec)/1000000;

    bool hasWrong = false;

    for (int i = 0; i < data_length - 1; i++)
        if (dout_1[i] != dout_2[i]) {
            printf("Wrong at %d \n", i);
            hasWrong = true;
        }

    if (hasWrong)
        for (int k = 0; k < data_length - 1; k++){
            printf("dout_1[%d]: %d, dout_2[%d]: %d \n", k, dout_1[k], k, dout_2[k]);
        }
    else printf("Results match \n");

    printf("Origin: %f ms \n", time_elapsed_1);
    printf("With Neon: %f ms \n", time_elapsed_2);

    return 0;
}