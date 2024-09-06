#ifndef TI_DEVICE_H
#define TI_DEVICE_H
#include <time.h>
#include "TiSchema.h"


typedef struct TiSchemaHdr_t TiSchemaHdr_t;
typedef struct TiSchema_t TiSchema_t;
typedef struct TiDevOps_t TiDevOps_t;

typedef enum TiNotifyCode_t
{
    TI_EVN_BIND_CHANGED = 0,
    TI_EVN_ATTR_CHANGED,
    TI_EVN_LIST_CHANGED
}TiNotifyCode_t;


typedef enum TiStateType_t
{
    TI_STATE_ATTR,
    TI_STATE_BIND
}TiStateType_t;


typedef struct ZclMsg_t
{
    uint16_t cid;
    uint16_t cmdId;
    int len;
    uint8_t* data;
}ZclMsg_t;


typedef struct Cluster_t
{
    CidTypeEnum type;
    uint16_t val;
}Cluster_t;


typedef struct ClusterList_t
{
    int len;
    Cluster_t cluster[128];
}ClusterList_t;


typedef struct Attr_t
{
    uint16_t cid;
    uint16_t aid;
    uint32_t tid;
    int len;
    uint8_t val[128];
    time_t time;
}Attr_t;


typedef struct AttrList_t
{
    int len;
    Attr_t attr[128];
}AttrList_t;


typedef struct TiDevSpecs_t
{
    const char* type;

    uint64_t mac;
    uint16_t nwk;
    uint16_t pid;
    uint16_t eid;
    uint16_t did;

    uint32_t mid; // Manufactured id

    uint8_t powerSource;
    time_t joinTime;
    int devType; // coord/router/ep/unknown
    int devLqi; // link quality

    ClusterList_t clusters;
}TiDevSpecs_t;


typedef struct TiDevState_t
{
    // SafeHashHdr_t* header;
    AttrList_t attrs;
    AttrList_t bindAttrs;
}TiDevState_t;


typedef struct TiDevice_t TiDevice_t;

typedef struct TiDevOps_t
{
    void (*init)(TiSchemaHdr_t* header, TiDevice_t* item);
    void (*exit)(TiSchemaHdr_t* header, TiDevice_t* item);
    void (*discovery)(TiSchemaHdr_t* header, TiDevice_t* item);
    Attr_t* (*getState)(TiSchemaHdr_t* header, TiDevice_t* item, TiStateType_t type, uint16_t cid, uint16_t aid);
    void (*setState)(TiSchemaHdr_t* header, TiDevice_t* item, TiStateType_t type, Attr_t attr);
    void (*request)(TiSchemaHdr_t* header, TiDevice_t* item, Cmd_t* cmd);
    void (*onEvent)(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg);
    void (*onTrack)(TiSchemaHdr_t* header, TiDevice_t* item);
}TiDevOps_t;


typedef struct TiDevice_t
{
    TiDevSpecs_t specs;
    TiDevOps_t ops;
    TiDevState_t state;
}TiDevice_t;


typedef struct TiCommon_t
{
    // @pxhoang: Gateway and Network proto file is not well designed, there are many duplicated fields
    // Using void* here can help to use for both gateway and network
    TiDevSpecs_t (*const renderSpecs)(TiSchemaHdr_t* header, void* device, void* endpoint);
    Attr_t* (*const getState)(TiSchemaHdr_t* header, TiDevice_t* self, TiStateType_t type, uint16_t cid, uint16_t aid);
    void (*const setState)(TiSchemaHdr_t* header, TiDevice_t* self, TiStateType_t type, Attr_t attr);
    void (*const request)(TiSchemaHdr_t* header, TiDevice_t* self, Cmd_t* cmd);
    void (*const bindState)(TiSchemaHdr_t* header, TiDevice_t* self, uint8_t eid, uint16_t cid, bool mode);
    void (*const initBattery)(TiSchemaHdr_t* header, TiDevice_t* self, int mode);
    void (*const evnGenOnOff)(TiSchemaHdr_t* header, TiDevice_t* self, uint16_t cmdId);
    void (*const evnGenLevelCtrl)(TiSchemaHdr_t* header, TiDevice_t* self, uint16_t cmdId);

}TiCommon_t;

extern TiCommon_t const TiCommon;

#endif // TI_DEVICE_H