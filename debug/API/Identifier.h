#ifndef IDENTIFIER_H
#define IDENTIFIER_H


typedef enum PidEnum
{
    epid_ha = 0x0104,
    epid_ba = 0x0104,
    epid_ts = 0x0107,
    epid_hc = 0x0108,
    epid_se = 0x0109,
    epid_rs = 0x010A,
    epid_ll = 0xC05E
}PidEnum;

typedef enum DidEnum
{
    // Home Automation
    edid_ha_onOffSwitch                   = 0,
    edid_ha_levelControlSwitch            = 1,
    edid_ha_onOffOutput                   = 2,
    edid_ha_levelControllableOutput       = 3,
    edid_ha_sceneSelector                 = 4,
    edid_ha_configurationTool             = 5,
    edid_ha_remoteControl                 = 6,
    edid_ha_combinedInterface             = 7,
    edid_ha_rangeExtender                 = 8,
    edid_ha_mainsPowerOutlet              = 9,
    edid_ha_doorLock                      = 10,
    edid_ha_doorLockController            = 11,
    edid_ha_simpleSensor                  = 12,
    edid_ha_consumptionAwarenessDevice    = 13,
    edid_ha_homeGateway                   = 80,
    edid_ha_smartPlug                     = 81,
    edid_ha_whiteGoods                    = 82,
    edid_ha_meterInterface                = 83,
    edid_ha_testDevice                    = 255,
    edid_ha_onOffLight                    = 256,
    edid_ha_dimmableLight                 = 257,
    edid_ha_coloredDimmableLight          = 258,
    edid_ha_onOffLightSwitch              = 259,
    edid_ha_dimmerSwitch                  = 260,
    edid_ha_colorDimmerSwitch             = 261,
    edid_ha_lightSensor                   = 262,
    edid_ha_occupancySensor               = 263,
    edid_ha_shade                         = 512,
    edid_ha_shadeController               = 513,
    edid_ha_windowCoveringDevice          = 514,
    edid_ha_windowCoveringController      = 515,
    edid_ha_heatingCoolingUnit            = 768,
    edid_ha_thermostat                    = 769,
    edid_ha_temperatureSensor             = 770,
    edid_ha_pump                          = 771,
    edid_ha_pumpController                = 772,
    edid_ha_pressureSensor                = 773,
    edid_ha_flowSensor                    = 774,
    edid_ha_miniSplitAc                   = 775,
    edid_ha_iasControlIndicatingEquipment = 1024,
    edid_ha_iasAncillaryControlEquipment  = 1025,
    edid_ha_iasZone                       = 1026,
    edid_ha_iasWarningDevice              = 1027,

    // @pxhoang: Unclassified devices
    edid_ha_aqaraDoorSensor               = 24321,

    // LIGHT LINK
    // Light Device
    edid_ll_onOffLight                    = 0,
    edid_ll_onOffPlugin                   = 16,
    edid_ll_dimmableLight                 = 256,
    edid_ll_dimmablePlugin                = 272,
    edid_ll_colorLight                    = 512,
    edid_ll_extendedColorLight            = 528,
    edid_ll_colorTemperatureLight         = 544,

    // Controller devices
    edid_ll_colorController               = 2048,
    edid_ll_colorScenceController         = 2064,
    edid_ll_nonColorController            = 2080,
    edid_ll_nonColorScenceController      = 2096,
    edid_ll_controlBridge                 = 2112,
    edid_ll_onOffSensor                   = 2128,
}DidEnum;

typedef enum CidEnum
{
    ecid_genBasic                          = 0,
    ecid_genPowerCfg                       = 1,
    ecid_genDeviceTempCfg                  = 2,
    ecid_genIdentify                       = 3,
    ecid_genGroups                         = 4,
    ecid_genScenes                         = 5,
    ecid_genOnOff                          = 6,
    ecid_genOnOffSwitchCfg                 = 7,
    ecid_genLevelCtrl                      = 8,
    ecid_genAlarms                         = 9,
    ecid_genTime                           = 10,
    ecid_genRssiLocation                   = 11,
    ecid_genAnalogInput                    = 12,
    ecid_genAnalogOutput                   = 13,
    ecid_genAnalogValue                    = 14,
    ecid_genBinaryInput                    = 15,
    ecid_genBinaryOutput                   = 16,
    ecid_genBinaryValue                    = 17,
    ecid_genMultistateInput                = 18,
    ecid_genMultistateOutput               = 19,
    ecid_genMultistateValue                = 20,
    ecid_genCommissioning                  = 21,
    ecid_genPartition                      = 22,
    ecid_genOta                            = 25,
    ecid_genPowerProfile                   = 26,
    ecid_genApplianceCtrl                  = 27,
    ecid_genPollCtrl                       = 32,
    ecid_genGreenPowerProxy                = 33,
    ecid_mobileDeviceCfg                   = 34,
    ecid_neighborCleaning                  = 35,
    ecid_nearestGateway                    = 36,
    ecid_closuresShadeCfg                  = 256,
    ecid_closuresDoorLock                  = 257,
    ecid_closuresWindowCovering            = 258,
    ecid_hvacPumpCfgCtrl                   = 512,
    ecid_hvacThermostat                    = 513,
    ecid_hvacFanCtrl                       = 514,
    ecid_hvacDehumidificationCtrl          = 515,
    ecid_hvacUserInterfaceCfg              = 516,
    ecid_lightingColorCtrl                 = 768,
    ecid_lightingBallastCfg                = 769,
    ecid_msIlluminanceMeasurement          = 1024,
    ecid_msIlluminanceLevelSensing         = 1025,
    ecid_msTemperatureMeasurement          = 1026,
    ecid_msPressureMeasurement             = 1027,
    ecid_msFlowMeasurement                 = 1028,
    ecid_msRelativeHumidity                = 1029,
    ecid_msOccupancySensing                = 1030,
    ecid_ssIasZone                         = 1280,
    ecid_ssIasAce                          = 1281,
    ecid_ssIasWd                           = 1282,
    ecid_piGenericTunnel                   = 1536,
    ecid_piBacnetProtocolTunnel            = 1537,
    ecid_piAnalogInputReg                  = 1538,
    ecid_piAnalogInputExt                  = 1539,
    ecid_piAnalogOutputReg                 = 1540,
    ecid_piAnalogOutputExt                 = 1541,
    ecid_piAnalogValueReg                  = 1542,
    ecid_piAnalogValueExt                  = 1543,
    ecid_piBinaryInputReg                  = 1544,
    ecid_piBinaryInputExt                  = 1545,
    ecid_piBinaryOutputReg                 = 1546,
    ecid_piBinaryOutputExt                 = 1547,
    ecid_piBinaryValueReg                  = 1548,
    ecid_piBinaryValueExt                  = 1549,
    ecid_piMultistateInputReg              = 1550,
    ecid_piMultistateInputExt              = 1551,
    ecid_piMultistateOutputReg             = 1552,
    ecid_piMultistateOutputExt             = 1553,
    ecid_piMultistateValueReg              = 1554,
    ecid_piMultistateValueExt              = 1555,
    ecid_pi11073ProtocolTunnel             = 1556,
    ecid_piIso7818ProtocolTunnel           = 1557,
    ecid_piRetailTunnel                    = 1559,
    ecid_sePrice                           = 1792,
    ecid_seDrlc                            = 1793,
    ecid_seMetering                        = 1794,
    ecid_seMessaging                       = 1795,
    ecid_seTunneling                       = 1796,
    ecid_sePrepayment                      = 1797,
    ecid_seEnergyMgmt                      = 1798,
    ecid_seCalendar                        = 1799,
    ecid_seDeviceMgmt                      = 1800,
    ecid_seEvents                          = 1801,
    ecid_seMduPairing                      = 1802,
    ecid_seKeyEstablishment                = 2048,
    ecid_telecommunicationsInformation     = 2304,
    ecid_telecommunicationsVoiceOverZigbee = 2308,
    ecid_telecommunicationsChatting        = 2309,
    ecid_haApplianceIdentification         = 2816,
    ecid_haMeterIdentification             = 2817,
    ecid_haApplianceEventsAlerts           = 2818,
    ecid_haApplianceStatistics             = 2819,
    ecid_haElectricalMeasurement           = 2820,
    ecid_haDiagnostic                      = 2821,
    ecid_lightLink                         = 4096,
    ecid_manuSpecificCluster               = 65535,
}CidEnum;

typedef enum TidEnum
{
    etid_noData       = 0,
    etid_data8        = 8,
    etid_data16       = 9,
    etid_data24       = 10,
    etid_data32       = 11,
    etid_data40       = 12,
    etid_data48       = 13,
    etid_data56       = 14,
    etid_data64       = 15,
    etid_boolean      = 16,
    etid_bitmap8      = 24,
    etid_bitmap16     = 25,
    etid_bitmap24     = 26,
    etid_bitmap32     = 27,
    etid_bitmap40     = 28,
    etid_bitmap48     = 29,
    etid_bitmap56     = 30,
    etid_bitmap64     = 31,
    etid_uint8        = 32,
    etid_uint16       = 33,
    etid_uint24       = 34,
    etid_uint32       = 35,
    etid_uint40       = 36,
    etid_uint48       = 37,
    etid_uint56       = 38,
    etid_uint64       = 39,
    etid_int8         = 40,
    etid_int16        = 41,
    etid_int24        = 42,
    etid_int32        = 43,
    etid_int40        = 44,
    etid_int48        = 45,
    etid_int56        = 46,
    etid_int64        = 47,
    etid_enum8        = 48,
    etid_enum16       = 49,
    etid_semiPrec     = 56,
    etid_singlePrec   = 57,
    etid_doublePrec   = 58,
    etid_octetStr     = 65,
    etid_charStr      = 66,
    etid_longOctetStr = 67,
    etid_longCharStr  = 68,
    etid_array        = 72,
    etid_structt      = 76,
    etid_set          = 80,
    etid_bag          = 81,
    etid_tod          = 224,
    etid_date         = 225,
    etid_utc          = 226,
    etid_clusterId    = 232,
    etid_attrId       = 233,
    etid_bacOid       = 234,
    etid_ieeeAddr     = 240,
    etid_secKey       = 241,
    etid_unknown      = 255,
}TidEnum;

typedef enum GwCmdEnum
{
    egw_ZIGBEE_GENERIC_CNF                            = 0,
    egw_ZIGBEE_GENERIC_RSP_IND                        = 1,
    egw_ADD_GROUP_REQ                                 = 2,
    egw_ADD_GROUP_RSP_IND                             = 3,
    egw_GET_GROUP_MEMBERSHIP_REQ                      = 4,
    egw_GET_GROUP_MEMBERSHIP_RSP_IND                  = 5,
    egw_REMOVE_FROM_GROUP_REQ                         = 6,
    egw_REMOVE_FROM_GROUP_RSP_IND                     = 7,
    egw_STORE_SCENE_REQ                               = 8,
    egw_STORE_SCENE_RSP_IND                           = 9,
    egw_REMOVE_SCENE_REQ                              = 10,
    egw_REMOVE_SCENE_RSP_IND                          = 11,
    egw_RECALL_SCENE_REQ                              = 12,
    egw_GET_SCENE_MEMBERSHIP_REQ                      = 13,
    egw_GET_SCENE_MEMBERSHIP_RSP_IND                  = 14,
    egw_SLEEPY_DEVICE_PACKET_PENDING_REQ              = 15,
    egw_SLEEPY_DEVICE_CHECK_IN_IND                    = 16,
    egw_ATTRIBUTE_CHANGE_IND                          = 17,
    egw_GET_DEVICE_ATTRIBUTE_LIST_REQ                 = 18,
    egw_GET_DEVICE_ATTRIBUTE_LIST_RSP_IND             = 19,
    egw_READ_DEVICE_ATTRIBUTE_REQ                     = 20,
    egw_READ_DEVICE_ATTRIBUTE_RSP_IND                 = 21,
    egw_WRITE_DEVICE_ATTRIBUTE_REQ                    = 22,
    egw_WRITE_DEVICE_ATTRIBUTE_RSP_IND                = 23,
    egw_SET_ATTRIBUTE_REPORTING_REQ                   = 24,
    egw_SET_ATTRIBUTE_REPORTING_RSP_IND               = 25,
    egw_ATTRIBUTE_REPORTING_IND                       = 26,
    egw_SEND_ZCL_FRAME_REQ                            = 27,
    egw_ZCL_FRAME_RECEIVE_IND                         = 28,
    egw_ALARM_IND                                     = 29,
    egw_ALARM_RESET_REQ                               = 30,
    egw_DEV_PROCESS_IDENTIFY_QUERY_RSP_IND            = 31,
    egw_DEV_ZONE_ENROLLMENT_REQ_IND                   = 32,
    egw_DEV_ZONE_ENROLLMENT_RSP                       = 33,
    egw_DEV_ZONE_STATUS_CHANGE_IND                    = 34,
    egw_DEV_ACE_ARM_REQ_IND                           = 35,
    egw_DEV_ACE_ARM_RSP                               = 36,
    egw_DEV_ACE_BYPASS_IND                            = 37,
    egw_DEV_ACE_EMERGENCY_CONDITION_IND               = 38,
    egw_DEV_ACE_GET_ZONE_ID_MAP_REQ_IND               = 39,
    egw_DEV_ACE_GET_ZONE_ID_MAP_RSP                   = 40,
    egw_DEV_ACE_GET_ZONE_INFORMATION_REQ_IND          = 41,
    egw_DEV_ACE_GET_ZONE_INFORMATION_RSP              = 42,
    egw_DEV_SET_IDENTIFY_MODE_REQ                     = 43,
    egw_DEV_SET_ONOFF_STATE_REQ                       = 44,
    egw_DEV_SET_LEVEL_REQ                             = 45,
    egw_DEV_GET_LEVEL_REQ                             = 46,
    egw_DEV_GET_LEVEL_RSP_IND                         = 47,
    egw_DEV_GET_ONOFF_STATE_REQ                       = 48,
    egw_DEV_GET_ONOFF_STATE_RSP_IND                   = 49,
    egw_DEV_SET_COLOR_REQ                             = 50,
    egw_DEV_GET_COLOR_REQ                             = 51,
    egw_DEV_GET_COLOR_RSP_IND                         = 52,
    egw_DEV_GET_TEMP_REQ                              = 53,
    egw_DEV_GET_TEMP_RSP_IND                          = 54,
    egw_DEV_GET_POWER_REQ                             = 55,
    egw_DEV_GET_POWER_RSP_IND                         = 56,
    egw_DEV_GET_HUMIDITY_REQ                          = 57,
    egw_DEV_GET_HUMIDITY_RSP_IND                      = 58,
    egw_DEV_SET_DOOR_LOCK_REQ                         = 59,
    egw_DEV_SET_DOOR_LOCK_RSP_IND                     = 60,
    egw_DEV_GET_DOOR_LOCK_STATE_REQ                   = 61,
    egw_DEV_GET_DOOR_LOCK_STATE_RSP_IND               = 62,
    egw_DEV_THERMOSTAT_SETPOINT_CHANGE_REQ            = 63,
    egw_DEV_WINDOW_COVERING_ACTION_REQ                = 64,
    egw_DEV_SET_COLOR_TEMP_REQ                        = 65,
    egw_DEV_GET_COLOR_TEMP_REQ                        = 66,
    egw_DEV_GET_COLOR_TEMP_RSP_IND                    = 67,
    egw_DEV_SEND_IDENTIFY_QUERY_REQ                   = 68,
    egw_SET_FINDING_AND_BINDING_TIMER_STATUS_REQ      = 69,
    egw_DEV_SET_FINDING_AND_BINDING_TIMER_STATUS_REQ  = 70,
}GwCmdEnum;

typedef enum NwCmdEnum
{
    enw_ZIGBEE_GENERIC_CNF               = 0,
    enw_ZIGBEE_GENERIC_RSP_IND           = 1,
    enw_ZIGBEE_SYSTEM_RESET_REQ          = 2,
    enw_ZIGBEE_SYSTEM_RESET_CNF          = 3,
    enw_ZIGBEE_SYSTEM_SELF_SHUTDOWN_REQ  = 4,
    enw_SET_ZIGBEE_POWER_MODE_REQ        = 5,
    enw_SET_ZIGBEE_POWER_MODE_CNF        = 6,
    enw_GET_LOCAL_DEVICE_INFO_REQ        = 7,
    enw_GET_LOCAL_DEVICE_INFO_CNF        = 8,
    enw_ZIGBEE_NWK_READY_IND             = 9,
    enw_ZIGBEE_NWK_INFO_REQ              = 10,
    enw_ZIGBEE_NWK_INFO_CNF              = 11,
    enw_SET_PERMIT_JOIN_REQ              = 12,
    enw_MANAGE_PERIODIC_MTO_ROUTE_REQ    = 13,
    enw_GET_NEIGHBOR_TABLE_REQ           = 14,
    enw_GET_NEIGHBOR_TABLE_RSP_IND       = 15,
    enw_GET_ROUTING_TABLE_REQ            = 16,
    enw_GET_ROUTING_TABLE_RSP_IND        = 17,
    enw_CHANGE_NWK_KEY_REQ               = 18,
    enw_GET_NWK_KEY_REQ                  = 19,
    enw_GET_NWK_KEY_CNF                  = 20,
    enw_ZIGBEE_DEVICE_IND                = 21,
    enw_GET_DEVICE_LIST_REQ              = 22,
    enw_GET_DEVICE_LIST_CNF              = 23,
    enw_DEVICE_LIST_MAINTENANCE_REQ      = 24,
    enw_REMOVE_DEVICE_REQ                = 25,
    enw_SET_BINDING_ENTRY_REQ            = 26,
    enw_SET_BINDING_ENTRY_RSP_IND        = 27,
    enw_START_COMMISSIONING_REQ          = 28,
    enw_START_COMMISSIONING_CNF          = 29,
    enw_GET_BINDING_TABLE_REQ            = 30,
    enw_GET_BINDING_TABLE_RSP_IND        = 31,
    enw_SET_INSTALLCODE_REQ              = 32,
    enw_GET_GW_ENDPOINT_INFO_REQ         = 33,
    enw_GET_GW_ENDPOINT_INFO_CNF         = 34,
    enw_SEND_SIMPLE_DESCRIPTOR_REQ       = 35,
    enw_SEND_SIMPLE_DESCRIPTOR_RSP_IND   = 36,
    enw_BIND_ITEM                        = 37,
}NwCmdEnum;


#endif // IDENTIFIER_H