#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <rtsavdef.h>
#include <rtscamkit.h>
#include <rtsavapi.h>
#include <rtsvideo.h>

#include "app.h"
#include "mtce_video.h"
#include "save_file.h"

#define MTCE_VIDEO_H264_LEVEL  H264_LEVEL_4
#define MTCE_VIDEO_H265_LEVEL  H265_LEVEL_4
#define MTCE_VIDEO_TIER		   MAIN_TIER
#define MTCE_VIDEO_QP_DEFAULT  30
#define MTCE_VIDEO_ENCODE_TYPE RTS_V_FMT_YUV420SEMIPLANAR
#define MTCE_VIDEO_ISP_BUF_NUM 2

#define RUN_CHANNEL 0

typedef struct {
	pthread_t pthread;
	bool bRun;
	pthread_mutex_t mt;
} mtce_videoThreadHandle_t;

typedef struct {
	int enable;
	int isp_chn;
	int osd_chn;
	int h264_chn;
	int h265_chn;
	int jpeg_chn;
	int channel;
	enum RTS_AV_MIRROR flipmirr;
} mtce_videoContext_t;

static mtce_videoEncodeConf_t g_videoConf[MTCE_MAX_STREAM_NUM];
static mtce_videoContext_t *g_videoContext[MTCE_MAX_STREAM_NUM];
static mtce_videoThreadHandle_t g_videoCtrlThr[MTCE_MAX_STREAM_NUM];

static mtce_sizePicture_t size_of_picture[] = {
	[MTCE_CAPTURE_SIZE_VGA]	  = {640,  360 },
	[MTCE_CAPTURE_SIZE_720P]  = {1280, 720 },
	[MTCE_CAPTURE_SIZE_1080P] = {1920, 1080},
	[MTCE_CAPTURE_SIZE_2K]	  = {2560, 1440},
	[MTCE_CAPTURE_SIZE_NONE]  = {0,	0	 }
};

static mtce_videoContext_t *_createCtx(void);
static int _streamCreate(int channel, mtce_videoContext_t *ctx);
static int _streamEnable(mtce_videoContext_t *ctx);
static void _streamDisable(mtce_videoContext_t *ctx);
static int _streamDestroy(mtce_videoContext_t *ctx);
static void _releaseCtx(mtce_videoContext_t *ctx);
static void *_stream(void *args);
static int _vcbrH264(int channel, mtce_videoContext_t *ctx, mtce_videoEncodeConf_t *conf);
static int _vcbrH265(int channel, mtce_videoContext_t *ctx, mtce_videoEncodeConf_t *conf);
static int _vidDisable();
static int _vidEnable();
static int _setProfile();

void mtce_videoInit() {
	int i;
	for (i = 0; i < MTCE_MAX_STREAM_NUM; i++) {
		g_videoContext[i] = NULL;
		memset(&g_videoConf[i], 0, sizeof(mtce_videoEncodeConf_t));
		g_videoConf[i].isp_conf.tier		= MTCE_VIDEO_TIER;
		g_videoConf[i].isp_conf.rotation	= RTS_AV_ROTATION_0;
		g_videoConf[i].isp_conf.quality		= MTCE_VIDEO_QP_DEFAULT;
		g_videoConf[i].isp_conf.mirror		= false;
		g_videoConf[i].isp_conf.flip		= false;
		g_videoConf[i].isp_conf.type		= MTCE_VIDEO_ENCODE_TYPE;
		g_videoConf[i].isp_conf.isp_buf_num = MTCE_VIDEO_ISP_BUF_NUM;
		g_videoConf[i].isp_conf.isp_id		= i;
	}
}

int mtce_videoCreateChannel() {
	int ret = 0;
	int i;
	mtce_videoContext_t *ctx = NULL;

	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		ctx = _createCtx();
		if (ctx == NULL) {
			printf("[Video] Can't allocate memory area\n");
			return -1;
		}

		ret = _streamCreate(i, ctx);
		if (ret != 0) {
			printf("[Video] Create stream failed\n");
			_releaseCtx(ctx);
			return -1;
		}

		ret = _streamEnable(ctx);
		if (ret != 0) {
			printf("[Video] Enable stream failed\n");
			_releaseCtx(ctx);
			return -1;
		}

		g_videoContext[i] = ctx;
	}

	return 0;
}

bool getIsThreadVideoRunning(int channel) {
	pthread_mutex_lock(&(g_videoCtrlThr[channel].mt));
	bool ret = g_videoCtrlThr[channel].bRun;
	pthread_mutex_unlock(&(g_videoCtrlThr[channel].mt));
	return ret;
}

void setIsThreadVideoRunning(int channel, bool st) {
	pthread_mutex_lock(&(g_videoCtrlThr[channel].mt));
	g_videoCtrlThr[channel].bRun = st;
	pthread_mutex_unlock(&(g_videoCtrlThr[channel].mt));
}

int mtce_videoStart() {
	int ret, i = 0;

	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		if (g_videoContext[i]) {
			if (!getIsThreadVideoRunning(i)) {
				setIsThreadVideoRunning(i, true);
				ret = pthread_create(&g_videoCtrlThr[i].pthread, NULL, _stream, (void *)g_videoContext[i]);
				if (ret != 0) {
					printf("[Video] Start video err\n");
					return -1;
				}
			}
		}
	}

	return 0;
}

void mtce_videoStop() {
	int i;
	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		if (g_videoContext[i] && getIsThreadVideoRunning(i)) {
			setIsThreadVideoRunning(i, false);
			pthread_join(g_videoCtrlThr[i].pthread, NULL);
		}
	}
}

void mtce_videoRelease() {
	int i;
	mtce_videoContext_t *ctx = NULL;
	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		ctx = g_videoContext[i];
		if (ctx) {
			_streamDisable(ctx);
			_streamDestroy(ctx);
			_releaseCtx(ctx);
		}
		g_videoContext[i] = NULL;
	}
}

int mtce_videoReStart() {
	int ret;

	mtce_videoStop();

	ret = _vidDisable();
	if (ret != 0) {
		printf("[Restart Video] Disable channel video fail\n");
		return ret;
	}

	ret = _setProfile();
	if (ret != 0) {
		printf("[Restart Video] Set ptofile fail\n");
		return ret;
	}

	ret = _vidEnable();
	if (ret != 0) {
		printf("[Restart Video] Enable channel fail\n");
		return ret;
	}

	ret = mtce_videoStart();
	if (ret != 0) {
		printf("[Restart Video] Start capture fail\n");
		return ret;
	}
	printf("\n");
	return 0;
}

int mtce_videoJPEGGet(int channel) {
	if ((channel >= MTCE_MAX_STREAM_NUM) || (g_videoContext[channel] == NULL))
		return -1;

	return g_videoContext[channel]->jpeg_chn;
}

int mtce_videoOSDGet(int channel) {
	if (channel >= MTCE_MAX_STREAM_NUM)
		return -1;
	return g_videoContext[channel]->osd_chn;
}

int mtce_videoResolutionGet(int channel) {
	if (channel > MTCE_MAX_STREAM_NUM)
		return -1;

	return g_videoConf[channel].mediaFmt.format.resolution;
}

int mtce_videoEncodeVerifyConfig(mtce_encode_t *encode) {
	mtce_mediaFormat_t *conf;
	int ret = 0;
	for (int i = 0; i < MTCE_MAX_STREAM_NUM && ret == 0; i++) {
		ret	 = -1;
		conf = ((mtce_mediaFormat_t *)encode) + i;
		if (i == 0 && (conf->format.resolution >= MTCE_CAPTURE_SIZE_NONE || conf->format.resolution < MTCE_CONFIG_NONE)) {
			printf("[Video] encode error resolution [%d] channel main [%d]\n", conf->format.resolution, i);
		}
		else if (i == 1 && (conf->format.resolution > MTCE_CAPTURE_SIZE_720P || conf->format.resolution < MTCE_CONFIG_NONE)) {
			printf("[Video] encode error resolution [%d] channel sub [%d]\n", conf->format.resolution, i);
		}
		else if (conf->format.bitRateControl >= MTCE_CAPTURE_BRC_NR || conf->format.bitRateControl < MTCE_CONFIG_NONE) {
			printf("[Video] encode error bitrate control [%d]\n", conf->format.bitRateControl);
		}
		else if (conf->format.compression >= MTCE_CAPTURE_COMP_NONE || conf->format.compression < MTCE_CONFIG_NONE) {
			printf("[Video] encode error compression [%d]\n", conf->format.compression);
		}
		else if (conf->format.bitRate > MTCE_MAX_BITRATE || conf->format.bitRate <= MTCE_CONFIG_NONE) {
			printf("[Video] encode error bitrate [%d]\n", conf->format.bitRate);
		}
		else if (conf->format.FPS > MTCE_MAX_FPS || conf->format.FPS <= MTCE_CONFIG_NONE) {
			printf("[Video] encode error fps [%d]\n", conf->format.FPS);
		}
		else if (conf->format.GOP > MTCE_MAX_FPS || conf->format.GOP <= MTCE_CONFIG_NONE) {
			printf("[Video] encode error gop [%d]\n", conf->format.GOP);
		}
		else if (conf->format.quality > MTCE_MAX_QUALITY || conf->format.quality <= MTCE_CONFIG_NONE) {
			printf("[Video] encode error quality [%d]\n", conf->format.quality);
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

int mtce_videoEncodeSet(mtce_encode_t *encode) {
	mtce_videoEncodeConf_t *conf;
	if (mtce_videoEncodeVerifyConfig(encode) == 0) {
		for (int i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
			memcpy(&g_videoConf[i].mediaFmt, ((mtce_mediaFormat_t *)encode) + i, sizeof(mtce_mediaFormat_t));
			conf						= &g_videoConf[i];
			mtce_sizePicture_t size_pic = size_of_picture[conf->mediaFmt.format.resolution];
			conf->isp_conf.bitrate		= conf->mediaFmt.format.bitRate;
			conf->isp_conf.bitrateCtrl	= conf->mediaFmt.format.bitRateControl;
			conf->isp_conf.gop			= conf->mediaFmt.format.GOP;
			conf->isp_conf.quality		= conf->mediaFmt.format.quality;
			conf->isp_conf.framerate	= conf->mediaFmt.format.FPS;
			conf->isp_conf.width		= size_pic.width;
			conf->isp_conf.height		= size_pic.height;

			printf("--- Config %s channel H264 ---\n", (i == MTCE_MAIN_STREAM) ? "MAIN" : "SUB");
			printf("    - FPS             :	%d\n", conf->isp_conf.framerate);
			printf("    - Resolution      :	%d\n", conf->mediaFmt.format.resolution);
			printf("    - Bitrate control :	%d\n", conf->isp_conf.bitrateCtrl);
			printf("    - Bitreate        :	%d\n", conf->isp_conf.bitrate);
			printf("    - GOP             :	%d\n", conf->isp_conf.gop);
			printf("    - Quality         :	%d\n", conf->isp_conf.quality);
			printf("\n");
		}
		return 0;
	}
	return -1;
}

int mtce_videoFPSGet(int channel) {
	return (g_videoConf[channel].isp_conf.framerate);
}

void mtce_videoSizePicGet(int channel, mtce_sizePicture_t *size_res) {
	size_res->width	 = g_videoConf[channel].isp_conf.width;
	size_res->height = g_videoConf[channel].isp_conf.height;
}

/**********************************************************************************
 * Private function
 ***********************************************************************************/

static mtce_videoContext_t *_createCtx(void) {
	mtce_videoContext_t *ctx;
	ctx = (mtce_videoContext_t *)calloc(1, sizeof(mtce_videoContext_t));
	if (ctx == NULL)
		return ctx;

	ctx->h264_chn = -1;
	ctx->h265_chn = -1;
	ctx->jpeg_chn = -1;

	return ctx;
}

static int _streamCreate(int channel, mtce_videoContext_t *ctx) {
	int ret						 = 0;
	mtce_videoEncodeConf_t *conf = &g_videoConf[channel];
	struct rts_vin_attr isp_attr;		  //= {0};
	struct rts_av_profile isp_profile;	  //= {0};
	ctx->channel = channel;
	/* vin create channel */
	isp_attr.vin_id		 = conf->isp_conf.isp_id;
	isp_attr.vin_buf_num = conf->isp_conf.isp_buf_num;
	isp_attr.vin_mode	 = RTS_AV_VIN_FRAME_MODE;
	if (isp_attr.vin_id != 0 && isp_attr.vin_mode == 1) {
		printf("[Video] vin ring mode only work in vin_id 0\n");
		return -1;
	}
	if (isp_attr.vin_id > 1 && isp_attr.vin_mode == 2) {
		printf("[Video] vin direct mode only work in vin_id 0/1\n");
		return -1;
	}
	if (isp_attr.vin_mode < 0 || isp_attr.vin_mode > 2) {
		printf("[Video] vin mode range [0~2]\n");
		return -1;
	}
	ctx->isp_chn = rts_av_create_vin_chn(&isp_attr);
	if (ctx->isp_chn < 0) {
		printf("[Video] Create ISP channel %d failed\n", conf->isp_conf.isp_id);
		_streamDestroy(ctx);
		return -1;
	}

	/* osd create channel */
	ctx->osd_chn = rts_av_create_osd_chn();
	if (ctx->osd_chn < 0) {
		printf("[Video] Create OSD channel failed\n");
		_streamDestroy(ctx);
		return -1;
	}

	struct rts_jpgenc_attr jpg_attr;
	memset(&jpg_attr, 0, sizeof(jpg_attr));
	jpg_attr.rotation = RTS_AV_ROTATION_0;
	ctx->jpeg_chn	  = rts_av_create_mjpeg_chn(&jpg_attr);
	if (ctx->jpeg_chn < 0) {
		printf("[Video] Create JPEG channel fail\n");
		_streamDestroy(ctx);
		return -1;
	}

	/* h264 create channel */
	struct rts_h264_attr h264_attr;
	memset(&h264_attr, 0, sizeof(h264_attr));
	h264_attr.level	   = conf->isp_conf.level;
	h264_attr.mirror   = RTS_AV_MIRROR_NO;
	h264_attr.rotation = RTS_AV_ROTATION_0;
	ctx->h264_chn	   = rts_av_create_h264_chn(&h264_attr);
	if (ctx->h264_chn < 0) {
		printf("[Video] Create H264 channel failed\n");
		_streamDestroy(ctx);
		return -1;
	}

	/* h265 create channel */
	struct rts_h265_attr h265_attr;
	memset(&h265_attr, 0, sizeof(h265_attr));
	h265_attr.level	   = MTCE_VIDEO_H265_LEVEL;
	h265_attr.tier	   = conf->isp_conf.tier;
	h265_attr.mirror   = RTS_AV_MIRROR_NO;
	h265_attr.rotation = RTS_AV_ROTATION_0;
	ctx->h265_chn	   = rts_av_create_h265_chn(&h265_attr);
	if (ctx->h265_chn < 0) {
		printf("[Video] Creat H265 channel error\n");
		_streamDestroy(ctx);
		return -1;
	}

	isp_profile.fmt				  = conf->isp_conf.type;
	isp_profile.video.width		  = conf->isp_conf.width;
	isp_profile.video.height	  = conf->isp_conf.height;
	isp_profile.video.numerator	  = 1;
	isp_profile.video.denominator = conf->isp_conf.framerate;
	ret							  = rts_av_set_profile(ctx->isp_chn, &isp_profile);
	if (ret != 0) {
		printf("[Video] Set profile VIN channel %d failed\n", conf->isp_conf.isp_id);
		_streamDestroy(ctx);
		return -1;
	}

	ret = rts_av_bind(ctx->isp_chn, ctx->osd_chn);
	if (ret != 0) {
		printf("[Video] Bind OSD to ISP failed\n");
		_streamDestroy(ctx);
		return -1;
	}

	ret = rts_av_bind(ctx->osd_chn, ctx->jpeg_chn);
	if (ret != 0) {
		printf("[Video] Bind JPEG to OSD fail\n");
		_streamDestroy(ctx);
		return -1;
	}

	ret = rts_av_bind(ctx->osd_chn, ctx->h264_chn);
	if (ret != 0) {
		printf("[Video] Bind H264 to OSD error\n");
		_streamDestroy(ctx);
		return -1;
	}

	ret = rts_av_bind(ctx->osd_chn, ctx->h265_chn);
	if (ret != 0) {
		printf("[Video] Bind H265 to OSD error\n");
		_streamDestroy(ctx);
		return -1;
	}

	return 0;
}

static int _streamEnable(mtce_videoContext_t *ctx) {
	int ret;

	if (ctx->isp_chn >= 0) {
		ret = rts_av_enable_chn(ctx->isp_chn);
		if (ret != 0) {
			printf("[Video] Enable ISP channel fail\n");
			return -1;
		}
	}
	if (ctx->osd_chn >= 0) {
		ret = rts_av_enable_chn(ctx->osd_chn);
		if (ret != 0) {
			printf("[Video] Enable OSD channel fail\n");
			return -1;
		}
	}

	if (ctx->jpeg_chn >= 0) {
		ret = rts_av_enable_chn(ctx->jpeg_chn);
		if (ret != 0) {
			printf("[Video] Enable JPEG channel fail\n");
			return -1;
		}
	}

	printf("--- Create channel encode %d ---\n", ctx->channel);
	printf("    - Create channel ISP:   %d\n", ctx->isp_chn);
	printf("    - Create channel H264:  %d\n", ctx->h264_chn);
	printf("    - Create channel H265:  %d\n", ctx->h265_chn);
	printf("    - Create channel OSD:   %d\n", ctx->osd_chn);
	printf("\n");

	return 0;
}

static void _streamDisable(mtce_videoContext_t *ctx) {
	if (ctx->jpeg_chn >= 0) {
		rts_av_disable_chn(ctx->jpeg_chn);
	}
	if (ctx->osd_chn >= 0) {
		rts_av_disable_chn(ctx->osd_chn);
	}
	if (ctx->h264_chn >= 0) {
		rts_av_stop_recv(ctx->h264_chn);
		rts_av_disable_chn(ctx->h264_chn);
	}
	if (ctx->h265_chn >= 0) {
		rts_av_stop_recv(ctx->h265_chn);
		rts_av_disable_chn(ctx->h265_chn);
	}
	if (ctx->isp_chn >= 0) {
		rts_av_disable_chn(ctx->isp_chn);
	}
}

static int _streamDestroy(mtce_videoContext_t *ctx) {
	if (ctx->osd_chn >= 0) {
		rts_av_destroy_chn(ctx->osd_chn);
		ctx->osd_chn = -1;
	}

	if (ctx->jpeg_chn >= 0) {
		rts_av_destroy_chn(ctx->jpeg_chn);
		ctx->jpeg_chn = -1;
	}

	if (ctx->h264_chn >= 0) {
		rts_av_destroy_chn(ctx->h264_chn);
		ctx->h264_chn = -1;
	}

	if (ctx->h265_chn >= 0) {
		rts_av_destroy_chn(ctx->h265_chn);
		ctx->h265_chn = -1;
	}

	if (ctx->isp_chn >= 0) {
		rts_av_destroy_chn(ctx->isp_chn);
		ctx->isp_chn = -1;
	}

	return 0;
}

static void _releaseCtx(mtce_videoContext_t *ctx) {
	if (ctx) {
		free(ctx);
	}
}

static void *_stream(void *args) {
	struct rts_av_buffer *buffer = NULL;
	mtce_videoContext_t ctx		 = {0};
	mtce_videoEncodeConf_t conf	 = {0};
	mtce_videoContext_t *ctxIn	 = (mtce_videoContext_t *)args;
	memcpy(&ctx, ctxIn, sizeof(ctx));
	memcpy(&conf, &g_videoConf[ctx.channel], sizeof(conf));

	int ret;
	unsigned int enc_channel = 0;

	printf("[Video] Start video capture channel %d\n", ctx.channel);

	if (conf.mediaFmt.format.compression == MTCE_CAPTURE_COMP_H264) {
		enc_channel = ctx.h264_chn;
		_vcbrH264(ctx.channel, &ctx, &conf);
		ret = rts_av_enable_chn(enc_channel);
		if (ret != 0) {
			printf("[Video] Enable channel enc H264 failed\n");
			return NULL;
		}
	}
	else if (conf.mediaFmt.format.compression == MTCE_CAPTURE_COMP_H265) {
		enc_channel = ctx.h265_chn;
		_vcbrH265(ctx.channel, &ctx, &conf);
		ret = rts_av_enable_chn(enc_channel);
		if (ret != 0) {
			printf("[Video] Enable channel enc H264 failed\n");
			return NULL;
		}
	}

	rts_av_start_recv(enc_channel);

	while (getIsThreadVideoRunning(ctx.channel)) {
		if (rts_av_recv_block(enc_channel, &buffer, 100))
			continue;
		if (buffer) {
	

			mtce_coreMediaVideoData(ctx.channel, buffer->vm_addr, buffer->bytesused, buffer->timestamp, buffer->type);

			rts_av_put_buffer(buffer);
		}
		usleep(100);
	}

	printf("[Video] Stop video capture channel %d\n", ctx.channel);
	return NULL;
}

static int _vcbrH264(int channel, mtce_videoContext_t *ctx, mtce_videoEncodeConf_t *conf) {
	struct rts_h264_ctrl *ctrl;
	int ret;

	ret = rts_av_query_h264_ctrl(ctx->h264_chn, &ctrl);
	if (ret != 0) {
		printf("[Video] Query H264 ctrl fail channel %d", ctx->h264_chn);
		rts_av_release_h264_ctrl(ctrl);
		return -1;
	}

	ret = rts_av_get_h264_ctrl(ctrl);
	if (ret != 0) {
		printf("[Video] Get H264 ctrl fail\n");
		rts_av_release_h264_ctrl(ctrl);
		return -1;
	}

	if (conf->isp_conf.bitrateCtrl == MTCE_CAPTURE_BRC_VBR) {
		ctrl->bitrate_mode = RTS_BITRATE_MODE_VBR;
		ctrl->bitrate	   = conf->isp_conf.bitrate * 1024;
		ctrl->min_bitrate  = ctrl->bitrate * 2 / 3;
		ctrl->max_bitrate  = ctrl->bitrate * 4 / 3;
		ctrl->qp		   = conf->isp_conf.quality;
		ctrl->max_qp	   = conf->isp_conf.quality;
		ctrl->min_qp	   = conf->isp_conf.quality;
		ctrl->gop_mode	   = RTS_GOP_MODE_NORMAL;
		ctrl->gop		   = conf->isp_conf.gop;
		printf("    - Set param VBR channel %d\n", channel);
	}
	else if (conf->isp_conf.bitrateCtrl == MTCE_CAPTURE_BRC_CBR) {
		ctrl->bitrate_mode	 = RTS_BITRATE_MODE_CBR;
		ctrl->intra_qp_delta = -3;
		ctrl->bitrate		 = conf->isp_conf.bitrate * 1024;
		ctrl->min_bitrate	 = ctrl->bitrate * 2 / 3;
		ctrl->max_bitrate	 = ctrl->bitrate * 4 / 3;
		ctrl->qp			 = conf->isp_conf.quality;
		ctrl->max_qp		 = conf->isp_conf.quality;
		ctrl->min_qp		 = conf->isp_conf.quality;
		ctrl->gop_mode		 = RTS_GOP_MODE_NORMAL;
		ctrl->gop			 = conf->isp_conf.gop;
		printf("    - Set param CBR channel %d\n", channel);
	}
	else {
		printf("[Video] Mode does not exist\n");
	}

	ret = rts_av_set_h264_ctrl(ctrl);
	if (ret != 0) {
		printf("[Video] Set bitrate control H264 fail\n");
		rts_av_release_h264_ctrl(ctrl);
		return -1;
	}

	rts_av_release_h264_ctrl(ctrl);
	return 0;
}

static int _vcbrH265(int channel, mtce_videoContext_t *ctx, mtce_videoEncodeConf_t *conf) {
	struct rts_h265_ctrl *ctrl = {0};
	int ret;

	ret = rts_av_query_h265_ctrl(ctx->h265_chn, &ctrl);
	if (ret != 0) {
		printf("[Video] Query H265 ctrl fail channel %d\n", channel);
		rts_av_release_h265_ctrl(ctrl);
		return -1;
	}

	ret = rts_av_get_h265_ctrl(ctrl);
	if (ret != 0) {
		printf("[Video] Get H265 control fail\n");
		rts_av_release_h265_ctrl(ctrl);
		return -1;
	}

	if (conf->isp_conf.bitrateCtrl == MTCE_CAPTURE_BRC_VBR) {
		ctrl->bitrate_mode = RTS_BITRATE_MODE_VBR;
		ctrl->bitrate	   = conf->isp_conf.bitrate * 1024;
		ctrl->min_bitrate  = ctrl->bitrate * 2 / 3;
		ctrl->max_bitrate  = ctrl->bitrate * 4 / 3;
		ctrl->qp		   = conf->isp_conf.quality;
		ctrl->max_qp	   = conf->isp_conf.quality;
		ctrl->min_qp	   = conf->isp_conf.quality;
		ctrl->gop_mode	   = RTS_GOP_MODE_NORMAL;
		ctrl->gop		   = conf->isp_conf.gop;
		printf("    - Set param VBR channel %d\n", channel);
	}
	else if (conf->isp_conf.bitrateCtrl == MTCE_CAPTURE_BRC_CBR) {
		ctrl->bitrate_mode = RTS_BITRATE_MODE_CBR;
		ctrl->bitrate	   = conf->isp_conf.bitrate * 1024;
		ctrl->min_bitrate  = ctrl->bitrate * 2 / 3;
		ctrl->max_bitrate  = ctrl->bitrate * 4 / 3;
		ctrl->qp		   = conf->isp_conf.quality;
		ctrl->max_qp	   = conf->isp_conf.quality;
		ctrl->min_qp	   = conf->isp_conf.quality;
		ctrl->gop_mode	   = RTS_GOP_MODE_NORMAL;
		ctrl->gop		   = conf->isp_conf.gop;
		printf("    - Set param CBR channel %d\n", channel);
	}
	else {
		printf("[Video] Mode does not exist\n");
	}

	ret = rts_av_set_h265_ctrl(ctrl);
	if (ret != 0) {
		printf("[Video] Set H265 control fail\n");
		rts_av_release_h265_ctrl(ctrl);
		return -1;
	}

	rts_av_release_h265_ctrl(ctrl);
	return 0;
}

static int _vidDisable() {
	int i = 0, ret = 0;

	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		if (!g_videoContext[i])
			continue;
		if (g_videoContext[i]->h264_chn >= 0) {
			ret = rts_av_stop_recv(g_videoContext[i]->h264_chn);
			ret = rts_av_disable_chn(g_videoContext[i]->h264_chn);
			if (ret != 0)
				return -1;
		}

		if (g_videoContext[i]->h265_chn >= 0) {
			ret = rts_av_stop_recv(g_videoContext[i]->h265_chn);
			ret = rts_av_disable_chn(g_videoContext[i]->h265_chn);
			if (ret != 0)
				return -1;
		}

		if (g_videoContext[i]->isp_chn >= 0) {
			rts_av_disable_chn(g_videoContext[i]->isp_chn);
		}
	}
	printf("[Video] Disable channel video OK\n");

	return 0;
}

static int _vidEnable() {
	int i = 0, ret;

	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		if (!g_videoContext[i])
			continue;
		if (g_videoContext[i]->isp_chn >= 0) {
			ret = rts_av_enable_chn(g_videoContext[i]->isp_chn);
			if (ret != 0) {
				printf("[Video] Enable chn ISP fail\n");
				return ret;
			}
		}
	}
	printf("[Video] Enable chn stream OK\n");
	return 0;
}

static int _setProfile() {
	struct rts_av_profile profile;

	int ret, i;
	mtce_videoEncodeConf_t *conf;
	for (i = RUN_CHANNEL; i < MTCE_MAX_STREAM_NUM; i++) {
		conf = &g_videoConf[i];
		ret	 = rts_av_get_profile(g_videoContext[i]->isp_chn, &profile);
		if (ret != 0) {
			printf("[Video] Change profile chn vin fail");
			return -1;
		}

		profile.video.width		  = conf->isp_conf.width;
		profile.video.height	  = conf->isp_conf.height;
		profile.video.numerator	  = 1;
		profile.video.denominator = conf->isp_conf.framerate;
		ret						  = rts_av_set_profile(conf->isp_conf.isp_id, &profile);
		if (ret != 0) {
			printf("[Video] Change set profile chn fail");
			return -1;
		}

		ret = rts_av_get_profile(conf->isp_conf.isp_id, &profile);
		if (ret != 0) {
			printf("[Video] Change get profile chn fail");
			return -1;
		}

		printf("--- Change [Video] profile ---\n");
		printf("		Channel %d\n", i);
		printf("    - Resolution: %dx%d\n", profile.video.width, profile.video.height);
		printf("    - FPS       :  %d\n", profile.video.denominator);
	}

	return 0;
}
