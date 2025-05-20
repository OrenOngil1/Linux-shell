#include <stdlib.h>
#include "parser.h"
#include <stdio.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

#define HISTLEN 20

typedef struct process{
    ParsedCmdLine* cmd;
    pid_t pid;
    int status;
    int index;
    struct process *next;
} process;

typedef struct strLink{
    char *line;
    struct strLink *next;
    struct strLink *prev;
} strLink;

typedef struct strQueue{
    strLink *first;
    strLink *last;
    int size;
} strQueue;

process **process_list = NULL;
strQueue *queue = NULL;
int is_debug = 1;

//forward declerations and gloabl variables freeing
void freeProcessList(process **process_list);

void freeStrQueue(strQueue *queue);

void addProcess(process** process_list, ParsedCmdLine* cmd, pid_t pid);

void create_pipe(ParsedCmdLine *line);

void updateProcessStatus(process *process_list, pid_t pid, int status);

void free_global()
{
    freeProcessList(process_list);
    freeStrQueue(queue);
}

//excecution
void execute(ParsedCmdLine *pCmdLine)
{
    if(pCmdLine->inputRedirect) //redirects I/O
    {
        int fd = open(pCmdLine->inputRedirect, O_RDONLY);
        dup2(fd, 0);
        close(fd);
    }

    if(pCmdLine->outputRedirect)
    {
        int fd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT, S_IRWXO);
        dup2(fd, 1);
        close(fd);
    }
    if(is_debug)
    {
        fprintf(stderr, "PID: %d\n", getpid());
        fprintf(stderr, "Executing command: %s\n", pCmdLine->args[0]);
        fflush(stderr);
    }
    execvp(pCmdLine->args[0], pCmdLine->args);
    freeCmdLines(pCmdLine);
    free_global();
    perror("ERROR");
    _exit(1);
}

void create_pipe(ParsedCmdLine *line)
{
    int pipeline[2]; pipe(pipeline);
    pid_t pid1, pid2;
    ParsedCmdLine *next = line->next;
    line->next = NULL;

    if(!(pid1 = fork()))
    {
        if(!line->outputRedirect)
        {
            close(1);
            dup(pipeline[1]);
            close(pipeline[1]);
            close(pipeline[0]);
            execute(line);
        }
        else fprintf(stderr, "USER ERROR: Can't redirect output from stdout while writing into pipe\n");
    }
    close(pipeline[1]);
    if(!(pid2 = fork()))
    {
        if(!next->inputRedirect)
        {
            close(0);
            dup(pipeline[0]);
            close(pipeline[0]);
            execute(next);
        }
        else fprintf(stderr, "USER ERROR: Can't redirect input from stdin while reading from pipe\n");
    }
    close(pipeline[0]);
    addProcess(process_list, line, pid1);
    addProcess(process_list, next, pid2);
    waitpid(pid1, NULL, 0);
    updateProcessStatus(*process_list, pid1, TERMINATED);
    waitpid(pid2, NULL, 0);
    updateProcessStatus(*process_list, pid2, TERMINATED);
}

//process list functions
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

void printLine(ParsedCmdLine *pCmdLine)
{
  for(int i = 0; i < pCmdLine->argCount; i++)
  {
    printf("%s ", pCmdLine->args[i]);
  }
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
        freeCmdLines(curr->cmd);
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
        printf("%i %i %s ", curr->index, curr->pid, status_name(curr->status));
        printLine(curr->cmd);
        printf("\n");
        if(curr->status == TERMINATED)
        {
            *curr_p = curr->next;
            freeCmdLines(curr->cmd);
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

//strQueue functions
char* dequeue(strQueue *queue)
{
    if(queue->size == 0) return NULL;
    char *ret = calloc(strlen(queue->first->line) + 1, 1);
    strcpy(ret, queue->first->line);
    free(queue->first->line);
    queue->size--;
    if(queue->size == 0) {
        free(queue->first); queue->first = NULL;
        free(queue->last); queue->last = NULL;
    }
    else
    {
        queue->first = queue->first->next;
        free(queue->first->prev);
    }
    return ret;
}

void enqueue(strQueue *queue, char* line)
{
    strLink *link = malloc(sizeof(strLink));
    line[strcspn(line, "\n")] = '\0';
    link->line = strdup(line); link->next = NULL; link->prev = NULL;
    if(queue->size == 0)
    {
        queue->first = link;
        queue->last = link;
    }
    else 
    {
        queue->last->next = link;
        link->prev = queue->last;
        queue->last = link;
    }
    if(queue->size + 1 > HISTLEN) free(dequeue(queue));
    queue->size++;
}

void freeStrQueue(strQueue *queue)
{
    if(!queue) return;
    strLink *curr = queue->first;
    strLink *temp;
    while(curr)
    {
        temp = curr;
        curr = curr->next;
        free(temp->line);
        free(temp);
    }
    free(queue);
    queue = NULL;
}

void printStrQueue(strQueue *queue)
{
    strLink *curr = queue->first;
    int entry = 1;
    while(curr)
    {
        printf("%i %s\n", entry++, curr->line);
        curr = curr->next;
    }
}

int isHistCmd(char *line)
{
    int a = 0;
    return !strcmp(line, "hist") || !strcmp(line, "!!") || sscanf(line, "!%i", &a) == 1;
}

char *getNonHistN(strQueue *queue, int n)
{
    if(n >= 1 && n <=queue->size)
    {
        int local_n = 1;
        strLink *curr = queue->first;
        while(curr && local_n++ < n) curr = curr->next;
        if(curr && !isHistCmd(curr->line)) return curr->line;
    }
    return NULL;
}

char *getNonHistLast(strQueue *queue)
{
    strLink *curr = queue->last;
    while(curr && isHistCmd(curr->line)) curr = curr->prev;
    if(curr) return curr->line;
    return NULL;
}

int main(int argc, char **argv)
{
    ParsedCmdLine *line = NULL;
    char directory[PATH_MAX + 1] = {0};
    char buffer[ARG_MAX] = {0};
    pid_t pid;
    process *first = NULL;
    process_list = &first;
    strLink *link = NULL;
    queue = malloc(sizeof(strQueue));
    queue->size = 0; queue->first = link; queue->last = link;
    int n = 0;

    while(1)
    {
        getcwd(directory, 2049);
        printf("%s> ", directory);
        if(!fgets(buffer, 2049, stdin))
        {
            perror("ERROR: Couldn't read input");
            free_global();
            _exit(1);
        }
        line = parseCmdLines(buffer);
        if(line->argCount == 1 && !strcmp(line->args[0], "quit")) //quits the shell
        {
            freeCmdLines(line);
            free_global();
            break;
        }
        else if(!strcmp(line->args[0], "cd")) //changes working directory
        {
            if(line->argCount < 2) fprintf(stderr, "USER ERROR: Wrong number of arguments for command %s\n", line->args[0]);
            else if(chdir(line->args[1])) 
            {
                perror("ERROR");
            }
            freeCmdLines(line);
            line = NULL;
        }
        else if(!strcmp(line->args[0], "halt")) //sends signals
        {
            if(line->argCount != 2) fprintf(stderr, "USER ERROR: Wrong number of aguments for command %s\n", line->args[0]);
            else 
            {
                if(!kill(atoi(line->args[1]), SIGSTOP))
                {
                    updateProcessStatus(*process_list, atoi(line->args[1]), SUSPENDED);
                    printf("Process %s suspended succesefuly\n", line->args[1]);
                }
                else 
                {
                    perror("ERROR");
                }
            }
            freeCmdLines(line);
            line = NULL;
        }
        else if(!strcmp(line->args[0], "wakeup"))
        {
            if(line->argCount != 2) fprintf(stderr, "USER ERROR: Wrong number of aguments for command %s\n", line->args[0]);
            else 
            {
                if(!kill(atoi(line->args[1]), SIGCONT)) 
                {
                    updateProcessStatus(*process_list, atoi(line->args[1]), RUNNING);
                    printf("Process %s continued successfully\n", line->args[1]);
                }
                else
                {
                    perror("ERROR");
                }
            }
            freeCmdLines(line);
            line = NULL;
        }
        else if(!strcmp(line->args[0], "ice"))
        {
            if(line->argCount != 2) fprintf(stderr, "USER ERROR: Wrong number of aguments for command %s\n", line->args[0]);
            else 
            {
                if(!kill(atoi(line->args[1]), SIGINT))
                {
                    updateProcessStatus(*process_list, atoi(line->args[1]), TERMINATED);
                    printf("Process %s teminated successfully\n", line->args[1]);
                }
                else
                {
                    perror("ERROR");
                }
            }
            freeCmdLines(line);
            line = NULL;
        }
        else if(line->argCount == 1 && !strcmp(line->args[0], "procs")) //prints processes
        {
            printProcessList(process_list);
            freeCmdLines(line);
            line = NULL;
        }
        else if(line->argCount == 1 && !strcmp(line->args[0], "hist"))  //history impl
        {
            enqueue(queue, buffer);
            printStrQueue(queue);
            freeCmdLines(line);
            continue;
        }
        
        else if(!strcmp(line->args[0], "!!"))
        {
            char *nonHist = NULL;
            if(line->argCount == 1) nonHist = getNonHistLast(queue);
            else fprintf(stderr, "USER ERROR: Wrong number of arguments for !! command\n");
            freeCmdLines(line);
            buffer[0] = '\0';
            if(nonHist) 
            {
                line = parseCmdLines(nonHist);
                strcpy(buffer, nonHist);
            }
        }
        else if(sscanf(line->args[0], "!%i", &n) == 1)
        {
            char *nonHist = NULL;
            if(line->argCount == 1) nonHist = getNonHistN(queue, n);
            else fprintf(stderr, "USER ERROR: Wrong number of arguments for !n command\n");
            freeCmdLines(line);
            buffer[0] = '\0';
            if(nonHist) 
            {
                line = parseCmdLines(nonHist);
                strcpy(buffer, nonHist);
            }
            else printf("No such non-history command exists\n");
        }
        if(line && line->next) create_pipe(line); //pipe and execution
        else if(line)
        {
            if(!(pid = fork())) execute(line);
            addProcess(process_list, line, pid);
            if(line->blocking) 
            {
                waitpid(pid, NULL, 0);
                updateProcessStatus(*process_list, pid, TERMINATED);
            }
        }
        if(strlen(buffer) > 0) enqueue(queue, buffer);
    }
    return 0;
}