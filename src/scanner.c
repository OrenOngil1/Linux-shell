#include "scanner.h"
#include "operator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 400

int first_word(char *line, char **tokens, int *n)
{
    *n = 0;
    int i = 0;
    char *word = NULL;
    char *op = NULL;

    for( ; line[i] != '\0' && line[i] != ' ' && (op = dup_op(&line[i])) == NULL; i++);

    if(i > 0) {
        word = strndup(line, i);
        *tokens++ = word;
        ++*n;
    }

    if(op != NULL) {
        *tokens = op;
        ++*n;
        i += strlen(op);
    }

    return i;
}

char **scan(char *line)
{
    int length; int n;

    char **tokens = malloc((MAX_TOKENS + 1) * sizeof(char *)); //null teminated array of tokens
    if(tokens == NULL) {
        perror("calloc error in scanner");
        exit(1);
    }

    char **tokens_curr = tokens;
    while (*line != '\0') {

        while(*line == ' ') {
            line++;
        }
        
        length = first_word(line, tokens_curr, &n);
        
        line += length;
        tokens_curr += n;
        
    }

    //number of tokens
    n = tokens_curr - tokens;
    tokens = realloc(tokens, sizeof(char *) * (n + 1));
    if(tokens == NULL) {
        perror("realloc error in scanner");
        exit(1);
    }

    tokens[n] = NULL;

    return tokens;
        
}

void free_tokens(char **tokens)
{
    if(tokens != NULL) {
        for(char **token_ptr = tokens; *token_ptr != NULL; token_ptr++) {
            free(*token_ptr);
        }
        free(tokens);
    }
}

//this function is mostly for tests
void print_tokens(char **tokens)
{
    printf("[");
    for(char **token_ptr = tokens; *token_ptr != NULL; token_ptr++) {
        printf("%s", *token_ptr);
        if(*(token_ptr + 1) != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}


//testing
// int main()
// {
//     char *test1 = "echo hello";
//     char *test2 = "(echo hello | echo hi) > out.txt";
//     char *test3 = "!! && echo hello";

//     char **tokens1 = scan(test1);
//     char **tokens2 = scan(test2);
//     char **tokens3 = scan(test3);

//     print_tokens(tokens1);
//     print_tokens(tokens2);
//     print_tokens(tokens3);

//     free_tokens(tokens1);
//     free_tokens(tokens2);
//     free_tokens(tokens3);

//     return 0;
// }