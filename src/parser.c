#include "parser.h"
#include "operator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LITERAL_LENGTH 100

Cmd *_parse(char **tokens, int min_bp, int *index);

//Returns a pointer to an exec Cmd from the tokens at the beginning of tokens array.
//n is for counting how many tokens are part of said exec, and thus, how much tokens have been read.
Cmd *get_exec(char **tokens, int *n)
{
    char **args;
    char **args_start = malloc((MAX_LITERAL_LENGTH + 1) * sizeof(char *));
    char **curr_args = args_start;
    *n = 0;

    while(*tokens != NULL && !is_op(*tokens)) {

        if(*n + 1 > MAX_LITERAL_LENGTH) {
            fprintf(stderr, "arguments count overflow\n");
            exit(1);
        }

        *curr_args = strdup(*tokens);
        ++*n;
        tokens++;
        curr_args++;
    }

    if(*n == 0) {
        free(args_start);
        return NULL;
    }

    args = realloc(args_start, sizeof(char *) * (*n + 1));
    if(args == NULL) {
        perror("realloc error in executable node creation");
        exit(1);
    }
    args[*n] = NULL;

    return init_executble(args, *n);
}

Cmd *_parse_round_brackets(char **tokens, int *index)
{
    Cmd *group;
    int i = *index;

    i++;
    group = _parse(tokens, 0, &i);
    if(!group) {
        *index = i;
        return NULL;
    }

    if(!tokens[i] || strcmp(tokens[i], ")") != 0) {
        *index = i;
        return NULL;
    }
    
    *index = ++i;
    return group;
}

Cmd *_parse_lhs(char **tokens, int *index)
{
    Cmd *lhs;
    int n, i = *index;

    if(strcmp(tokens[i], "(") == 0) {

        lhs = _parse_round_brackets(tokens, &i);
        if(!lhs) {
            *index = i;
            return NULL;
        }

        lhs = init_subshell(lhs);

    } else {
        lhs = get_exec(&tokens[i], &n);
        i += n;
    }

    *index = i;
    return lhs;
}

Cmd *_parse(char **tokens, int min_bp, int *index)
{
    int i = *index;
    Cmd *lhs = NULL, *rhs = NULL;

    if(!tokens[i]) return init_empty();

    lhs = _parse_lhs(tokens, &i);
    if(!lhs) {
        fprintf(stderr, "Unexpected token: %s\n", tokens[i]);
        *index = i;
        return NULL;
    }

    while(tokens[i] && strcmp(tokens[i], ")") != 0 && is_op(tokens[i])) {

        Operator op = get_op(tokens[i]);
        if(op.l_bp < min_bp) {
            break;
        }

        i++;
        rhs = _parse(tokens, op.r_bp, &i);
        if (!rhs && !is_terminator(op.str)) { 
            *index = i;
            return NULL; 
        }

        lhs = make_cmd(op.str, lhs, rhs);
        if (!lhs) { 
            *index = i;
            return NULL; 
        }

        *index = i;
    }

    *index = i;
    return lhs;
}

Cmd *parse(char **tokens) 
{
    int index = 0;
    return _parse(tokens, 0, &index);
}