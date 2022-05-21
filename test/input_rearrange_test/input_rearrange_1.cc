#include <arm_neon.h>

#include "neon_rearrange.h"

void InputRearrange_1(int8_t* din, int8_t* dout, const int c, const int h,
        const int w, const int pad){
    int8_t* dout_array[INPUT_EXTEND_SCALE];
    int idx_fpga_idata = 0;
    for(int i = 0; i < UpRound(c, INPUT_EXTEND_SCALE); i++) {
        // dout_array: (1, h_pad_ w_pad)
        dout_array[0] = din + i * ((h + 2 * pad) * (w + 2 * pad) * INPUT_EXTEND_SCALE);
        for(int n = 0; n < INPUT_EXTEND_SCALE; n++) {
            if (n > 0)
                dout_array[n] = dout_array[n - 1] + (h + 2 * pad) * (w + 2 * pad);
            int idx = 0;
            int8_t* dout_addr = dout_array[n];
            for (int r = 0; r < (h + 2 * pad); r ++)
                for (int c = 0; c < (w + 2 * pad); c ++)
                    dout[n + 16 * (idx ++)] = *(dout_addr ++);
        }
    }
}