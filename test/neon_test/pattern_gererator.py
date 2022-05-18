cnt = 0
for lane in range(8):
    for array_i in range(16):
        print(f"dout[offset + {cnt}] = vget_lane_s8(pixel_array_{array_i}, {lane});")
        cnt += 1