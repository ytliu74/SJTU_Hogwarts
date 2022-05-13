/* Copyright (c) 2020 AWCloud. All Rights Reserved.
// Copyright (c) 2020 AWCloud. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifndef _FPGA_H_
#define _FPGA_H_

// Version
#define FPGA_VERSION 0x01000005

// Board name
#define FPGA_BRDNAME "c5soc"
#define FPGA_DRVNAME "fpgadrv"

// Activation type
#define FPGA_ACT_NONE 0
#define FPGA_ACT_RELU 1
#define FPGA_ACT_RELU6 2
#define FPGA_ACT_LEAKYRELU 3

// Device information
struct fpga_info_s {
	uint32_t ver; // Version, 00.00.0000
};

struct fpga_reset_s {
	uint32_t val; // reset command, N/A
};

// Memory copy
struct fpga_mcopy_s {
	void*  src; // source address
	void*  dst; // destination adddress
	size_t size; // size in bytes
};

// Memory block
struct fpga_memblk_s {
	void * addr; // base address
	size_t size; // size in bytes
};

// Kernel
struct fpga_kernel_s {
	uint32_t kw; // width
	uint32_t kh; // height
	uint32_t ws; // width stride(s)
	uint32_t hs; // height stride(s)
};

// Input parameters, nchw
struct fpga_input_s {
	uint32_t in; // nbr of batch {1}
	uint32_t ic; // nbr of channels {1}
	uint32_t iw; // width
	uint32_t ih; // height
	uint32_t pl; // padding x in bytes {0}
	uint32_t pr; // padding x in bytes {0}
	uint32_t pt; // padding y in bytes {0}
	uint32_t pb; // padding y in bytes {0}
	uint32_t dx; // dilation for x {1}
	uint32_t dy; // dilation for y {1}
};

// Output parameters, nchw
struct fpga_output_s {
	uint32_t on; // nbr of batch {1}
	uint32_t oc; // nbr of channels {1}
	uint32_t ow; // width
	uint32_t oh; // height
};

// Basic convolution
struct fpga_conv_s {
	uint32_t            at; // activation type {0}, None=0, RELU=1
	uint32_t            ng; // nbr of groups {1}
	int8_t *            ia; // input address, INT8[N,Ci,Hi,Wi]
	int8_t *            ka; // kernel address, INT32[Co,Ci,Hk,Wk]
	int32_t*            ba; // bias address, INT32[Co,1]
	int32_t*            oa; // output address, INT32[N,Co,Ho,Wo]
	struct fpga_input_s  ip; // input
	struct fpga_kernel_s kp; // kernel
	struct fpga_output_s op; // output
};

// Pooling convolution
struct fpga_pool_s {
	uint8_t             gp; // global pooling {0}
	uint8_t             pm; // pooling mode {0}, Max=0, AVG=1
	uint8_t             cm; // ceil mode {0}, ceil=0, floor=1
	uint8_t             ex; // exclusive {1}, if ignore padding in avg pooling
	int32_t*            ia; // input address, INT32[N,Ci,Hi,Wi]
	int32_t*            oa; // output address, INT32[N,Ci,Ho,Wo]
	struct fpga_input_s  ip; // input
	struct fpga_kernel_s kp; // kernel
	struct fpga_output_s op; // output
};

// Full connection
struct fpga_fcon_s {
	uint32_t at; // activation type {0}, None=0, RELU=1
	int8_t * ia; // input address, INT8[M,K]
	int8_t * ka; // kernel address, INT8[K,N]
	int32_t* ba; // bias address, INT32[M,N]
	int32_t* oa; // output address, INT32[M,N] = ia[M,K] * wa[K,N] + ba[M,N]
	int m, n, k; // dims
};

// Regisger access
struct fpga_creg_s {
	uint32_t addr;
	uint32_t data;
};

#define FPGA_MAGIC_ID (('F' + 'P' + 'G' + 'A') / 4)

/* Ioctls */
#define FPGA_IOCTL_MAKE(cmd)  ( _IO( FPGA_MAGIC_ID, cmd))
#define FPGA_IOCTL_GET(cmd)   ( _IOC_NR(cmd))
#define FPGA_IOCTL_VALID(cmd) ((_IOC_TYPE(cmd)==FPGA_MAGIC_ID) ? 1 : 0)

#define FPGA_CMD_INFO      0x00 // struct fpga_info_s
#define FPGA_CMD_RESET     0x01 // struct fpga_reset_s

#define FPGA_CMD_MCOPY     0x10 // struct fpga_mcopy_s
#define FPGA_CMD_INVAL     0x11 // struct fpga_cache_s
#define FPGA_CMD_FLUSH     0x12 // struct fpga_cache_s

#define FPGA_CMD_CONV      0x20 // struct fpga_conv_s
#define FPGA_CMD_POOL      0x21 // struct fpga_pool_s
#define FPGA_CMD_FCON      0x22 // struct fpga_fcon_s

#define FPGA_CMD_REGRD     0xC0 // struct fpga_creg_s
#define FPGA_CMD_REGWR     0xC1 // struct fpga_creg_s

#endif

