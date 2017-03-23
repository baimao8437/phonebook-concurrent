#include <stdbool.h>
#include <time.h>

typedef struct timespec Time;

typedef struct {
    bool running;
    Time last_time;
    Time total;
} *Stopwatch, Stopwatch_struct;

void Stopwatch_reset(Stopwatch Q);
Stopwatch Stopwatch_new(void);
void Stopwatch_delete(Stopwatch S);
void Stopwatch_start(Stopwatch Q);
void Stopwatch_resume(Stopwatch Q);
void Stopwatch_stop(Stopwatch Q);
double Stopwatch_read(Stopwatch Q);