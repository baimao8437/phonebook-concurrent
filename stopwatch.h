#ifndef _STOPWATCH_H
#define _STOPWATCH_H

typedef struct Stopwatch_struct *watch_p;

extern struct __STOPWATCH_API__ {
    watch_p(*create)();
    void (*destroy)(watch_p);

    void (*start)(watch_p);
    void (*restart)(watch_p);
    void (*stop)(watch_p);
    void (*resume)(watch_p);

    double (*read)(watch_p);
} Stopwatch;

#endif
