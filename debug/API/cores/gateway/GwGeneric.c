#include "Logger.h"
#include "gateway.pb-c.h"
#include "SafeHash.h"
#include "UniqueKey.h"
#include "Controller.h"
#include "GwGeneric.h"

/**-------------------------------------------------------------------------------------------------
 * @pxhoang: This command get called to confirm that GW has received the message. Async indicator
 * would be processed then.
 * Generic response
 *------------------------------------------------------------------------------------------------*/

void _RSP_Generic(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*       ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*              cmdHdr  = (Cmd_t*)msg;
    GwZigbeeGenericCnf* body    = NULL;

    body = gw_zigbee_generic_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - Success", __FUNCTION__);
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    gw_zigbee_generic_cnf__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

void _IND_Generic(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*       ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*              cmdHdr  = (Cmd_t*)msg;
    GwZigbeeGenericRspInd* body    = NULL;

    Logger.writeLog(LOG_DEBUG, "pxhoang: %s - len = %d\n", __FUNCTION__, cmdHdr->msg->header.len);
    body = gw_zigbee_generic_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case GW_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "Dev: %s - Success", __FUNCTION__);
            break;

        case GW_STATUS_T__STATUS_FAILURE:
        case GW_STATUS_T__STATUS_BUSY:
        case GW_STATUS_T__STATUS_INVALID_PARAMETER:
        case GW_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "Dev: %s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    gw_zigbee_generic_rsp_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Auto Enroll Response
 *------------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * ZCL Frame
 *------------------------------------------------------------------------------------------------*/

void _IND_ZclFrame(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*         ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                cmdHdr  = (Cmd_t*)msg;
    GwZclFrameReceiveInd* body    = NULL;

    body = gw_zcl_frame_receive_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Kid_t itemKid = {.item = {body->srcaddress->ieeeaddr, body->srcaddress->endpointid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);
    Logger.writeLog(LOG_DEBUG, "%s: itemkey = %s, statelen = %d", __FUNCTION__, itemKey.data);

    TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
    if (item == NULL)
        return;

    ZclMsg_t zclMsg = {body->clusterid, body->commandid, body->payload.len, body->payload.data};
    item->ops.onEvent(ctrlHdr->tiHdr, item, zclMsg);
    // Logger.writeLog(LOG_DEBUG, "%s: clientserverdirection    = %d",   __FUNCTION__, body->clientserverdirection);
    // Logger.writeLog(LOG_DEBUG, "%s: clusterid                = %d",   __FUNCTION__, body->clusterid);
    // Logger.writeLog(LOG_DEBUG, "%s: cmdid                    = %d",   __FUNCTION__, body->cmdid);
    // Logger.writeLog(LOG_DEBUG, "%s: commandid                = %d",   __FUNCTION__, body->commandid);
    // Logger.writeLog(LOG_DEBUG, "%s: disabledefaultrsp        = %d",   __FUNCTION__, body->disabledefaultrsp);
    // Logger.writeLog(LOG_DEBUG, "%s: endpointiddest           = %d",   __FUNCTION__, body->endpointiddest);
    // Logger.writeLog(LOG_DEBUG, "%s: frametype                = %d",   __FUNCTION__, body->frametype);
    // Logger.writeLog(LOG_DEBUG, "%s: manufacturercode         = %d",   __FUNCTION__, body->manufacturercode);
    // Logger.writeLog(LOG_DEBUG, "%s: manufacturerspecificflag = %d",   __FUNCTION__, body->manufacturerspecificflag);
    // Logger.writeLog(LOG_DEBUG, "%s: profileid                = %d",   __FUNCTION__, body->profileid);
    // Logger.writeLog(LOG_DEBUG, "%s: sequencenumber           = %d",   __FUNCTION__, body->sequencenumber);
    // Logger.writeLog(LOG_DEBUG, "%s: endpointid               = %d",   __FUNCTION__, body->srcaddress->endpointid);
    // Logger.writeLog(LOG_DEBUG, "%s: ieeeaddr                 = %llX", __FUNCTION__, body->srcaddress->ieeeaddr);

    for (int i = 0; i < item->state.attrs.len; i++)
        Logger.writeLog(LOG_DEBUG, "     %s: [len, cid, aid, time] = (%d, %d, %d, %ld)", __FUNCTION__, item->state.attrs.len,
        item->state.attrs.attr[i].cid, item->state.attrs.attr[i].aid, item->state.attrs.attr[i].time);

    gw_zcl_frame_receive_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/
