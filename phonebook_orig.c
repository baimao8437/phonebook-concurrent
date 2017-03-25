#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "phonebook.h"

/* original version */
struct __PHONE_BOOK_ENTRY {
    char lastName[MAX_LAST_NAME_SIZE];
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
    struct __PHONE_BOOK_ENTRY *pNext;
};

static entry append(char lastName[], entry e)
{
    /* allocate memory for the new entry and put lastName */
    e->pNext = (entry) malloc(sizeof(struct __PHONE_BOOK_ENTRY));
    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

static entry naiveAppend(FILE* fp)
{
    entry pHead, e;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];

    pHead = (entry) malloc(sizeof(struct __PHONE_BOOK_ENTRY));
    e = pHead;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }
    return pHead;
}

entry fileAppend(char fileName[], watch_p timer)
{
    printf("size of entry : %lu bytes\n", sizeof(struct __PHONE_BOOK_ENTRY));

    /* File preprocessing */
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return 0;
    }

    Stopwatch.start(timer);
    entry pHead = naiveAppend(fp);
    Stopwatch.stop(timer);

    /* close file as soon as possible */
    fclose(fp);
    return pHead;
}

entry findName(char lastName[], entry pHead)
{
    while (pHead) {
        if (strcasecmp(lastName, pHead->lastName) == 0)
            return pHead;
        pHead = pHead->pNext;
    }
    return NULL;
}

entry deleteName(char lastName[], entry pHead)
{
    entry first = pHead;
    entry previous = 0;
    while (pHead) {
        if (strcasecmp(lastName, pHead->lastName) == 0)
            break;
        previous = pHead;
        pHead = pHead->pNext;
    }
    if (!previous) {
        // delete at head
        first = pHead -> pNext;
        free(pHead);
    } else {
        if (pHead->pNext) {
            // delete at middle
            previous->pNext = pHead->pNext;
            free(pHead);
        } else {
            // delete at tail
            previous->pNext = 0;
            free(pHead);
        }
    }
    return first;
}

void display(entry pHead)
{
    while (pHead) {
        printf("%s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
}

void release(entry pHead)
{
    entry e;
    while (pHead) {
        e = pHead;
        pHead = pHead->pNext;
        free(e);
    }
}

/* API gateway */
struct __PHONE_BOOK_API__ Phonebook = {
    .fileAppend = fileAppend,
    .deleteName = deleteName,
    .findName = findName,
    .display = display,
    .release = release,
};