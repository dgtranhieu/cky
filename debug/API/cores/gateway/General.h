#ifndef GENERAL_H
#define GENERAL_H
#include "Command.h"


// Exports
Cmd_t* _REQ_On();
Cmd_t* _REQ_Off();
Cmd_t* _REQ_Toggle();
Cmd_t* _REQ_GroupAdd(EpAddr_t epAddr, uint16_t gid);
Cmd_t* _REQ_GroupRemove(EpAddr_t epAddr, uint16_t gid);
Cmd_t* _REQ_GroupGet(EpAddr_t epAddr);
Cmd_t* _REQ_MoveToLevel(EpAddr_t epAddr, uint32_t transitionTime, uint32_t level);
void _IND_GroupGet(void* header, void* msg);


#endif // GENERAL_H