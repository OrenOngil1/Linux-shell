#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <linux/limits.h>
#include "process.h"

void add_process(Process** process_list, Cmd* cmd, pid_t pid)
{
    Process *new_p = malloc(sizeof(Process));
    int i = 0;
    if(!(*process_list)) {
        *process_list = new_p;

    } else {
        Process *curr = *process_list;
        while(curr->next) {
            curr = curr->next;
        }

        i = curr->index + 1;
        curr->next = new_p;
    }

    new_p->pid = pid;
    new_p->cmd = cmd;
    new_p->index = i;
    new_p->next = NULL;
    new_p->status = RUNNING; 
}

void update_process_list(Process **process_list)
{
    Process* curr = *process_list;
    int status;
    while(curr) {
        if(waitpid(curr->pid, &status, WNOHANG) > 0) {
            if(WIFEXITED(status) || WIFSIGNALED(status))  {
                curr->status = TERMINATED;
            } else if(WIFSTOPPED(status)) {
                curr->status = SUSPENDED;
            } else if(WIFCONTINUED(status)) {
                curr->status = RUNNING;
            }
        }
        curr = curr->next;
    }
}

void free_process_list(Process **process_list)
{
    if(!process_list) return;
    Process *curr = *process_list, *temp;
    while(curr) {
        temp = curr;
        free_cmd(curr->cmd);
        curr = curr->next;
        free(temp);
    }
    process_list = NULL;
}

char *status_name(int status)
{
    switch (status) {
        case RUNNING: return "RUNNING";
        case SUSPENDED: return "SUSPENDED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

char *str_array_to_str(char **arr, int num) {

    int len = 0;
    char *string = calloc(1, ARG_MAX + 1);
    
    for(int i = 0; i < num; i++) {
        strcat(string, arr[i]);

        len += strlen(arr[i]);

        if(i < num - 1){
            len++;
            string[len - 1] = ' ';
        }

        string[len] = '\0';
    }

    string = realloc(string, len + 1);
    if(string == NULL) {
        perror("realloc error in command unparsing");
        exit(1);
    }
    string[len] = '\0';

    return string;
}

void print_process_list(Process** process_list)
{
    char *cmd_str;
    Process* curr;
    Process **curr_p = process_list;

    update_process_list(process_list);

    while(*curr_p) {
        curr = *curr_p;

        cmd_str = str_array_to_str(curr->cmd->exec.args, curr->cmd->exec.num);

        printf("%i %i %s %s\n", curr->index, curr->pid, status_name(curr->status), cmd_str);

        if(curr->status == TERMINATED) {
            *curr_p = curr->next;
            free_cmd(curr->cmd);
            free(curr);
        }

        else curr_p = &curr->next;

        if(strlen(cmd_str) > 0) free(cmd_str);
    }
}


void update_process_status(Process* process_list, pid_t pid, int status)
{
    Process* curr = process_list;
    while(curr) {
        if(curr->pid == pid) {
            curr->status = status;
            break;
        }
        curr = curr->next;
    }
}