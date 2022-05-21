#include "neon_rearrange.h"

void NeonInputRearrange_3(int8_t* din, int8_t* dout,
    const int c, const int h, const int w, const int pad) {

    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int dout_offset = 0;
    int dout_array_length = (h + 2 * pad) * (w + 2 * pad);
    for (int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        // dout_array: (1, h_pad_ w_pad)
        dout_array[0] = din + i * (dout_array_length * INPUT_EXTEND_SCALE);

        for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + dout_array_length;
        }

        int num_iter = dout_array_length / 8;
        int leftover = dout_array_length % 8;

        if (num_iter > 0) {
            asm_rearrange_16_layers(dout_array[0], dout_array[1], dout_array[2], dout_array[3],
                dout_array[4], dout_array[5], dout_array[6], dout_array[7],
                dout, num_iter, dout_array_length);

            dout += num_iter * 8 * 16;

            for (int k = 0; k < INPUT_EXTEND_SCALE; k++)
                dout_array[k] += 8 * num_iter;
        }

        if (leftover > 0)
            for (int i = 0; i < leftover; i++)
                for (int j = 0; j < INPUT_EXTEND_SCALE; j++)
                    *(dout++) = *(dout_array[j]++);
    }
}