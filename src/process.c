#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include "process.h"

void addProcess(process** process_list, ParsedCmdLine* cmd, pid_t pid)
{
    process *new_p = malloc(sizeof(process));
    int i = 0;
    if(!(*process_list)) *process_list = new_p;
    else
    {
        process *curr = *process_list;
        while(curr->next)
        {
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

void updateProcessList(process **process_list)
{
    process* curr = *process_list;
    int status;
    while(curr)
    {
        if(waitpid(curr->pid, &status, WNOHANG) > 0)
        {
            if(WIFEXITED(status) || WIFSIGNALED(status)) 
            {
                curr->status = TERMINATED;
            }
            else if(WIFSTOPPED(status)) 
            {
                curr->status = SUSPENDED;
            }
            else if(WIFCONTINUED(status))
            {
                curr->status = RUNNING;
            }
        }
        curr = curr->next;
    }
}

void freeProcessList(process **process_list)
{
    if(!process_list) return;
    process *curr = *process_list, *temp;
    while(curr)
    {
        temp = curr;
        freeCmdLine(curr->cmd);
        curr = curr->next;
        free(temp);
    }
    process_list = NULL;
}

char *status_name(int status)
{
    switch (status)
    {
        case RUNNING: return "RUNNING";
        case SUSPENDED: return "SUSPENDED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

void printProcessList(process** process_list)
{
    updateProcessList(process_list);
    process **curr_p = process_list;
    process* curr;
    while(*curr_p)
    {
        curr = *curr_p;
        printf("%i %i %s %s\n", curr->index, curr->pid, status_name(curr->status), unparseCmdLine(curr->cmd));
        if(curr->status == TERMINATED)
        {
            *curr_p = curr->next;
            freeCmdLine(curr->cmd);
            free(curr);
        }
        else curr_p = &curr->next;
    }
}


void updateProcessStatus(process* process_list, pid_t pid, int status)
{
    process* curr = process_list;
    while(curr)
    {
        if(curr->pid == pid)
        {
            curr->status = status;
            break;
        }
        curr = curr->next;
    }

}