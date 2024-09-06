#include "gateway.pb-c.h"
#include "UniqueKey.h"
#include "Logger.h"
#include "Controller.h"
#include "GwUtils.h"
#include "Lighting.h"

/**-------------------------------------------------------------------------------------------------
 * @lightingColorCtrl
 * [NONE] lightingColorCtrl_moveToHueCommand                      = { hue, direction, transtime }
 * [NONE] lightingColorCtrl_moveHueCommand                        = { movemode, rate }
 * [NONE] lightingColorCtrl_stepHueCommand                        = { stepmode, stepsize, transtime }
 * [NONE] lightingColorCtrl_moveToSaturationCommand               = { saturation, transtime }
 * [NONE] lightingColorCtrl_moveSaturationCommand                 = { movemode, rate }
 * [NONE] lightingColorCtrl_stepSaturationCommand                 = { stepmode, stepsize, transtime }
 * [PASS] lightingColorCtrl_moveToHueAndSaturationCommand         = { hue, saturation, transtime }
 * [NONE] lightingColorCtrl_moveToColorCommand                    = { colorx, colory, transtime }
 * [NONE] lightingColorCtrl_moveColorCommand                      = { ratex, ratey }
 * [NONE] lightingColorCtrl_stepColorCommand                      = { stepx, stepy, transtime }
 * [NONE] lightingColorCtrl_moveToColorTempCommand                = { colortemp, transtime }
 * [NONE] lightingColorCtrl_enhancedMoveToHueCommand              = { enhancehue, direction, transtime }
 * [NONE] lightingColorCtrl_enhancedMoveHueCommand                = { movemode, rate }
 * [NONE] lightingColorCtrl_enhancedStepHueCommand                = { stepmode, stepsize, transtime }
 * [NONE] lightingColorCtrl_enhancedMoveToHueAndSaturationCommand = { enhancehue, saturation, transtime }
 * [NONE] lightingColorCtrl_colorLoopSetCommand                   = { bits, bytee, action, direction, time, starthue }
 * [NONE] lightingColorCtrl_stopMoveStepCommand                   = { bits, bytee, action, direction, time, starthue }
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_MoveToHueAndSaturation(EpAddr_t epAddr, uint8_t hue, uint8_t saturation, uint16_t transtime)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    DevSetColorReq   body    = DEV_SET_COLOR_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_SET_COLOR_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    Logger.writeLog(LOG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    Logger.writeLog(LOG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);

    body.dstaddress      = &dstAddr;
    body.huevalue        = hue;
    body.saturationvalue = saturation;
    header->msg = malloc(sizeof(PacketHeader) + dev_set_color_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_set_color_req__pack(&body, header->msg->body);

    Logger.writeLog(LOG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);

    // Init header
    header->msg->header.len    = dev_set_color_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_SET_COLOR_REQ;

    return header;
}


// @pxhoang: It seems unnecessary since we can read these value via foundation
Cmd_t* _REQ_GetHueAndSaturation(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    DevGetColorReq   body    = DEV_GET_COLOR_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_GET_COLOR_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress      = &dstAddr;
    header->msg = malloc(sizeof(PacketHeader) + dev_get_color_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_get_color_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_get_color_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_GET_COLOR_REQ;

    return header;
}


// @pxhoang: It seems to unnecessary since we can read these value via foundation
void _IND_GetHueAndSaturation(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*     ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*            cmdHdr  = (Cmd_t*)msg;
    DevGetColorRspInd* body    = NULL;

    body = dev_get_color_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Failed to parse\n", __FUNCTION__);
        return;
    }

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep] = [0x%llX,%d]\n", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
            Logger.writeLog(LOG_DEBUG, "%s - [hue,sat,status] = [%d, %d, %d]\n", __FUNCTION__, body->huevalue, body->satvalue, body->status);
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)\n", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    dev_get_color_rsp_ind__free_unpacked(body, NULL);
}


Cmd_t* _REQ_MoveToColorTemp(EpAddr_t epAddr, uint16_t colortemp, uint16_t transtime)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*            header  = Cmder.create(MAJOR, NULL);
    DevSetColorTempReq body    = DEV_SET_COLOR_TEMP_REQ__INIT;
    GwAddressStructT   dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_SET_COLOR_TEMP_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress       = &dstAddr;
    body.temperaturevalue = (uint32_t)colortemp;

    header->msg = malloc(sizeof(PacketHeader) + dev_set_color_temp_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_set_color_temp_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_set_color_temp_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_SET_COLOR_TEMP_REQ;

    return header;
}


// @pxhoang: It seems unnecessary since we can read these value via foundation
Cmd_t* _REQ_GetColorTemp(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    DevGetTempReq   body    = DEV_GET_TEMP_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_GET_COLOR_TEMP_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress      = &dstAddr;
    header->msg = malloc(sizeof(PacketHeader) + dev_get_temp_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_get_temp_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_get_temp_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_GET_COLOR_TEMP_REQ;

    return header;
}


// @pxhoang: It seems unnecessary since we can read these value via foundation
void _IND_GetColorTemp(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*     ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*            cmdHdr  = (Cmd_t*)msg;
    DevGetColorTempRspInd* body    = NULL;

    body = dev_get_color_temp_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Failed to parse", __FUNCTION__);
        return;
    }

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep] = [0x%llX,%d]\n", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
            Logger.writeLog(LOG_DEBUG, "%s - [temp,status] = [%d, %d]\n", __FUNCTION__, body->temperaturevalue, body->status);
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)\n", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    dev_get_color_temp_rsp_ind__free_unpacked(body, NULL);
}





