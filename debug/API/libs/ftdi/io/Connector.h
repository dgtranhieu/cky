
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include "UniqueKey.h"


typedef struct IoHdr_t IoHdr_t;


struct IoHdr_t
{
    CmdDomain_t domain;
    struct sockaddr_in sAddr;
    int fdIndex;
    int fdDesc;
    bool connected;
    uint8_t layer;
};


typedef struct
{
    IoHdr_t* (*const create)(CmdDomain_t domain, uint8_t layer, char* host, u_short port);
    void (*const destroy)(IoHdr_t* header);
}_Connector;


extern _Connector const Connector;

#endif /* TCP_CLIENT_H */
