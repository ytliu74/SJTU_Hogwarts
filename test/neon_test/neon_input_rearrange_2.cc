#include "neon_rearrange.h"

void NeonInputRearrange_2(int8_t* din, int8_t* dout, const int c, const int h, const int w, const int pad) {
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int dout_offset = 0;
    for (int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        // dout_array: (1, h_pad_ w_pad)
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for (int n = 1; n < INPUT_EXTEND_SCALE; n++) {
            dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
        }

        int num_batch = (h + 2 * pad) * (w + 2 * pad) / 8;
        for (int batch_i = 0; batch_i < num_batch; batch_i++) {
            rearrange_8_layers(dout_array[0], dout_array[1], dout_array[2], dout_array[3],
                dout_array[4], dout_array[5], dout_array[6], dout_array[7], batch_i, (int64_t*)dout, dout_offset, 0);

            rearrange_8_layers(dout_array[8], dout_array[9], dout_array[10], dout_array[11],
                dout_array[12], dout_array[13], dout_array[14], dout_array[15], batch_i, (int64_t*)dout, dout_offset, 1);

            dout_offset += 16;
        }
    }
}