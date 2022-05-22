#include "output_rearrange.h"

int ch[11] = {  16,  32,  64, 64, 128, 128, 256, 512, 512, 1024, 1024 };
int he[11] = { 224, 112, 112, 56,  56,  28,  28,  14,   7,    7,    1 };
int wi[11] = { 224, 112, 112, 56,  56,  28,  28,  14,   7,    7,    1 };

int UpRound(int a, int b) {
    return (a - 1) / b + 1;
}

void OutputRearrange(int8_t* din, int8_t* dout, const int c, const int h,
    const int w) {
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for (int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        dout_array[0] = dout + i * h * w * INPUT_EXTEND_SCALE;
        for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + h * w;
        }
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++) {
                for (int k = 0; k < 16; k++) {
                    *(dout_array[k]++) = din[idx_fpga_idata++];
                }
            }
        }
    }
}

bool test_functionality(void (*rearrange)(int8_t*, int8_t*, const int, const int, const int)) {
    bool pass = true;

    for (int i = 0; i < 11; i ++) {
        int c = ch[i];
        int h = he[i];
        int w = wi[i];

        int data_length = c * h * w;

        int8_t* din = new int8_t[data_length];
        for (int i = 0; i < data_length; i++)
            din[i] = i % 175;

        int8_t* dout_0 = new int8_t[data_length];
        int8_t* dout_1 = new int8_t[data_length];

        OutputRearrange(din, dout_0, c, h, w);
        (*rearrange)(din, dout_1, c, h, w);

        for (int i = 0; i < data_length - 1; i++)
            if (dout_0[i] != dout_1[i]) {
                pass = false;
                std::cout << "Functionality failed at "
                    << "c: " << c << " h: " << h << " w:" << w << std::endl;
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

void test_performance(void (*rearrange)(int8_t*, int8_t*, const int, const int, const int)) {

    for (int i = 0; i < 11; i++) {
        int c = ch[i];
        int h = he[i];
        int w = wi[i];

        int data_length = c * h * w;

        int8_t* din = new int8_t[data_length];
        for (int i = 0; i < data_length; i++)
            din[i] = i % 143;

        struct timespec time1 = { 0, 0 };
        struct timespec time2 = { 0, 0 };
        struct timespec time3 = { 0, 0 };
        struct timespec time4 = { 0, 0 };

        int8_t* dout_0 = new int8_t[data_length];
        int8_t* dout_1 = new int8_t[data_length];

        clock_gettime(CLOCK_REALTIME, &time1);
        for (int n = 0; n < 500; n++)
            OutputRearrange(din, dout_1, c, h, w);
        clock_gettime(CLOCK_REALTIME, &time2);

        float time_elapsed_1 = (time2.tv_sec - time1.tv_sec) * 1000 + (time2.tv_nsec - time1.tv_nsec) / 1000000;

        clock_gettime(CLOCK_REALTIME, &time3);
        for (int n = 0; n < 500; n++)
            (*rearrange)(din, dout_1, c, h, w);
        clock_gettime(CLOCK_REALTIME, &time4);

        float time_elapsed_2 = (time4.tv_sec - time3.tv_sec) * 1000 + (time4.tv_nsec - time3.tv_nsec) / 1000000;

        delete[] din;
        delete[] dout_0;
        delete[] dout_1;

        float speed_retio = time_elapsed_1 / time_elapsed_2;

        std::cout << "For: " << "c: " << c << " h: " << h << " w:" << w  << std::endl;
        std::cout << "Origin: " << time_elapsed_1 << " ms" << std::endl;
        std::cout << "Neon: " << time_elapsed_2 << " ms" << std::endl;
        std::cout << "Speed ratio: " << speed_retio << std::endl;
        std::cout << "----------------------" << std::endl;
    }
}

int main(int argc, char** argv) {

    if (test_functionality(NeonRearrange_1))
        test_performance(NeonRearrange_1);

    // int h = 2;
    // int w = 4;
    // int8_t* dout = new int8_t[h * w * 8];

    // int8_t din[64] = {0, 0, 0, 0, 0, 0, 0, 0,
    //                1, 1, 1, 1, 1, 1, 1, 1,
    //                2, 2, 2, 2, 2, 2, 2, 2,
    //                3, 3, 3, 3, 3, 3, 3, 3,
    //                4, 4, 4, 4, 4, 4, 4, 4,
    //                5, 5, 5, 5, 5, 5, 5, 5,
    //                6, 6, 6, 6, 6, 6, 6, 6,
    //                7, 7, 7, 7, 7, 7, 7, 7};

    // int8_t* dout_array[INPUT_EXTEND_SCALE];

    // dout_array[0] = dout + h * w * INPUT_EXTEND_SCALE;
    // for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
    //     dout_array[n] = dout_array[n - 1] + h * w;
    // }

    // asm_output_rearrange_16_layers(din, dout_array[0], dout_array[1], dout_array[2], dout_array[3],
    //             dout_array[4], dout_array[5], dout_array[6], dout_array[7]);

    // for (int i = 0; i < 8; i++)
    //     for (int j = 0; j < 8; j++)
    //         std::cout << "dout_array[" << i << "][" << j << "]: " << (int)dout_array[i][j] << std::endl;

    return 0;
}