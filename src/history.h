#ifndef HISTORY_H
#define HISTORY_H

#define HISTLEN 20

#include "queue.h"
#include "executor.h"

//Adds the line to the queue history
void archive(Queue *history, char *line);

//Executes history command arg with the environment Env.
int execute_history(char *arg, Env *env);

//Returns 1 if the arg is args descibe a history command or 0 otherwise.
int is_history(char *args);

//Prints the queue history.
void print_history(Queue *history);

///Returns a pointer to the last n-th given line by user that is not a history command.
char *get_non_hist_n(Queue *queue, int n);

//Returns a pointer to the last given line by user that is not a history command.
char *get_non_hist_last(Queue *queue);

#endif