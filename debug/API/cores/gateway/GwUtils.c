#include <stdbool.h>
#include "GwUtils.h"


void genAddr(EpAddr_t* addr, GwAddressStructT* dstaddr)
{
    GwAddressStructT dstaddr_temp = GW_ADDRESS_STRUCT_T__INIT;
    memcpy(dstaddr, &dstaddr_temp, sizeof(GwAddressStructT));

    if (addr->ieee_addr != 0)
    {
        dstaddr->addresstype    = GW_ADDRESS_TYPE_T__UNICAST;
        dstaddr->has_ieeeaddr   = true;
        dstaddr->ieeeaddr       = addr->ieee_addr;
        dstaddr->has_endpointid = true;
        dstaddr->endpointid     = addr->endpoint;
    }
    else
    {
        if (addr->groupaddr == 0xFFFFFFFF)
        {
            dstaddr->addresstype       = GW_ADDRESS_TYPE_T__BROADCAST;
            dstaddr->has_broadcastaddr = true;
            dstaddr->broadcastaddr     = 0xFFFF;
            dstaddr->has_endpointid    = true;
            dstaddr->endpointid        = 0xFF;
        }
        else if (addr->groupaddr == 0xFFFFFFFE)
        {
            dstaddr->addresstype    = GW_ADDRESS_TYPE_T__NONE;
            dstaddr->has_endpointid = true;
            dstaddr->endpointid     = addr->endpoint;
        }
        else
        {
            dstaddr->addresstype    = GW_ADDRESS_TYPE_T__GROUPCAST;
            dstaddr->has_groupaddr  = true;
            dstaddr->groupaddr      = (uint32_t)addr->groupaddr;
            dstaddr->has_endpointid = true; //TBD: is it needed?
            dstaddr->endpointid     = 0xFF;
        }
    }
	dstaddr->has_gwendpointid = true;
	dstaddr->gwendpointid = (uint32_t)(0x04);
}
