#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

typedef struct QueueHdr_t QueueHdr_t;

typedef struct SafeQueue_t
{
    QueueHdr_t* (*const create)();
    void (*const destroy)(QueueHdr_t* header);
    void (*const push)(QueueHdr_t* header, void* elem);
    void* (*const pop)(QueueHdr_t* header);
}SafeQueue_t;


extern SafeQueue_t const SafeQueue;

#endif // SAFE_QUEUE_H