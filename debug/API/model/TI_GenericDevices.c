
#include "Logger.h"
#include "Identifier.h"
#include "TiDevice.h"
#include "Gateway.h"
#include "TI_GenericDevices.h"

/**#################################################################################################
 # GENERIC DEVICES
 # edid_onOffSwitch
 # edid_levelControlSwitch
 # edid_onOffOutput
 # edid_levelControllableOutput
 # edid_sceneSelector
 # edid_configurationTool
 # edid_remoteControl
 # edid_combinedInterface
 # edid_rangeExtender
 # edid_mainsPowerOutlet
 # edid_doorLock
 # edid_doorLockController
 # edid_simpleSensor
 # edid_consumptionAwarenessDevice
 # edid_homeGateway
 # edid_smartPlug
 # edid_whiteGoods
 # edid_meterInterface
 #################################################################################################*/

/**-------------------------------------------------------------------------------------------------
 * edid_onOffSwitch
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_OnOffSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 4, ecid_genOnOff, false);
}


static void _TI_EXIT_OnOffSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 4, ecid_genOnOff, true); // Unbind
}


static void _TI_DISCOV_OnOffSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_OnOffSwitch(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    // Bind Only 0x0006
    if (zclMsg.cid != (uint32_t)ecid_genOnOff)
        return;

    TiCommon.evnGenOnOff(header, self, zclMsg.cmdId);

    Kid_t itemKid = {.item = {.mac = self->specs.mac, .eid = self->specs.eid}};
    Key_t itemKey = UniqueKey.type("item").render(itemKid);

    if (header->apiHdr->tiNotifyCB)
        header->apiHdr->tiNotifyCB(TI_EVN_BIND_CHANGED, itemKey);
}


static void _TI_TRACK_OnOffSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


TiDevice_t* _TI_RENDER_OnOffSwitch(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_OnOffSwitch;
    tiDevice->ops.exit      = _TI_EXIT_OnOffSwitch;
    tiDevice->ops.discovery = _TI_DISCOV_OnOffSwitch;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_OnOffSwitch;
    tiDevice->ops.onTrack   = _TI_TRACK_OnOffSwitch;

    return tiDevice;
}



/**-------------------------------------------------------------------------------------------------
 * edid_levelControlSwitch
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_LevelControlSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 4, ecid_genOnOff, false);
    TiCommon.bindState(header, self, 4, ecid_genLevelCtrl, false);

    // Get battery level
    TiCommon.initBattery(header, self, 0);
}


static void _TI_EXIT_LevelControlSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiCommon.bindState(header, self, 4, ecid_genOnOff, true);
    TiCommon.bindState(header, self, 4, ecid_genLevelCtrl, true);
}


static void _TI_DISCOV_LevelControlSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_LevelControlSwitch(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    // Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    switch ((uint16_t)zclMsg.cid)
    {
        case ecid_genOnOff:
            Logger.writeLog(LOG_DEBUG, "%s - ecid_genOnOff", __FUNCTION__);
            TiCommon.evnGenOnOff(header, self, zclMsg.cmdId);
            break;

        case ecid_genLevelCtrl:
            Logger.writeLog(LOG_DEBUG, "%s - ecid_genLevelCtrl", __FUNCTION__);
            TiCommon.evnGenLevelCtrl(header, self, zclMsg.cmdId);
            break;

        default:
            Logger.writeLog(LOG_DEBUG, "%s: Not supported", __FUNCTION__);
            break;
    }

    // Kid_t itemKid = {.item = {.mac = self->specs.mac, .eid = self->specs.eid}};
    // Key_t itemKey = UniqueKey.type("item").render(itemKid);

    // header->apiHdr->tiNotifyCB(TI_EVN_BIND_CHANGED, itemKey);
}


static void _TI_TRACK_LevelControlSwitch(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
    EpAddr_t epAddr;
    epAddr.ieee_addr = self->specs.mac;
    epAddr.endpoint = self->specs.eid;

    // Tracking Reachable/unreachable
    ReadRec readRec;
    readRec.len    = 1;
    readRec.aid[0] = 0x0000;
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_genBasic, readRec));

    // Tracking battery voltage
    readRec.len    = 1;
    readRec.aid[0] = 0x0020;
    self->ops.request(header, self, Gateway.foundation.REQ_Read(epAddr, ecid_genPowerCfg, readRec));
}


TiDevice_t* _TI_RENDER_LevelControlSwitch(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_LevelControlSwitch;
    tiDevice->ops.exit      = _TI_EXIT_LevelControlSwitch;
    tiDevice->ops.discovery = _TI_DISCOV_LevelControlSwitch;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_LevelControlSwitch;
    tiDevice->ops.onTrack   = _TI_TRACK_LevelControlSwitch;


    return tiDevice;
}



/**-------------------------------------------------------------------------------------------------
 * edid_onOffOutput
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_OnOffOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_OnOffOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_OnOffOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_OnOffOutput(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_OnOffOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_OnOffOutput(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_OnOffOutput;
    tiDevice->ops.exit      = _TI_EXIT_OnOffOutput;
    tiDevice->ops.discovery = _TI_DISCOV_OnOffOutput;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_OnOffOutput;
    tiDevice->ops.onTrack   = _TI_TRACK_OnOffOutput;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_levelControllableOutput
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_LevelControllableOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_LevelControllableOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_LevelControllableOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_LevelControllableOutput(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_LevelControllableOutput(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_LevelControllableOutput(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_LevelControllableOutput;
    tiDevice->ops.exit      = _TI_EXIT_LevelControllableOutput;
    tiDevice->ops.discovery = _TI_DISCOV_LevelControllableOutput;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_LevelControllableOutput;
    tiDevice->ops.onTrack   = _TI_TRACK_LevelControllableOutput;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_sceneSelector
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_SceneSelector(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_SceneSelector(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_SceneSelector(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_SceneSelector(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_SceneSelector(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_SceneSelector(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_SceneSelector;
    tiDevice->ops.exit      = _TI_EXIT_SceneSelector;
    tiDevice->ops.discovery = _TI_DISCOV_SceneSelector;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_SceneSelector;
    tiDevice->ops.onTrack   = _TI_TRACK_SceneSelector;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_configurationTool
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_ConfigurationTool(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_ConfigurationTool(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_ConfigurationTool(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_ConfigurationTool(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_ConfigurationTool(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_ConfigurationTool(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_ConfigurationTool;
    tiDevice->ops.exit      = _TI_EXIT_ConfigurationTool;
    tiDevice->ops.discovery = _TI_DISCOV_ConfigurationTool;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_ConfigurationTool;
    tiDevice->ops.onTrack   = _TI_TRACK_ConfigurationTool;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_remoteControl
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_RemoteControl(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_RemoteControl(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_RemoteControl(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_RemoteControl(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_RemoteControl(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}

static TiDevice_t* _TI_RENDER_RemoteControl(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_RemoteControl;
    tiDevice->ops.exit      = _TI_EXIT_RemoteControl;
    tiDevice->ops.discovery = _TI_DISCOV_RemoteControl;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_RemoteControl;
    tiDevice->ops.onTrack   = _TI_TRACK_RemoteControl;

    return tiDevice;
}



/**-------------------------------------------------------------------------------------------------
 * edid_CombinedInterface
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_CombinedInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_CombinedInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_CombinedInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_CombinedInterface(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_CombinedInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_CombinedInterface(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_CombinedInterface;
    tiDevice->ops.exit      = _TI_EXIT_CombinedInterface;
    tiDevice->ops.discovery = _TI_DISCOV_CombinedInterface;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_CombinedInterface;
    tiDevice->ops.onTrack   = _TI_TRACK_CombinedInterface;

    return tiDevice;
}



/**-------------------------------------------------------------------------------------------------
 * edid_rangeExtender
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_RangeExtender(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_RangeExtender(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_RangeExtender(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_RangeExtender(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_RangeExtender(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_RangeExtender(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_RangeExtender;
    tiDevice->ops.exit      = _TI_EXIT_RangeExtender;
    tiDevice->ops.discovery = _TI_DISCOV_RangeExtender;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_RangeExtender;
    tiDevice->ops.onTrack   = _TI_TRACK_RangeExtender;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_mainsPowerOutlet
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_MainsPowerOutlet(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_MainsPowerOutlet(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_MainsPowerOutlet(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_MainsPowerOutlet(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_MainsPowerOutlet(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_MainsPowerOutlet(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_MainsPowerOutlet;
    tiDevice->ops.exit      = _TI_EXIT_MainsPowerOutlet;
    tiDevice->ops.discovery = _TI_DISCOV_MainsPowerOutlet;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_MainsPowerOutlet;
    tiDevice->ops.onTrack   = _TI_TRACK_MainsPowerOutlet;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_doorLock
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_DoorLock(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_DoorLock(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_DoorLock(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_DoorLock(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_DoorLock(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_DoorLock(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_DoorLock;
    tiDevice->ops.exit      = _TI_EXIT_DoorLock;
    tiDevice->ops.discovery = _TI_DISCOV_DoorLock;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_DoorLock;
    tiDevice->ops.onTrack   = _TI_TRACK_DoorLock;

    return tiDevice;
}



/**-------------------------------------------------------------------------------------------------
 * edid_doorLockController
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_DoorLockController(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_DoorLockController(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_DoorLockController(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_DoorLockController(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_DoorLockController(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_DoorLockController(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_DoorLockController;
    tiDevice->ops.exit      = _TI_EXIT_DoorLockController;
    tiDevice->ops.discovery = _TI_DISCOV_DoorLockController;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_DoorLockController;
    tiDevice->ops.onTrack   = _TI_TRACK_DoorLockController;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_simpleSensor
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_SimpleSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_SimpleSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_SimpleSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_SimpleSensor(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_SimpleSensor(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_SimpleSensor(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_SimpleSensor;
    tiDevice->ops.exit      = _TI_EXIT_SimpleSensor;
    tiDevice->ops.discovery = _TI_DISCOV_SimpleSensor;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_SimpleSensor;
    tiDevice->ops.onTrack   = _TI_TRACK_SimpleSensor;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_consumptionAwarenessDevice
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_ConsumptionAwarenessDevice(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_ConsumptionAwarenessDevice;
    tiDevice->ops.exit      = _TI_EXIT_ConsumptionAwarenessDevice;
    tiDevice->ops.discovery = _TI_DISCOV_ConsumptionAwarenessDevice;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_ConsumptionAwarenessDevice;
    tiDevice->ops.onTrack   = _TI_TRACK_ConsumptionAwarenessDevice;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_homeGateway
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_HomeGateway(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_HomeGateway(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_HomeGateway(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_HomeGateway(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_HomeGateway(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_HomeGateway(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_HomeGateway;
    tiDevice->ops.exit      = _TI_EXIT_HomeGateway;
    tiDevice->ops.discovery = _TI_DISCOV_HomeGateway;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_HomeGateway;
    tiDevice->ops.onTrack   = _TI_TRACK_HomeGateway;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_smartPlug
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_SmartPlug(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_SmartPlug(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_SmartPlug(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_SmartPlug(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_SmartPlug(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_SmartPlug(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_SmartPlug;
    tiDevice->ops.exit      = _TI_EXIT_SmartPlug;
    tiDevice->ops.discovery = _TI_DISCOV_SmartPlug;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_SmartPlug;
    tiDevice->ops.onTrack   = _TI_TRACK_SmartPlug;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_whiteGoods
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_WhiteGoods(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_WhiteGoods(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_WhiteGoods(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_WhiteGoods(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_WhiteGoods(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_WhiteGoods(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_WhiteGoods;
    tiDevice->ops.exit      = _TI_EXIT_WhiteGoods;
    tiDevice->ops.discovery = _TI_DISCOV_WhiteGoods;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_WhiteGoods;
    tiDevice->ops.onTrack   = _TI_TRACK_WhiteGoods;

    return tiDevice;
}


/**-------------------------------------------------------------------------------------------------
 * edid_meterInterface
 *------------------------------------------------------------------------------------------------*/

static void _TI_INIT_MeterInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EXIT_MeterInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_DISCOV_MeterInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_EVENT_MeterInterface(TiSchemaHdr_t* header, TiDevice_t* self, ZclMsg_t zclMsg)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}


static void _TI_TRACK_MeterInterface(TiSchemaHdr_t* header, TiDevice_t* self)
{
    Logger.writeLog(LOG_INFO, "%s", __FUNCTION__);
}


static TiDevice_t* _TI_RENDER_MeterInterface(TiSchemaHdr_t* header, void* device, void* endpoint)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    TiDevice_t* tiDevice = calloc(1, sizeof(TiDevice_t));

    tiDevice->specs = TiCommon.renderSpecs(header, device, endpoint);

    tiDevice->ops.init      = _TI_INIT_MeterInterface;
    tiDevice->ops.exit      = _TI_EXIT_MeterInterface;
    tiDevice->ops.discovery = _TI_DISCOV_MeterInterface;
    tiDevice->ops.getState  = TiCommon.getState;
    tiDevice->ops.setState  = TiCommon.setState;
    tiDevice->ops.request   = TiCommon.request;
    tiDevice->ops.onEvent   = _TI_EVENT_MeterInterface;
    tiDevice->ops.onTrack   = _TI_TRACK_MeterInterface;

    return tiDevice;
}


TiSchema_t TI_HA_OnOffSwitch                = { .pid = epid_ha  ,.did = edid_ha_onOffSwitch                ,.ops = { _TI_RENDER_OnOffSwitch} };
TiSchema_t TI_HA_LevelControlSwitch         = { .pid = epid_ha  ,.did = edid_ha_levelControlSwitch         ,.ops = { _TI_RENDER_LevelControlSwitch} };
TiSchema_t TI_HA_OnOffOutput                = { .pid = epid_ha  ,.did = edid_ha_onOffOutput                ,.ops = { _TI_RENDER_OnOffOutput} };
TiSchema_t TI_HA_LevelControllableOutput    = { .pid = epid_ha  ,.did = edid_ha_levelControllableOutput    ,.ops = { _TI_RENDER_LevelControllableOutput} };
TiSchema_t TI_HA_SceneSelector              = { .pid = epid_ha  ,.did = edid_ha_sceneSelector              ,.ops = { _TI_RENDER_SceneSelector} };
TiSchema_t TI_HA_ConfigurationTool          = { .pid = epid_ha  ,.did = edid_ha_configurationTool          ,.ops = { _TI_RENDER_ConfigurationTool} };
TiSchema_t TI_HA_RemoteControl              = { .pid = epid_ha  ,.did = edid_ha_remoteControl              ,.ops = { _TI_RENDER_RemoteControl} };
TiSchema_t TI_HA_CombinedInterface          = { .pid = epid_ha  ,.did = edid_ha_combinedInterface          ,.ops = { _TI_RENDER_CombinedInterface} };
TiSchema_t TI_HA_RangeExtender              = { .pid = epid_ha  ,.did = edid_ha_rangeExtender              ,.ops = { _TI_RENDER_RangeExtender} };
TiSchema_t TI_HA_MainsPowerOutlet           = { .pid = epid_ha  ,.did = edid_ha_mainsPowerOutlet           ,.ops = { _TI_RENDER_MainsPowerOutlet} };
TiSchema_t TI_HA_DoorLock                   = { .pid = epid_ha  ,.did = edid_ha_doorLock                   ,.ops = { _TI_RENDER_DoorLock} };
TiSchema_t TI_HA_DoorLockController         = { .pid = epid_ha  ,.did = edid_ha_doorLockController         ,.ops = { _TI_RENDER_DoorLockController} };
TiSchema_t TI_HA_SimpleSensor               = { .pid = epid_ha  ,.did = edid_ha_simpleSensor               ,.ops = { _TI_RENDER_SimpleSensor} };
TiSchema_t TI_HA_ConsumptionAwarenessDevice = { .pid = epid_ha  ,.did = edid_ha_consumptionAwarenessDevice ,.ops = { _TI_RENDER_ConsumptionAwarenessDevice} };
TiSchema_t TI_HA_HomeGateway                = { .pid = epid_ha  ,.did = edid_ha_homeGateway                ,.ops = { _TI_RENDER_HomeGateway} };
TiSchema_t TI_HA_SmartPlug                  = { .pid = epid_ha  ,.did = edid_ha_smartPlug                  ,.ops = { _TI_RENDER_SmartPlug} };
TiSchema_t TI_HA_WhiteGoods                 = { .pid = epid_ha  ,.did = edid_ha_whiteGoods                 ,.ops = { _TI_RENDER_WhiteGoods} };
TiSchema_t TI_HA_MeterInterface             = { .pid = epid_ha  ,.did = edid_ha_meterInterface             ,.ops = { _TI_RENDER_MeterInterface} };