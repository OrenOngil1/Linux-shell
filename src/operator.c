#include "operator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_SIMPLE_OPS (sizeof simple_ops / sizeof simple_ops[0])

const Entry simple_ops[] = {
    {"&&", 4},
    {"||", 4},
    {"(", 1},
    {")", 1},
    {"<", 2},
    {">", 2},
    {"|", 3},
    {"&", 5},
    {";", 5}
};

char *get_op(char *line)
{
    int length;

    for(int i = 0; i < NUM_SIMPLE_OPS; i++) {
        length = strlen(simple_ops[i].key);

        if(strncmp(simple_ops[i].key, line, length) == 0) {
            return strdup(simple_ops[i].key);
        }
    }

    return NULL;
}


int is_op(char *token)
{
    return precedence(token) != -1;
}

int precedence(char *op)
{
    for(int i = 0; i < NUM_SIMPLE_OPS; i++) {
        if(strcmp(simple_ops[i].key, op) == 0) {
            return simple_ops[i].value;
        }
    }

    return -1;
}