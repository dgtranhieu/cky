#ifndef UNIQUE_KEY_H
#define UNIQUE_KEY_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



#define MAX_KEY_SIZE     64

/**-------------------------------------------------------------------------------------------------
 * CmdKey_t
 *------------------------------------------------------------------------------------------------*/
typedef struct Key_t
{
    int len;
    char data[256];
}Key_t;

typedef enum CmdDomain_t
{
    eDomainNet = 0,
    eDomainDev = 1
}CmdDomain_t;

typedef enum CmdMode_t
{
    eModeSync = 0,
    eModeAsync = 1
}CmdMode_t;


typedef enum CidTypeEnum
{
    ICLUSTER,
    OCLUSTER
}CidTypeEnum;




typedef struct CmdKey_t
{
    CmdDomain_t domain;
    CmdMode_t mode;
    uint8_t cmdId;
}CmdKey_t;


typedef struct ItemKey_t
{
    uint64_t mac;
    uint16_t eid;
}ItemKey_t;


typedef struct UuidKey_t
{
    uint64_t mac;
    uint16_t eid;
    int plg;
}UuidKey_t;




/**-------------------------------------------------------------------------------------------------
 * CmdKey_t
 *------------------------------------------------------------------------------------------------*/

typedef union Kid_t
{
    CmdKey_t cmd;
    ItemKey_t item;
    UuidKey_t uuid;
}Kid_t;


typedef struct KeyOps_t
{
    const char* name;
    Key_t (*const render)(Kid_t id);
    Kid_t (*const parser)(Key_t key);
    Key_t (*const adapter)(char* data);
}KeyOps_t;


typedef struct UniqueKey_t
{
    KeyOps_t (*const type)(const char* name);
}UniqueKey_t;


extern UniqueKey_t const UniqueKey;


#endif // UNIQUE_KEY