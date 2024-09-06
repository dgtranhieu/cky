#ifndef FOUNDATION_H
#define FOUNDATION_H
#include "Command.h"
#include "Controller.h"


typedef struct AttrRec
{
    int len;
    uint16_t aid;
    uint32_t tid;
    uint8_t val[16];
}AttrRec;

struct _ReadRec
{
    int len;
    uint32_t aid[16];
    uint32_t tid[16];
}_ReadRec;
typedef struct _ReadRec ReadRec;

struct _WriteRec
{
    int len;
    AttrRec attr[16];
}_WriteRec;
typedef struct _WriteRec WriteRec;

struct _RptRec
{
    uint8_t direction;
    AttrRec attribute;
    uint16_t minRepIntval;
    uint16_t maxRepIntval;
    uint32_t repChange;
}_RptRec;
typedef struct _RptRec RptRec;

struct _CfgRptRec
{
    RptRec rptRec[16];
    int len;
}_CfgRptRec;
typedef struct _CfgRptRec CfgRptRec;


// Exports
Cmd_t* _REQ_Read(EpAddr_t epAddr, uint16_t cid, ReadRec readRec);
Cmd_t* _REQ_Write(EpAddr_t epAddr, uint16_t cid, WriteRec writeRec);
Cmd_t* _REQ_ConfigReport(EpAddr_t epAddr, uint16_t cid, CfgRptRec cfgRptRec);
Cmd_t* _REQ_Discover(EpAddr_t epAddr);
void _IND_Read(void* header, void* msg);
void _IND_Write(void* header, void* msg);
void _IND_ConfigReport(void* header, void* msg);
void _IND_Report(void* header, void* msg);
void _IND_Discover(void* header, void* msg);

#endif // FOUNDATION_H