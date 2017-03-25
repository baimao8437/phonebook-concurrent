#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#include "stopwatch.h"

#define MAX_LAST_NAME_SIZE 16

typedef struct __PHONE_BOOK_ENTRY *entry;

extern struct __PHONE_BOOK_API__ {
    entry (*findName)(char lastname[], entry pHead);
    entry (*fileAppend)(char fileName[], watch_p timer);
    entry (*deleteName)(char lastname[], entry pHead);
    void (*display)(entry pHead);
    void (*release)(entry pHead);
} Phonebook;

#endif
