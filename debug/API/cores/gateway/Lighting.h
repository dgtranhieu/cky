#ifndef LIGHTING_H
#define LIGHTING_H
#include "Command.h"


Cmd_t* _REQ_MoveToHueAndSaturation(EpAddr_t epAddr, uint8_t hue, uint8_t saturation, uint16_t transtime);
Cmd_t* _REQ_GetHueAndSaturation(EpAddr_t epAddr);
Cmd_t* _REQ_MoveToColorTemp(EpAddr_t epAddr, uint16_t colortemp, uint16_t transtime);
Cmd_t* _REQ_GetColorTemp(EpAddr_t epAddr);
void _IND_GetHueAndSaturation(void* header, void* msg);
void _IND_GetColorTemp(void* header, void* msg);


#endif // LIGHTING_H