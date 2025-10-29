#include "operator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_SIMPLE_OPS (sizeof simple_ops / sizeof simple_ops[0] - 1)

const Operator simple_ops[] = {
    {"&&", 3, 4},
    {"||", 3, 4},
    {"(", 0, 0},
    {")", 0, 0},
    {"<", 7, 8},
    {">", 7, 8},
    {"|", 5, 6},
    {"&", 1, 2},
    {";", 1, 2},
    {NULL, -1, -1} //dummy
};

char *dup_op(char *line)
{
    int length;

    for(int i = 0; i < NUM_SIMPLE_OPS; i++) {
        length = strlen(simple_ops[i].str);

        if(strncmp(simple_ops[i].str, line, length) == 0) {
            return strdup(simple_ops[i].str);
        }
    }

    return NULL;
}


int is_op(char *token)
{
    return get_op(token).str != NULL;
}


Operator get_op(char *op)
{
    for(int i = 0; i < NUM_SIMPLE_OPS; i++) {
        if(strcmp(op, simple_ops[i].str) == 0) {
            return simple_ops[i];
        }
    }

    return simple_ops[NUM_SIMPLE_OPS];
}

int is_terminator(char *op)
{
    return strcmp(op, ";") == 0 || strcmp(op, "&") == 0;
}


int is_close_bracket(char *op)
{
    return strcmp(op, ")") == 0;
}