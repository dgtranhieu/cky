#include "nwkmgr.pb-c.h"
#include "Logger.h"
#include "TiDevice.h"
#include "TI_GenericDevices.h"
#include "TI_LightingDevices.h"
#include "TI_IasDevices.h"
#include "TI_HvacDevices.h"
#include "TI_UnclassifiedDevices.h"
#include "TiSchema.h"

static void _register(TiSchemaHdr_t* header, TiSchema_t schema)
{
    int index = header->schema->len;
    header->schema[index].specs = schema;
    header->schema->len++;
}


static TiSchemaHdr_t* _instance(ControlHdr_t* apiHdr)
{
    TiSchemaHdr_t* header = calloc(1, sizeof(TiSchemaHdr_t));
    header->apiHdr = apiHdr;

    memset(header->schema, 0, sizeof(128 * sizeof(TiSchemaElem_t)));

    // Generic devices
    _register(header, TI_HA_OnOffSwitch);
    _register(header, TI_HA_LevelControlSwitch);
    _register(header, TI_HA_OnOffOutput);
    _register(header, TI_HA_LevelControllableOutput);
    _register(header, TI_HA_SceneSelector);
    _register(header, TI_HA_ConfigurationTool);
    _register(header, TI_HA_RemoteControl);
    _register(header, TI_HA_CombinedInterface);
    _register(header, TI_HA_RangeExtender);
    _register(header, TI_HA_MainsPowerOutlet);
    _register(header, TI_HA_DoorLock);
    _register(header, TI_HA_DoorLockController);
    _register(header, TI_HA_SimpleSensor);
    _register(header, TI_HA_ConsumptionAwarenessDevice);
    _register(header, TI_HA_HomeGateway);
    _register(header, TI_HA_SmartPlug);
    _register(header, TI_HA_WhiteGoods);
    _register(header, TI_HA_MeterInterface);

    // Lighting devices
    _register(header, TI_HA_OnOffLight);
    _register(header, TI_HA_DimmableLight);
    _register(header, TI_HA_ColoredDimmableLight);
    _register(header, TI_HA_OnOffLightSwitch);
    _register(header, TI_HA_DimmerSwitch);
    _register(header, TI_HA_ColorDimmerSwitch);
    _register(header, TI_HA_LightSensor);
    _register(header, TI_HA_OccupancySensor);

    // HVAC
    _register(header, TI_HA_TemperatureSensor);

    // IAS devices
    _register(header, TI_HA_IasZone);
    _register(header, TI_HA_IasWarningDevice);

    // LightLink
    _register(header, TI_LL_DimmableLight);

    // Unclassified devices
    _register(header, TI_HA_AqaraDoorSensor);

    return header;
}


static void _destroy(TiSchemaHdr_t* header)
{
    free(header);
}


static TiDevice_t* _render(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    NwkSimpleDescriptorT* endpointPtr = (NwkSimpleDescriptorT*)endpoint;

    for (int i = 0; i < header->schema->len; i++)
    {
        if (header->schema[i].specs.pid == endpointPtr->profileid && header->schema[i].specs.did == endpointPtr->deviceid)
            return header->schema[i].specs.ops.render(header, device, endpoint);
    }

    return NULL;
}


TiFactory_t TiFactory = {
    _instance,
    _destroy,
    _render
};