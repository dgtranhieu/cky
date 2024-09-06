#include "Identifier.h"
#include "Network.h"
#include "Gateway.h"
#include "Logger.h"
#include "nwkmgr.pb-c.h"
#include "TiDevice.h"


static void _ti_init_bind_to(TiSchemaHdr_t* header, TiDevice_t* self, uint8_t eid, uint16_t cid, bool mode)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    EpAddr_t coordAddr;

    Logger.writeLog(LOG_DEBUG, "%s - coordAddr = 0x%llX", __FUNCTION__, header->apiHdr->sysHdr.coordinator);
    coordAddr.ieee_addr = header->apiHdr->sysHdr.coordinator;
    coordAddr.endpoint = eid;

    EpAddr_t endpAddr;
    endpAddr.ieee_addr = self->specs.mac;
    endpAddr.endpoint  = self->specs.eid;

    Cmd_t* cmd = Network.REQ_BindEntry(endpAddr, coordAddr, cid, mode);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);
}

static void _ti_init_battery(TiSchemaHdr_t* header, TiDevice_t* self, int mode)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    if (self == NULL) return;
    Cmd_t* cmd = NULL;
    ReadRec readRec;
    EpAddr_t epAddr = {self->specs.mac, self->specs.eid, 0};

    switch (mode)
    {
        case 0: // BatteryVoltage
            readRec.len    = 1;
            readRec.aid[0] = 0x0020;
            break;

        case 1: // BatteryPercentageRemaining
            readRec.len    = 1;
            readRec.aid[0] = 0x0021;
            break;

        case 2: // All
            readRec.len    = 2;
            readRec.aid[0] = 0x0020;
            readRec.aid[1] = 0x0021;
            break;

        default:
            break;
    }

    cmd = Gateway.foundation.REQ_Read(epAddr, ecid_genPowerCfg, readRec);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);
}

static void _ti_event_genOnOff(TiSchemaHdr_t* header, TiDevice_t* item, uint16_t cmdId)
{
    // Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    int index = 0;

    Attr_t record;
    record.cid = ecid_genOnOff;
    record.aid = 0x0000;
    record.tid = etid_uint8;
    record.len = 1;
    record.val[0] = 0x00;

    switch ((uint8_t)cmdId)
    {
        case 0x00: // Off
            Logger.writeLog(LOG_DEBUG, "%s - OFF", __FUNCTION__);
            record.val[0] = 0x00;
            break;

        case 0x01: // On
            Logger.writeLog(LOG_DEBUG, "%s - ON", __FUNCTION__);
            record.val[0] = 0x01;
            break;

        case 0x02: // Toogle
            Logger.writeLog(LOG_DEBUG, "%s - Toogle", __FUNCTION__);
            break;

        default:
            Logger.writeLog(LOG_DEBUG, "%s - NOt Supported", __FUNCTION__);
            break;
    }

    item->ops.setState(header, item, TI_STATE_BIND, record);
}

static void _ti_event_genLevelCtrl(TiSchemaHdr_t* header, TiDevice_t* item, uint16_t cmdId)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

TiDevSpecs_t _ti_render_specs(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    TiDevSpecs_t specs;
    NwkDeviceInfoT*       devicePtr   = (NwkDeviceInfoT*)device;
    NwkSimpleDescriptorT* endpointPtr = (NwkSimpleDescriptorT*)endpoint;

    specs.type = (devicePtr->networkaddress == 0x0000) ? "coord" : "router";
    specs.mac = devicePtr->ieeeaddress;
    specs.nwk = devicePtr->networkaddress;
    specs.mid = devicePtr->manufacturerid;

    specs.pid = endpointPtr->profileid;
    specs.did = endpointPtr->deviceid;
    specs.eid = endpointPtr->endpointid;

    Logger.writeLog(LOG_DEBUG, "%s: dev = [type,pid,did,eid,mac,mid] = [%s, %d, %d, %d, %llX, 0x%x]", __FUNCTION__, specs.type, specs.pid, specs.did, specs.eid, specs.mac, specs.mid);

    for (int i = 0; i < endpointPtr->n_inputclusters; i++)
    {
        int index = specs.clusters.len;
        specs.clusters.cluster[index].type = ICLUSTER;
        specs.clusters.cluster[index].val = endpointPtr->inputclusters[i];
        specs.clusters.len++;
    }

    for (int i = 0; i < endpointPtr->n_outputclusters; i++)
    {
        int index = specs.clusters.len;
        specs.clusters.cluster[index].type = OCLUSTER;
        specs.clusters.cluster[index].val = endpointPtr->inputclusters[i];
        specs.clusters.len++;
    }

    for (int i = 0; i < specs.clusters.len; i++)
    {
        if (specs.clusters.cluster[i].type == ICLUSTER)
            Logger.writeLog(LOG_DEBUG, "   icluster: 0x%x", specs.clusters.cluster[i].val);
    }

    for (int i = 0; i < specs.clusters.len; i++)
    {
        if (specs.clusters.cluster[i].type == OCLUSTER)
            Logger.writeLog(LOG_DEBUG, "   ocluster: 0x%x", specs.clusters.cluster[i].val);
    }

    return specs;
}

static Attr_t* _TI_STATE_Get(TiSchemaHdr_t* header, TiDevice_t* item, TiStateType_t type, uint16_t cid, uint16_t aid)
{
    // Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    int index = 0;
    bool found = false;

    AttrList_t* attrList = (type == TI_STATE_ATTR) ? &item->state.attrs : &item->state.bindAttrs;

    for (int i = 0; i < attrList->len; i++)
    {
        if (attrList->attr[i].cid == cid && attrList->attr[i].aid == aid)
        {
            found = true;
            index = i;
            break;
        }
    }

    return (found) ? &attrList->attr[index] : NULL;
}

static void _TI_STATE_Set(TiSchemaHdr_t* header, TiDevice_t* item, TiStateType_t type, Attr_t attr)
{
    Logger.writeLog(LOG_DEBUG, "%s - type = %s", __FUNCTION__, type == 0 ? "TI_STATE_ATTR" : "TI_STATE_BIND");
    int index = 0;
    bool found = false;

    AttrList_t* attrList = (type == TI_STATE_ATTR) ? &item->state.attrs : &item->state.bindAttrs;
    for (int i = 0; i < attrList->len; i++)
    {
        if (attrList->attr[i].cid == attr.cid && attrList->attr[i].aid == attr.aid)
        {
            // Logger.writeLog(LOG_DEBUG, "\t%s - Found", __FUNCTION__);
            found = true;
            index = i;
            break;
        }
    }

    index = (found) ? index : attrList->len;
    if (!found) attrList->len++;

    attrList->attr[index].time = time(NULL);
    attrList->attr[index].cid = attr.cid;
    attrList->attr[index].aid = attr.aid;
    attrList->attr[index].tid = attr.tid;
    attrList->attr[index].len = attr.len;
    memcpy(attrList->attr[index].val, attr.val, attr.len);

    // for (int i = 0; i < attr.len; i++)
    //     Logger.writeLog(LOG_DEBUG, "\t%s - [val - new] = (%d, %d)", __FUNCTION__, attrList->attr[index].val[i], attr.val[i]);

    Kid_t itemKid = {.item = {.mac = item->specs.mac, .eid = item->specs.eid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);

    if (header->apiHdr->tiNotifyCB)
    {
        TiNotifyCode_t notifCode = (type == TI_STATE_ATTR) ? TI_EVN_ATTR_CHANGED : TI_EVN_BIND_CHANGED;
        header->apiHdr->tiNotifyCB(notifCode, itemKey);
    }
}

static void _TI_CMD_Request(TiSchemaHdr_t* header, TiDevice_t* self, Cmd_t* cmd)
{
    if (cmd == NULL || self == NULL)
        return;

    EpAddr_t epAddr = {self->specs.mac, self->specs.eid, 0};
    Controller.request(header->apiHdr, cmd);
}


/**-------------------------------------------------------------------------------------------------
 * Exports
 *------------------------------------------------------------------------------------------------*/

// TiSchema_t Ti_OnOffSwitch          = { .pid = epid_ha  ,.did = edid_ha_onOffSwitch          ,.ops = { _TI_RENDER_OnOffSwitch} };
// TiSchema_t Ti_LevelControlSwitch   = { .pid = epid_ha  ,.did = edid_ha_levelControlSwitch   ,.ops = { _TI_RENDER_LevelControlSwitch} };
// TiSchema_t Ti_IasZone              = { .pid = epid_ha  ,.did = edid_ha_iasZone              ,.ops = { _TI_RENDER_IasZone} };
// TiSchema_t Ti_ColoredDimmableLight = { .pid = epid_ha  ,.did = edid_ha_coloredDimmableLight ,.ops = { _TI_RENDER_ColoredDimmableLight} };
// TiSchema_t Ti_MainsPowerOutlet     = { .pid = epid_ha  ,.did = edid_ha_coloredDimmableLight ,.ops = { _TI_RENDER_MainsPowerOutlet} };



TiCommon_t const TiCommon = {
    _ti_render_specs,
    _TI_STATE_Get,
    _TI_STATE_Set,
    _TI_CMD_Request,
    _ti_init_bind_to,
    _ti_init_battery,
    _ti_event_genOnOff,
    _ti_event_genLevelCtrl
};
