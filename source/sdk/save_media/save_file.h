#ifndef __SAVE_FILE_H__
#define __SAVE_FILE_H__

#include <stdint.h>

void mtce_coreMediaVideoData(int chn, uint8_t *data, uint32_t len, uint64_t pts, uint8_t frame_type);

#endif