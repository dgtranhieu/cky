#include "Logger.h"
#include "gateway.pb-c.h"
#include "Controller.h"
#include "UniqueKey.h"
#include "Identifier.h"
#include "SafeHash.h"
#include "GwUtils.h"
#include "IAS.h"

static void _attrChanged(void* header, TiDevice_t* item, uint16_t cid, uint16_t aid, GwAttributeRecordT* record)
{
    Logger.writeLog(LOG_DEBUG, "_attrChanged");
    ControlHdr_t* ctrlHdr = (ControlHdr_t*)header;

    Logger.writeLog(LOG_DEBUG, "\t%s - [aid,len] = [%d,%d]", __FUNCTION__, record->attributeid, record->attributevalue.len);
    for (int j = 0; j < record->attributevalue.len; j++)
        Logger.writeLog(LOG_DEBUG, "\t%s - [val] = [0x%x]",  __FUNCTION__, record->attributevalue.data[j]);

    Attr_t attr;
    attr.cid = cid;
    attr.aid = aid;
    attr.tid = record->attributetype;
    attr.len = record->attributevalue.len;
    memcpy(attr.val, record->attributevalue.data, record->attributevalue.len);

    item->ops.setState(ctrlHdr->tiHdr, item, TI_STATE_ATTR, attr);

    // printf("pxhoang: %s - [aid,tid] = [%d,%d]\n", __FUNCTION__, item->state.attrs.attr[index].aid, item->state.attrs.attr[index].tid);
    // printf("pxhoang: %s - len = %d\n", __FUNCTION__, item->state.attrs.attr[index].len);
    // for (int i = 0; i < item->state.attrs.attr[index].len; i++)
    //     printf("pxhoang: %s - val = 0x%x\n", __FUNCTION__, item->state.attrs.attr[index].val[i]);
}

/**-------------------------------------------------------------------------------------------------
 * @ssIasZone
 * [PASS] enrollRsp                = C2S -> {enrollrspcode, zoneid}
 * [NONE] statusChangeNotification = S2C -> {zonestatus, extendedstatus}
 * [NONE] enrollReq                = S2C -> {zonetype, manucode}
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_AutoEnrollRsp(EpAddr_t epAddr, uint32_t zoneId, int rspCode)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    static uint32_t pxhoangZoneId = 0;
    Cmd_t*             header  = Cmder.create(MAJOR, NULL);
    DevPxhoangEnrollRsp body    = DEV_PXHOANG_ENROLL_RSP__INIT;
    GwAddressStructT    dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_PXHOANG_ENROLL_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress             = &dstAddr;
    body.zoneid                 = pxhoangZoneId;
    body.enrollmentresponsecode = GW_ENROLL_RSP_CODE_T__ZONE_ENROLL_SUCCESS;

    header->msg = malloc(sizeof(PacketHeader) + dev_pxhoang_enroll_rsp__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    dev_pxhoang_enroll_rsp__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_pxhoang_enroll_rsp__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_PXHOANG_ENROLL_REQ;

    return header;
}


void _IND_StatusChangeNotification(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*           ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                  cmdHdr  = (Cmd_t*)msg;
    DevZoneStatusChangeInd* body    = NULL;

    body = dev_zone_status_change_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);
    Logger.writeLog(LOG_DEBUG, "%s - itemkey = %s", __FUNCTION__, itemKey.data);
    TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
    if (item == NULL)
        return;

    Logger.writeLog(LOG_DEBUG, "%s:  [mac,eid]       = [0x%llX, %d]",   __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
    Logger.writeLog(LOG_DEBUG, "%s:  zoneStatus      = 0x%x",   __FUNCTION__, body->zonestatus);
    Logger.writeLog(LOG_DEBUG, "%s:  extendedstatus  = 0x%x",   __FUNCTION__, body->extendedstatus);

    // Set zone status attribute
    GwAttributeRecordT record;
    memset(&record, 0, sizeof(GwAttributeRecordT));

    record.attributeid         = 0x0002;
    record.attributetype       = etid_bitmap16;                  // 2 bytes
    record.attributevalue.len  = 2;                              // Len in bytes
    record.attributevalue.data = (uint8_t*)(&body->zonestatus);

    _attrChanged(header, item, ecid_ssIasZone, 0x0002, &record);

    dev_zone_status_change_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * @ssIasAce
 * [NONE] arm                 = C2S -> {armmode}
 * [NONE] bypass              = C2S -> {numofzones, zoneidlist}
 * [NONE] emergency           = C2S -> {}
 * [NONE] fire                = C2S -> {}
 * [NONE] panic               = C2S -> {}
 * [NONE] getZoneIDMap        = C2S -> {}
 * [NONE] getZoneInfo         = C2S -> {zoneid}
 * [NONE] getPanelStatus      = C2S -> {}
 * [NONE] getBypassedZoneList = C2S -> {}
 * [NONE] getZoneStatus       = C2S -> {startzoneid, maxnumzoneid, zonestatusmaskflag, zonestatusmask}
 * [NONE] armRsp              = S2C -> {armnotification}
 * [NONE] getZoneIDMapRsp     = S2C -> {zoneidmapsection0, zoneidmapsection1, zoneidmapsection2, zoneidmapsection3,
 *                              zoneidmapsection4, zoneidmapsection5, zoneidmapsection6, zoneidmapsection7,
 *                              zoneidmapsection8, zoneidmapsection9, zoneidmapsection10, zoneidmapsection11,
 *                              zoneidmapsection12, zoneidmapsection13, zoneidmapsection14, zoneidmapsection15}
 * [NONE] getZoneInfoRsp      = S2C -> {zoneid, zonetype, ieeeaddr}
 * [NONE] zoneStatusChanged   = S2C -> {zoneid, zonestatus, audiblenotif, strlen, string}
 * [NONE] panelStatusChanged  = S2C -> {panelstatus, secondsremain, audiblenotif, alarmstatus}
 * [NONE] getPanelStatusRsp   = S2C -> {panelstatus, secondsremain, audiblenotif, alarmstatus}
 * [NONE] setBypassedZoneList = S2C -> {numofzones, zoneid}
 * [NONE] bypassRsp           = S2C -> {numofzones, bypassresult}
 * [NONE] getZoneStatusRsp    = S2C -> {zonestatuscomplete, numofzones, zoneinfo}
 **-----------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * @ssIasWd
 * [NONE] startWarning = S2C -> {startwarninginfo, warningduration}
 * [NONE] squawk       = c2s -> {squawkinfo }
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_StartWarning(EpAddr_t epAddr, uint8_t warningmessage, uint16_t warningDuration, uint8_t strobeDutyCycle, uint8_t strobeLevel)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                    header  = Cmder.create(MAJOR, NULL);
    DevPxhoangStartWarningReq body    = DEV_PXHOANG_START_WARNING_REQ__INIT;
    GwAddressStructT          dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__DEV_PXHOANG_START_WARNING_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress      = &dstAddr;
    body.warningmessage  = (uint32_t)warningmessage;
    body.warningduration = (uint32_t)warningDuration;
    body.strobedutycycle = (uint32_t)strobeDutyCycle;
    body.strobelevel     = (uint32_t)strobeLevel;

    header->msg = malloc(sizeof(PacketHeader) + dev_pxhoang_start_warning_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Error: Could not pack msg\n", __FUNCTION__);
        return NULL;
    }
    dev_pxhoang_start_warning_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = dev_pxhoang_start_warning_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__DEV_PXHOANG_START_WARNING_REQ;

    return header;
}
