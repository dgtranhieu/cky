#include "Logger.h"
#include "Identifier.h"
#include "Gateway.h"
#include "TI_HvacDevices.h"

/**-------------------------------------------------------------------------------------------------
 * HVAC DEVICES
 * edid_ha_heatingCoolingUnit
 * edid_ha_thermostat
 * edid_ha_temperatureSensor
 * edid_ha_pump
 * edid_ha_pumpController
 * edid_ha_pressureSensor
 * edid_ha_flowSensor
 *------------------------------------------------------------------------------------------------*/


/**-------------------------------------------------------------------------------------------------
 * edid_ha_temperatureSensor
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_TemperatureSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 1, ecid_msTemperatureMeasurement, false);
    TiCommon.bindState(header, self, 1, ecid_msRelativeHumidity, false);
    TiCommon.bindState(header, self, 1, ecid_msPressureMeasurement, false);
}

static void _TI_EXIT_TemperatureSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 1, ecid_msTemperatureMeasurement, true);
    TiCommon.bindState(header, self, 1, ecid_msRelativeHumidity, true);
    TiCommon.bindState(header, self, 1, ecid_msPressureMeasurement, true);
}

static void _TI_DISCOV_TemperatureSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_EVENT_TemperatureSensor(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

static void _TI_TRACK_TemperatureSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

TiDevice_t* _TI_RENDER_TemperatureSensor(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_TemperatureSensor;
    tiDevice->ops.exit      = _TI_EXIT_TemperatureSensor;
    tiDevice->ops.discovery = _TI_DISCOV_TemperatureSensor;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_TemperatureSensor;
    tiDevice->ops.onTrack   = _TI_TRACK_TemperatureSensor;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * Exports
 *------------------------------------------------------------------------------------------------*/
TiSchema_t TI_HA_TemperatureSensor               = { .pid = epid_ha  ,.did = edid_ha_temperatureSensor                ,.ops = { _TI_RENDER_TemperatureSensor} };
