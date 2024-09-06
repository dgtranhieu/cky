#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "Logger.h"
#include "RetryTimer.h"
#include "SafeQueue.h"
#include "UniqueKey.h"
#include "SafeHash.h"
#include "Poller.h"


#define SERVER_RECONNECTION_RETRY_TIME  2000
#define RPC_DUMMYSYS_connectION_INFO    0x1F
#define RPC_CMD_CONNTCTION_ID           0
#define RPC_CMD_AREQ                    0x40
#define APP_LAYER_NUMNER                5
#define INVALID_LAYER_NUMBER            6
#define APP_NAME                        "pxhoang"
#define RPC_CMD_AREQ                    0x40


typedef struct PollEntry_t
{
	bool inUse;
    bool isReady;
    IoHdr_t* ioHeader;
    RetryHdr_t* retryHeader;
}PollEntry_t;


typedef struct _PollHdr_t
{
    struct pollfd pollList[32];
    PollEntry_t pollDesc[32];
    int last;
    bool quit;
    bool isFull;

    pthread_t cmdThreadId;
    pthread_t evnThreadId;

    QueueHdr_t* evnQueue;
    QueueHdr_t* cmdQueue;
    SafeHashHdr_t* evnHash;
    SafeHashHdr_t* cmdHash;

    bool waitRsp;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}_PollHdr_t;


/**-------------------------------------------------------------------------------------------------
 * Forward declaration
 *------------------------------------------------------------------------------------------------*/

// Extern
static PollHdr_t* _create(QueueHdr_t* cmdQueue, QueueHdr_t* evnQueue, SafeHashHdr_t* cmdHash, SafeHashHdr_t* evnHash);
static void _destroy(PollHdr_t* header);
static bool _insert(PollHdr_t* header, IoHdr_t* ioHeader);
static void _remove(PollHdr_t* header, int index);
static void _start(PollHdr_t* header);
static void _stop(PollHdr_t* header);
static bool _ready(PollHdr_t* header);
static void _request(PollHdr_t* header, Cmd_t* cmd);

// Internal
static int _connect(PollHdr_t* header, int index);
static int _disconnect(PollHdr_t* header, int index);
static int _reconnect(PollHdr_t* header, int index);
static int _send(PollHdr_t* header, int index, uint8_t* pktBody, int len);
static bool _poll(PollHdr_t* header);
static void _rxHandler(PollHdr_t* header, int index);
static void _timerHandler(PollHdr_t* header, int index);
static void* _evnThread(void *data);
static void* _cmdThread(void *data);
static void _evnPush(PollHdr_t* header, int index, Packet_t* entry);


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

static PollHdr_t* _create(QueueHdr_t* cmdQueue, QueueHdr_t* evnQueue, SafeHashHdr_t* cmdHash, SafeHashHdr_t* evnHash)
{
    int i = 0;
    PollHdr_t* header = calloc(1, sizeof(PollHdr_t));

    int descLen = sizeof(header->pollDesc) / sizeof(header->pollDesc[0]);
    for (i = 0; i < descLen; i++)
    {
        bzero(&header->pollList[i], sizeof(header->pollList[i]));
        header->pollList[i].fd       = -1;
        header->pollDesc[i].inUse    = false;
        header->pollDesc[i].ioHeader = NULL;
    }

    header->quit   = false;
    header->last   = 0;
    header->isFull = false;

    header->cmdQueue = cmdQueue;
    header->evnQueue = evnQueue;
    header->cmdHash  = cmdHash;
    header->evnHash  = evnHash;

    pthread_mutex_init(&header->mutex, NULL);
    pthread_cond_init(&header->cond, NULL);
    header->waitRsp = false;

    return header;
}


static void _destroy(PollHdr_t* header)
{
    Logger.writeLog(LOG_DEBUG, "Poller: %s", __FUNCTION__);
    _stop(header);
    free(header);
}


static bool _insert(PollHdr_t* header, IoHdr_t* ioHeader)
{
	int i = 0;
    int descLen = sizeof(header->pollDesc) / sizeof(header->pollDesc[0]);

    if (header->isFull)
    {
        Logger.writeLog(LOG_DEBUG, "Poller: %s - Polling list is full", __FUNCTION__);
        return false;
    }

    // Get index of an empty desc
    for (i = 0; i < descLen; i++)
    {
        if (!header->pollDesc[i].inUse)
            break;
    }

    header->pollList[i].fd          = ioHeader->fdDesc;
    header->pollList[i].events      = POLLIN;
    header->pollList[i].revents     = 0;
    header->pollDesc[i].ioHeader    = ioHeader;
    header->pollDesc[i].inUse       = true;
    header->pollDesc[i].retryHeader = RetryTimer.create(RETRY_ONESHOT, 2000);
    header->last++;

    if (i == header->last)
        header->isFull = true;

    return true;
}


static void _remove(PollHdr_t* header, int index)
{
    Logger.writeLog(LOG_DEBUG, "Poller: %s\n", __FUNCTION__);
    _disconnect(header, index);
    header->pollDesc[index].inUse = false;
}


static void _start(PollHdr_t* header)
{
    // @pxhoang: Thread to poll fds
    pthread_create(&header->evnThreadId, NULL, _evnThread, header);

    // @pxhoang: Handle cmd Queue
    pthread_create(&header->cmdThreadId, NULL, _cmdThread, header);
}


static void _stop(PollHdr_t* header)
{
    int i = 0;
    int descLen = sizeof(header->pollDesc) / sizeof(header->pollDesc[0]);

    pthread_join(header->evnThreadId, NULL);
    pthread_join(header->cmdThreadId, NULL);

    for (i = 0; i < descLen; i++)
    {
        if (header->pollDesc[i].inUse)
        {
            _disconnect(header, i);
        }
    }
}


static bool _ready(PollHdr_t* header)
{
    Logger.writeLog(LOG_DEBUG, "Poller: %s", __FUNCTION__);
    bool ready = true;

    for (int i = 0; i < header->last; i++)
    {
        if (header->pollDesc[i].inUse)
        {
            ready = ready & header->pollDesc[i].isReady;
        }
    }

    return ready;
}


static void _request(PollHdr_t* header, Cmd_t* cmd)
{
    SafeQueue.push(header->cmdQueue, (void*)cmd);
}


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

static int _connect(PollHdr_t* header, int index)
{
    Logger.writeLog(LOG_DEBUG, "Poller: %s", __FUNCTION__);
    uint8_t tmpstr[strlen(APP_NAME) + 6];

    IoHdr_t* ioHeader = header->pollDesc[index].ioHeader;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int ret = connect(fd, (const struct sockaddr*)&ioHeader->sAddr, sizeof(ioHeader->sAddr));
    if (ret < 0)
    {
        Logger.writeLog(LOG_DEBUG, "Poller: %s: Failed to connect", __FUNCTION__);
        _disconnect(header, index);

        RetryTimer.start(header->pollDesc[index].retryHeader);

        // @pxhoang: change polling fds to timer fds
        header->pollList[index].fd = header->pollDesc[index].retryHeader->fdDesc;
        return -1;
    }

    Logger.writeLog(LOG_DEBUG, "Poller: %s - Connected", __FUNCTION__);
    header->pollDesc[index].isReady = true;

    // Update ioHeader and than update
    header->pollDesc[index].ioHeader->fdDesc    = fd;
    header->pollList[index].fd                  = fd;
    header->pollDesc[index].ioHeader->connected = true;

    tmpstr[0] = strlen(APP_NAME) + 1;
    tmpstr[1] = 0;
    tmpstr[2] = RPC_DUMMYSYS_connectION_INFO | RPC_CMD_AREQ;
    tmpstr[3] = RPC_CMD_CONNTCTION_ID;
    tmpstr[4] = APP_LAYER_NUMNER;
    strcpy((char *)tmpstr + 5, APP_NAME);

    _send(header, index, tmpstr, strlen(APP_NAME) + 5);

    return 0;
}


static int _disconnect(PollHdr_t* header, int index)
{
    Logger.writeLog(LOG_DEBUG, "Poller: %s", __FUNCTION__);
    close(header->pollList[index].fd);
    header->pollList[index].fd                  = -1;
    header->pollDesc[index].ioHeader->fdDesc    = -1;
    header->pollDesc[index].ioHeader->connected = false;

    return 0;
}


static int _reconnect(PollHdr_t* header, int index)
{
    if (header->pollDesc[index].ioHeader->connected)
        return 0;

    return _connect(header, index);

    return 0;
}


static bool _poll(PollHdr_t* header)
{
	int i = 0;

	if (poll(header->pollList, header->last, -1) > 0)
	{
		for (i = 0; i < header->last; i++)
		{
            if (header->pollList[i].revents & header->pollList[i].events)
            {
                if (header->pollDesc[i].ioHeader->connected)
                    _rxHandler(header, i);
                else
                    _timerHandler(header, i);
            }
        }
        header->quit = false;
	}
	else
	{
        Logger.writeLog(LOG_DEBUG, "Poller: %s - Polling failed", __FUNCTION__);
		header->quit = true;
	}

	return (!header->quit);
}


static int _send(PollHdr_t* header, int index, uint8_t* pktEntry, int len)
{
    if (write(header->pollList[index].fd, pktEntry, len) != len)
    {
        printf("Poller: %s - Failed to send [fd] = [%d]", __FUNCTION__, header->pollList[index].fd);
        return -1;
    }

    return 0;
}


static void _rxHandler(PollHdr_t* header, int index)
{
    int remaining = 0;
    char rxPacket[MAX_TCP_PACKET_SIZE];
    Packet_t* rxPtr = (Packet_t*)rxPacket;
    bzero(rxPtr, MAX_TCP_PACKET_SIZE);

    remaining = recv(header->pollList[index].fd, rxPacket, MAX_TCP_PACKET_SIZE-1, MSG_DONTWAIT);
    if (remaining < 0)
    {
        Logger.writeLog(LOG_DEBUG, "Poller: %s - remaining < 0", __FUNCTION__);
        return;
    }

    if (remaining == 0)
    {
        Logger.writeLog(LOG_DEBUG, "Poller: %s - remaining = 0", __FUNCTION__);
        close(header->pollList[index].fd );
        header->pollDesc[index].ioHeader->connected = false;
        header->pollDesc[index].ioHeader->fdDesc    = -1;
        header->pollList[index].fd                  = -1;
        _reconnect(header, index);
        return;
    }

    while (remaining > 0)
    {
        if (remaining < sizeof(rxPtr->header))
        {
            Logger.writeLog(LOG_DEBUG, "Poller: %s - ERROR: Packet header incomplete. Len[expect,actual] = [%d, %d]", __FUNCTION__, (int)sizeof(rxPtr->header), remaining);
        }
        else if (remaining < (rxPtr->header.len + 4))
        {
            Logger.writeLog(LOG_DEBUG, "Poller: %s - ERROR: Packet truncated. Len[expect,actual] = [%d, %d]", __FUNCTION__, (rxPtr->header.len + 4), remaining);
        }
        else
        {
            // @pxhoang: Do deep copy
            _evnPush(header, index, rxPtr);

            remaining -= (rxPtr->header.len + 4);
            rxPtr = ((Packet_t*)(((uint8_t *)rxPtr) + (rxPtr->header.len + 4)));

            if (remaining > 0)
                Logger.writeLog(LOG_DEBUG, "Poller: %s Additional API command in the same TCP packet", __FUNCTION__);
        }
    }
}


static void _evnPush(PollHdr_t* header, int index, Packet_t* rxData)
{
    uint8_t    cmdId  = rxData->header.cmdId;
    CmdDomain_t domain = header->pollDesc[index].ioHeader->domain;

    // Check in sync table
    Kid_t sEvnKid = {.cmd = {domain, eModeSync, cmdId}};
    Key_t sEvnKey = UniqueKey.type("cmd").render(sEvnKid);
    Cmd_t* evnEntry = SafeHash.find(header->evnHash, sEvnKey);
    if (evnEntry != NULL)
    {
        Cmd_t* newHdr = Cmder.create(MAJOR, NULL);
        newHdr->key = sEvnKey;
        newHdr->msg = malloc(sizeof(Packet_t));
        memcpy(newHdr->msg, rxData, sizeof(Packet_t));

        SafeQueue.push(header->evnQueue, newHdr);

        pthread_mutex_lock(&header->mutex);
        header->waitRsp = false;
        pthread_cond_signal(&header->cond);
        pthread_mutex_unlock(&header->mutex);

        return;
    }

    // @pxhoang: Duplicate code, come back later
    Kid_t aEvnKid = {.cmd = {domain, eModeAsync, cmdId}};
    Key_t aEvnKey = UniqueKey.type("cmd").render(aEvnKid);
    evnEntry = SafeHash.find(header->evnHash, aEvnKey);
    if (evnEntry != NULL)
    {
        Cmd_t* evnHdr = malloc(sizeof(Cmd_t));
        evnHdr->msg = malloc(sizeof(Packet_t));
        evnHdr->key = aEvnKey;
        memcpy(evnHdr->msg, rxData, sizeof(Packet_t));
        SafeQueue.push(header->evnQueue, evnHdr);
        return;
    }

    Logger.writeLog(LOG_DEBUG, "Poller: %s - Command (%d) is not supported...", __FUNCTION__, cmdId);
}


static void _timerHandler(PollHdr_t* header, int index)
{
    uint64_t timersElapsed = 0;
    header->pollList[index].events = POLLIN;
    header->pollList[index].revents = 0;

    switch (header->pollDesc[index].retryHeader->mode)
    {
        case RETRY_PERIODIC:
            (void) read(header->pollList[index].fd, &timersElapsed, 8);
            break;

        case RETRY_ONESHOT:
            RetryTimer.stop(header->pollDesc[index].retryHeader);
            break;

        default:
            break;
    }

    _reconnect(header, index);
}


/**-------------------------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------------------------*/

static void* _evnThread(void *params)
{
    PollHdr_t* header = (PollHdr_t*)params;

    int i = 0;
    int descLen = sizeof(header->pollDesc) / sizeof(header->pollDesc[0]);

    for (i = 0; i < descLen; i++)
    {
        if (header->pollDesc[i].inUse)
        {
            _connect(header, i);
        }
    }

    while (_poll(header));

    return NULL;
}


static void* _cmdThread(void *params)
{
    Logger.writeLog(LOG_DEBUG, "Poller._cmdThread: Running cmdThread");
    PollHdr_t* header = (PollHdr_t*)params;

    while (true)
    {
        Cmd_t* cmd = SafeQueue.pop(header->cmdQueue);
        if (cmd == NULL)
        {
            Logger.writeLog(LOG_DEBUG, "Poller._cmdThread: cmd Null skipped");
            continue;
        }

        pthread_mutex_lock(&header->mutex);

        Kid_t cmdKid = UniqueKey.type("cmd").parser(cmd->key);

        // @pxhoang: The destination to send is depending on the key
        for (int i = 0; i < header->last; i++)
        {
            if (header->pollDesc[i].inUse)
            {
                if (cmdKid.cmd.domain == header->pollDesc[i].ioHeader->domain)
                    _send(header, i, (uint8_t*)cmd->msg, sizeof(PacketHeader) + cmd->msg->header.len);

                header->waitRsp = true;
            }
        }

        struct timespec expirytime;
        struct timeval curtime;
        int result = 0;

        gettimeofday(&curtime, NULL);
        expirytime.tv_sec = curtime.tv_sec + 3;
        expirytime.tv_nsec = (curtime.tv_usec * 1000);

        while (header->waitRsp)
        {
            result = pthread_cond_timedwait(&header->cond, &header->mutex, &expirytime);
            if (result == ETIMEDOUT)
                Logger.writeLog(LOG_DEBUG, "%s: Command request timeout\n", __FUNCTION__);
            header->waitRsp = false;
        }

        header->waitRsp = false;
        sleep(1);
        Cmder.destroy(cmd);
        pthread_mutex_unlock(&header->mutex);
    }

    return NULL;
}


_Poller const Poller = {_create, _destroy, _insert, _remove, _start, _stop, _ready, _request};