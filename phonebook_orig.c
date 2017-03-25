#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "phonebook_orig.h"

/* original version */
entry *findName(char lastname[], entry *pHead)
{
    while (pHead) {
        if (strcasecmp(lastname, pHead->lastName) == 0)
            return pHead;
        pHead = pHead->pNext;
    }
    return NULL;
}

entry *append(char lastName[], entry *e)
{
    /* allocate memory for the new entry and put lastName */
    e->pNext = (entry *) malloc(sizeof(entry));
    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

entry *deleteName(char lastname[], entry *pHead)
{
    entry *first = pHead;
    entry *previous = 0;
    while (pHead) {
        if (strcasecmp(lastname, pHead->lastName) == 0)
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

void show_entry(entry *pHead)
{
    while (pHead) {
        printf("%s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
}
