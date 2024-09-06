#include "SafeHash.h"
#include "Logger.h"
#include "nwkmgr.pb-c.h"
#include "Identifier.h"
#include "Controller.h"
#include "TiDevice.h"
#include "Network.h"

/**-------------------------------------------------------------------------------------------------
 * @static local
 *------------------------------------------------------------------------------------------------*/

static void _debug_device(NwkDeviceInfoT* devList)
{
    if (devList == NULL)
        return;

    Logger.writeLog(LOG_DEBUG, "%s: ieeeaddress        = 0x%llx", __FUNCTION__, devList->ieeeaddress);
    Logger.writeLog(LOG_DEBUG, "%s: networkaddress     = 0x%lx" , __FUNCTION__, devList->networkaddress);
    Logger.writeLog(LOG_DEBUG, "%s: parentieeeaddress  = 0x%lx" , __FUNCTION__, devList->parentieeeaddress);
    Logger.writeLog(LOG_DEBUG, "%s: manufacturerid     = 0x%lx" , __FUNCTION__, devList->manufacturerid);
    Logger.writeLog(LOG_DEBUG, "%s: devicestatus       = %d"    , __FUNCTION__, devList->devicestatus);
    Logger.writeLog(LOG_DEBUG, "%s: n_simpledesclist   = %d"    , __FUNCTION__, devList->n_simpledesclist);

    for (int i = 0; i < devList->n_simpledesclist; i++)
    {
        Logger.writeLog(LOG_DEBUG, "     Endpoint %d", i);
        Logger.writeLog(LOG_DEBUG, "     endpointid         = %d", devList->simpledesclist[i]->endpointid);
        Logger.writeLog(LOG_DEBUG, "     profileid          = %d", devList->simpledesclist[i]->profileid);
        Logger.writeLog(LOG_DEBUG, "     deviceid           = %d", devList->simpledesclist[i]->deviceid);
        Logger.writeLog(LOG_DEBUG, "     devicever          = %d", devList->simpledesclist[i]->devicever);
        Logger.writeLog(LOG_DEBUG, "     n_inputclusters    = %d", devList->simpledesclist[i]->n_inputclusters);
        Logger.writeLog(LOG_DEBUG, "     n_outputclusters   = %d", devList->simpledesclist[i]->n_outputclusters);

        for (int j = 0; j < devList->simpledesclist[i]->n_inputclusters; j++)
            Logger.writeLog(LOG_DEBUG, "     iclusters: 0x%x", (uint16_t)devList->simpledesclist[i]->inputclusters[j]);

        for (int j = 0; j < devList->simpledesclist[i]->n_outputclusters; j++)
            Logger.writeLog(LOG_DEBUG, "     oclusters: 0x%x", (uint16_t)devList->simpledesclist[i]->outputclusters[j]);
        Logger.writeLog(LOG_DEBUG, "");
        Logger.writeLog(LOG_DEBUG, "");
    }
}


/**-------------------------------------------------------------------------------------------------
 * @static local
 *------------------------------------------------------------------------------------------------*/

static void _removeItems(void* header, NwkDeviceInfoT* device)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t* ctrlHdr = (ControlHdr_t*)header;

    for (int i = 0; i < device->n_simpledesclist; i++)
    {
        NwkSimpleDescriptorT* endpoint = device->simpledesclist[i];
        Kid_t itemKid ={.item = {device->ieeeaddress, endpoint->endpointid}};
        Key_t itemKey = UniqueKey.type("item").render(itemKid);

        Logger.writeLog(LOG_DEBUG, "%s - key = %s", __FUNCTION__, itemKey.data);

        TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
        if (item == NULL)
        {
            Logger.writeLog(LOG_DEBUG, "%s - Item not found (%s)", __FUNCTION__, itemKey.data);
            continue;
        }

        if (item->ops.exit)
            item->ops.exit(ctrlHdr->tiHdr, item);

        SafeHash.remove(ctrlHdr->itemHdr, itemKey);

        if (ctrlHdr->tiNotifyCB)
            ctrlHdr->tiNotifyCB(TI_EVN_LIST_CHANGED, itemKey);

        TiDevice_t* readBack = SafeHash.find(ctrlHdr->itemHdr, itemKey);
        if (readBack == NULL)
            Logger.writeLog(LOG_DEBUG, "%s - item %d was removed", __FUNCTION__, item->specs.eid);
    }
}


static void _CreateItems(void* header, NwkDeviceInfoT* device, bool isJoined)
{
    Logger.writeLog(LOG_DEBUG, "%s - mac = 0x%llX", __FUNCTION__, device->ieeeaddress);
    ControlHdr_t*   ctrlHdr  = (ControlHdr_t*)header;

    for (int i = 0; i < device->n_simpledesclist; i++)
    {
        NwkSimpleDescriptorT* endpoint = device->simpledesclist[i];
        Kid_t   itemKid  = {.item = {device->ieeeaddress, endpoint->endpointid}};
        Key_t   itemKey = UniqueKey.type("item").render(itemKid);
        TiDevice_t* item = SafeHash.find(ctrlHdr->itemHdr, itemKey);
        if (item == NULL)
        {
            TiDevice_t* item = TiFactory.render(ctrlHdr->tiHdr, device, endpoint);
            if (item != NULL)
            {
                Logger.writeLog(LOG_DEBUG, "%s - TiDevice adding - [pid,did,eid] = (%d, %d, %d)", __FUNCTION__, endpoint->profileid, endpoint->deviceid, endpoint->endpointid);
                SafeHash.insert(ctrlHdr->itemHdr, itemKey, item);

                if (device->ieeeaddress != ctrlHdr->sysHdr.coordinator)
                {
                    if (item->ops.discovery)
                        item->ops.discovery(ctrlHdr->tiHdr, item);
                }
            }
            else
            {
                Logger.writeLog(LOG_DEBUG, "%s - TiDevice Not supported - [pid,did,eid] = (%d, %d, %d)", __FUNCTION__, endpoint->profileid, endpoint->deviceid, endpoint->endpointid);
                continue;
            }

            if (!isJoined)
            {
                if (item->ops.init)
                    item->ops.init(ctrlHdr->tiHdr, item);

                if (ctrlHdr->tiNotifyCB)
                    ctrlHdr->tiNotifyCB(TI_EVN_LIST_CHANGED, itemKey);
            }
        }
        else
        {
            // @pxhoang: Update only
            Logger.writeLog(LOG_DEBUG, "%s - TiDevice existed", __FUNCTION__);
        }
    }
}



/**-------------------------------------------------------------------------------------------------
 * @pxhoang: This command get called to confirm that NW has received the message. Async indicator
 * would be processed then.
 * Generic response
 *------------------------------------------------------------------------------------------------*/

static void _RSP_Generic(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*        ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*               cmdHdr  = (Cmd_t*)msg;
    NwkZigbeeGenericCnf* body    = NULL;

    body = nwk_zigbee_generic_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case NWK_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - Success", __FUNCTION__);
            break;

        case NWK_STATUS_T__STATUS_FAILURE:
        case NWK_STATUS_T__STATUS_BUSY:
        case NWK_STATUS_T__STATUS_INVALID_PARAMETER:
        case NWK_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    if (body == NULL)
        Logger.writeLog(LOG_DEBUG, "%s - body NULL", __FUNCTION__);

    nwk_zigbee_generic_cnf__free_unpacked(body, NULL);
}




/**-------------------------------------------------------------------------------------------------
 * Coordinator information request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_CoordInfo()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                   header = Cmder.create(MAJOR, NULL);
    NwkGetLocalDeviceInfoReq body   = NWK_GET_LOCAL_DEVICE_INFO_REQ__INIT;

    header->msg = malloc(sizeof(PacketHeader) + nwk_get_local_device_info_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_GET_LOCAL_DEVICE_INFO_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_get_local_device_info_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_GET_LOCAL_DEVICE_INFO_REQ;


    // Init body
    nwk_get_local_device_info_req__pack(&body, header->msg->body);

    return header;
}


static void _RSP_CoordInfo(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*             ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                    cmdHdr  = (Cmd_t*)msg;
    NwkGetLocalDeviceInfoCnf* body    = NULL;

    body = nwk_get_local_device_info_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    // Update coordinator mac before creating items
    NwkDeviceInfoT* device = body->deviceinfolist;
    ctrlHdr->sysHdr.coordinator = device->ieeeaddress;
    // Logger.writeLog(LOG_DEBUG, "%s: coord = %llX", __FUNCTION__, ctrlHdr->sysHdr.coordinator);

    _CreateItems(header, device, true);
    SafeHash.display(ctrlHdr->itemHdr);

    nwk_get_local_device_info_cnf__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Device List request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_DevList()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*              header = Cmder.create(MAJOR, NULL);
    NwkGetDeviceListReq body   = NWK_GET_DEVICE_LIST_REQ__INIT;

    header->msg = malloc(sizeof(PacketHeader) + nwk_get_device_list_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_GET_DEVICE_LIST_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_get_device_list_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_GET_DEVICE_LIST_REQ;

    // Init body
    nwk_get_device_list_req__pack(&body, header->msg->body);

    return header;
}


static void _RSP_DevList(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*        ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*               cmdHdr  = (Cmd_t*)msg;
    NwkGetDeviceListCnf* body    = NULL;

    body = nwk_get_device_list_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    Logger.writeLog(LOG_DEBUG, "%s: NumDevice = %d", __FUNCTION__, body->n_devicelist);
    for (int i = 0; i < body->n_devicelist; i++)
    {
        NwkDeviceInfoT* device = body->devicelist[i];
        // _debug_device(device);
        _CreateItems(header, device, true);
    }

    SafeHash.display(ctrlHdr->itemHdr);

    nwk_get_device_list_cnf__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Network information request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_NetworkInfo()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*              header = Cmder.create(MAJOR, NULL);
    NwkZigbeeNwkInfoReq body   = NWK_ZIGBEE_NWK_INFO_REQ__INIT;

    header->msg = malloc(sizeof(PacketHeader) + nwk_zigbee_nwk_info_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_ZIGBEE_NWK_INFO_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_zigbee_nwk_info_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_ZIGBEE_NWK_INFO_REQ;

    // Init body
    nwk_zigbee_nwk_info_req__pack(&body, header->msg->body);

    return header;
}


static void _RSP_NetworkInfo(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*        ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*               cmdHdr  = (Cmd_t*)msg;
    NwkZigbeeNwkInfoCnf* body    = NULL;

    body = nwk_zigbee_nwk_info_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    ctrlHdr->sysHdr.channel  = body->nwkchannel;
    ctrlHdr->sysHdr.panId    = body->panid;
    ctrlHdr->sysHdr.extPanId = body->extpanid;
    ctrlHdr->sysHdr.status   = body->status; // up or down

    Logger.writeLog(LOG_DEBUG, "%s: [channel,panid,extpanid,status] = (%d, 0x%x, 0x%llx, %d)", __FUNCTION__,
    ctrlHdr->sysHdr.channel, ctrlHdr->sysHdr.panId, ctrlHdr->sysHdr.extPanId, ctrlHdr->sysHdr.status);

    nwk_zigbee_nwk_info_cnf__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * @pxhoang: Dropped since _coordInfoEvn
 * Endpoint info request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_EndpointInfo()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                  header = Cmder.create(MAJOR, NULL);
    NwkGetGwEndpointInfoReq body   = NWK_GET_GW_ENDPOINT_INFO_REQ__INIT;

    header->msg = malloc(sizeof(PacketHeader) + nwk_get_gw_endpoint_info_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_GET_GW_ENDPOINT_INFO_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_get_gw_endpoint_info_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_GET_GW_ENDPOINT_INFO_REQ;

    // Init body
    nwk_get_gw_endpoint_info_req__pack(&body, header->msg->body);

    return header;
}


static void _RSP_EndpointInfo(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*            ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                   cmdHdr  = (Cmd_t*)msg;
    NwkGetGwEndpointInfoCnf* body    = NULL;

    body = nwk_get_gw_endpoint_info_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    NwkDeviceInfoT* device = body->deviceinfolist;
    // _debug_device(device);
    _CreateItems(header, device, true);

    nwk_get_gw_endpoint_info_cnf__free_unpacked(body, NULL);
}



/**-------------------------------------------------------------------------------------------------
 * Network key request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_NetworkKey()
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*          header = Cmder.create(MAJOR, NULL);
    NwkGetNwkKeyReq body   = NWK_GET_NWK_KEY_REQ__INIT;

    header->msg = malloc(sizeof(PacketHeader) + nwk_get_nwk_key_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_GET_GW_ENDPOINT_INFO_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_get_nwk_key_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_GET_NWK_KEY_REQ;

    // Init body
    nwk_get_nwk_key_req__pack(&body, header->msg->body);

    return header;
}


static void _RSP_NetworkKey(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*    ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*           cmdHdr  = (Cmd_t*)msg;
    NwkGetNwkKeyCnf* body    = NULL;

    body = nwk_get_nwk_key_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    for (int i = 0; i < body->newkey.len; i++)
        ctrlHdr->sysHdr.nwkKey[i] = body->newkey.data[i];

    nwk_get_nwk_key_cnf__free_unpacked(body, NULL);
}



/**-------------------------------------------------------------------------------------------------
 * Link quality request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_LinkQuality(EpAddr_t epAddr, uint8_t startIndex)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                 header  = Cmder.create(MAJOR, NULL);
    NwkGetNeighborTableReq body    = NWK_GET_NEIGHBOR_TABLE_REQ__INIT;
    NwkAddressStructT      dstAddr = NWK_ADDRESS_STRUCT_T__INIT;
    // Key-> Body -> header: This order must be respected to avoid crashed

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_GET_NEIGHBOR_TABLE_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init body
    dstAddr.addresstype    = NWK_ADDRESS_TYPE_T__UNICAST;
    dstAddr.has_ieeeaddr   = true;
    dstAddr.ieeeaddr       = epAddr.ieee_addr;
    dstAddr.has_endpointid = true;
    dstAddr.endpointid     = epAddr.endpoint;
    body.dstaddr           = &dstAddr;
    body.startindex        = startIndex;
    header->msg = malloc(sizeof(PacketHeader) + nwk_get_neighbor_table_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    nwk_get_neighbor_table_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = nwk_get_neighbor_table_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_GET_NEIGHBOR_TABLE_REQ;

    return header;
}


static void _IND_LinkQuality(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*              ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                     cmdHdr  = (Cmd_t*)msg;
    NwkGetNeighborTableRspInd* body    = NULL;

    body = nwk_get_neighbor_table_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case NWK_STATUS_T__STATUS_SUCCESS:
            for (int i = 0; i < body->n_neighborlist; i++)
            {
                Logger.writeLog(LOG_DEBUG, "%s - lqi                   = %d", __FUNCTION__, body->neighborlist[i]->lqi);
                Logger.writeLog(LOG_DEBUG, "%s - relation              = %d", __FUNCTION__, body->neighborlist[i]->relation);
                Logger.writeLog(LOG_DEBUG, "%s - networkaddress        = %d", __FUNCTION__, body->neighborlist[i]->networkaddress);
                Logger.writeLog(LOG_DEBUG, "%s - devicetype            = %d", __FUNCTION__, body->neighborlist[i]->devicetype);
                Logger.writeLog(LOG_DEBUG, "%s - permitjoining         = %d", __FUNCTION__, body->neighborlist[i]->permitjoining);
                Logger.writeLog(LOG_DEBUG, "%s - neighbortableentries  = %d", __FUNCTION__, body->neighbortableentries);
            }
            break;

        case NWK_STATUS_T__STATUS_FAILURE:
        case NWK_STATUS_T__STATUS_BUSY:
        case NWK_STATUS_T__STATUS_INVALID_PARAMETER:
        case NWK_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    nwk_get_neighbor_table_rsp_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Network ready indicator
 *------------------------------------------------------------------------------------------------*/

static void _IND_NetworkReady(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*         ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                cmdHdr  = (Cmd_t*)msg;
    NwkZigbeeNwkReadyInd* body    = NULL;

    body = nwk_zigbee_nwk_ready_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    ctrlHdr->sysHdr.channel  = body->nwkchannel;
    ctrlHdr->sysHdr.panId    = body->panid;
    ctrlHdr->sysHdr.extPanId = body->extpanid;
    ctrlHdr->sysHdr.status   = 1; // UP
    ctrlHdr->sysHdr.coordinator = body->extpanid;
    Logger.writeLog(LOG_DEBUG, "%s: %d", __FUNCTION__, ctrlHdr->sysHdr.channel);
    Logger.writeLog(LOG_DEBUG, "%s: 0x%x", __FUNCTION__, ctrlHdr->sysHdr.panId);
    Logger.writeLog(LOG_DEBUG, "%s: 0x%llX", __FUNCTION__, ctrlHdr->sysHdr.extPanId);

    nwk_zigbee_nwk_ready_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Permit Join request/response
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_PermitJoin(uint8_t interval)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*              header = Cmder.create(MAJOR, NULL);
    NwkSetPermitJoinReq body   = NWK_SET_PERMIT_JOIN_REQ__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_SET_PERMIT_JOIN_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init body
    body.permitjoin = NWK_PERMIT_JOIN_TYPE_T__PERMIT_NETWORK;
    body.permitjointime = interval;
    header->msg = malloc(sizeof(PacketHeader) + nwk_set_permit_join_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    nwk_set_permit_join_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = nwk_set_permit_join_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_SET_PERMIT_JOIN_REQ;

    return header;
}


/**-------------------------------------------------------------------------------------------------
 * Remove device request
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_RemoveEndpoint(EpAddr_t epAddr)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*             header  = Cmder.create(MAJOR, NULL);
    NwkRemoveDeviceReq body    = NWK_REMOVE_DEVICE_REQ__INIT;
    NwkAddressStructT  dstAddr = NWK_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_REMOVE_DEVICE_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init body
    dstAddr.addresstype    = NWK_ADDRESS_TYPE_T__UNICAST;
    dstAddr.has_ieeeaddr   = true;
    dstAddr.ieeeaddr       = epAddr.ieee_addr;
    dstAddr.has_endpointid = true;
    dstAddr.endpointid     = epAddr.endpoint;
    body.dstaddr           = &dstAddr;
    body.leavemode         = NWK_LEAVE_MODE_T__LEAVE;

    header->msg = malloc(sizeof(PacketHeader) + nwk_remove_device_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    nwk_remove_device_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = nwk_remove_device_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_REMOVE_DEVICE_REQ;

    return header;
}


/**-------------------------------------------------------------------------------------------------
 * Binding request/response
 *------------------------------------------------------------------------------------------------*/

typedef enum _ZdpStatus
{
    NWK_ZDP_STATUS__SUCCESS            = 0,
    NWK_ZDP_STATUS__INVALID_REQTYPE    = 128,
    NWK_ZDP_STATUS__DEVICE_NOT_FOUND   = 129,
    NWK_ZDP_STATUS__INVALID_EP         = 130,
    NWK_ZDP_STATUS__NOT_ACTIVE         = 131,
    NWK_ZDP_STATUS__NOT_SUPPORTED      = 132,
    NWK_ZDP_STATUS__TIMEOUT            = 133,
    NWK_ZDP_STATUS__NO_MATCH           = 134,
    NWK_ZDP_STATUS__NO_ENTRY           = 136,
    NWK_ZDP_STATUS__NO_DESCRIPTOR      = 137,
    NWK_ZDP_STATUS__INSUFFICIENT_SPACE = 138,
    NWK_ZDP_STATUS__NOT_PERMITTED      = 139,
    NWK_ZDP_STATUS__TABLE_FULL         = 140,
    NWK_ZDP_STATUS__NOT_AUTHORIZED     = 141,
    NWK_ZDP_STATUS__BINDING_TABLE_FULL = 142
} _ZdpStatus;


static Cmd_t* _REQ_BindEntry(EpAddr_t srcEp, EpAddr_t dstEp, uint16_t cid, bool mode)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                header  = Cmder.create(MAJOR, NULL);
    NwkSetBindingEntryReq body    = NWK_SET_BINDING_ENTRY_REQ__INIT;
    NwkAddressStructT     srcAddr = NWK_ADDRESS_STRUCT_T__INIT;
    NwkAddressStructT     dstAddr = NWK_ADDRESS_STRUCT_T__INIT;

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_SET_BINDING_ENTRY_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init body
    srcAddr.addresstype    = NWK_ADDRESS_TYPE_T__UNICAST;
    srcAddr.has_ieeeaddr   = true;
    srcAddr.ieeeaddr       = (uint64_t)srcEp.ieee_addr;
    srcAddr.has_endpointid = true;
    srcAddr.endpointid     = (uint32_t)srcEp.endpoint;

    dstAddr.addresstype    = NWK_ADDRESS_TYPE_T__UNICAST;
    dstAddr.has_ieeeaddr   = true;
    dstAddr.ieeeaddr       = (uint64_t)dstEp.ieee_addr;
    dstAddr.has_endpointid = true;
    dstAddr.endpointid     = (uint32_t)dstEp.endpoint;

    Logger.writeLog(LOG_DEBUG, "%s - Binding to dstAddr.endpointid = %d\n", __FUNCTION__, dstAddr.endpointid);

    body.srcaddr     = &srcAddr;
    body.dstaddr     = &dstAddr;
    body.bindingmode = (mode ? NWK_BINDING_MODE_T__UNBIND : NWK_BINDING_MODE_T__BIND);  // 1: Unbind, 0: Bind
    body.clusterid   = (uint32_t)cid;

    header->msg = malloc(sizeof(PacketHeader) + nwk_set_binding_entry_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }
    nwk_set_binding_entry_req__pack(&body, header->msg->body);

    // Init header
    header->msg->header.len    = nwk_set_binding_entry_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_SET_BINDING_ENTRY_REQ;

    return header;
}


static void _IND_BindEntry(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*             ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*                    cmdHdr  = (Cmd_t*)msg;
    NwkSetBindingEntryRspInd* body    = NULL;

    body = nwk_set_binding_entry_rsp_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    NwkAddressStructT* srcAddr = body->srcaddr;
    switch (body->status)
    {
        case NWK_ZDP_STATUS__SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - Binding success", __FUNCTION__);
            Logger.writeLog(LOG_DEBUG, "%s - _IND_BindEntry - [mac,eid] [0x%llX, %d]", __FUNCTION__, srcAddr->ieeeaddr, srcAddr->endpointid);
            break;

        case NWK_ZDP_STATUS__TABLE_FULL:
            Logger.writeLog(LOG_DEBUG, "%s - Binding success", __FUNCTION__);
            break;

        case NWK_ZDP_STATUS__INVALID_REQTYPE:
        case NWK_ZDP_STATUS__DEVICE_NOT_FOUND:
        case NWK_ZDP_STATUS__INVALID_EP:
        case NWK_ZDP_STATUS__NOT_ACTIVE:
        case NWK_ZDP_STATUS__NOT_SUPPORTED:
        case NWK_ZDP_STATUS__TIMEOUT:
        case NWK_ZDP_STATUS__NO_MATCH:
        case NWK_ZDP_STATUS__NO_ENTRY:
        case NWK_ZDP_STATUS__NO_DESCRIPTOR:
        case NWK_ZDP_STATUS__INSUFFICIENT_SPACE:
        case NWK_ZDP_STATUS__NOT_PERMITTED:
        case NWK_ZDP_STATUS__NOT_AUTHORIZED:
        case NWK_ZDP_STATUS__BINDING_TABLE_FULL:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    nwk_set_binding_entry_rsp_ind__free_unpacked(body, NULL);
}




/**-------------------------------------------------------------------------------------------------
 * Indicator device join/leave network
 *------------------------------------------------------------------------------------------------*/

static void _IND_Device(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*       ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*              cmdHdr  = (Cmd_t*)msg;
    NwkZigbeeDeviceInd* body    = NULL;

    body = nwk_zigbee_device_ind__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    NwkDeviceInfoT* device = body->deviceinfo;
    switch (body->deviceinfo->devicestatus)
    {
        case NWK_DEVICE_STATUS_T__DEVICE_ON_LINE:
            Logger.writeLog(LOG_DEBUG, "%s - DEVICE_ON_LINE", __FUNCTION__);
            _CreateItems(header, device, false);
            break;

        case NWK_DEVICE_STATUS_T__DEVICE_OFF_LINE:
            Logger.writeLog(LOG_DEBUG, "%s - DEVICE_OFF_LINE", __FUNCTION__);
            break;

        case NWK_DEVICE_STATUS_T__DEVICE_REMOVED:
            Logger.writeLog(LOG_DEBUG, "%s - DEVICE_REMOVED", __FUNCTION__);
            _removeItems(header, device);
            break;

        case NWK_DEVICE_STATUS_T__DEVICE_NA:
            Logger.writeLog(LOG_DEBUG, "%s - DEVICE_NA", __FUNCTION__);
            break;

        default:
            break;
    }

    nwk_zigbee_device_ind__free_unpacked(body, NULL);
}


/**-------------------------------------------------------------------------------------------------
 * Reset
 * Soft reset: This will reset all the Linux Gateways. The ZNP device and all devices that were
 * previously on the network will remain connected to the network.
 * Hard reset: This will reset all the Linux Gateways, the ZNP device, clear all devices that were
 * previously on the network
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _REQ_Reset(bool mode)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t*                  header = Cmder.create(MAJOR, NULL);
    NwkZigbeeSystemResetReq body   = NWK_ZIGBEE_SYSTEM_RESET_REQ__INIT;

    body.mode = mode ? NWK_RESET_MODE_T__SOFT_RESET : NWK_RESET_MODE_T__HARD_RESET;

    header->msg = malloc(sizeof(PacketHeader) + nwk_zigbee_system_reset_req__get_packed_size(&body));
    if (header->msg == NULL)
    {
        Logger.writeLog(LOG_DEBUG, "Net: %s - Could not pack msg", __FUNCTION__);
        return NULL;
    }

    // Init Key
    Kid_t cmdKid = {.cmd = {eDomainNet, eModeSync, NWK_MGR_CMD_ID_T__NWK_ZIGBEE_SYSTEM_RESET_REQ}};
    header->key = UniqueKey.type("cmd").render(cmdKid);

    // Init header
    header->msg->header.len    = nwk_zigbee_system_reset_req__get_packed_size(&body);
    header->msg->header.subSys = Z_STACK_NWK_MGR_SYS_ID_T__RPC_SYS_PB_NWK_MGR;
    header->msg->header.cmdId  = NWK_MGR_CMD_ID_T__NWK_ZIGBEE_SYSTEM_RESET_REQ;

    // Init body
    nwk_zigbee_system_reset_req__pack(&body, header->msg->body);

    return header;
}

static void _RSP_Reset(void* header, void* msg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t*        ctrlHdr = (ControlHdr_t*)header;
    Cmd_t*               cmdHdr  = (Cmd_t*)msg;
    NwkZigbeeGenericCnf* body    = NULL;

    body = nwk_zigbee_generic_cnf__unpack(NULL, cmdHdr->msg->header.len, cmdHdr->msg->body);
    if (!body) return;

    switch (body->status)
    {
        case NWK_STATUS_T__STATUS_SUCCESS:
            Logger.writeLog(LOG_DEBUG, "%s - Success", __FUNCTION__);
            break;

        case NWK_STATUS_T__STATUS_FAILURE:
        case NWK_STATUS_T__STATUS_BUSY:
        case NWK_STATUS_T__STATUS_INVALID_PARAMETER:
        case NWK_STATUS_T__STATUS_TIMEOUT:
            Logger.writeLog(LOG_DEBUG, "%s - Failed (%d)", __FUNCTION__, body->status);
            break;

        default:
            break;
    }

    if (body == NULL)
        Logger.writeLog(LOG_DEBUG, "%s - body NULL", __FUNCTION__);

    nwk_zigbee_generic_cnf__free_unpacked(body, NULL);
}



/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

_Network const Network = {
    _REQ_PermitJoin,
    _REQ_RemoveEndpoint,
    _REQ_CoordInfo,
    _REQ_DevList,
    _REQ_NetworkInfo,
    _REQ_EndpointInfo,
    _REQ_NetworkKey,
    _REQ_LinkQuality,
    _REQ_BindEntry,
    _REQ_Reset,

    _RSP_Generic,
    _RSP_CoordInfo,
    _RSP_DevList,
    _RSP_NetworkInfo,
    _RSP_EndpointInfo,
    _RSP_NetworkKey,
    _RSP_Reset,

    _IND_NetworkReady,
    _IND_LinkQuality,
    _IND_BindEntry,
    _IND_Device
};