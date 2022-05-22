#include "output_rearrange.h"

void NeonRearrange_1(int8_t* din, int8_t* dout, const int c, const int h,
    const int w) {
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    int array_length = h * w;
    for (int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        dout_array[0] = dout + i * array_length * INPUT_EXTEND_SCALE;
        for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + array_length;
        }

        int num_iter = array_length / 8;
        int leftover = array_length % 8;

        if (num_iter > 0) {
            asm_output_rearrange_16_layers(din, dout_array[0], dout_array[1], dout_array[2], dout_array[3],
                dout_array[4], dout_array[5], dout_array[6], dout_array[7], num_iter, array_length);

            din += num_iter * 8 * 16;

            for (int k = 0; k < INPUT_EXTEND_SCALE; k++)
                dout_array[k] += 8 * num_iter;
        }
        if (leftover > 0) {
            for (int i = 0; i < leftover; i ++)
                for (int j = 0; j < INPUT_EXTEND_SCALE; j++)
                    *(dout_array[j]++) = *(din++);
        }
    }
}