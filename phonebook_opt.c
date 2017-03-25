#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#include "phonebook.h"
#include "debug.h"
#include "text_align.h"

#define ALIGN_FILE "align.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

#define THREAD_SET(l)\
{\
    thread_args[l] = createThread_arg(map + MAX_LAST_NAME_SIZE * l, map + file_size, l,\
                                          THREAD_NUM, entry_pool + l);\
    pthread_create(&threads[l], NULL, (void *)&append, (void *)thread_args[l]);\
}

typedef struct __PHONE_BOOK_DETAIL {
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
} detail;

struct __PHONE_BOOK_ENTRY {
    char *lastName;
    struct __PHONE_BOOK_ENTRY *pNext;
    struct __PHONE_BOOK_DETAIL *dtl;
};

typedef struct _thread_argument {
    char *data_begin;
    char *data_end;
    int threadID;
    int numOfThread;
    entry lEntryPool_begin;    /* The local entry pool */
    entry lEntry_head;         /* local entry linked list */
    entry lEntry_tail;         /* local entry linked list */
} thread_arg;

static char *map;
static entry entry_pool;
static pthread_t threads[THREAD_NUM];
static thread_arg *thread_args[THREAD_NUM];
static off_t file_size;
static int fd;

static thread_arg *createThread_arg(char *data_begin, char *data_end,
                                    int threadID, int numOfThread,
                                    entry entryPool)
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

/* Generate a local linked list in thread. */
static void append(void *arg)
{
    watch_p cpu_time = Stopwatch.create();

    Stopwatch.start(cpu_time);

    thread_arg *t_arg = (thread_arg *) arg;

    int count = 0;
    entry j = t_arg->lEntryPool_begin;
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

static void alignAndMap(char fileName[])
{
    text_align(fileName, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    file_size = fsize(ALIGN_FILE);
    fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);

    map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");

    // Remove the align.txt file
    assert(!remove(ALIGN_FILE) && "File remove falid");
    close(fd);
}

static entry threadProcess()
{
    entry pHead, e;

    /* Prepare for multi-threading */
    pthread_setconcurrency(THREAD_NUM + 1);

    /* Deliver the jobs to all threads and wait for completing */
    for (int i = 0; i < THREAD_NUM; i++)
        THREAD_SET(i);

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    /* Connect the linked list of each thread, saperate the head */
    pHead = thread_args[0]->lEntry_head;
    DEBUG_LOG("Connect %d head string %s %p\n", 0,
              pHead->lastName, thread_args[0]->data_begin);
    e = thread_args[0]->lEntry_tail;
    DEBUG_LOG("Connect %d tail string %s %p\n", 0,
              e->lastName, thread_args[0]->data_begin);
    DEBUG_LOG("round %d\n", 0);

    for (int i = 1; i < THREAD_NUM; i++) {
        e->pNext = thread_args[i]->lEntry_head;
        DEBUG_LOG("Connect %d head string %s %p\n", i,
                  e->pNext->lastName, thread_args[i]->data_begin);
        e = thread_args[i]->lEntry_tail;
        DEBUG_LOG("Connect %d tail string %s %p\n", i,
                  e->lastName, thread_args[i]->data_begin);
        DEBUG_LOG("round %d\n", i);
    }
    return pHead;
}

entry fileAppend(char fileName[], watch_p timer)
{
    printf("size of entry : %lu bytes\n", sizeof(struct __PHONE_BOOK_ENTRY));

    alignAndMap(fileName);

    /* Allocate the resource at first */
    entry_pool = (entry)malloc(sizeof(struct __PHONE_BOOK_ENTRY) *
                               file_size / MAX_LAST_NAME_SIZE);
    assert(entry_pool && "entry_pool error");

    Stopwatch.start(timer);
    entry pHead = threadProcess();
    Stopwatch.stop(timer);

    return pHead;
}

entry findName(char lastName[], entry pHead)
{
    size_t len = strlen(lastName);
    while (pHead) {
        if (strncasecmp(lastName, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            if (!pHead->dtl)
                pHead->dtl = (detail *) malloc(sizeof(detail));
            return pHead;
        }
        DEBUG_LOG("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}

entry deleteName(char lastName[], entry pHead)
{
    entry first = pHead;
    entry previous = 0;
    size_t len = strlen(lastName);
    while (pHead) {
        if (strncasecmp(lastName, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0'))
            break;
        previous = pHead;
        pHead = pHead->pNext;
    }

    if (!previous) {
        // deletion at head
        first = pHead -> pNext;
        free(pHead->dtl);
    } else {
        if (pHead->pNext) {
            // deletion at middle
            previous->pNext = pHead->pNext;
            free(pHead->dtl);
        } else {
            // deletion at tail
            previous->pNext = 0;
            free(pHead->dtl);
        }
    }
    return first;
}

void display(entry pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}

void release(entry pHead)
{
    entry e;
    e = pHead;
    while (e) {
        free(e->dtl);
        e = e->pNext;
    }

    free(entry_pool);

    for (int i = 0; i < THREAD_NUM; ++i)
        free(thread_args[i]);

    munmap(map, file_size);
}

/* API gateway */
struct __PHONE_BOOK_API__ Phonebook = {
    .fileAppend = fileAppend,
    .deleteName = deleteName,
    .findName = findName,
    .display = display,
    .release = release,
};