#ifndef COMMAND_H
#define COMMAND_H
#include <stdlib.h>
#include <stdint.h>
#include "UniqueKey.h"


#define MAX_TCP_PACKET_SIZE 1024

typedef enum
{
    MAJOR,
    MINOR
}CmdSeverity_t;

typedef struct
{
    uint16_t len;
    uint8_t subSys;
    uint8_t cmdId;
} PacketHeader;

typedef struct
{
    PacketHeader header;
    uint8_t body[MAX_TCP_PACKET_SIZE];
} Packet_t;

typedef struct EpAddr_t
{
    uint64_t ieee_addr;
    uint8_t endpoint;
    uint32_t groupaddr;
}EpAddr_t;

typedef struct Cmd_t
{
    Key_t key;
    CmdSeverity_t severity;
    Packet_t* msg;
    void (*execute)(void* header, void* msg);
}Cmd_t;

typedef struct _Cmder
{
    Cmd_t* (*const create)(CmdSeverity_t severity, void (*execute)(void* header, void* msg));
    void (*const destroy)(Cmd_t* header);
}_Cmder;

extern _Cmder const Cmder;

#endif // COMMAND_H