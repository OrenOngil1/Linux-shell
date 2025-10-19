#include "stack.h"
#include <stdio.h> 

#define CAPACITY 100

Stack *init_stack()
{
    Stack *stack = malloc(sizeof(Stack));
    if(stack == NULL) {
        perror("Error on malloc of stack");
        exit(1);
    }

    stack->s = calloc(CAPACITY,  sizeof(void *));
    if(stack->s == NULL) {
        perror("Error on malloc in stack");
        exit(1);
    }

    stack->size = 0;
    stack->capacity = CAPACITY;

    return stack;
}

void free_stack(Stack *stack)
{
    if(stack != NULL) {
        for(int i = 0; i < stack->size; i++) {
            free(stack->s[i]);
        }

        free(stack->s);
        
        free(stack);
    }
}

void push(Stack *stack, void *element)
{
    if(element == NULL) {
        fprintf(stderr, "Stack error: Cannot push NULL into stack\n");
        exit(1);
    }

    if(stack->size == stack->capacity && (stack->s = realloc(stack->s, stack->size *2)) == NULL) {
        perror("Error on memory reallocation of stack");
        exit(1);
    }

    stack->s[stack->size++] = element;
}

void *pop(Stack *stack)
{
    if(stack->size == 0) {
        fprintf(stderr, "Stack error: Attampted pop when stack is empty\n");
        exit(1);
    }
    return stack->s[--stack->size];
}

void *peek(Stack *stack)
{
    if(stack->size == 0) {
        fprintf(stderr, "Stack error: Attampted peek when stack is empty\n");
        exit(1);
    }
    return stack->s[stack->size - 1];
}


//basic testing for the stack
// int main()
// {
//     char *one, *two, *three;
//     Stack *stack = init_stack();
//     push(stack, "1", 2);
//     push(stack, "2", 2);
//     push(stack, "3", );
//     printf("stack is: [%s, %s, %s]\n", (char *)stack->s[0]->value, (char *)stack->s[1]->value, (char *)stack->s[2]->value);
//     three = (char *)pop(stack);
//     printf("poping first element in stack: %s\n", three);
//     printf("peeking at next element of stack: %s\n", (char *)peek(stack));
//     two = (char *)pop(stack);
//     printf("poping next element in stack: %s\n", two);
//     one = (char *)pop(stack);
//     printf("poping next element in stack: %s\n", one);
//     free_stack(stack);
//     free(one); free(two); free(three);
//     return 0;
// }
