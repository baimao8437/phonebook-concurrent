#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include "stopwatch.h"

#include IMPL

#ifndef OPT
#define OUTPUT_FILE "orig.txt"

#else
#include "text_align.h"
#include "debug.h"
#include <fcntl.h>
#define ALIGN_FILE "align.txt"
#define OUTPUT_FILE "opt.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

#endif

#define DICT_FILE "./dictionary/words.txt"

int main(int argc, char *argv[])
{
#ifndef OPT
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
#endif
    Stopwatch_struct *timer = Stopwatch_new();
    double cpu_time1, cpu_time2;

    /* File preprocessing */
#ifndef OPT
    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return -1;
    }
#else
    text_align(DICT_FILE, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    off_t file_size = fsize(ALIGN_FILE);
#endif

    /* Build the entry */
    entry *pHead, *e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

#if defined(OPT)
    char *map;
    entry *entry_pool;
    pthread_t threads[THREAD_NUM];
    thread_arg *thread_args[THREAD_NUM];

    /* Start timing */
    Stopwatch_start(timer);

    /* Allocate the resource at first */
    map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");
    entry_pool = (entry *)malloc(sizeof(entry) *
                                 file_size / MAX_LAST_NAME_SIZE);
    assert(entry_pool && "entry_pool error");

    /* Prepare for multi-threading */
    pthread_setconcurrency(THREAD_NUM + 1);

    for (int i = 0; i < THREAD_NUM; i++)
        // Created by malloc, remeber to free them.
        thread_args[i] = createThread_arg(map + MAX_LAST_NAME_SIZE * i, map + file_size, i,
                                          THREAD_NUM, entry_pool + i);

    /* Deliver the jobs to all threads and wait for completing */
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create(&threads[i], NULL, (void *)&append, (void *)thread_args[i]);

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    /* Connect the linked list of each thread, saperate the head */
    pHead = thread_args[0]->lEntry_head->pNext;
    DEBUG_LOG("Connect %d head string %s %p\n", 0,
              pHead->lastName, thread_args[0]->data_begin);
    e = thread_args[0]->lEntry_tail;
    DEBUG_LOG("Connect %d tail string %s %p\n", 0,
              e->lastName, thread_args[0]->data_begin);
    DEBUG_LOG("round %d\n", 0);

    for (int i = 1; i < THREAD_NUM; i++) {
        e->pNext = thread_args[i]->lEntry_head->pNext;
        DEBUG_LOG("Connect %d head string %s %p\n", i,
                  e->pNext->lastName, thread_args[i]->data_begin);
        e = thread_args[i]->lEntry_tail;
        DEBUG_LOG("Connect %d tail string %s %p\n", i,
                  e->lastName, thread_args[i]->data_begin);
        DEBUG_LOG("round %d\n", i);
    }
    /* Stop timing */
    Stopwatch_stop(timer);

#else /* ! OPT */
    pHead = (entry *) malloc(sizeof(entry));
    e = pHead;
    e->pNext = NULL;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Start timing */
    Stopwatch_start(timer);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    /* Stop timing */
    Stopwatch_stop(timer);

    /* close file as soon as possible */
    fclose(fp);
#endif

    cpu_time1 = Stopwatch_read(timer);

    /* Find the given entry */
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
    e = pHead;

    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL " ? ");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Compute the execution time */
    Stopwatch_reset(timer);
    Stopwatch_start(timer);
    findName(input, e);
    cpu_time2 = Stopwatch_read(timer);

    /* Write the execution time to file. */
    FILE *output;
    output = fopen(OUTPUT_FILE, "a");
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);

    /* Release memory */
    Stopwatch_delete(timer);
#ifndef OPT
    while (pHead) {
        e = pHead;
        pHead = pHead->pNext;
        free(e);
    }
#else
    /* Free the allocated detail entry */
    e = pHead;
    while (e) {
        free(e->dtl);
        e = e->pNext;
    }

    free(entry_pool);
    for (int i = 0; i < THREAD_NUM; ++i)
        free(thread_args[i]);

    munmap(map, file_size);
    close(fd);
#endif
    return 0;
}
