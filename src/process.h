#include "parser.h"
#include <sys/types.h>

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
    ParsedCmdLine* cmd;     /* the parsed command which spawned the process */
    pid_t pid;              /* the process's pid */
    int status;             /* a procces can be RUNNING, SUSPENDED or TERMINATED */
    int index;              /* its index in the process list, may be removed */
    struct process *next;   /* pointer to the next process */
} process;

/* Appends a process to the end of the process list */
void addProcess(process** process_list, ParsedCmdLine* cmd, pid_t pid);

/* Frees the memory allocated to the entire process list */
void freeProcessList(process **process_list);

/* Prints the process list invoked bt the shell, can be viewed by w/ input "procs" to shell */
void printProcessList(process** process_list);

/* updating the first entry's status field with a matching pid  */
void updateProcessStatus(process* process_list, pid_t pid, int status);