#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "history.h"

int isHistCmd(char *line)
{
    int a = 0;
    return !strcmp(line, "hist") || !strcmp(line, "!!") || sscanf(line, "!%i", &a) == 1;
}

char *getNonHistN(strQueue *queue, int n)
{
    if(n >= 1 && n <=queue->size)
    {
        int local_n = 1;
        strLink *curr = queue->first;
        while(curr && local_n++ < n) curr = curr->next;
        if(curr && !isHistCmd(curr->line)) return curr->line;
    }
    return NULL;
}

char *getNonHistLast(strQueue *queue)
{
    strLink *curr = queue->last;
    while(curr && isHistCmd(curr->line)) curr = curr->prev;
    if(curr) return curr->line;
    return NULL;
}