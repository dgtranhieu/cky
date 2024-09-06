#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "types.h"
#include "Logger.h"
#include "Connector.h"





/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

#define SERVER_RECONNECTION_RETRY_TIME  2000
#define MAX_TCP_PACKET_SIZE             2048
#define RPC_DUMMYSYS_connectION_INFO    0x1F
#define RPC_CMD_CONNTCTION_ID           0
#define RPC_CMD_AREQ                    0x40
#define APP_LAYER_NUMNER                5
#define INVALID_LAYER_NUMBER            6
#define MAX_TCP_PACKET_SIZE             2048
#define APP_NAME                        "pxhoang"
#define RPC_CMD_AREQ                    0x40


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/
static IoHdr_t* _create(CmdDomain_t domain, uint8_t layer, char* host, u_short port);
static void destroy(IoHdr_t* header);


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

static IoHdr_t* _create(CmdDomain_t domain, uint8_t layer, char* host, u_short port)
{
    IoHdr_t* header = malloc(sizeof(IoHdr_t));
    struct hostent* server = gethostbyname(host);
    if (server == NULL)
    {
        printf("pxhoang: %s - Failed to connect to %s\n", __FUNCTION__, host);
        return NULL;
    }

    bzero((char*)&header->sAddr, sizeof(header->sAddr));
    bcopy((char*)server->h_addr, (char*)&header->sAddr.sin_addr.s_addr, server->h_length);
    header->sAddr.sin_family = AF_INET;
    header->sAddr.sin_port = htons(port);;

    header->domain  = domain;
    header->layer   = layer;
    header->fdDesc  = -1;
    header->fdIndex = -1;
}


static void _destroy(IoHdr_t* header)
{
    free(header);
}


_Connector const Connector = {_create, _destroy};







