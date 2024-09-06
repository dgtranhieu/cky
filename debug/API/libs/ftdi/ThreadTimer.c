#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "ThreadTimer.h"


typedef struct ThreadTimerHdr_t
{
    timer_t tid;
    struct sigevent sigevent;
    struct itimerspec specs;
    ThreadTimerHandler_t handler;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
}ThreadTimerHdr_t;


static ThreadTimerHdr_t* _create(ThreadTimerHandler_t handler)
{
    printf("pxhoang: %s\n", __FUNCTION__);
    ThreadTimerHdr_t* header = malloc(sizeof(ThreadTimerHdr_t));
    memset(&header->sigevent, 0, sizeof(struct sigevent));

    header->sigevent.sigev_value.sival_int = 111;           // It also identifies the timer, the callback function can be obtained
    header->sigevent.sigev_notify          = SIGEV_THREAD;  // The way of thread notification, send a new thread
    header->sigevent.sigev_notify_function = handler;       // Thread function address

    if (timer_create(CLOCK_REALTIME, &header->sigevent, &header->tid) == -1)
        printf("pxhoang: Failed to create timer");

    return header;
}


static void _start(ThreadTimerHdr_t* header, long long nanosecs, TimerMode_t mode)
{
    printf("pxhoang: %s - nanosecs = %lld\n", __FUNCTION__, nanosecs);
    switch(mode)
    {
        case PERIODIC: // starts after specified period of nanoseconds
            header->specs.it_value.tv_sec     = nanosecs / 1000000000;
            header->specs.it_value.tv_nsec    = nanosecs % 1000000000;
            header->specs.it_interval.tv_sec  = nanosecs / 1000000000;
            header->specs.it_interval.tv_nsec = nanosecs % 1000000000;
            break;

        case ONESHOT: // fires once after specified period of nanoseconds
            header->specs.it_value.tv_sec     = nanosecs / 1000000000;
            header->specs.it_value.tv_nsec    = nanosecs % 1000000000;
            header->specs.it_interval.tv_sec  = 0;
            header->specs.it_interval.tv_nsec = 0;
            printf("pxhoang: %s - it_value.tv_sec = %ld\n", __FUNCTION__, header->specs.it_value.tv_sec);
            printf("pxhoang: %s - it_value.tv_nsec = %ld\n", __FUNCTION__, header->specs.it_value.tv_nsec);
            break;

        default:
            break;
    }

    if (timer_settime(header->tid, 0, &header->specs, NULL) == -1)
    {
        printf("pxhoang: Failed to start timer\n");
        return;
    }
}


static void _stop(ThreadTimerHdr_t* header)
{
    printf("pxhoang: %s\n", __FUNCTION__);
    struct itimerspec newTimerSpecs;
    newTimerSpecs.it_value.tv_sec     = 0;
    newTimerSpecs.it_value.tv_nsec    = 0;
    newTimerSpecs.it_interval.tv_sec  = 0;
    newTimerSpecs.it_interval.tv_nsec = 0;
    timer_settime(header->tid, 0, &newTimerSpecs, &header->specs);
}


static void _destroy(ThreadTimerHdr_t* header)
{
    _stop(header);
    timer_delete(header->tid);
}

ThreadTimer_t const ThreadTimer = {_create, _start, _stop, _destroy};