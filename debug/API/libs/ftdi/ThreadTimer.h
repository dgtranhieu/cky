#ifndef THREAD_TIMER_H
#define THREAD_TIMER_H
#include <signal.h>

typedef struct ThreadTimerHdr_t ThreadTimerHdr_t;
typedef void (*ThreadTimerHandler_t)(union sigval v);

typedef enum TimerMode_t
{
	PERIODIC,
	ONESHOT
}TimerMode_t;

typedef struct ThreadTimer_t
{
    ThreadTimerHdr_t* (*const create)(ThreadTimerHandler_t handler);
    void (*const start)(ThreadTimerHdr_t* header, long long nanosecs, TimerMode_t mode);
    void (*const stop)(ThreadTimerHdr_t* header);
    void (*const destroy)(ThreadTimerHdr_t* header);
}ThreadTimer_t;

extern ThreadTimer_t const ThreadTimer;


#endif // THREAD_TIMER_H