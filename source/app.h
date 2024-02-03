/**
****************************************************************************************
* @author: PND
 * @ Modified by: Your name
 * @ Modified time: 2023-11-06 10:29:30
* @brief: mtce
*****************************************************************************************
**/
#ifndef __H_APP__
#define __H_APP__

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


/*****************************************************************************
 *  global define variable
 *
 *****************************************************************************/

/* mtce debug */
#define MTCE_MEDIA_DEBUG                            1
#define MTCE_RTSP_EN                                1

/* mtce define sensor */
#define MTCE_SERSOR_SC4335                          0
#define MTCE_SENSOR_JX_F37H                         1

/* mtce define stream channel */
#define MTCE_MAX_STREAM_NUM                         2
#define MTCE_MAIN_STREAM                            0 
#define MTCE_SUB_STREAM                             1

#define MTCE_GPIO_INPUT_MODE                        0
#define MTCE_GPIO_NUM                               3

#define MTCE_WATCHDOG_TIMEOUT                       5

#define MTCE_SERIAL_FILE                            "Serial"
#define MTCE_ACCOUNT_FILE                           "Account"
#define MTCE_P2P_FILE                               "P2P"
#define MTCE_ENCODE_FILE                            "Encode"
#define MTCE_DEVINFO_FILE                           "DeviceInfo"
#define MTCE_MOTION_FILE                            "Motion"
#define MTCE_WIFI_FILE                              "Wifi"
#define MTCE_WATERMARK_FILE                         "Watermark"
#define MTCE_S3_FILE                                "S3"
#define MTCE_RTMP_FILE                              "RTMP"
#define MTCE_OSD_FONT_FILE                          "NimbusMono-Regular.otf"
#define MTCE_NETWORK_CA_FILE                        "Bundle_RapidSSL_2023.cert"
#define MTCE_NETWORK_MQTT_FILE                      "MQTTService"
#define MTCE_NETWORK_WPA_FILE                       "wpa_supplicant.conf"
#define MTCE_CAMERA_PARAM_FILE                      "CameraParam"
#define MTCE_PTZ_FILE                               "PTZ"
#define MTCE_VPN_CONFIG_FILE                        "wg0.conf"

#define MTCE_USER_CONF_PATH                         "/conf/user"
#define MTCE_DFAUL_CONF_PATH                        "/conf/default"
#define MTCE_MEDIA_VIDEO_PATH                       "/media/video"
#define MTCE_MEDIA_AUDIO_PATH                       "/media/audio"
#define MTCE_MEDIA_JPEG_PATH                        "/media/jpeg"

#define MTCE_RB_VIDEO_FILE                          "/var/tmp/capture_video_profile%c.shm"

#define MTCE_AUDIO_AAC_EXT                          ".aac"
#define MTCE_AUDIO_G711_EXT                         ".g711"


#define MTCE_MQTT_KEEPALIVE                         90
#define MTCE_SSL_VERIFY_NONE                        0
#define MTCE_SSL_VERIFY_PEER                        1

#define MTCE_RTMP_CHANNEL                           0

extern const char g_app_version[];


#endif // __H_APP__
