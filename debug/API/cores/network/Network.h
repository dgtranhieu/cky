#ifndef NETWORK_H
#define NETWORK_H
#include <stdbool.h>
#include "Command.h"


void _evnHandler(void* header, Cmd_t* params);


typedef struct _Network
{
    Cmd_t* (*const REQ_PermitJoin)(uint8_t interval);
    Cmd_t* (*const REQ_RemoveEndpoint)(EpAddr_t epAddr);
    Cmd_t* (*const REQ_CoordInfo)();
    Cmd_t* (*const REQ_DevList)();
    Cmd_t* (*const REQ_NetworkInfo)();
    Cmd_t* (*const REQ_EndpointInfo)();
    Cmd_t* (*const REQ_NetworkKey)();
    Cmd_t* (*const REQ_LinkQuality)(EpAddr_t epAddr, uint8_t startIndex);
    Cmd_t* (*const REQ_BindEntry)(EpAddr_t srcEp, EpAddr_t dstEp, uint16_t cid, bool mode);
    Cmd_t* (*const REQ_Reset)(bool mode);

    void (*const RSP_Generic)(void* header, void* msg);
    void (*const RSP_coordInfo)(void* header, void* msg);
    void (*const RSP_DevList)(void* header, void* cmdHdr);
    void (*const RSP_NetworkInfo)(void* header, void* cmdHdr);
    void (*const RSP_EndpointInfo)(void* header, void* cmdHdr);
    void (*const RSP_NetworkKey)(void* header, void* cmdHdr);
    void (*const RSP_Reset)(void* header, void* msg);

    void (*const IND_NetworkReady)(void* header, void* msg);
    void (*const IND_LinkQuality)(void* header, void* cmdHdr);
    void (*const IND_BindEntry)(void* header, void* msg);
    void (*const IND_Device)(void* header, void* msg);
}_Network;


extern _Network const Network;


#endif // NETWORK_H