#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>  
#include <sys/mman.h>
#include <sys/times.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <assert.h>
#include <chrono>  // NOLINT(build/c++11)
//#include <osfile.h>
#include "qWorks.h"
#include "intelfpga.h"


// FD of nna
static int nna_fd = -1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int idx_layer = 0;

int UpRound(int a, int b){
    return (a - 1) / b + 1;
}

class Timer {
    private:
        std::chrono::high_resolution_clock::time_point inTime, outTime;

    public:
        void startTimer() { inTime = std::chrono::high_resolution_clock::now(); }

        // unit millisecond
        float getCostTimer() {
            outTime = std::chrono::high_resolution_clock::now();
            return static_cast<float>(
                    std::chrono::duration_cast<std::chrono::microseconds>(outTime - inTime)
                    .count() /
                    1e+3);
        }
};

class Timer2 {
    private:
        tms time_start, time_end;
        int sc_clk_tck;
        clock_t head, tail;

    public:
        Timer2() {
            sc_clk_tck = sysconf(_SC_CLK_TCK);
        }
        void startTimer() {
            head = times(&time_start);
        }
        void getCostTimer() {
            tail = times(&time_end);
            std::cout << "Real time (second): " << (tail - head) / (double)sc_clk_tck << "\n";
            std::cout << " User time: " << (time_end.tms_utime - time_start.tms_utime) / (double) sc_clk_tck << "\n";
            std::cout << " Sys time: " << (time_end.tms_stime - time_start.tms_stime) / (double) sc_clk_tck << "\n";
            std::cout << " Child sys time: " << (time_end.tms_cstime - time_start.tms_cstime) / (double) sc_clk_tck << "\n";
        }


};


void* intelfpga_malloc(size_t size) {
    return malloc(size);
}
void intelfpga_free(void *ptr) {
    free(ptr);
}

void fpga_copy_s8(int8_t* dst, const int8_t* src, int len) {
    memcpy(dst, src, len);
}

void fpga_copy_s32(int32_t* dst, const int32_t* src, int len) {
    memcpy(dst, src, 4 * len);
}

void fpga_copy_f32(float* dst, const float* src, int len) {
    memcpy(dst, src, 4 * len);
}


int intelfpga_open(void) 
{
    if (nna_fd<0) {
        nna_fd = open("/dev/nnadrv0", O_RDWR);
        if (nna_fd<0) return -1;
    }

    return 0;
}

void intelfpga_close(void) 
{
    if (nna_fd<0) 
        return;
    close(nna_fd);
    nna_fd = -1;
}


void PrintConvParam(nna_conv_s* conv_param) {
    static int idx = 0;
    std::cout <<"//This op is param is: \n";
    // std::cout <<"conv.extent_in_size = " << conv_param->extent_in_size <<";\n";
    // std::cout <<"conv.extent_weight_size = " << conv_param->extent_weight_size <<";\n";
    // std::cout <<"conv.extent_out_size = " << conv_param->extent_out_size <<";\n";
    // std::cout <<"conv.scale_size = " << conv_param->scale_size <<";\n";
    // std::cout <<"conv.param_size = " << conv_param->param_size <<";\n";
    // std::cout <<"conv.param"<<"["<<idx<<"]_size = " << conv_param->param_size <<";\n";
    std::cout <<"param"<<"["<<idx<<"].input_offset = " << conv_param->ip->input_offset <<";\n";
    std::cout <<"param"<<"["<<idx<<"].weight_offset = " << conv_param->ip->weight_offset <<";\n";
    std::cout <<"param"<<"["<<idx<<"].scale_offset = " << conv_param->ip->scale_offset <<";\n";
    std::cout <<"param"<<"["<<idx<<"].output_offset = " << conv_param->ip->output_offset <<";\n";
    std::cout <<"param"<<"["<<idx<<"].in_c = " << conv_param->ip->in_c <<";\n";
    std::cout <<"param"<<"["<<idx<<"].in_h = " << conv_param->ip->in_h <<";\n";
    std::cout <<"param"<<"["<<idx<<"].in_w = " << conv_param->ip->in_w <<";\n";
    std::cout <<"param"<<"["<<idx<<"].output_c = " << conv_param->ip->output_c <<";\n";
    std::cout <<"param"<<"["<<idx<<"].output_h = " << conv_param->ip->output_h <<";\n";
    std::cout <<"param"<<"["<<idx<<"].output_w = " << conv_param->ip->output_w <<";\n";
    std::cout <<"param"<<"["<<idx<<"].kernel = " << conv_param->ip->kernel <<";\n";
    std::cout <<"param"<<"["<<idx<<"].in_pad = " << conv_param->ip->in_pad <<";\n";
    std::cout <<"param"<<"["<<idx<<"].out_pad = " << conv_param->ip->out_pad <<";\n";
    std::cout <<"param"<<"["<<idx<<"].stride = " << conv_param->ip->stride <<";\n";
    std::cout <<"param"<<"["<<idx<<"].relu = " << conv_param->ip->relu <<";\n";
    std::cout <<"param"<<"["<<idx<<"].type = " << (int)conv_param->ip->type<<";\n";

    // std::cout <<"extent input:\n";
    // for(int i = 0; i <  conv_param->extent_in_size; i++) {
    //   std::cout << (int)conv_param->ia[i] << " ";
    // }
    std::cout <<"//------------------------------------------------------------\n";
    idx++;

}

bool IntelFpgaConvDepthwise(DeviceGraphNode* node, int8_t* din,
        const std::map<DeviceGraphNode*, int8_t*>& output_nodes,
        int max_output_size, int8_t* buff) { 
    // 1) Init device and device memory.
    if (intelfpga_open()) {
        return false;
    }
    nna_conv_s* conv = node->device_param_;
    IntelFpgaConvParam* param = conv->ip;

    // Inpput reorder for subgraph.
    int8_t* din_reordered = buff;
    for(int i = 0; i < conv->extent_in_size; i++){
        if (param->in_pad) {
            *(din_reordered + i) = IFPAD_FILL;
        } else {
            if (i >= param->in_c * param->in_w * param->in_h) {
                *(din_reordered + i) = IFNOTPAD_FILL;
            }
        }
        for(int j = 0; j < 20; j++) {
            asm("nop");
        }
    }
    if(param->in_pad) {
        conv_chw_pad(din, din_reordered, param->in_c, param->in_h, param->in_w, param->in_pad);
    } else {
        for(int c = 0; c < param->in_c; c++) {
            for(int w = 0; w < param->in_w; w++) {
                for(int h = 0; h < param->in_h; h++) {
                    *(din_reordered 
                            + (c * param->in_w * param->in_h)
                            + (w * param->in_h)
                            + h)
                        = *(din
                                + (c * param->in_w * param->in_h)
                                + (w * param->in_h)
                                + h);
                }
            }
        }
    }

    InputRearrange(din_reordered,
            conv->ia,
            param->in_c,
            param->in_h, 
            param->in_w,
            param->in_pad
            );

    // Call IP.
    //auto cur_node= node; 
    //while(cur_node) {
    //  cur_node->device_param_->ip->in_c = UpRound(cur_node->device_param_->ip->in_c, 
    //      INPUT_EXTEND_SCALE) * INPUT_EXTEND_SCALE;
    //  cur_node = cur_node->next_;
    //}
    pthread_mutex_lock(&mutex);
    if (ioctl(nna_fd, NNA_IOCTL_MAKE(NNA_CMD_CONV_SUBGRAPH), node)) {
        pthread_mutex_unlock(&mutex);
        printf("Interfpga conv-> failed\n");
        return false;
    }
    pthread_mutex_unlock(&mutex);


    // 5) Reordered output nodes and copy output from sdk to software.
    int8_t* dout_reordered = din_reordered;
    //  memset(din_reordered, 0, max_output_size);
    for(auto node2addr: output_nodes) { 
        //PrintConvParam(node2addr.first->device_param_);
        OutputRearrange(node2addr.first->device_param_->oa,
                dout_reordered,
                node2addr.first->device_param_->ip->output_c,
                node2addr.first->device_param_->ip->output_h,
                node2addr.first->device_param_->ip->output_w
                );
        for(int c = 0; c < node2addr.first->device_param_->ip->output_c; c++) {
            for(int w = 0; w < node2addr.first->device_param_->ip->output_w; w++) {
                for(int h = 0; h < node2addr.first->device_param_->ip->output_h; h++) {
                    *(node2addr.second 
                            + (c * node2addr.first->device_param_->ip->output_w * node2addr.first->device_param_->ip->output_h)
                            + (w * node2addr.first->device_param_->ip->output_h)
                            + h)
                        = *(dout_reordered
                                + (c * node2addr.first->device_param_->ip->output_w * node2addr.first->device_param_->ip->output_h)
                                + (w * node2addr.first->device_param_->ip->output_h)
                                + h);
                }
            }
        }
    }

    return true;
}

