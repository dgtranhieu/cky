#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Poller.h"
#include "Command.h"
#include "SafeHash.h"
#include "TiSchema.h"
#include "TiDevice.h"

// typedef struct ControlHdr_t ControlHdr_t;
typedef struct TiSchemaHdr_t TiSchemaHdr_t;
typedef void (*TiNotifyCB_t)(int code, Key_t itemKey);

typedef struct SysHdr
{
    int status; // up or down
    uint32_t channel;
    uint32_t panId;
    uint64_t extPanId;
    uint64_t coordinator; // coordinator mac
    uint8_t nwkKey[128];
}SysHdr;


typedef struct ControlHdr_t
{
    bool ready;

    pthread_t cmdTid;
    pthread_t evnTid;
    pthread_t mngTid;

    QueueHdr_t* evnQueue;
    QueueHdr_t* cmdQueue;
    PollHdr_t* pollHdr;

    SafeHashHdr_t* evnHash;
    SafeHashHdr_t* cmdHash;

    TiSchemaHdr_t* tiHdr;
    SafeHashHdr_t* itemHdr;
    SysHdr sysHdr;

    TiNotifyCB_t tiNotifyCB;

}ControlHdr_t;


typedef struct _Controller
{
    ControlHdr_t* (*const create)(TiNotifyCB_t tiNotifyCB);
    // void (*const register)(TiNotifyCB_t tiNotifyCB);

    void (*const destroy)(ControlHdr_t* header);
    void (*const start)(ControlHdr_t* header);
    void (*const stop)(ControlHdr_t* header);
    bool (*const ready)(ControlHdr_t* header);
    void (*const request)(ControlHdr_t* header, Cmd_t* cmd);

}_Controller;


extern _Controller const Controller;

#endif // CONTROLLER_H