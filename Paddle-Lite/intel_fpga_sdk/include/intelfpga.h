#ifndef _INTELFPGA_H_
#define _INTELFPGA_H_

#include <string.h>
#include <vector>
#include <map>
#include <stdint.h>
#include "utils.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint32;
typedef unsigned int uint;
typedef int32_t Dtype;//origin data type

typedef int8_t INtype;
typedef int8_t Wtype;//weights data type

/*convolutional layer macro parameter*/
#define INPUT_CHANNEL_TILE		16
#define INPUT_EXTEND_SCALE	INPUT_CHANNEL_TILE
#define EXTEND_INPUT_CHANNEL_TILE	(INPUT_CHANNEL_TILE/INPUT_EXTEND_SCALE)
#define OUTPUT_CHANNEL_TILE  	16
#define EXTEND_OUTPUT_CHANNEL_TILE	(OUTPUT_CHANNEL_TILE/INPUT_EXTEND_SCALE)
#define WEIGHT_EXTEND_SCALE	OUTPUT_CHANNEL_TILE
#define OUTPUT_ROW_TILE			7
#define OUTPUT_SIDE				9
#define INPUT_ROW_TILE			16
#define max_kernel				3
#define image_h 				256
#define image_w 				image_h
#define MAX_OUTPUT_W			image_h
/*Depth wise convolutional layer macro parameter*/
#define DW_OUTPUT_CHANNEL_TILE  	16
#define DW_WEIGHT_OUTPUT_CHANNEL	((DW_OUTPUT_CHANNEL_TILE-1)/WEIGHT_EXTEND_SCALE+1)
#define FILTER_OFFSET 16

/*quantification macro parameter*/
#define Q_MAX		127
#define Q_MIN		-127
#define W_MAX		127
#define W_MIN		-127

enum class IntelFpgaOpType : int {
	kRnn = 0,
	kConv = 1,
	kFc =  2,
	kConvPool = 3,
	kConvDepthwise = 4
};

enum class IntelFpgaReluType : int {
	kNoRelu = 0,
	kRelu = 1,
	kRelu6 =  2,
	kLeakyRelu = 3,
};

struct IntelFpgaConvParam {
  int input_offset;
  int weight_offset;
  int scale_offset;
  int output_offset;

  int in_c;
  int in_h;
  int in_w;
  int output_c;
  int output_h;
  int output_w;
  int kernel;
  int in_pad;//b[9,0]:conv_pad //b[19,10]:row_pad for alignment
  int out_pad;//b[9,0]:conv_pad //b[19,10]:row_pad for alignment
  int stride;
  int relu;
  //conv 1; fc 2; conv+pool 3; first_layer_conv 4;first_layer_pool 5
  IntelFpgaOpType type;
};

using IntelFpgaConvScale = std::vector<float>;


//----------------define for cma.------------------------
// Version
#define NNA_VERSION 0x01000002

// Board name
#define NNA_BRDNAME "c5soc"
#define NNA_DRVNAME "nnadrv"

// Activation types
#define NNA_ACT_NONE 0
#define NNA_ACT_RELU 1
#define NNA_ACT_RELU6 2
#define NNA_ACT_LEAKYRELU 3

// Data memory offset
#define NNA_IDM_SIZE 0x00800000
#define NNA_KDM_SIZE 0x00800000
#define NNA_ODM_SIZE 0x00800000
#define NNA_BSDM_SIZE 0x3000 
#define NNA_PDM_SIZE 0x1000

// Variable
#define IFPAD_FILL 0
#define IFNOTPAD_FILL 0

// Device information
struct nna_info_s {
	uint32_t ver; // Version, xx.yy.zzzz
};

struct nna_reset_s {
	uint32_t val; // reset command, N/A
};

// Regisger access
struct nna_creg_s {
	uint32_t addr;
	uint32_t data;
};

// Memory copy
struct nna_mcpy_s {
	unsigned long src; // source address
	unsigned long dst; // destination adddress
	size_t len; // size in bytes
};

// Memory block
struct nna_mblk_s {
	void* addr; // base address
	void* virt; // kernel address
	unsigned long phys; // Pysical address
	size_t size; // size in bytes
};

// Kernel
struct nna_kprm_s {
	uint32_t kw; // width
	uint32_t kh; // height
	uint32_t ws; // width stride(s)
	uint32_t hs; // height stride(s)
};

// Input parameters, nchw
struct nna_iprm_s {
	uint32_t in; // nbr of batch {1}
	uint32_t ic; // nbr of channels {1}
	uint32_t iw; // width
	uint32_t ih; // height
	uint32_t pl; // padding x-left in bytes {0}
	uint32_t pr; // padding x-right in bytes {0}
	uint32_t pt; // padding y-top in bytes {0}
	uint32_t pb; // padding y-bottom in bytes {0}
	uint32_t dx; // dilation for x {1}
	uint32_t dy; // dilation for y {1}
};

// Output parameters, nchw
struct nna_oprm_s {
	uint32_t on; // nbr of batch {1}
	uint32_t oc; // nbr of channels {1}
	uint32_t ow; // width
	uint32_t oh; // height
};

// Basic convolution
struct nna_conv_s {
	int8_t *          ia; // input address, INT8[N,Ci,Hi,Wi]
	int8_t *          ka; // kernel address, INT32[Co,Ci,Hk,Wk]
	float*          ba; // bias and scale address, INT32[Co,1]
	int8_t*          oa; // output address, INT32[N,Co,Ho,Wo]
	struct IntelFpgaConvParam* ip; // input parameters
	int extent_in_size;  // Input size after reorder input.
	int extent_weight_size;  // Weight size after reorder weight.
	int extent_out_size;  // Output size to store reordered output .
	int scale_size;  // Scale size.
	int param_size;  // Param size.
};

struct DeviceGraphNode {
	int id;
	int ddr_block_idx;
	struct nna_conv_s* device_param_;
	bool is_output;
	int output_ref_count_;  // When the output tensor of this op is referenced 
	                       // by more than 1 node, it should have it's  own ddr
												 // space.
	struct DeviceGraphNode* parent_; // The op which it's input tensor belong to.
	struct DeviceGraphNode* next_;  // Next op to be executed in topological order.
};

// Pooling convolution
struct nna_pool_s {
	uint8_t           gp; // global pooling {0}
	uint8_t           pm; // pooling mode {0}, Max=0, AVG=1
	uint8_t           cm; // ceil mode {0}, ceil=0, floor=1
	uint8_t           ex; // exclusive {1}, if ignore padding in avg pooling
	int32_t*          ia; // input address, INT32[N,Ci,Hi,Wi]
	int32_t*          oa; // output address, INT32[N,Ci,Ho,Wo]
	struct nna_iprm_s ip; // input parameters
	struct nna_kprm_s kp; // kernel parameters
	struct nna_oprm_s op; // output parameters
};

// Full connection
struct nna_fcon_s {
	uint32_t at; // activation type {0}, None=0, RELU=1
	int8_t * ia; // input address, INT8[M,K]
	int8_t * ka; // kernel address, INT8[K,N]
	int32_t* ba; // bias address, INT32[M,N]
	int32_t* oa; // output address, INT32[M,N] = ia[M,K] * wa[K,N] + ba[M,N]
	int m, n, k; // dims
};

#define NNA_MAGIC_ID (('N' + 'N' + 'A' + 'D') / 4)

/* Ioctls */
#define NNA_IOCTL_MAKE(cmd)  ( _IO( NNA_MAGIC_ID, cmd))
#define NNA_IOCTL_GET(cmd)   ( _IOC_NR(cmd))
#define NNA_IOCTL_VALID(cmd) ((_IOC_TYPE(cmd)==NNA_MAGIC_ID) ? 1 : 0)

#define NNA_CMD_INFO               0x00 // struct nna_info_s
#define NNA_CMD_RESET              0x01 // struct nna_reset_s
#define NNA_CMD_REGRD              0x02 // struct nna_creg_s
#define NNA_CMD_REGWR              0x03 // struct nna_creg_s

#define NNA_CMD_MCPY               0x10 // struct nna_mcpy_s
#define NNA_CMD_MGET               0x11 // struct nna_mblk_s
#define NNA_CMD_FREE               0x12 // struct nna_mblk_s

#define NNA_CMD_CONV               0x20 // struct nna_conv_s
#define NNA_CMD_POOL               0x21 // struct nna_pool_s
#define NNA_CMD_FCON               0x22 // struct nna_fcon_s
#define NNA_CMD_CONV_SUBGRAPH      0x23 // struct DeviceGraphNode 
#ifdef __cplusplus
}

int UpRound(int a, int b);

void fpga_copy_uint8(uint8_t* dst, const uint8_t* src, int len);

void fpga_copy_s32(int32_t* dst, const int32_t* src, int len);

 void fpga_copy_f32(float* dst, const float* src, int len);

void* intelfpga_malloc(size_t size);
void intelfpga_free(void *ptr);

bool IntelFpgaConvDepthwise(DeviceGraphNode* node, int8_t* din,
    const std::map<DeviceGraphNode*, int8_t*>& output_nodes,
		int max_output_size, int8_t* buff);
#endif

#endif
