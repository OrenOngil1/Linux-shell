#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

//Definition of stack data structure for pointer of Elements.
typedef struct stack
{
    void **s;
    size_t size;
    size_t capacity;
} Stack;

//Creates stack of pointers of capacity 100.
Stack *init_stack();

//Frees all memory allocated to this stack. Frees all present stack entries.
void free_stack(Stack *stack);

//Adds element to the top of the stack, if stack is full will print error message and exit.
void push(Stack *stack, void *element);

//Returns and removes the element at the top of the stack if not empty, if stack is empty will print error message and exit.
void* pop(Stack *stack);

//Returns a pointer to the element at the top of the stack if not empty, if stack is empty will print error message and exit.
void* peek(Stack *stack);

#endif