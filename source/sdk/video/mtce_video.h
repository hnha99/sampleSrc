#ifndef __MTCE_VIDEO_H__
#define __MTCE_VIDEO_H__

#include <stdio.h>
#include "MTCEDEFs.h"
#include <rtsavdef.h>
#include <rtscamkit.h>
#include <rtsavapi.h>
#include <rtsvideo.h>


typedef struct {
	int isp_id;
	int isp_buf_num;
	enum RTS_AV_FMT type;
	int width;
	int height;
	int framerate;
	bool mirror;
	bool flip;

	// h264
	enum RTS_H264_LEVEL level;
	int quality;
	int bitrateCtrl;
	unsigned int bitrate;
	unsigned int gop;
	int rotation;
	// h255
	enum RTS_H265_TIER tier;

} mtce_ISPConf_t;

typedef struct {
	mtce_ISPConf_t isp_conf;
	mtce_mediaFormat_t mediaFmt;
} mtce_videoEncodeConf_t;

typedef struct {
	int width;
	int height;
} mtce_sizePicture_t;

void mtce_videoInit();
int mtce_videoCreateChannel();
int mtce_videoStart();
void mtce_videoStop();
void mtce_videoRelease(void);

// /***************************************/
extern int mtce_videoEncodeVerifyConfig(mtce_encode_t *encode);
int mtce_videoEncodeSet(mtce_encode_t *encode);
int mtce_videoReStart();

/**************************************/
int mtce_videoJPEGGet(int channel);
int mtce_videoOSDGet(int channel);
int mtce_videoResolutionGet(int channel);
int mtce_videoFPSGet(int channel);
void mtce_videoSizePicGet(int channel, mtce_sizePicture_t *size_res);


// /**************************************/


#endif	  // __MTCE_VIDEO_H__
