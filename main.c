#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stopwatch.h"
#include "phonebook.h"

#ifndef OPT
#define OUTPUT_FILE "orig.txt"
#else
#define OUTPUT_FILE "opt.txt"
#endif

#define DICT_FILE "./dictionary/words.txt"

#define DELETE(name)\
{\
    pHead = Phonebook.deleteName(name, pHead);\
    e = pHead;\
}

int main(int argc, char *argv[])
{
    /* Build the entry */
    entry pHead, e;

    watch_p timer = Stopwatch.create();

    /* File append processing */
    pHead = Phonebook.fileAppend(DICT_FILE, timer);
    double appendTime = Stopwatch.read(timer);

    /* Find the given entry */
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
    e = pHead;

    assert(Phonebook.findName(input, e) && "The name is not in list");

    /* Can delete name from list here */
    // deletion at head
    char delete_name[MAX_LAST_NAME_SIZE] = "aaaa";
    DELETE(delete_name);
    // deletion at middle
    strcpy(delete_name, "zyxels");
    DELETE(delete_name);
    // deletion at tail
    strcpy(delete_name, "zzzzzzzz");
    DELETE(delete_name);

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

    /* Find name in list */
    Stopwatch.restart(timer);
    assert(Phonebook.findName(input, e) && "The name has been deleted");
    double findTime = Stopwatch.read(timer);

    /* Write the execution time to file. */
    FILE *output;
    output = fopen(OUTPUT_FILE, "a");
    fprintf(output, "append() findName() %lf %lf\n", appendTime, findTime);
    fclose(output);

    printf("execution time of append() : %lf sec\n", appendTime);
    printf("execution time of findName() : %lf sec\n", findTime);

    /* Release memory */
    Stopwatch.destroy(timer);
    Phonebook.release(pHead);

    return 0;
}
