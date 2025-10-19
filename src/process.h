#ifndef PROCESS_H
#define PROCESS_H

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

#include "ast.h"
#include <sys/types.h>

typedef struct process {
    Cmd* cmd;
    pid_t pid;
    int status;
    int index;
    struct process *next;
} Process;

// Appends a process to the end of the process list
void add_process(Process** process_list, Cmd* cmd, pid_t pid);

// Frees the memory allocated to the entire process list
void free_process_list(Process **process_list);

// Prints the process list invoked bt the shell, can be viewed by w/ input "procs" to shell
void print_process_list(Process** process_list);

// updating the first entry's status field with a matching pid 
void update_process_status(Process* process_list, pid_t pid, int status);

#endif