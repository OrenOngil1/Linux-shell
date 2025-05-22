#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "strQueue.h"

char* dequeue(strQueue *queue)
{
    if(queue->size == 0) return NULL;
    char *ret = calloc(strlen(queue->first->line) + 1, 1);
    strcpy(ret, queue->first->line);
    free(queue->first->line);
    queue->size--;
    if(queue->size == 0) {
        free(queue->first); queue->first = NULL;
        free(queue->last); queue->last = NULL;
    }
    else
    {
        queue->first = queue->first->next;
        free(queue->first->prev);
    }
    return ret;
}

void enqueue(strQueue *queue, char* line)
{
    strLink *link = malloc(sizeof(strLink));
    line[strcspn(line, "\n")] = '\0';
    link->line = strdup(line); link->next = NULL; link->prev = NULL;
    if(queue->size == 0)
    {
        queue->first = link;
        queue->last = link;
    }
    else 
    {
        queue->last->next = link;
        link->prev = queue->last;
        queue->last = link;
    }
    if(queue->size + 1 > HISTLEN) free(dequeue(queue));
    queue->size++;
}

void freeStrQueue(strQueue *queue)
{
    if(!queue) return;
    strLink *curr = queue->first;
    strLink *temp;
    while(curr)
    {
        temp = curr;
        curr = curr->next;
        free(temp->line);
        free(temp);
    }
    free(queue);
    queue = NULL;
}

void printStrQueue(strQueue *queue)
{
    strLink *curr = queue->first;
    int entry = 1;
    while(curr)
    {
        printf("%i %s\n", entry++, curr->line);
        curr = curr->next;
    }
}