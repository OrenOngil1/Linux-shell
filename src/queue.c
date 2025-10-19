#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

Link *init_link(void *value)
{
    Link *link = malloc(sizeof(Link));
    if(link == NULL) {
        perror("malloc error on link initialization");
        exit(1);
    }

    link->element = value;
    link->next = NULL;
    link->prev = NULL;

    return link;
}

Queue *init_queue()
{
    Queue *queue = malloc(sizeof(Queue));
    if(queue == NULL) {
        perror("malloc error on queue initialization");
        exit(1);
    }

    queue->start = NULL;
    queue->end = NULL;
    queue->size = 0;

    return queue;
}

void free_queue(Queue *queue)
{

    if(queue != NULL) {

        while(queue->size > 0) {
            free(dequeue(queue));
        }

        free(queue);
    }
}

void enqueue(Queue *queue, void *element)
{
    Link *new_link;

    new_link = init_link(element);

    if(queue->size == 0) {
        queue->end = new_link;
        queue->start = queue->end;

    } else {
        queue->end->next = new_link;
        new_link->prev = queue->end;
        queue->end = new_link;
    }

    queue->size++;
}

void *dequeue(Queue *queue)
{
    void *ret;
    Link *link_ret = queue->start;

    ret = queue->start->element;
    
    if(queue->size-- == 1) {
        queue->start = NULL;
        queue->end = NULL;
    } else {
        queue->start = link_ret->next;
        queue->start->prev = NULL;
    }

    free(link_ret);

    return ret;
}

void *front(Queue *queue)
{
    return queue->start->element;
}

//basic testing for the queue
// int main()
// {
//     char *one, *two, *three; 
//     Link *curr;
//     Queue *queue = init_queue();
//     enqueue(queue, "1", 2);
//     enqueue(queue, "2", 2);
//     enqueue(queue, "3", 2);
//     curr = queue->start;
//     printf("[");
//     while(curr != NULL) {
//         printf("%s", (char *)curr->element->value);
//         if(curr->next != NULL) {
//             printf(", ");
//         }
//         curr = curr->next;
//     }
//     printf("]\n");
//     one = (char *)dequeue(queue);
//     printf("dequeuing first element in queue: %s\n", one);
//     printf("peeking at next element of queue: %s\n", (char *)peek(queue));
//     two = (char *)dequeue(queue);
//     printf("dequeuing next element in queue: %s\n", two);
//     three = (char *)dequeue(queue);
//     printf("dequeuing next element in queue: %s\n", three);
//     free_queue(queue);
//     free(one); free(two); free(three);
//     return 0;
// }