#include "Logger.h"
#include "gateway.pb-c.h"
#include "UniqueKey.h"
#include "GwUtils.h"
#include "SafeHash.h"
#include "Foundation.h"


static void _attrChanged(void* header, TiDevice_t* item, uint16_t cid, uint16_t aid, GwAttributeRecordT* record)
{
    Logger.writeLog(LOG_DEBUG, "_attrChanged");
    ControlHdr_t* ctrlHdr = (ControlHdr_t*)header;

    Logger.writeLog(LOG_DEBUG, "\t%s - [aid,len,time] = [%d,%d,%ld]", __FUNCTION__, record->attributeid, record->attributevalue.len, time(NULL));
    // for (int i = 0; i < record->attributevalue.len; i++)
    //     Logger.writeLog(LOG_DEBUG, "    pxhoang: %s - [val] = [0x%x]", __FUNCTION__, record->attributevalue.data[i]);

    Attr_t attr;
    attr.cid = cid;
    attr.aid = aid;
    attr.tid = record->attributetype;
    attr.len = record->attributevalue.len;
    attr.time = time(NULL);
    memcpy(attr.val, record->attributevalue.data, record->attributevalue.len);

    item->ops.setState(ctrlHdr->tiHdr, item, TI_STATE_ATTR, attr);
}


/**-------------------------------------------------------------------------------------------------
 * @Foundation
 * [PASS] read                = Read attributes	                        [ readRec, ... ]	           none
 * [PASS] readRsp             = Read attributes response	            [ readStatusRec, ... ]	       none
 * [PASS] write               = Write attributes	                    [ writeRec, ... ]	           none
 * [    ] writeUndiv          = Write attributes undivided	            [ writeRec, ... ]	           none
 * [PASS] writeRsp            = Write attributes response	            [ writeStatusRec, ... ]	       none
 * [    ] writeNoRsp          = Write attributes no response	        [ writeRec, ... ]	           none
 * [PASS] configReport        = Configure reporting	                    [ cfgRptRec, ... ]	           none
 * [PASS] configReportRsp     = Configure reporting response	        [ cfgRptStatusRec, ... ]	   none
 * [    ] readReportConfig    = Read reporting configuration	        [ readRptRec, ... ]	           none
 * [    ] readReportConfigRsp = Read reporting configuration response	[ readRptStatusRec, ... ]	   none
 * [PASS] report              = Report attributes	                    [ reportRec, ... ]	           none
 * [    ] defaultRsp          = Default response	                    { cmdId, statusCode }	       uint8, uint8
 * [    ] discover            = Discover attributes	                    { startAttrId, maxAttrIds }	   uint16, uint8
 * [    ] discoverRsp         = Discover attributes response	        { discComplete, attrInfos }	   uint16, array(attrInfo)
 * [NONE] readStruct          = Read attributes structured	            [ readStructRec, ... ]	       none
 * [NONE] writeStruct         = Write attributes structured	            [ writeStructRec, ... ]	       none
 * [    ] writeStructRsp      = Write attributes structured response	[ writeStructStstusRec, ... ]  none
 **-----------------------------------------------------------------------------------------------*/

Cmd_t* _REQ_Read(EpAddr_t epAddr, uint16_t cid, ReadRec readRec)
{
    Logger.writeLog(LOG_DEBUG, "_REQ_Read - 0x%llX", epAddr.ieee_addr);
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    GwReadDeviceAttributeReq   body    = GW_READ_DEVICE_ATTRIBUTE_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_READ_DEVICE_ATTRIBUTE_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);
    // Logger.writeLog(LOG_DEBUG, "%s - key = %s", __FUNCTION__, UniqueKey.type("cmd").render(cmdKid).data);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress       = &dstAddr;
    body.clusterid        = cid;
    body.attributelist    = readRec.aid;
    body.n_attributelist  = readRec.len;
    body.isservertoclient = false;
    header->msg = malloc(sizeof(PacketHeader) + gw_read_device_attribute_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    gw_read_device_attribute_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_read_device_attribute_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_READ_DEVICE_ATTRIBUTE_REQ;

    return header;
}


void _IND_Read(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_Read");
    ControlHdr_t*               ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                      cmdHdr  = (Cmd_t*)msg;
    GwReadDeviceAttributeRspInd* body    = NULL;

    body = gw_read_device_attribute_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
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
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep,cid] = [0x%llX, %d, %d]", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid, body->clusterid);

            for (int i = 0; i < body->n_attributerecordlist; i++)
            {
                GwAttributeRecordT* record = body->attributerecordlist[i];
                _attrChanged(header, item, body->clusterid, body->attributerecordlist[i]->attributeid, record);
            }
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep,cid,code] = (0x%llX, %d, %d, %d)", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid, body->clusterid, body->status);
            break;

        default:
            break;
    }

    gw_read_device_attribute_rsp_ind__free_unpacked(body, NULL);
}


Cmd_t* _REQ_Write(EpAddr_t epAddr, uint16_t cid, WriteRec writeRec)
{
    Logger.writeLog(LOG_DEBUG, "_REQ_Write");
    Cmd_t*                   header  = Cmder.create(MAJOR, NULL);
    GwWriteDeviceAttributeReq body    = GW_WRITE_DEVICE_ATTRIBUTE_REQ__INIT;
    GwAddressStructT          dstAddr = GW_ADDRESS_STRUCT_T__INIT;
    GwAttributeRecordT aRecList[16] = {GW_ATTRIBUTE_RECORD_T__INIT};
    GwAttributeRecordT *aRecPtr[16];

    for (int i = 0; i < writeRec.len; i++)
    {
        aRecList[i].attributeid         = (uint32_t)writeRec.attr->aid;
        aRecList[i].attributetype       = (GwZclAttributeDataTypesT)writeRec.attr->tid;
        aRecList[i].attributevalue.len  = (size_t)writeRec.attr->len;
        aRecList[i].attributevalue.data = writeRec.attr->val;
        aRecPtr[i] = &aRecList[i];
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_WRITE_DEVICE_ATTRIBUTE_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress            = &dstAddr;
    body.clusterid             = (uint32_t)cid;
    body.attributerecordlist   = (GwAttributeRecordT**)&aRecPtr;
    body.n_attributerecordlist = (size_t)writeRec.len;
    header->msg = malloc(sizeof(PacketHeader) + gw_write_device_attribute_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    gw_write_device_attribute_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_write_device_attribute_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_WRITE_DEVICE_ATTRIBUTE_REQ;

    return header;
}


void _IND_Write(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_Write");
    ControlHdr_t*               ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                      cmdHdr  = (Cmd_t*)msg;
    GwWriteDeviceAttributeRspInd* body    = NULL;

    body = gw_write_device_attribute_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep] = [0x%llX,%d]\n", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
            Logger.writeLog(LOG_DEBUG, "%s - [cid] = [%d]\n", __FUNCTION__, body->clusterid);

            for (int i = 0; i < body->n_attributewriteerrorlist; i++)
                Logger.writeLog(LOG_DEBUG, "\t%s - [aid,status] = [%d,%d]\n", __FUNCTION__, body->attributewriteerrorlist[i]->attributeid, body->attributewriteerrorlist[i]->status);
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

    gw_write_device_attribute_rsp_ind__free_unpacked(body, NULL);
}


Cmd_t* _REQ_ConfigReport(EpAddr_t epAddr, uint16_t cid, CfgRptRec cfgRptRec)
{
    Logger.writeLog(LOG_DEBUG, "_REQ_ConfigReport");
    Cmd_t*                   header  = Cmder.create(MAJOR, NULL);
    GwSetAttributeReportingReq body    = GW_SET_ATTRIBUTE_REPORTING_REQ__INIT;
    GwAddressStructT          dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    GwAttributeReportT aRptList[16] = {GW_ATTRIBUTE_REPORT_T__INIT};
    GwAttributeReportT *aRptPtr[16];

    for (int i = 0; i < cfgRptRec.len; i++)
    {
        aRptList[i].attributeid        = (uint32_t)cfgRptRec.rptRec[i].attribute.aid;
        aRptList[i].attributetype      = (GwZclAttributeDataTypesT)cfgRptRec.rptRec[i].attribute.tid;
        aRptList[i].minreportinterval  = cfgRptRec.rptRec[i].minRepIntval;
        aRptList[i].maxreportinterval  = cfgRptRec.rptRec[i].maxRepIntval;
        aRptList[i].n_reportablechange = 1;
        aRptList[i].reportablechange   = &cfgRptRec.rptRec[i].repChange;
        aRptPtr [i]                    = &aRptList[i];
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_SET_ATTRIBUTE_REPORTING_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress            = &dstAddr;
    body.clusterid             = (uint32_t)cid;
    body.n_attributereportlist = cfgRptRec.len;
    body.attributereportlist   = (GwAttributeReportT**)&aRptPtr;

    header->msg = malloc(sizeof(PacketHeader) + gw_set_attribute_reporting_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    gw_set_attribute_reporting_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_set_attribute_reporting_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_SET_ATTRIBUTE_REPORTING_REQ;

    return header;
}


void _IND_ConfigReport(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_ConfigReport");
    ControlHdr_t*               ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                      cmdHdr  = (Cmd_t*)msg;
    GwSetAttributeReportingRspInd* body    = NULL;

    body = gw_set_attribute_reporting_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep] = [0x%llX,%d]\n", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
            Logger.writeLog(LOG_DEBUG, "%s - [cid] = [%d]\n", __FUNCTION__, body->clusterid);
            Logger.writeLog(LOG_DEBUG, "%s - [nList] = [%d]\n", __FUNCTION__, body->n_attributereportconfiglist);

            for (int i = 0; i < body->n_attributereportconfiglist; i++)
                Logger.writeLog(LOG_DEBUG, "\t%s - [aid,status] = [%d,%d]\n", __FUNCTION__, body->attributereportconfiglist[i]->attributeid, body->attributereportconfiglist[i]->status);

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

    gw_set_attribute_reporting_rsp_ind__free_unpacked(body, NULL);
}


void _IND_Report(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_Report");
    ControlHdr_t*               ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                      cmdHdr  = (Cmd_t*)msg;
    GwAttributeReportingInd* body    = NULL;

    body = gw_attribute_reporting_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);
    TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
    if (item == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "%s - Item NOT found", __FUNCTION__);
        return;
    }

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - [mac,ep] = [0x%llX,%d]\n", __FUNCTION__, body->srcaddress->ieeeaddr, body->srcaddress->endpointid);
            Logger.writeLog(LOG_DEBUG, "%s - [cid] = [%d]\n", __FUNCTION__, body->clusterid);
            Logger.writeLog(LOG_DEBUG, "%s - [nattr] = [%d]\n", __FUNCTION__, body->n_attributerecordlist);

            for (int i = 0; i < body->n_attributerecordlist; i++)
            {
                GwAttributeRecordT* record = body->attributerecordlist[i];
                _attrChanged(header, item, body->clusterid, record->attributeid, record);
            }
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            printf("%s - Failed (%d)\n", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    gw_attribute_reporting_ind__free_unpacked(body, NULL);
}


Cmd_t* _REQ_Discover(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "_REQ_Discover");
    Cmd_t*          header  = Cmder.create(MAJOR, NULL);
    GwGetDeviceAttributeListReq  body    = GW_GET_DEVICE_ATTRIBUTE_LIST_REQ__INIT;
    GwAddressStructT dstAddr = GW_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainDev, eModeSync, GW_CMD_ID_T__GW_GET_DEVICE_ATTRIBUTE_LIST_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init Body
    genAddr(&epAddr, &dstAddr);

    body.dstaddress       = &dstAddr;
    header->msg = malloc(sizeof(PacketHeader) + gw_get_device_attribute_list_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "pxhoang: %s - Error: Could not pack msg", __FUNCTION__);
        return NULL;
    }
    gw_get_device_attribute_list_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = gw_get_device_attribute_list_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_GW_SYS_ID_T__RPC_SYS_PB_GW;
    header->msg->header.cmdId  = GW_CMD_ID_T__GW_GET_DEVICE_ATTRIBUTE_LIST_REQ;

    return header;
}


void _IND_Discover(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "_IND_Discover");
    ControlHdr_t*                   ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                          cmdHdr  = (Cmd_t*)msg;
    GwGetDeviceAttributeListRspInd* body    = NULL;

    body = gw_get_device_attribute_list_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);
    TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - Success\n", __FUNCTION__);
            Logger.writeLog(LOG_DEBUG, "%s - numCluster = %d\n", __FUNCTION__, body->n_clusterlist);
            for (int i = 0; i < body->n_clusterlist; i++)
            {
                GwClusterListT* cluster = body->clusterlist[i];
                Logger.writeLog(LOG_DEBUG, "%s - cluster(%d)\n", __FUNCTION__, cluster->clusterid);
                Logger.writeLog(LOG_DEBUG, "%s - numAttr(%d)\n", __FUNCTION__, cluster->n_attributelist);
                for (int j = 0; j < cluster->n_attributelist; j++)
                {
                    Logger.writeLog(LOG_DEBUG, "\t%s - attr = %d\n", __FUNCTION__, cluster->attributelist[j]);
                    GwAttributeRecordT record;
                    memset(&record, 0, sizeof(GwAttributeRecordT));

                    uint32_t attr = body->clusterlist[i]->attributelist[j];
                    _attrChanged(header, item, cluster->clusterid, attr, &record);
                }
            }
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

    gw_get_device_attribute_list_rsp_ind__free_unpacked(body, NULL);
}
