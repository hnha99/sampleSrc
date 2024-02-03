#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "app.h"

#define MTCE_MEDIA_DEBUG 1

void mtce_coreMediaVideoSave(int chn, uint8_t *data, int len)
{
    char path[256];

    memset(path, 0, sizeof(path));
    sprintf(path, "%s/video-%d", MTCE_MEDIA_VIDEO_PATH, chn);

    FILE *f = fopen(path, "a");
    if (f == NULL)
    {
        printf("Path file VIDEO not exist");
        return;
    }

    fwrite(data, 1, len, f);
    fclose(f);
}

void mtce_coreMediaVideoData(int chn, uint8_t *data, uint32_t len, uint64_t pts, uint8_t frame_type)
{
#if MTCE_MEDIA_DEBUG
        printf("video buffer chn %d: len: %d type: [%02X]\n", chn, len, data[4]);
        mtce_coreMediaVideoSave(chn, data, len);
#endif

}