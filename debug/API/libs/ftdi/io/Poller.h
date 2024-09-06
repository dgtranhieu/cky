#ifndef POLLER_H
#define POLLER_H
#include <netdb.h>
#include <poll.h>
#include <stdbool.h>
#include "Connector.h"
#include "Command.h"
#include "SafeQueue.h"
// #include "HashTable.h"
#include "SafeHash.h"


typedef struct _PollHdr_t PollHdr_t;


typedef struct _Poller
{
    PollHdr_t* (*const create)(QueueHdr_t* cmdQueue, QueueHdr_t* evnQueue, SafeHashHdr_t* cmdHash, SafeHashHdr_t* evnHash);
    void (*const destroy)(PollHdr_t* header);
    bool (*const insert)(PollHdr_t* header, IoHdr_t* ioHeader);
    void (*const remove)(PollHdr_t* header, int index);
    void (*const start)(PollHdr_t* header);
    void (*const stop)(PollHdr_t* header);
    bool (*const ready)(PollHdr_t* header);
    void (*const request)(PollHdr_t* header, Cmd_t* cmd);
}_Poller;

extern _Poller const Poller;

#endif // POLLER_H