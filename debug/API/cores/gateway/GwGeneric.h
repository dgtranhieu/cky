#ifndef GW_GENERIC_H
#define GW_GENERIC_H
#include "Controller.h"


void _RSP_Generic(void* header, void* msg);
void _IND_Generic(void* header, void* msg);
void _IND_ZclFrame(void* header, void* msg);


#endif // GW_GENERIC_H