#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "SafeQueue.h"
#include "SafeHash.h"
#include "UniqueKey.h"
#include "Command.h"
#include "Logger.h"
#include "Network.h"
#include "Gateway.h"
#include "Identifier.h"
#include "Controller.h"

static inline void EVN_REG(
    SafeHashHdr_t* header,
    CmdDomain_t domain,
    CmdMode_t mode,
    uint8_t cmdId,
    CmdSeverity_t severity,
    void (*execute)(void* header, void* msg))
{
    Kid_t kid = {.cmd = {domain, mode, cmdId}};
    Key_t key = UniqueKey.type("cmd").render(kid);
    void* elem = Cmder.create(severity, execute);

    SafeHash.insert(header, key, elem);

    Cmd_t* readback = SafeHash.find(header, key);
    if (readback == NULL)
        Logger.writeLog(LOG_DEBUG, "%s - failed key = %s\n", __FUNCTION__, key.data);
}



/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

static ControlHdr_t* _create();
static void _destroy(ControlHdr_t* header);
static void _start(ControlHdr_t* header);
static void _stop(ControlHdr_t* header);
static void _request(ControlHdr_t* header, Cmd_t* cmd);
static void* _evnThread(void *data);
static void* _mngThread(void *data);


/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

static ControlHdr_t* _create(TiNotifyCB_t tiNotifyCB)
{
    ControlHdr_t* header = malloc(sizeof(ControlHdr_t));
    header->evnHash    = SafeHash.instance(256);
    header->cmdHash    = SafeHash.instance(256);
    header->cmdQueue   = SafeQueue.create();
    header->evnQueue   = SafeQueue.create();
    header->tiNotifyCB = tiNotifyCB;
    header->ready      = false;
    header->tiHdr      = TiFactory.instance(header);
    header->itemHdr    = SafeHash.instance(128);

    IoHdr_t* netIO = Connector.create(eDomainNet, 1, "127.0.0.1", 2540);
    IoHdr_t* devIO = Connector.create(eDomainDev, 1, "127.0.0.1", 2541);
    header->pollHdr = Poller.create(header->cmdQueue, header->evnQueue, header->cmdHash, header->evnHash);
    Poller.insert(header->pollHdr, devIO);
    Poller.insert(header->pollHdr, netIO);

    // Network events
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_ZIGBEE_GENERIC_CNF         ,MAJOR ,Network.RSP_Generic);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_GET_LOCAL_DEVICE_INFO_CNF  ,MAJOR ,Network.RSP_coordInfo);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_GET_DEVICE_LIST_CNF        ,MAJOR ,Network.RSP_DevList);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_ZIGBEE_NWK_INFO_CNF        ,MAJOR ,Network.RSP_NetworkInfo);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_GET_GW_ENDPOINT_INFO_CNF   ,MAJOR ,Network.RSP_EndpointInfo);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_GET_NWK_KEY_CNF            ,MAJOR ,Network.RSP_NetworkKey);
    EVN_REG(header->evnHash, eDomainNet, eModeSync  ,enw_ZIGBEE_SYSTEM_RESET_CNF    ,MAJOR ,Network.RSP_Reset);
    EVN_REG(header->evnHash, eDomainNet, eModeAsync ,enw_ZIGBEE_NWK_READY_IND       ,MAJOR ,Network.IND_NetworkReady);
    EVN_REG(header->evnHash, eDomainNet, eModeAsync ,enw_GET_NEIGHBOR_TABLE_RSP_IND ,MAJOR ,Network.IND_LinkQuality);
    EVN_REG(header->evnHash, eDomainNet, eModeAsync ,enw_ZIGBEE_DEVICE_IND          ,MAJOR ,Network.IND_Device);
    EVN_REG(header->evnHash, eDomainNet, eModeAsync ,enw_SET_BINDING_ENTRY_RSP_IND  ,MAJOR ,Network.IND_BindEntry);

    // Gateway events
    EVN_REG(header->evnHash, eDomainDev, eModeSync  ,egw_ZIGBEE_GENERIC_CNF                ,MAJOR ,Gateway.generic.RSP_Generic);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_ZIGBEE_GENERIC_RSP_IND            ,MAJOR ,Gateway.generic.IND_Generic);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_READ_DEVICE_ATTRIBUTE_RSP_IND     ,MAJOR ,Gateway.foundation.IND_Read);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_WRITE_DEVICE_ATTRIBUTE_RSP_IND    ,MAJOR ,Gateway.foundation.IND_Write);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_SET_ATTRIBUTE_REPORTING_RSP_IND   ,MAJOR ,Gateway.foundation.IND_ConfigReport);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_ATTRIBUTE_REPORTING_IND           ,MAJOR ,Gateway.foundation.IND_Report);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_DEV_GET_COLOR_RSP_IND             ,MAJOR ,Gateway.lighting.IND_GetHueAndSaturation);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_DEV_GET_COLOR_TEMP_RSP_IND        ,MAJOR ,Gateway.lighting.IND_GetColorTemp);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_ZCL_FRAME_RECEIVE_IND             ,MAJOR ,Gateway.generic.IND_ZclFrame);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_GET_DEVICE_ATTRIBUTE_LIST_RSP_IND ,MAJOR ,Gateway.foundation.IND_Discover);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_DEV_ZONE_STATUS_CHANGE_IND        ,MAJOR ,Gateway.ias.IND_StatusChangeNotification);
    EVN_REG(header->evnHash, eDomainDev, eModeAsync ,egw_GET_GROUP_MEMBERSHIP_RSP_IND      ,MAJOR ,Gateway.genGroups.IND_GroupGet);

    return header;
}


static void _destroy(ControlHdr_t* header)
{
    _stop(header);

    SafeHash.destroy(header->evnHash);

    // CmdQueue
    SafeQueue.destroy(header->cmdQueue);

    // EvnQueue
    SafeQueue.destroy(header->evnQueue);

    // IO
    Poller.destroy(header->pollHdr);

    // TiFactory
    TiFactory.destroy(header->tiHdr);

    // TiDevice
    SafeHash.destroy(header->itemHdr);
}

static bool _ready(ControlHdr_t* header);

static void _start(ControlHdr_t* header)
{
    Poller.start(header->pollHdr);

    // Waiting for the connection establish
    while (!Poller.ready(header->pollHdr))
    {
        Logger.writeLog(LOG_DEBUG, "%s - Not ready", __FUNCTION__);
        sleep(1);
    }

    pthread_create(&header->evnTid, NULL, _evnThread, header);

    // pthread_create(&header->mngTid, NULL, _mngThread, header);

    // Get network information
    // _request(header, Network.REQ_NetworkInfo()); // Remote

    // Get coordinator
    _request(header, Network.REQ_CoordInfo()); // Local

    // Get device list
    _request(header, Network.REQ_DevList()); // Local
}


static void _stop(ControlHdr_t* header)
{
    pthread_join(header->evnTid, NULL);
    pthread_join(header->mngTid, NULL);
}


static bool _ready(ControlHdr_t* header)
{
    return Poller.ready(header->pollHdr) && header->ready;
}

static void _request(ControlHdr_t* header, Cmd_t* cmd)
{
    Logger.writeLog(LOG_DEBUG, "REQ ---------->(%s)\n", cmd->key.data);
    SafeQueue.push(header->cmdQueue, (void*)cmd);
    // sleep(1);
}


/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

static void* _evnThread(void *data)
{
    static int initPhase = 0;
    ControlHdr_t* header = (ControlHdr_t*) data;

    while (true)
    {
        Cmd_t* cmd = SafeQueue.pop(header->evnQueue);
        Cmd_t* elem = SafeHash.find(header->evnHash, cmd->key);

        Logger.writeLog(LOG_DEBUG, "RSP <----------(%s)", cmd->key.data);
        elem->execute(header, cmd);
        Logger.writeLog(LOG_DEBUG, "");

        if (!header->ready)
        {
            initPhase++;
            header->ready = (initPhase == 2) ? true : false;
            if (header->ready)
            {
                _request(header, Network.REQ_NetworkInfo()); // Remote
                Logger.writeLog(LOG_DEBUG, "==============================> READY");
            }
        }

        Cmder.destroy(cmd);
    }

    return NULL;
}


static void* _mngThread(void *data)
{
    Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
    ControlHdr_t* header = (ControlHdr_t*)data;

    while(true)
    {
        Iterator_t tiIter = SH_ITERATOR(header->itemHdr);
        Key_t tiDevKey = SafeHash.iterateKeys(&tiIter);
        while (tiDevKey.len > 0)
        {
            Kid_t tiDevKid = UniqueKey.type("item").parser(tiDevKey);
            if (tiDevKid.uuid.mac != header->sysHdr.coordinator)
            {
                TiDevice_t* item = SafeHash.find(header->itemHdr, tiDevKey);
                if (item != NULL)
                {
                    if (item->ops.onTrack)
                        item->ops.onTrack(header->tiHdr, item);
                }

                // Logger.writeLog(LOG_INFO, "%s - Tracking (0x%llX)", __FUNCTION__, tiDevKid.uuid.mac);
                // EpAddr_t epAddr;
                // epAddr.ieee_addr = tiDevKid.uuid.mac;
                // epAddr.endpoint = tiDevKid.uuid.eid;

                // ReadRec readRec;
                // readRec.len    = 1;
                // readRec.aid[0] = 0x0000;
                // _request(header, Gateway.foundation.REQ_Read(epAddr, ecid_genBasic, readRec));
            }

            tiDevKey = SafeHash.iterateKeys(&tiIter);
        }
        sleep(60);
    }

    return NULL;
}



_Controller const Controller = {_create, _destroy, _start, _stop, _ready, _request};
