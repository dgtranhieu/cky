#ifndef GATEWAY_H
#define GATEWAY_H
#include "GwGeneric.h"
#include "General.h"
#include "Foundation.h"
#include "Lighting.h"
#include "IAS.h"


/**-------------------------------------------------------------------------------------------------
 * Generic
 **-----------------------------------------------------------------------------------------------*/

typedef struct _GwGeneric
{
    void (*const RSP_Generic)(void* header, void* msg);
    void (*const IND_Generic)(void* header, void* msg);
    void (*const IND_ZclFrame)(void* header, void* msg);
}_GwGeneric;


/**-------------------------------------------------------------------------------------------------
 * General
 **-----------------------------------------------------------------------------------------------*/

typedef struct _GenOnOff
{
    Cmd_t* (*const REQ_On)(EpAddr_t epAddr);
    Cmd_t* (*const REQ_Off)(EpAddr_t epAddr);
    Cmd_t* (*const REQ_Toggle)(EpAddr_t epAddr);
}_GenOnOff;

typedef struct _GenGroups
{
    Cmd_t* (*const REQ_GroupAdd)(EpAddr_t epAddr, uint16_t gid);
    Cmd_t* (*const REQ_GroupRemove)(EpAddr_t epAddr, uint16_t gid);
    Cmd_t* (*const REQ_GroupGet)(EpAddr_t epAddr);
    void (*const IND_GroupGet)(void* header, void* msg);
}_GenGroups;

typedef struct _GenLevel
{
    Cmd_t* (*const REQ_MoveToLevel)(EpAddr_t epAddr, uint32_t transitionTime, uint32_t level);
}_GenLevel;


/**-------------------------------------------------------------------------------------------------
 * Foundation
 **-----------------------------------------------------------------------------------------------*/

typedef struct _Foundation
{
    Cmd_t* (*const REQ_Read)(EpAddr_t epAddr, uint16_t cid, ReadRec readRec);
    Cmd_t* (*const REQ_Write)(EpAddr_t epAddr, uint16_t cid, WriteRec writeRec);
    Cmd_t* (*const REQ_ConfigReport)(EpAddr_t epAddr, uint16_t cid, CfgRptRec cfgRptRec);
    Cmd_t* (*const REQ_Discover)(EpAddr_t epAddr);

    void (*const IND_Read)(void* header, void* msg);
    void (*const IND_Write)(void* header, void* msg);
    void (*const IND_ConfigReport)(void* header, void* msg);
    void (*const IND_Report)(void* header, void* msg);
    void (*const IND_Discover)(void* header, void* msg);
}_Foundation;


/**-------------------------------------------------------------------------------------------------
 * Lighting
 **-----------------------------------------------------------------------------------------------*/

typedef struct _Lighting
{
    Cmd_t* (*const REQ_MoveToHueAndSaturation)(EpAddr_t epAddr, uint8_t hue, uint8_t saturation, uint16_t transtime);
    Cmd_t* (*const REQ_GetHueAndSaturation)(EpAddr_t epAddr);
    Cmd_t* (*const REQ_MoveToColorTemp)(EpAddr_t epAddr, uint16_t colortemp, uint16_t transtime);
    Cmd_t* (*const REQ_GetColorTemp)(EpAddr_t epAddr);

    void (*const IND_GetHueAndSaturation)(void* header, void* msg);
    void (*const IND_GetColorTemp)(void* header, void* msg);
}_Lighting;


/**-------------------------------------------------------------------------------------------------
 * IAS
 **-----------------------------------------------------------------------------------------------*/

typedef struct _Ias
{
    Cmd_t* (*const REQ_AutoEnrollRsp)(EpAddr_t epAddr, uint32_t zoneId, int rspCode);
    Cmd_t* (*const REQ_StartWarning)(EpAddr_t epAddr, uint8_t warningmessage, uint16_t warningDuration, uint8_t strobeDutyCycle, uint8_t strobeLevel);

    void (* const IND_StatusChangeNotification)(void* header, void* msg);
}_Ias;


/**-------------------------------------------------------------------------------------------------
 * Interface
 **-----------------------------------------------------------------------------------------------*/

typedef struct _Gateway
{
    _GwGeneric generic;
    _GenOnOff genOnOff;
    _GenLevel genLevelCtrl;
    _GenGroups genGroups;
    _Foundation foundation;
    _Lighting lighting;
    _Ias ias;
}_Gateway;

extern _Gateway const Gateway;

#endif // GATEWAY_H


