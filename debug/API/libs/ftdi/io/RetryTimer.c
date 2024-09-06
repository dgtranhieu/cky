#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <strings.h>
#include "RetryTimer.h"


static RetryHdr_t* _create(RetryTimerMode mode, long millisecs);
static void _start(RetryHdr_t* header);
static void _stop(RetryHdr_t* header);
static void _destroy(RetryHdr_t* header);



static RetryHdr_t* _create(RetryTimerMode mode, long millisecs)
{
    RetryHdr_t* header = malloc(sizeof(RetryHdr_t));

    header->fdDesc                         = -1;
    header->mode                           = mode;
    header->millisecs                      = millisecs;
    header->timerSpecs.it_value.tv_sec     = (millisecs * 1000000) / 1000000000;
    header->timerSpecs.it_value.tv_nsec    = (millisecs * 1000000) % 1000000000;
    header->timerSpecs.it_interval.tv_sec  = (header->mode == RETRY_PERIODIC) ? (millisecs * 1000000) / 1000000000 : 0;
    header->timerSpecs.it_interval.tv_nsec = (header->mode == RETRY_PERIODIC) ? (millisecs * 1000000) % 1000000000 : 0;

    return header;
}


static void _start(RetryHdr_t* header)
{
    // _stop(header); // @pxhoang: Close previous session before using

    header->fdDesc = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (header->fdDesc == -1)
		printf("pxhoang: %s - Failed to run timer\n", __FUNCTION__);

    if (timerfd_settime(header->fdDesc, 0, &header->timerSpecs, NULL) != 0)
        printf("pxhoang: %s - Failed to start timer\n", __FUNCTION__);
}


static void _stop(RetryHdr_t* header)
{
    struct itimerspec timerSpecs;
    timerSpecs.it_value.tv_sec     = 0;
    timerSpecs.it_value.tv_nsec    = 0;
    timerSpecs.it_interval.tv_sec  = 0;
    timerSpecs.it_interval.tv_nsec = 0;

    if (timerfd_settime(header->fdDesc, 0, &timerSpecs, NULL) != 0)
        printf("pxhoang: Failed to stop timer\n");

    close(header->fdDesc);
}


static void _destroy(RetryHdr_t* header)
{
    _stop(header);
    close(header->fdDesc);
    free(header);
}


_RetryTimer const RetryTimer = {_create, _start, _stop, _destroy};