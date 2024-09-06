
typedef int (*TimerCB)(void* params);


typedef enum
{
	RETRY_PERIODIC,
	RETRY_ONESHOT
} RetryTimerMode;


typedef struct RetryHdr_t
{
    int fdDesc;
    struct itimerspec timerSpecs;
    RetryTimerMode mode;
    long millisecs;
}RetryHdr_t;


typedef struct _RetryTimer
{
    RetryHdr_t* (*const create)(RetryTimerMode mode, long millisecs);
    void (*const start)(RetryHdr_t* header);
    void (*const stop)(RetryHdr_t* header);
    void (*const destroy)(RetryHdr_t* header);
}_RetryTimer;

extern _RetryTimer const RetryTimer;