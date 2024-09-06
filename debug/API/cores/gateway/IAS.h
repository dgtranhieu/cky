#ifndef IAS_H
#define IAS_H
#include "Command.h"


Cmd_t* _REQ_AutoEnrollRsp(EpAddr_t epAddr, uint32_t zoneId, int rspCode);
Cmd_t* _REQ_StartWarning(EpAddr_t epAddr, uint8_t warningmessage, uint16_t warningDuration, uint8_t strobeDutyCycle, uint8_t strobeLevel);
void _IND_StatusChangeNotification(void* header, void* msg);


#endif // IAS_H