#include <stdio.h>
#include "gateway.pb-c.h"
#include "Controller.h"
#include "UniqueKey.h"
#include "Logger.h"
#include "GwUtils.h"
#include "SafeHash.h"
#include "General.h"



/**-------------------------------------------------------------------------------------------------
 * @genBasic
 * [PASS] genBasic_resetFactDefault = C2S {}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @genIdentify
 * genIdentify_identifyCommand              = c2s -> {identifytime}
 * genIdentify_identifyQueryCommand         = c2s -> {}
 * genIdentify_ezmodeInvokeCommand          = c2s -> {action}
 * genIdentify_updateCommissionStateCommand = c2s -> {action, commstatemask}
 * genIdentify_triggerEffectCommand         = c2s -> {effectid, effectvariant}
 * genIdentify_identifyQueryRsp             = s2c -> {timeout}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @genGroups
 * add              = c2s -> {groupid, groupname}
 * view             = c2s -> {groupid}
 * getMembership    = c2s -> {groupcount, grouplist}
 * remove           = c2s -> {groupid}
 * removeAll        = c2s -> {}
 * addIfIdentifying = c2s -> {groupid, groupname}
 * addRsp           = s2c -> {status, groupid}
 * viewRsp          = s2c -> {status, groupid, groupname}
 * getMembershipRsp = s2c -> {capacity, groupcount, grouplist}
 * removeRsp        = s2c -> {status, groupid}
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_GroupAdd(EpAddr_t epAddr, uint16_t gid)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*           header  = Cmder.create(MAJOR, NULL);
    GwAddGroupReq    body    = GW_ADD_GROUP_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    char groupName[5] = {'G','W','F','n','B'};

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_ADD_GROUP_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);
    body.dstaddress = &dstAddr;
    body.groupid    = (uint32_t)gid;
    body.groupname  = groupName;

    header->msg = malloc(sizeof(PacketHeader) + gw_add_group_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Error: Could not pack msg\n", __FUNCTION__);
        return NULL;
    }
    gw_add_group_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_add_group_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_ADD_GROUP_REQ;

    return header;
}


Cmd_t* _REQ_GroupRemove(EpAddr_t epAddr, uint16_t gid)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*               header  = Cmder.create(MAJOR, NULL);
    GwRemoveFromGroupReq body    = GW_REMOVE_FROM_GROUP_REQ__INIT;
    GwAddressStructT     dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_REMOVE_FROM_GROUP_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);
    body.dstaddress = &dstAddr;
    body.has_groupid = true;
    body.groupid = (uint32_t)gid;

    header->msg = malloc(sizeof(PacketHeader) + gw_remove_from_group_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Error: Could not pack msg\n", __FUNCTION__);
        return NULL;
    }
    gw_remove_from_group_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_remove_from_group_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_REMOVE_FROM_GROUP_REQ;

    return header;
}

Cmd_t* _REQ_GroupGet(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*               header  = Cmder.create(MAJOR, NULL);
    GwGetGroupMembershipReq body = GW_GET_GROUP_MEMBERSHIP_REQ__INIT;
    GwAddressStructT     dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_GET_GROUP_MEMBERSHIP_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);
    body.dstaddress = &dstAddr;

    header->msg = malloc(sizeof(PacketHeader) + gw_get_group_membership_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Error: Could not pack msg\n", __FUNCTION__);
        return NULL;
    }
    gw_get_group_membership_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_get_group_membership_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_GET_GROUP_MEMBERSHIP_REQ;

    return header;
}

void _IND_GroupGet(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_Read");
    ControlHdr_t*               ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                      cmdHdr  = (Cmd_t*)msg;
    GwGetGroupMembershipRspInd* body    = NULL;

    body = gw_get_group_membership_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
    Logger.writeLog(LOG_DEBUG, "%s - [mac,eid] =(0x%llX, %d)", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);

    Key_t itemKey = UniqueKey.type("item").render(itemKid);
    TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
    if (item == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Item NOT found. [key,status] = (%s, %d)", __FUNCTION__, itemKey.data, body->status);
        return;
    }

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Capacity = %d", __FUNCTION__, body->capacity);
            for (int i = 0; i < body->capacity; i++)
                Logger.writeLog(LOG_DEBUG, "pxhoang: %s - groupList = %d", __FUNCTION__, body->grouplist[i]);
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Failed [code] = [%d]", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    gw_get_group_membership_rsp_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * @genScenes
 * add                   = c2s -> {groupid, sceneid, transtime, scenename, extensionfieldsets}
 * view                  = c2s -> {groupid, sceneid}
 * remove                = c2s -> {groupid, sceneid}
 * removeAll             = c2s -> {groupid}
 * store                 = c2s -> {groupid, sceneid}
 * recall                = c2s -> {groupid, sceneid}
 * getSceneMembership    = c2s -> {groupid}
 * enhancedAdd           = c2s -> {groupid, sceneid, transtime, scenename, extensionfieldsets}
 * enhancedView          = c2s -> {groupid, sceneid}
 * copy                  = c2s -> {mode, groupidfrom, sceneidfrom, groupidto, sceneidto}
 * addRsp                = s2c -> {status, groupId, sceneId}
 * viewRsp               = s2c -> {status, groupid, sceneid, transtime, scenename, extensionfieldsets}
 * removeRsp             = s2c -> {status, groupid, sceneid}
 * removeAllRsp          = s2c -> {status, groupid}
 * storeRsp              = s2c -> {status, groupid, sceneid}
 * getSceneMembershipRsp = s2c -> {status, capacity, groupid, scenecount, scenelist}
 * enhancedAddRsp        = s2c -> {}
 * enhancedViewRsp       = s2c -> {status, groupid, sceneid, transtime, scenename, extensionfieldsets}
 * copyRsp               = s2c -> {status, groupidfrom, sceneidfrom}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @genOnOff
 * [PASS] genOnOff_OffCommand                     = C2S -> {}
 * [PASS] genOnOff_OnCommand                      = C2S -> {}
 * [PASS] genOnOff_ToggleCommand                  = C2S -> {}
 * [NONE] genOnOff_OffWithEffectCommand           = C2S -> {effectid, effectvariant}
 * [NONE] genOnOff_OnWithRecallGlobalSceneCommand = C2S -> {}
 * [NONE] genOnOff_OnWithTimedOffCommand          = C2S -> {ctrlbits, ctrlbyte, ontime, offwaittime}
 **-----------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_State(EpAddr_t epAddr, GwOnOffStateT state)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*             header  = Cmder.create(MAJOR, NULL);
    DevSetOnOffStateReq body    = DEV_SET_ON_OFF_STATE_REQ__INIT;
    GwAddressStructT    dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_SET_ONOFF_STATE_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress = &dstAddr;
    body.state = state;
    header->msg = malloc(sizeof(PacketHeader) + dev_set_on_off_state_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_set_on_off_state_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_set_on_off_state_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_SET_ONOFF_STATE_REQ;

    return header;
}


Cmd_t* _REQ_On(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    return _REQ_State(epAddr, GW_ON_OFF_STATE_T__ON_STATE);
}


Cmd_t* _REQ_Off(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    return _REQ_State(epAddr, GW_ON_OFF_STATE_T__OFF_STATE);
}


Cmd_t* _REQ_Toggle()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    return NULL;
}


/**-------------------------------------------------------------------------------------------------
 * @genLevelCtrl
 * [PASS] genLevelCtrl_moveToLevelCommand          = C2S -> {level, transtime}
 * [NONE] genLevelCtrl_moveCommand                 = C2S -> {movemode, rate}
 * [NONE] genLevelCtrl_stepCommand                 = C2S -> {stepmode, stepsize, transtime}
 * [NONE] genLevelCtrl_stopCommand                 = C2S -> {}
 * [NONE] genLevelCtrl_moveToLevelWithOnOffCommand = C2S -> {level, transtime}
 * [NONE] genLevelCtrl_moveWithOnOffCommand        = C2S -> {movemode, rate}
 * [NONE] genLevelCtrl_stepWithOnOffCommand        = C2S -> {stepmode, stepsize, transtime}
 * [NONE] genLevelCtrl_stopWithOnOffCommand        = C2S -> {}
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_MoveToLevel(EpAddr_t epAddr, uint32_t transitionTime, uint32_t level)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    DevSetLevelReq   body    = DEV_SET_LEVEL_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_SET_LEVEL_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress     = &dstAddr;
    body.levelvalue     = level;
    body.transitiontime = transitionTime;
    header->msg = malloc(sizeof(PacketHeader) + dev_set_level_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Error: Could not pack msg\n", __FUNCTION__);
        return NULL;
    }
    dev_set_level_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_set_level_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_SET_LEVEL_REQ;

    return header;
}


/**-------------------------------------------------------------------------------------------------
 * @genAlarms
 * reset           = c2s -> {alarmcode, clusterid}
 * resetAll        = c2s -> {}
 * get             = c2s -> {}
 * resetLog        = c2s -> {}
 * publishEventLog = c2s -> {}
 * alarm           = s2c -> {alarmcode, clusterid}
 * getRsp          = s2c -> {status, alarmcode, clusterid, timestamp}
 * getEventLog     = s2c -> {}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @genRssiLocation
 * setAbsolute      = c2s -> {coord1, coord2, coord3, power, pathlossexponent}
 * setDevCfg        = c2s -> {power, pathlossexponent, calperiod, numrssimeasurements, reportingperiod}
 * getDevCfg        = c2s -> {targetaddr}
 * getData          = c2s -> {getdatainfo, numrsp, targetaddr}
 * devCfgRsp        = s2c -> {status, power, pathlossexp, calperiod, numrssimeasurements, reportingperiod}
 * dataRsp          = s2c -> {status, locationtype, coord1, coord2, coord3, power, pathlossexp, locationmethod, qualitymeasure, locationage}
 * dataNotif        = s2c -> {locationtype, coord1, coord2, coord3, power, pathlossexp, locationmethod, qualitymeasure, locationage}
 * compactDataNotif = s2c -> {locationtype, coord1, coord2, coord3, qualitymeasure, locationage}
 * rssiPing         = s2c -> {locationtype}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @genCommissioning
 * restartDevice           = c2s -> {options, delay, jitter}
 * saveStartupParams       = c2s -> {options, index}
 * restoreStartupParams    = c2s -> {options, index}
 * resetStartupParams      = c2s -> {options, index}
 * restartDeviceRsp        = s2c -> {status}
 * saveStartupParamsRsp    = s2c -> {status}
 * restoreStartupParamsRsp = s2c -> {status}
 * resetStartupParamsRsp   = s2c -> {status}
 **-----------------------------------------------------------------------------------------------*/
