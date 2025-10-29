#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "history.h"
#include "scanner.h"
#include "parser.h"

void archive(Queue *history, char *line) {
    if(history->size == HISTLEN) {
        free(dequeue(history));
    }

    enqueue(history, (void *)line);
}

int re_exec(char *line, Env *env) {
    int ret;
    char **tokens;
    Cmd *cmd;

    if(line == NULL) {
        return 1;
    }

    tokens = scan(line);
    cmd = parse(tokens);
    ret = execute(cmd, env);

    free_tokens(tokens);
    free_cmd(cmd);

    return ret;

}

int execute_history(char *arg, Env *env)
{
    int n;

    if(strcmp(arg, "hist") == 0) {
        print_history(env->history);
        return 0;

    } else if(strcmp(arg, "!!") == 0) {
        return re_exec(get_non_hist_last(env->history), env);

    } else if(sscanf(arg, "!%i", &n) == 1) {
        return re_exec(get_non_hist_n(env->history, n), env);
    }

    fprintf(stderr, "Executor error: Unhandled history command\n");
    return 1;
}

int is_history(char *arg)
{
    int n;
    return strcmp(arg, "hist") == 0 || strcmp(arg, "!!") == 0 || sscanf(arg, "!%i", &n) == 1;
}

void print_history(Queue *history)
{
    for(Link *curr = history->end; curr != NULL; curr = curr->prev) {
        printf("%s\n", (char *)curr->element);
    }
}

char *get_non_hist_n(Queue *queue, int n)
{
    int i;
    Link *curr;

    if(n >= 1 && n <=queue->size)
    {
        i = 1;
        curr = queue->end;

        while(curr && i++ < n) curr = curr->prev;

        if(curr && !is_history((char *)curr->element)) { 
            return (char *)curr->element;
        }
    }

    return NULL;
}

char *get_non_hist_last(Queue *queue)
{
    Link *curr = queue->end;

    for(int i = 1; i < queue->size && is_history((char *)curr->element); i++) {
        curr = curr->prev;
    }

    if(curr) return (char *)curr->element;
    return NULL;
}