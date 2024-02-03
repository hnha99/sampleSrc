/**
****************************************************************************************
* @author: PND
* @date: 2023-04-17
* @file: main.c
* @brief: mtce
*****************************************************************************************
**/
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "app.h"
#include "MTCEDEFs.h"
#include "mtce_video.h"

static int bRun = true;
static void load_configEncode(mtce_encode_t *encode);

void exit_handler()
{
    bRun = false;
}

int main(void)
{
    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
    signal(SIGTERM, exit_handler);
    signal(SIGKILL, exit_handler);

    mtce_encode_t encodeConf;

    load_configEncode(&encodeConf);

    mtce_videoInit();
    if (mtce_videoEncodeSet(&encodeConf) == 0)
    {
        if (mtce_videoCreateChannel() == 0)
            mtce_videoStart();
        else
        {
            printf("Create channel stream err\n");
            return -1;
        }
    }
    else
    {
        printf("Encode config set fail\n");
        return -1;
    }

    while (bRun)
    {

        usleep(1000);
    }

    printf("Realse main success\n");

    return 0;
}

static void load_configEncode(mtce_encode_t *encode)
{
    encode->mainFmt.format.compression = MTCE_CAPTURE_COMP_H264;
    encode->mainFmt.format.resolution = MTCE_CAPTURE_SIZE_1080P;
    encode->mainFmt.format.FPS = 15;
    encode->mainFmt.format.GOP = 15;
    encode->mainFmt.format.bitRateControl = MTCE_CAPTURE_BRC_VBR;
    encode->mainFmt.format.bitRate = 1024;
    encode->mainFmt.format.quality = 30;
    encode->mainFmt.format.virtualGOP = 1;

    encode->extraFmt.format.compression = MTCE_CAPTURE_COMP_H264;
    encode->extraFmt.format.resolution = MTCE_CAPTURE_SIZE_720P;
    encode->extraFmt.format.FPS = 15;
    encode->extraFmt.format.GOP = 15;
    encode->extraFmt.format.bitRateControl = MTCE_CAPTURE_BRC_VBR;
    encode->extraFmt.format.bitRate = 512;
    encode->extraFmt.format.quality = 30;
    encode->extraFmt.format.virtualGOP = 1;
}