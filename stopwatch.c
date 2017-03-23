#include <stdlib.h>
#include "stopwatch.h"

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

void Stopwatch_reset(Stopwatch Q)
{
    Q->running = false;
    Q->last_time = (Time) {
        0, 0
    };
    Q->total = (Time) {
        0, 0
    };
}

Stopwatch Stopwatch_new(void)
{
    Stopwatch S = (Stopwatch) malloc(sizeof(Stopwatch_struct));
    if (!S)
        return NULL;

    Stopwatch_reset(S);
    return S;
}

void Stopwatch_delete(Stopwatch S)
{
    free(S);
}

/* Start resets the timer to 0.0; use resume for continued total */

void Stopwatch_start(Stopwatch Q)
{
    if (!(Q->running)) {
        Q->running = true;  /* true */
        Q->total = (Time) {
            0, 0
        };
        Q->last_time = clock_time();
    }
}

/*
    Resume timing, after stopping.  (Does not wipe out
        accumulated times.)
*/

void Stopwatch_resume(Stopwatch Q)
{
    if (!(Q->running)) {
        Q-> last_time = clock_time();
        Q->running = true;
    }
}

void Stopwatch_stop(Stopwatch Q)
{
    if (Q->running) {
        Q->total = timeAdd(Q->total, timeDiff((Q->last_time), clock_time()));
        Q->running = false;
    }
}

double Stopwatch_read(Stopwatch Q)
{

    if (Q->running) {
        Time t = clock_time();
        Q->total = timeAdd(Q->total, timeDiff(Q->last_time, t));
        Q->last_time = t;
    }
    return (Q->total.tv_sec * 1000000.0 + Q->total.tv_nsec / 1000.0) / 1000000.0;
}
