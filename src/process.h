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

//Temporarily holds info on commands before their documentation in process_list.
typedef struct p_record {
    pid_t pid;
    Cmd cmd;
} Process_record;

// Appends a process to the end of the process list
void add_process(Process** process_list, Cmd* cmd, pid_t pid);

// Frees the memory allocated to the entire process list
void free_process_list(Process **process_list);

// Prints the process list invoked bt the shell, can be viewed by w/ input "procs" to shell
void print_process_list(Process** process_list);

// updating the first entry's status field with a matching pid 
void update_process_status(Process* process_list, pid_t pid, int status);

//Writes pid and exec_cmd to write-end of pipe.
void add_process_record(pid_t pid, Cmd *exec_cmd, int pipe[2]);

//Reads pid and cmd of process from read-end of pipe, then creates a new entry from them to add to process_list. 
void update_processes(Process **process_list, int pipe[2]);

#endif