#include "Logger.h"
#include "SafeHash.h"
#include "Gateway.h"
#include "Identifier.h"
#include "TI_IasDevices.h"



/**-------------------------------------------------------------------------------------------------
 * INTRUDER ALARM SYSTEM DEVICES
 * edid_iasControlIndicatingEquipment
 * edid_iasAncillaryControlEquipment
 * edid_iasZone
 * edid_iasWarningDevice
 *------------------------------------------------------------------------------------------------*/

/**-------------------------------------------------------------------------------------------------
 * edid_iasZone
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_IasZone(TiSchemaHdr_t* header, TiDevice_t* self)
{
    // Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    EpAddr_t coordAddr;
    Cmd_t* cmd = NULL;

    // Coordinator address
    Kid_t coordKid = {.item = {header->apiHdr->sysHdr.coordinator, 4}};
    Key_t coordKey = UniqueKey.type("item").render(coordKid);
    TiDevice_t* coord = SafeHash.find(header->apiHdr->itemHdr, coordKey);

    // Endpoint address
    EpAddr_t epAddr;
    epAddr.ieee_addr = self->specs.mac;
    epAddr.endpoint  = self->specs.eid;

    // Step 1: Read zone type
    Logger.writeLog(LOG_DEBUG, "%s - Read zoneType", __FUNCTION__);
    cmd = Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0001}, {}});
    self->ops.request(header, self, cmd);
    // Controller.request(header->apiHdr, cmd);

    // Step 2: Write coord mac to IAS endpoint
    Logger.writeLog(LOG_DEBUG, "%s - Write CIE", __FUNCTION__);
    WriteRec writeRec;
    uint64_t mask = 0x00000000000000FF;
    writeRec.len = 1;
    writeRec.attr[0].tid = etid_ieeeAddr;
    writeRec.attr[0].aid = 0x0010;
    writeRec.attr[0].len = 8;
    Logger.writeLog(LOG_DEBUG, "%s:%d", __FUNCTION__);
    for (int i = 0; i < writeRec.attr[0].len; i++)
    {
        writeRec.attr[0].val[i] = (coordAddr.ieee_addr & mask) >> (8 * i);
        mask = (mask << 8);
    }
    cmd = Gateway.foundation.REQ_Write(epAddr, ecid_ssIasZone, writeRec);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);

    Logger.writeLog(LOG_DEBUG, "%s - Read CIE", __FUNCTION__);
    cmd = Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0010}, {}});
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);

    Logger.writeLog(LOG_DEBUG, "%s - Write RSP", __FUNCTION__);
    cmd = Gateway.ias.REQ_AutoEnrollRsp(epAddr, 0, 0);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);

    Logger.writeLog(LOG_DEBUG, "%s - Confirm", __FUNCTION__);
    cmd = Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0000}, {}});
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);
}


static void _TI_EXIT_IasZone(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_IasZone(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    Cmd_t* cmd = NULL;

    // Endpoint address
    EpAddr_t epAddr;
    epAddr.ieee_addr = self->specs.mac;
    epAddr.endpoint  = self->specs.eid;

    // Read zone type
    Logger.writeLog(LOG_DEBUG, "%s: Discov zoneType", __FUNCTION__);
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0001}, {}}));

    // Read zone status
    Logger.writeLog(LOG_DEBUG, "%s: Discov zoneStatus", __FUNCTION__);
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0002}, {}}));
}


static void _TI_EVENT_IasZone(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_IasZone(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
    EpAddr_t epAddr;
    epAddr.ieee_addr = self->specs.mac;
    epAddr.endpoint  = self->specs.eid;

    // Read zone type
    Logger.writeLog(LOG_DEBUG, "%s: Discov zoneType", __FUNCTION__);
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0001}, {}}));

    // Read zone status
    Logger.writeLog(LOG_DEBUG, "%s: Discov zoneStatus", __FUNCTION__);
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_ssIasZone, (ReadRec){1, {0x0002}, {}}));
}


static TiDevice_t* _TI_RENDER_IasZone(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_IasZone;
    tiDevice->ops.exit      = _TI_EXIT_IasZone;
    tiDevice->ops.discovery = _TI_DISCOV_IasZone;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_IasZone;
    tiDevice->ops.onTrack   = _TI_TRACK_IasZone;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_ha_iasWarningDevice
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_IasWarningDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_IasWarningDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_IasWarningDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_IasWarningDevice(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_IasWarningDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_IasWarningDevice(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_IasWarningDevice;
    tiDevice->ops.exit      = _TI_EXIT_IasWarningDevice;
    tiDevice->ops.discovery = _TI_DISCOV_IasWarningDevice;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_IasWarningDevice;
    tiDevice->ops.onTrack   = _TI_TRACK_IasWarningDevice;

    return tiDevice;
}






TiSchema_t TI_HA_IasZone              = { .pid = epid_ha  ,.did = edid_ha_iasZone              ,.ops = { _TI_RENDER_IasZone} };
TiSchema_t TI_HA_IasWarningDevice     = { .pid = epid_ha  ,.did = edid_ha_iasWarningDevice     ,.ops = { _TI_RENDER_IasWarningDevice} };
