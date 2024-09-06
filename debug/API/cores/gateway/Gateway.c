#include "Gateway.h"



_Gateway const Gateway  = {
    // GwGeneric
    {
        _RSP_Generic,
        _IND_Generic,
        _IND_ZclFrame
    },
    // GenOnOff
    {
        _REQ_On,
        _REQ_Off,
        _REQ_Toggle
    },
    // GenLevel
    {
        _REQ_MoveToLevel
    },
    // GenGroups
    {
        _REQ_GroupAdd,
        _REQ_GroupRemove,
        _REQ_GroupGet,
        _IND_GroupGet
    },
    // Foundation
    {
        _REQ_Read,
        _REQ_Write,
        _REQ_ConfigReport,
        _REQ_Discover,

        _IND_Read,
        _IND_Write,
        _IND_ConfigReport,
        _IND_Report,
        _IND_Discover
    },
    // Lighting
    {
        _REQ_MoveToHueAndSaturation,
        _REQ_GetHueAndSaturation,
        _REQ_MoveToColorTemp,
        _REQ_GetColorTemp,

        _IND_GetHueAndSaturation,
        _IND_GetColorTemp
    },
    // IAS
    {
        _REQ_AutoEnrollRsp,
        _REQ_StartWarning,

        _IND_StatusChangeNotification
    }
};

