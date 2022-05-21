#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <string.h>

#include <arm_neon.h>
#include "neon_rearrange.h"

int ch[11] = { 16,  32,  64, 64, 128, 128, 256, 512, 512, 1024, 1024 };
int he[11] = { 224, 112, 112, 56,  56,  28,  28,  14,   7,    7,    1 };
int wi[11] = { 224, 112, 112, 56,  56,  28,  28,  14,   7,    7,    1 };
int padd[11] = { 0 };

int UpRound(int a, int b) {
    return (a - 1) / b + 1;
}

void InputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
    const int w, const int pad) {
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for (int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        // dout_array: (1, h_pad_ w_pad)
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
        }
        for (int r = 0; r < (h + 2 * pad); r++) {
            for (int c = 0; c < (w + 2 * pad); c++) {
                for (int k = 0; k < 16; k++) {
                    dout[idx_fpga_idata++] = *(dout_array[k]++);
                }
            }
        }
    }
}

bool test_functionality(void (*rearrange)(int8_t*, int8_t*, const int, const int, const int, const int)) {
    bool pass = true;

    for (int i = 0; i < 11; i++) {
        int c = ch[i];
        int h = he[i];
        int w = wi[i];
        int pad = padd[i];

        int data_length = c * (h + 2 * pad) * (w + 2 * pad);

        int8_t* din = new int8_t[data_length];
        for (int i = 0; i < data_length; i++)
            din[i] = i % 175;

        int8_t* dout_0 = new int8_t[data_length];
        int8_t* dout_1 = new int8_t[data_length];

        InputRearrange(din, dout_0, c, h, w, pad);
        (*rearrange)(din, dout_1, c, h, w, pad);

        for (int i = 0; i < data_length - 1; i++)
            if (dout_0[i] != dout_1[i]) {
                pass = false;
                std::cout << "Functionality failed at "
                    << "c: " << c << " h: " << h << " w:" << w << " pad: " << pad << std::endl;
                break;
            }

        delete[] din;
        delete[] dout_0;
        delete[] dout_1;
    }

    if (pass) {
        std::cout << "Functionality test passed." << std::endl;
        std::cout << "----------------------" << std::endl;
    }
    return pass;
}

void test_performance(void (*rearrange)(int8_t*, int8_t*, const int, const int, const int, const int)) {

    for (int i = 0; i < 11; i++) {
        int c = ch[i];
        int h = he[i];
        int w = wi[i];
        int pad = padd[i];

        int data_length = c * (h + 2 * pad) * (w + 2 * pad);

        int8_t* din = new int8_t[data_length];
        for (int i = 0; i < data_length; i++)
            din[i] = i % 16;

        struct timespec time1 = { 0, 0 };
        struct timespec time2 = { 0, 0 };
        struct timespec time3 = { 0, 0 };
        struct timespec time4 = { 0, 0 };

        int8_t* dout_0 = new int8_t[data_length];
        int8_t* dout_1 = new int8_t[data_length];

        clock_gettime(CLOCK_REALTIME, &time1);
        for (int n = 0; n < 500; n++)
            InputRearrange(din, dout_1, c, h, w, pad);
        clock_gettime(CLOCK_REALTIME, &time2);

        float time_elapsed_1 = (time2.tv_sec - time1.tv_sec) * 1000 + (time2.tv_nsec - time1.tv_nsec) / 1000000;

        clock_gettime(CLOCK_REALTIME, &time3);
        for (int n = 0; n < 500; n++)
            (*rearrange)(din, dout_1, c, h, w, pad);
        clock_gettime(CLOCK_REALTIME, &time4);

        float time_elapsed_2 = (time4.tv_sec - time3.tv_sec) * 1000 + (time4.tv_nsec - time3.tv_nsec) / 1000000;

        delete[] din;
        delete[] dout_0;
        delete[] dout_1;

        float speed_retio = time_elapsed_1 / time_elapsed_2;

        std::cout << "For: " << "c: " << c << " h: " << h << " w:" << w << " pad: " << pad << std::endl;
        std::cout << "Origin: " << time_elapsed_1 << " ms" << std::endl;
        std::cout << "Neon: " << time_elapsed_2 << " ms" << std::endl;
        std::cout << "Speed ratio: " << speed_retio << std::endl;
        std::cout << "----------------------" << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Need a function name" << std::endl;
        return -1;
    }

    if (strcmp(argv[1], "Neon_1") == 0) {
        if (test_functionality(NeonInputRearrange_1))
            test_performance(NeonInputRearrange_1);
    }
    else if (strcmp(argv[1], "Neon_2") == 0) {
        if (test_functionality(NeonInputRearrange_2))
            test_performance(NeonInputRearrange_2);
    }
    else if (strcmp(argv[1], "Neon_3") == 0) {
        if (test_functionality(NeonInputRearrange_3))
            test_performance(NeonInputRearrange_3);
    }
    else
        std::cout << "Invalid arguments" << std::endl;


    return 0;
}