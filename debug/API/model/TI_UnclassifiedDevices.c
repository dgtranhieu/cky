
#include "Logger.h"
#include "Identifier.h"
#include "Gateway.h"
#include "TI_UnclassifiedDevices.h"


/**-------------------------------------------------------------------------------------------------
 * These devices do not follow home automation specification strictly
 * Xiaomi door sensor. deviceID = 0x5F01
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_AqaraDoorSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 1, ecid_genOnOff, false);

    EpAddr_t epAddr = {self->specs.mac, self->specs.eid, 0};

    CfgRptRec cfgRptRec;
    cfgRptRec.len                     = 1;
    cfgRptRec.rptRec[0].attribute.aid = 0x0000;
    cfgRptRec.rptRec[0].attribute.tid = etid_boolean;
    cfgRptRec.rptRec[0].minRepIntval  = 5;
    cfgRptRec.rptRec[0].maxRepIntval  = 5;
    cfgRptRec.rptRec[0].repChange     = 1;
    self->ops.request(header, self, Gateway.foundation.REQ_ConfigReport(epAddr, ecid_genOnOff, cfgRptRec));
}


static void _TI_EXIT_AqaraDoorSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 1, ecid_genOnOff, true); // Unbind
}


static void _TI_DISCOV_AqaraDoorSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    EpAddr_t epAddr = {self->specs.mac, self->specs.eid, 0};
    Logger.writeLog(LOG_DEBUG, "%s - [mac,eid] = (0x%llX, %d) - eid = %d", __FUNCTION__, epAddr.ieee_addr, epAddr.endpoint, self->specs.eid);

    ReadRec readRec;
    readRec.len    = 1;
    readRec.aid[0] = 0x0005;
    Cmd_t* cmd = Gateway.foundation.REQ_Read(epAddr, ecid_genBasic, readRec);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);
}


static void _TI_EVENT_AqaraDoorSensor(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_AqaraDoorSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


TiDevice_t* _TI_RENDER_AqaraDoorSensor(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_AqaraDoorSensor;
    tiDevice->ops.exit      = _TI_EXIT_AqaraDoorSensor;
    tiDevice->ops.discovery = _TI_DISCOV_AqaraDoorSensor;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_AqaraDoorSensor;
    tiDevice->ops.onTrack   = _TI_TRACK_AqaraDoorSensor;

    return tiDevice;
}




/**-------------------------------------------------------------------------------------------------
 * Exports
 *------------------------------------------------------------------------------------------------*/

TiSchema_t TI_HA_AqaraDoorSensor                = { .pid = epid_ha  ,.did = edid_ha_aqaraDoorSensor                ,.ops = { _TI_RENDER_AqaraDoorSensor} };

