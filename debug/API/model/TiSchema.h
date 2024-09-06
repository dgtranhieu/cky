#ifndef TI_SCHEMA_H
#define TI_SCHEMA_H
#include "Controller.h"
#include "UniqueKey.h"
#include "TiDevice.h"

/**-------------------------------------------------------------------------------------------------
 * Schema
 *------------------------------------------------------------------------------------------------*/

typedef struct TiDevice_t TiDevice_t;
typedef struct TiSchemaHdr_t TiSchemaHdr_t;
typedef struct ControlHdr_t ControlHdr_t;

typedef struct TiSchemaOps_t
{
    TiDevice_t* (*render)(TiSchemaHdr_t* header, void* device, void* endpoint);
}TiSchemaOps_t;

typedef struct TiSchema_t
{
    uint16_t pid;
    uint16_t did;
    TiSchemaOps_t ops;
}TiSchema_t;

typedef struct TiSchemaElem_t
{
    int len;
    TiSchema_t specs;
}TiSchemaElem_t;

typedef struct TiSchemaHdr_t
{
    ControlHdr_t* apiHdr;
    TiSchemaElem_t schema[128];
}TiSchemaHdr_t;


/**-------------------------------------------------------------------------------------------------
 * Factory
 *------------------------------------------------------------------------------------------------*/

typedef struct TiFactory_t
{
    TiSchemaHdr_t* (*const instance)(ControlHdr_t* apiHdr);
    void (*const destroy)(TiSchemaHdr_t* header);
    TiDevice_t* (*const render)(TiSchemaHdr_t* header, void* device, void* endpoint);
}TiFactory_t;

extern TiFactory_t TiFactory;

#endif // TI_SCHEMA_H