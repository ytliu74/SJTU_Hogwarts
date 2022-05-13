#ifndef _INTELFPGA_H_
#define _INTELFPGA_H_

#include <string.h>
#include <vector>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

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

}

#endif

#endif
