#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "stopwatch.h"

typedef struct timespec Time;

struct Stopwatch_struct {
    bool running;
    Time last_time;
    Time total;
};

static Time clock_time()
{
    Time time_now;
    clock_gettime(CLOCK_REALTIME, &time_now);
    return time_now;
}

static Time timeDiff(Time t1, Time t2)
{
    Time diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return diff;
}

static Time timeAdd(Time t1, Time t2)
{
    long sec = t2.tv_sec + t1.tv_sec;
    long nsec = t2.tv_nsec + t1.tv_nsec;
    if (nsec >= 1000000000) {
        nsec -= 1000000000;
        sec++;
    }
    return (Time) {
        .tv_sec = sec, .tv_nsec = nsec
    };
}

watch_p create(void)
{
    watch_p S = malloc(sizeof(struct Stopwatch_struct));
    if (!S)
        return NULL;

    S->running = false;
    S->last_time = (Time) {
        0, 0
    };
    S->total = (Time) {
        0, 0
    };
    return S;
}

void destroy(watch_p S)
{
    free(S);
}

/* Start resets the timer to 0.0; use resume for continued total */

void start(watch_p Q)
{
    if (!(Q->running)) {
        Q->running = true;
        Q->total = (Time) {
            0, 0
        };
        Q->last_time = clock_time();
    }
}

/* Reset and start */

void restart(watch_p Q)
{
    Q->running = true;
    Q->total = (Time) {
        0, 0
    };
    Q->last_time = clock_time();
}

/*
    Resume timing, after stopping.  (Does not wipe out
        accumulated times.)
*/

void resume(watch_p Q)
{
    if (!(Q->running)) {
        Q-> last_time = clock_time();
        Q->running = true;
    }
}

void stop(watch_p Q)
{
    if (Q->running) {
        Q->total = timeAdd(Q->total, timeDiff((Q->last_time), clock_time()));
        Q->running = false;
    }
}

double read(watch_p Q)
{
    if (Q->running) {
        Time t = clock_time();
        Q->total = timeAdd(Q->total, timeDiff(Q->last_time, t));
        Q->last_time = t;
    }
    return (Q->total.tv_sec * 1000000.0 + Q->total.tv_nsec / 1000.0) / 1000000.0;
}

/* API gateway */
struct __STOPWATCH_API__ Stopwatch = {
    .create = create,
    .destroy = destroy,
    .start = start,
    .restart = restart,
    .stop = stop,
    .resume = resume,
    .read = read,
};