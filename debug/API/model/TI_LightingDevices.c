#include "Logger.h"
#include "TiDevice.h"
#include "Identifier.h"
#include "Gateway.h"
#include "TI_LightingDevices.h"

// Forward declaration
typedef struct TiDevices_t TiDevices_t;

/**-------------------------------------------------------------------------------------------------
 * LIGHTING DEVICES
 * edid_onOffLight
 * edid_dimmableLight
 * edid_coloredDimmableLight
 * edid_onOffLightSwitch
 * edid_dimmerSwitch
 * edid_colorDimmerSwitch
 * edid_lightSensor
 * edid_occupancySensor
 *------------------------------------------------------------------------------------------------*/



/**-------------------------------------------------------------------------------------------------
 * edid_onOffLight
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_OnOffLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_OnOffLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_OnOffLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_OnOffLight(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static void _TI_TRACK_OnOffLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_OnOffLight(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_OnOffLight;
    item->ops.exit      = _TI_EXIT_OnOffLight;
    item->ops.discovery = _TI_DISCOV_OnOffLight;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_OnOffLight;
    item->ops.onTrack   = _TI_TRACK_OnOffLight;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_dimmableLight
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_DimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_DimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_DimmableLight(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    EpAddr_t epAddr = {self->specs.mac, self->specs.eid, 0};

    ReadRec readRec;
    readRec.len    = 1;
    readRec.aid[0] = 0x0000;
    Cmd_t* cmd = Gateway.foundation.REQ_Read(epAddr, ecid_genOnOff, readRec);
    // Controller.request(header->apiHdr, cmd);
    self->ops.request(header, self, cmd);

}

static void _TI_EVENT_DimmableLight(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_DimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_DimmableLight(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_DimmableLight;
    item->ops.exit      = _TI_EXIT_DimmableLight;
    item->ops.discovery = _TI_DISCOV_DimmableLight;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_DimmableLight;
    item->ops.onTrack   = _TI_TRACK_DimmableLight;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_coloredDimmableLight
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_ColoredDimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_ColoredDimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_ColoredDimmableLight(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_ColoredDimmableLight(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_ColoredDimmableLight(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
    EpAddr_t epAddr;
    epAddr.ieee_addr = self->specs.mac;
    epAddr.endpoint = self->specs.eid;

    ReadRec readRec;
    readRec.len    = 1;
    readRec.aid[0] = 0x0000;
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_genBasic, readRec));
}

static TiDevice_t* _TI_RENDER_ColoredDimmableLight(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_ColoredDimmableLight;
    item->ops.exit      = _TI_EXIT_ColoredDimmableLight;
    item->ops.discovery = _TI_DISCOV_ColoredDimmableLight;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_ColoredDimmableLight;
    item->ops.onTrack   = _TI_TRACK_ColoredDimmableLight;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_onOffLightSwitch
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_OnOffLightSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_OnOffLightSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_OnOffLightSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_OnOffLightSwitch(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_OnOffLightSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_OnOffLightSwitch(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_OnOffLightSwitch;
    item->ops.exit      = _TI_EXIT_OnOffLightSwitch;
    item->ops.discovery = _TI_DISCOV_OnOffLightSwitch;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_OnOffLightSwitch;
    item->ops.onTrack   = _TI_TRACK_OnOffLightSwitch;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_dimmerSwitch
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_DimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_DimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_DimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_DimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_DimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_DimmerSwitch(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_DimmerSwitch;
    item->ops.exit      = _TI_EXIT_DimmerSwitch;
    item->ops.discovery = _TI_DISCOV_DimmerSwitch;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_DimmerSwitch;
    item->ops.onTrack   = _TI_TRACK_DimmerSwitch;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_colorDimmerSwitch
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_ColorDimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_ColorDimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_ColorDimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_ColorDimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_ColorDimmerSwitch(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_ColorDimmerSwitch(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_ColorDimmerSwitch;
    item->ops.exit      = _TI_EXIT_ColorDimmerSwitch;
    item->ops.discovery = _TI_DISCOV_ColorDimmerSwitch;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_ColorDimmerSwitch;
    item->ops.onTrack   = _TI_TRACK_ColorDimmerSwitch;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_lightSensor
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_LightSensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_LightSensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_LightSensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_LightSensor(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_LightSensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_LightSensor(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_LightSensor;
    item->ops.exit      = _TI_EXIT_LightSensor;
    item->ops.discovery = _TI_DISCOV_LightSensor;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_LightSensor;
    item->ops.onTrack   = _TI_TRACK_LightSensor;

    return item;
}


/**-------------------------------------------------------------------------------------------------
 * edid_occupancySensor
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_OccupancySensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EXIT_OccupancySensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_DISCOV_OccupancySensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_OccupancySensor(TiSchemaHdr_t* header, TiDevice_t* item, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_OccupancySensor(TiSchemaHdr_t* header, TiDevice_t* item)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_OccupancySensor(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* item = calloc(1, sizeof(TiDevice_t));

    item->specs = TiCommon.renderSpecs(header, device, endpoint);

    item->ops.init      = _TI_INIT_OccupancySensor;
    item->ops.exit      = _TI_EXIT_OccupancySensor;
    item->ops.discovery = _TI_DISCOV_OccupancySensor;
    item->ops.getState  = TiCommon.getState;
    item->ops.setState  = TiCommon.setState;
    item->ops.request   = TiCommon.request;
    item->ops.onEvent   = _TI_EVENT_OccupancySensor;
    item->ops.onTrack   = _TI_TRACK_OccupancySensor;

    return item;
}

// HomeAutomation
TiSchema_t TI_HA_OnOffLight           = { .pid = epid_ha  ,.did = edid_ha_onOffLight            ,.ops = { _TI_RENDER_OnOffLight} };
TiSchema_t TI_HA_DimmableLight        = { .pid = epid_ha  ,.did = edid_ha_dimmableLight         ,.ops = { _TI_RENDER_DimmableLight} };
TiSchema_t TI_HA_ColoredDimmableLight = { .pid = epid_ha  ,.did = edid_ha_coloredDimmableLight  ,.ops = { _TI_RENDER_ColoredDimmableLight} };
TiSchema_t TI_HA_OnOffLightSwitch     = { .pid = epid_ha  ,.did = edid_ha_onOffLightSwitch      ,.ops = { _TI_RENDER_OnOffLightSwitch} };
TiSchema_t TI_HA_DimmerSwitch         = { .pid = epid_ha  ,.did = edid_ha_dimmerSwitch          ,.ops = { _TI_RENDER_DimmerSwitch} };
TiSchema_t TI_HA_ColorDimmerSwitch    = { .pid = epid_ha  ,.did = edid_ha_colorDimmerSwitch     ,.ops = { _TI_RENDER_ColorDimmerSwitch} };
TiSchema_t TI_HA_LightSensor          = { .pid = epid_ha  ,.did = edid_ha_lightSensor           ,.ops = { _TI_RENDER_LightSensor} };
TiSchema_t TI_HA_OccupancySensor      = { .pid = epid_ha  ,.did = edid_ha_occupancySensor       ,.ops = { _TI_RENDER_OccupancySensor} };

// Lightlink
TiSchema_t TI_LL_DimmableLight        = { .pid = epid_ll  ,.did = edid_ll_dimmableLight         ,.ops = { _TI_RENDER_DimmableLight} };



