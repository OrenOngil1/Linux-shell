#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>

//Definition of Queue data structure for generic pointers (void *).

//Double link structure for queue.
typedef struct link {
    void *element;
    struct link *next;
    struct link *prev;
} Link;

//Queue is implemented as doubly linked list
typedef struct queue {
    Link *start;
    Link *end;
    size_t size;
} Queue;

//Creates Queue of pointers of capacity 100.
Queue *init_queue();

//Frees all memory allocated to this Queue.
void free_queue(Queue *Queue);

//Adds element to the end of the queue, if queue is full may cause undefined behavior.
void enqueue(Queue *queue, void *element);

//Returns and removes the string at the beginning of the queue if not empty, otherwise may cause undefined behavior.
void* dequeue(Queue *queue);

//Returns a pointer to the string at the beginning of the queue if not empty, otherwise may cause undefined behavior.
void* front(Queue *queue);

#endif