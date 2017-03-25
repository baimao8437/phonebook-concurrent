#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "phonebook_opt.h"
#include "debug.h"
#include "stopwatch.h"

entry *findName(char lastname[], entry *pHead)
{
    size_t len = strlen(lastname);
    while (pHead) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            if (!pHead->dtl)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        DEBUG_LOG("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}

thread_arg *createThread_arg(char *data_begin, char *data_end,
                             int threadID, int numOfThread,
                             entry *entryPool)
{
    thread_arg *new_arg = (thread_arg *) malloc(sizeof(thread_arg));

    new_arg->data_begin = data_begin;
    new_arg->data_end = data_end;
    new_arg->threadID = threadID;
    new_arg->numOfThread = numOfThread;
    new_arg->lEntryPool_begin = entryPool;
    new_arg->lEntry_head = new_arg->lEntry_tail = entryPool;
    return new_arg;
}

/**
 * Generate a local linked list in thread.
 */
void append(void *arg)
{
    watch_p cpu_time = Stopwatch.create();

    Stopwatch.start(cpu_time);

    thread_arg *t_arg = (thread_arg *) arg;

    int count = 0;
    entry *j = t_arg->lEntryPool_begin;
    for (char *i = t_arg->data_begin; i < t_arg->data_end;
            i += MAX_LAST_NAME_SIZE * t_arg->numOfThread,
            j += t_arg->numOfThread, count++) {
        /* Append the new at the end of the local linked list */
        t_arg->lEntry_tail->pNext = j;
        t_arg->lEntry_tail = t_arg->lEntry_tail->pNext;
        t_arg->lEntry_tail->lastName = i;
        t_arg->lEntry_tail->pNext = NULL;
        t_arg->lEntry_tail->dtl = NULL;
        DEBUG_LOG("thread %d t_argend string = %s\n",
                  t_arg->threadID, t_arg->lEntry_tail->lastName);
    }

    DEBUG_LOG("thread take %lf sec, count %d\n", Stopwatch.read(cpu_time), count);
    pthread_exit(NULL);
}

entry *deleteName(char lastname[], entry *pHead)
{
    entry *first = pHead;
    entry *previous = 0;
    size_t len = strlen(lastname);
    while (pHead) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0'))
            break;
        previous = pHead;
        pHead = pHead->pNext;
    }

    if (!previous) {
        // delete at head
        first = pHead -> pNext;
        free(pHead->dtl);
    } else {
        if (pHead->pNext) {
            // delete at middle
            previous->pNext = pHead->pNext;
            free(pHead->dtl);
        } else {
            // delete at tail
            previous->pNext = 0;
            free(pHead->dtl);
        }
    }
    return first;
}

void show_entry(entry *pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}
