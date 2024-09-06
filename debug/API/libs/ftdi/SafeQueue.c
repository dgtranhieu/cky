#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "SafeQueue.h"


typedef struct Qelement_t
{
    void* next;
    void* value;
}Qelement_t;


typedef struct QueueHdr_t
{
    Qelement_t* head;
    Qelement_t* tail;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
}QueueHdr_t;


static QueueHdr_t* create();
static void destroy(QueueHdr_t* header);
static void push(QueueHdr_t* header, void* elem);
static void* pop(QueueHdr_t* header);


static QueueHdr_t* _create()
{
    QueueHdr_t* header = malloc(sizeof(QueueHdr_t));

    header->head     = NULL;
    header->tail     = NULL;
    header->mutex    = malloc(sizeof(pthread_mutex_t));
    header->cond     = malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(header->mutex, NULL);
    pthread_cond_init(header->cond, NULL);
    // *(header->mutex) = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    // *(header->cond)  = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    return header;
}


static void _destroy(QueueHdr_t* header)
{
    if (header->mutex) free(header->mutex);
    if (header->cond) free(header->cond);
    if (header) free(header);
    header = NULL;
}


static void _push(QueueHdr_t* header, void* elem)
{
    pthread_mutex_lock(header->mutex);
    Qelement_t* element = malloc(sizeof(Qelement_t));
    element->value   = elem;
    element->next    = NULL;

    if (header->head == NULL)
    {
        header->head = element;
        header->tail = element;
    }
    else
    {
        Qelement_t* oldTail = header->tail;
        oldTail->next    = element;
        header->tail     = element;
    }

    pthread_cond_signal(header->cond);
    pthread_mutex_unlock(header->mutex);
}


static void* _pop(QueueHdr_t* header)
{
    pthread_mutex_lock(header->mutex);

    if (header->head == NULL)
    {
        pthread_cond_wait(header->cond, header->mutex);
    }

    Qelement_t* head = header->head;
    header->head = head->next;

    // Get head and free element memory
    void* value = head->value;
    free(head);

    pthread_mutex_unlock(header->mutex);
    return value;
}


SafeQueue_t const SafeQueue = {_create, _destroy, _push, _pop};