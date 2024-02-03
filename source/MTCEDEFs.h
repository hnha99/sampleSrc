/**
****************************************************************************************
* @author: PND
* @ Modified by: Your name
* @ Modified time: 2023-05-16 14:54:16
* @brief: mtce
*****************************************************************************************
**/
#ifndef __MTCE_PARAMETER_H__
#define __MTCE_PARAMETER_H__

#include <stdbool.h>

#define MTCE_MAX_VERSION_LEN        32
#define MTCE_MAX_URL_LEN            256
#define MTCE_MAX_FW_ACTION          16
#define MTCE_MAX_FW_TOKEN           16
#define MTCE_NAME_LEN               64
#define MTCE_MD_REGION_ROW          32
#define MTCE_N_WEEKS                7
#define MTCE_N_TSECTION             4
#define MTCE_NAME_PASSWORD_LEN      64
#define MTCE_CLIENT_ID_LEN          64

/*********video***********/
#define MTCE_MAX_BITRATE            10 * 1024
#define MTCE_MAX_FPS                25
#define MTCE_MAX_GOP                25
#define MTCE_MAX_QUALITY            50
/*********motion*********/
#define MTCE_MAX_SENSITIVE          7
#define MTCE_MAX_LENG_GUARDZONE     64

#define MTCE_CONFIG_NONE            0

#define MTCE_MAX_LINE                               100
#define MAX_LINE_LENGTH                             1024

typedef struct
{
    char *serial;
    char *status;
    char *localIp;
    char *firmwareVer;
    long long timestamps;
    char *MAC;
    char *hostIP;
} mtce_appDefine_t;

/*******************************************************************************
 * Message Type Define
 ********************************************************************************/
typedef enum
{
    MQTT_DISCONNECTED,
    MQTT_CONNECTED,
} MQTT_STATE_E;

typedef enum
{
    /* Message type Control refer to MTCE_CTRL_CONFIGTYPE_E*/
    MTCE_USER_CONFIG_GET = 0x1300,
    MTCE_USER_CONFIG_SET = 0x1301,

} MTCE_CTRL_MSGTYPE_E;

typedef enum
{
    MTS_CONFIG_NOTSUPPORT,
    /* Account */
    MTCE_CONFIG_ADD_USER = 2,    // MTCE_userInfo_t
    MTCE_CONFIG_MODIFY_USER = 3, // MTCE_modifyUser_t
    MTCE_CONFIG_DELETE_USER = 4, // MTCE_userInfo_t
    MTCE_CONFIG_MODIFY_PSW = 8,  // MTCE_modifyUserPsw_t

    /*System*/
    MTCE_CONFIG_VIDEO_WIDGET = 12, // MTCE_videoWidget_t
    MTCE_CONFIG_ENCODE = 13,       // mtce_encode_t
    MTCE_CONFIG_CAMERAPARAM        = 14, // MTCE_cameraParam_t
    

    /*Network*/
    MTCE_CONFIG_NET_COMMON = 16, // MTCE_netCommon_t

    /*NetService*/
    MTCE_CONFIG_NET_WIFI = 21,         // MTCE_netWifi_t
    MTCE_CONFIG_NET_WIFI_AP_LIST = 22, // MTCE_netWifiDeviceList_t
    MTCE_CONFIG_NET_WIFI_STATUS = 23,  // MTCE_netWifiStatus_t
    MTCE_CONFIG_NET_MQTT = 35,         // MTCE_netMQTT_t

    /*Notification*/
    MTCE_CONFIG_MOTION = 36, // MTCE_motionDetect_t

    /*Advance*/
    MTCE_CONFIG_UPGRADE_INFO = 51, // MTCE_upgradeInfo_t
    MTCE_CONFIG_REBOOT_DEV = 54,   // Feature

    /* Others */
    MTCE_CONFIG_START_UPGRADE = 55, // MTST_systemUpgrade_t
    MTCE_CONFIG_DEVICE_INFO = 56,   // MTCE_deviceInfo_t
    MTCE_CONFIG_MQTT = 57,
    MTCE_CONFIG_WIFI = 58,
    MTCE_CONFIG_WATERMARK = 59,
    MTCE_CONFIG_ACCOUNT = 60,
    MTCE_CONFIG_P2P = 61,
    MTCE_CONFIG_S3 = 62,
    MTCE_CONFIG_RTMP = 63,
    MTCE_CONFIG_PTZ = 64,
    MTCE_CONFIG_VPN = 65,

} MTCE_CTRL_CONFIGTYPE_E;

/*Capture compression mode*/
enum MTCE_CAPTURE_COMP_E
{
    MTCE_CAPTURE_COMP_H264, // H.264
    MTCE_CAPTURE_COMP_H265, // H.265
    MTCE_CAPTURE_COMP_NONE  //
};

/*capture size image*/
enum MTCE_CAPTURE_SIZE_E
{
    MTCE_CAPTURE_SIZE_VGA,   // 640*480(PAL)		640*480(NTSC)
    MTCE_CAPTURE_SIZE_720P,  // 1280*720
    MTCE_CAPTURE_SIZE_1080P, // 1920*1080
    MTCE_CAPTURE_SIZE_2K,    // 2560*1440
    MTCE_CAPTURE_SIZE_NONE,
};

/*BitRate Control*/
enum MTCE_CAPTURE_BRC_E
{
    MTCE_CAPTURE_BRC_CBR,
    MTCE_CAPTURE_BRC_VBR,
    MTCE_CAPTURE_BRC_MBR,
    MTCE_CAPTURE_BRC_NR
};

/*system time*/
typedef struct
{
    int year;   //
    int month;  // anuary = 1, February = 2, and so on.
    int day;    // 1 - 31
    int wday;   // Sunday = 0, Monday = 1, and so on
    int hour;   // 0 - 23
    int minute; // 0 - 59
    int second; // 0 - 59
    int dst;    //
} mtce_systemTime_t;

/*NetMQTT*/
typedef struct
{
    int bEnable; /* 1: Enable; 0: Disable */
    int bTLSEnable;
    char clientID[MTCE_CLIENT_ID_LEN];
    char username[MTCE_NAME_LEN];
    char password[MTCE_NAME_PASSWORD_LEN];
    char host[MTCE_NAME_LEN];
    char TLSVersion[8];
    int keepAlive;
    int QOS;
    int retain;
    int port;
    char protocol[16]; // IPCP_MQTT_PROTOCOL_TYPE_E
} mtce_netMQTT_t;

/*Write log*/
typedef struct
{
    int num_line;
    char line[MTCE_MAX_LINE][MAX_LINE_LENGTH];
} mtce_getlog_t;

/*Video Format*/
typedef struct
{
    int compression;    // enum mtce_CAPTURE_COMP_E
    int resolution;     // enum mtce_CAPTURE_SIZE_E
    int bitRateControl; // enum mtce_CAPTURE_BRC_E
    int quality;        // 15 - 51
    int FPS;            // 6 - 25/30
    int bitRate;        // 250 - 4096
    int virtualGOP;     
    int GOP;            // 2 - 12
} mtce_videoFormat_t;

/*Media format*/
typedef struct
{
    mtce_videoFormat_t format; //
    int bVideoEnable;           //
    int bAudioEnable;           //
} mtce_mediaFormat_t;

/*Config encode simplify*/
typedef struct
{
    mtce_mediaFormat_t mainFmt;  // main format for channel 0
    mtce_mediaFormat_t extraFmt; // extra format for channel 1
} mtce_encode_t;

typedef struct
{
    bool enable;
    char ssid[32];
    char keys[32];
    int keyType;
    char auth[32];
    int channel;
    char encrypType[16];
    char gateWay[64];
    char hostIP[64];
    char submask[64];
    bool dhcp;
} mtce_netWifi_t;

/*Account */
typedef struct
{
    char username[32];
    char password[32];
    bool reserved;
    bool shareable;
    int len;
    char authoList[2][32];
} mtce_account_t;

typedef struct
{
    mtce_account_t account[32];
    int num;
} mtce_listAccount_t;

/*Account */
typedef struct
{
    char host[64];
    int port;
    char token[64];
} mtce_p2p_t;

/*Camera Param*/
typedef struct
{
    int nightVisionMode;
    bool pictureFlip;
    bool pictureMirror;
    bool ircutSwap;
} mtce_cameraParam_t;

/*Upgrade info*/
typedef struct
{
    char serial[MTCE_NAME_LEN];
    char hardware[MTCE_NAME_LEN];
    char vendor[MTCE_NAME_LEN];
    char productId[MTCE_NAME_LEN];
    char firmware[MTCE_NAME_LEN];
} mtce_upgradeInfo_t;

typedef struct
{
    int bEnable; /* 1: Enable; 0: Disable */
    int startHour;
    int startMinute;
    int startSecond;
    int endHour;
    int endMinute;
    int endSecond;
} mtce_timeSection_t;

/*Worksheet preriod */
typedef struct
{
    mtce_timeSection_t schedule[MTCE_N_WEEKS][MTCE_N_TSECTION];
} mtce_workSheet_t;

/*Event handler*/
typedef struct
{
    int channel;    // Channel camera
    int eventLatch; // Linkage start delay time, s in units
    int interval;
    int recordChannel;

    int bAlarmOut;
    int bFTP;
    int bLog;
    int bMail;
    int bMessage;
    int bSnap;
    int bRecordEn;
    int bVoice;

    mtce_workSheet_t workSheet;

} mtce_eventHandle_t; // event type refer to IPCP_eventType_e

/*Motion detect*/
typedef struct
{
    int bEnable;                             /* 1: Enable; 0: Disable */
    int level;                               /* The Sensitivity refer to IPCP_MD_SENSI_LEVEL_E*/
    unsigned int region[MTCE_MD_REGION_ROW]; /**/
    mtce_eventHandle_t hEvent;             /**/
    int reserved;                            // Feature
} mtce_motionDetect_t;

/*Motion Setting*/
typedef struct
{
    int sensitive;
    int interval;
    int duration;
    bool enable;
    bool videoAtt;
    bool pictureAtt;
    bool humanDetAtt;
    bool guardZoneAtt;
    int lenGuardZone;
    int guardZone[64];

} mtce_motionSetting_t;

/* OSD Setting*/
typedef struct 
{
    char name[32];
    bool timeAtt;
    bool nameAtt;  
} mtce_watermark_t;

/* S3 Setting*/
typedef struct 
{
    char accessKey[128]; 
} mtce_s3_t;

/* RTMP Setting*/
typedef struct 
{
    char host[64];
    bool enable;
    int port;
    char token[64];
    char node[64];
} mtce_rtmp_t;

/* PTZ*/
typedef struct 
{
    char direction[16];
    int step;
    int x;
    int y;
} mtce_ptz_t;

/*VPN*/
typedef struct
{
    char address[32];
    char prikey_client[64];
    char pubkey_server[64];
    char allowedIPs[32]; 
    char host[32];
} mtce_vpn_t;

/*device infomation*/
typedef struct
{
    char productID[16];
    char publicIP[16];
    char softBuildTime[64];
    char softWareVersion[64];
    char hardWareVersion[64];
    char hardWare[64];
    char encryptVersion[64];
    char serialNumber[64];

    unsigned int chanNum;         // channel number
    unsigned int extraChannel;    // Extended channel number
    unsigned int alarmInPortNum;  // Number of alarm input channels
    unsigned int alarmOutPortNum; // Number of alarm out channels
    int videoOutChannel;
    int audioOutChannel;
    int talkInChannel;
    int talkOutChannel;
    int devType; // refer to MTCE_DEVICE_TYPE_E

    mtce_systemTime_t buildTime;
    unsigned int runTime;

    char updataTime[20];     // format example: 2021-12-12 12-12-12
    unsigned int updataType; // Update content
    char cloudStatus[64];    // Online , Offline
    int status[32];          // Feature
} mtce_deviceInfo_t;

/*Uprgade firmware*/
typedef struct
{
    char action[MTCE_MAX_FW_ACTION];
    char url[MTCE_MAX_URL_LEN];
    char token[MTCE_MAX_FW_TOKEN];
} mtce_systemUpgrade_t;

typedef struct
{
    int method;
    int type;

    union {
        char data;
        mtce_motionSetting_t motion;
        mtce_systemUpgrade_t sys_upgrade;
        mtce_encode_t encode;
        mtce_netMQTT_t mqtt;
        mtce_cameraParam_t param;
        mtce_watermark_t watermark;
        mtce_netWifi_t wifi;
        mtce_account_t account;
        mtce_p2p_t p2p;
        mtce_s3_t s3;
        mtce_rtmp_t rtmp;
        mtce_ptz_t ptz;
        mtce_vpn_t wg;
    } data;

    union
    {
        char *data;
        mtce_motionSetting_t *motion;
        mtce_systemUpgrade_t *sys_upgrade;
        mtce_encode_t *encode;
        mtce_netMQTT_t *mqtt;
        mtce_cameraParam_t *param;
        mtce_watermark_t *watermark;
        mtce_netWifi_t *wifi;
        mtce_account_t *account;
        mtce_p2p_t *p2p;
        mtce_s3_t *s3;
        mtce_rtmp_t *rtmp;
        mtce_ptz_t *ptz;
        mtce_vpn_t *wg;
    } val;
} mtce_msgCtrl_t;

#endif // __MTCE_PARAMETER_H__
