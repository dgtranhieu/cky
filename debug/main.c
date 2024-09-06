#include "Command.h"
#include "Connector.h"
#include "Controller.h"
#include "Gateway.h"
#include "GwUtils.h"
#include "HashTable.h"
#include "Identifier.h"
#include "Logger.h"
#include "Network.h"
#include "Poller.h"
#include "SafeQueue.h"
#include "UniqueKey.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void _zbee_TiNotify(int code, Key_t itemKey) {
  Logger.writeLog(LOG_DEBUG, "%s", __FUNCTION__);
}

int main() {
  Logger.setLineWrap(false);
  ControlHdr_t *controlHdr = Controller.create(_zbee_TiNotify);
  Controller.start(controlHdr);

  while (!Controller.ready(controlHdr))
    sleep(2);

  Logger.writeLog(LOG_INFO, "System is ready");
  Logger.writeLog(
      LOG_INFO,
      "==================================================================");

  // OSRAM Light
  EpAddr_t osramAddr;
  osramAddr.endpoint = 3;
  osramAddr.ieee_addr = 0x841826000005ac28;
  EpAddr_t coordAddr;
  EpAddr_t gaddr;

  EpAddr_t lvcAddr;
  lvcAddr.endpoint = 1;
  lvcAddr.ieee_addr = 0x8418260000E8D1CA;

  EpAddr_t cwAddr;
  cwAddr.endpoint = 1;
  cwAddr.ieee_addr = 0x158D0001CEE229;

  EpAddr_t iasDoorAddr;
  iasDoorAddr.endpoint = 1;
  iasDoorAddr.ieee_addr = 0x158D00045C2823;

  EpAddr_t gangSwitchAddr;
  gangSwitchAddr.endpoint = 1;
  gangSwitchAddr.ieee_addr = 0x70AC08FFFE45C7B3;

  while (true) {
    Cmd_t *cmd;
    ReadRec readRec;
    ReadRec readRec1;
    ReadRec readRec2;
    WriteRec writeRec;
    CfgRptRec cfgRptRec;
    int reqNum = 0;
    scanf("%d", &reqNum);
    static unsigned int repChange[1] = {1};

    switch (reqNum) {
    case 0:
      cmd = Network.REQ_CoordInfo();
      Controller.request(controlHdr, cmd);
      break;

    case 1:
      cmd = Network.REQ_DevList();
      Controller.request(controlHdr, cmd);
      break;

    case 2:
      cmd = Network.REQ_NetworkInfo();
      Controller.request(controlHdr, cmd);
      break;

    case 3:
      cmd = Network.REQ_EndpointInfo();
      Controller.request(controlHdr, cmd);
      break;

    case 4: // REQ_PermitJoin
      cmd = Network.REQ_PermitJoin(120);
      Controller.request(controlHdr, cmd);
      break;

    case 5: // Add osRam to group
      cmd = Gateway.genGroups.REQ_GroupAdd(osramAddr, 7);
      Controller.request(controlHdr, cmd);
      break;

    case 6: // Turn on the light
      gaddr.ieee_addr = 0;
      gaddr.groupaddr = 7;
      cmd = Gateway.genOnOff.REQ_On(gaddr);
      Controller.request(controlHdr, cmd);
      break;

    case 7: // Turn off light
      gaddr.ieee_addr = 0;
      gaddr.groupaddr = 7;
      cmd = Gateway.genOnOff.REQ_Off(gaddr);
      Controller.request(controlHdr, cmd);
      break;

    case 8: // Remove device from group
      cmd = Gateway.genGroups.REQ_GroupRemove(osramAddr, 7);
      Controller.request(controlHdr, cmd);
      break;

    case 9: // Add osRam to group
      cmd = Gateway.genGroups.REQ_GroupGet(osramAddr);
      Controller.request(controlHdr, cmd);
      break;

    case 10: // Turn MAC on
      cmd = Gateway.genOnOff.REQ_On(osramAddr);
      Controller.request(controlHdr, cmd);
      break;

    case 11: // Turn MAC on
      cmd = Gateway.genOnOff.REQ_Off(osramAddr);
      Controller.request(controlHdr, cmd);
      break;

    case 12: // Read power config attributes
      readRec.len = 2;
      readRec.aid[0] = 0x0020;
      readRec.aid[1] = 0x0021;
      cmd = Gateway.foundation.REQ_Read(lvcAddr, ecid_genPowerCfg, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 13: // Read battery level
      readRec.len = 1;
      readRec.aid[0] = 0x0007;
      cmd = Gateway.foundation.REQ_Read(iasDoorAddr, ecid_genBasic, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 14: // ZCLVersion
      readRec.len = 1;
      readRec.aid[0] = 0x0000;
      cmd = Gateway.foundation.REQ_Read(iasDoorAddr, ecid_genBasic, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 15: // Read PowerSource
      readRec.len = 1;
      readRec.aid[0] = 0x0007;
      cmd = Gateway.foundation.REQ_Read(iasDoorAddr, ecid_genBasic, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 16: // Read ON/OFF
      readRec.len = 1;
      readRec.aid[0] = 0x0000;
      cmd = Gateway.foundation.REQ_Read(iasDoorAddr, ecid_genOnOff, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 17: // Read ON/OFF
      // {0x00, {0x0000, identifier::tid::boolean, {0,0,0,0}}, 5, 5, 1}
      cfgRptRec.len = 1;
      cfgRptRec.rptRec[0].attribute.aid = 0x0000;
      cfgRptRec.rptRec[0].attribute.tid = etid_boolean;
      cfgRptRec.rptRec[0].minRepIntval = 5;
      cfgRptRec.rptRec[0].maxRepIntval = 5;
      cfgRptRec.rptRec[0].repChange = 1;
      cmd = Gateway.foundation.REQ_ConfigReport(iasDoorAddr, ecid_genOnOff,
                                                cfgRptRec);
      Controller.request(controlHdr, cmd);
      break;

    // Contact switch
    case 18: // Genbasic
      readRec.len = 1;
      readRec.aid[0] = 0x0000;
      cmd = Gateway.foundation.REQ_Read(cwAddr, ecid_genBasic, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 19: // zoneType
      readRec.len = 1;
      readRec.aid[0] = 0x0001;
      cmd = Gateway.foundation.REQ_Read(cwAddr, ecid_ssIasZone, readRec);
      Controller.request(controlHdr, cmd);
      break;

    case 20:
      readRec.len = 2;
      readRec.aid[0] = (uint32_t)0x0000;
      readRec.aid[1] = (uint32_t)0x0001;
      cmd = Gateway.foundation.REQ_Read(osramAddr, ecid_lightingColorCtrl,
                                        readRec);
      Controller.request(controlHdr, cmd);

      Logger.writeLog(LOG_DEBUG, "%s - Discov OnOff", __FUNCTION__);
      readRec1.len = 1;
      readRec1.aid[0] = 0x0000;
      cmd = Gateway.foundation.REQ_Read(osramAddr, ecid_genOnOff, readRec1);
      Controller.request(controlHdr, cmd);

      Logger.writeLog(LOG_DEBUG, "%s - Discov Level", __FUNCTION__);
      readRec2.len = 1;
      readRec2.aid[0] = (uint32_t)0x0000;
      cmd = Gateway.foundation.REQ_Read(osramAddr, ecid_genLevelCtrl, readRec2);
      Controller.request(controlHdr, cmd);
      break;

    // 4Gang
    case 21:
      Logger.writeLog(LOG_DEBUG, "[4Gang] - [genOnOff](Set_ON)");
      cmd = Gateway.genOnOff.REQ_On(gangSwitchAddr);
      Controller.request(controlHdr, cmd);
      break;

    case 22:
      Logger.writeLog(LOG_DEBUG, "[4Gang] - [genOnOff](Set_OFF)");
      cmd = Gateway.genOnOff.REQ_Off(gangSwitchAddr);
      Controller.request(controlHdr, cmd);
      break;

    case default:
      break;
    }
  }

  Controller.stop(controlHdr);
  Controller.destroy(controlHdr);
  printf("pxhoang - end\n");
  return 0;
}
