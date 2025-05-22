#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "process.h"
#include "history.h"

// gloabl variables - declarations and memory freeing functions freeing

process **process_list = NULL;
strQueue *queue = NULL;

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
    execvp(pCmdLine->args[0], pCmdLine->args);
    freeCmdLine(pCmdLine);
    free_global();
    perror("ERROR");
    _exit(1);
}


//pipe control
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

//main
int main(int argc, char **argv)
{
    ParsedCmdLine *line = NULL;
    char directory[PATH_MAX + 1] = {0};
    char buffer[ARG_MAX + 1] = {0};
    pid_t pid;
    process *first = NULL;
    process_list = &first;
    strLink *link = NULL;
    queue = malloc(sizeof(strQueue));
    queue->size = 0; queue->first = link; queue->last = link;
    int n = 0;

    while(1)
    {
        getcwd(directory, PATH_MAX);
        printf("%s> ", directory);
        if(!fgets(buffer, ARG_MAX, stdin))
        {
            perror("ERROR: Couldn't read input");
            free_global();
            _exit(1);
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        line = parseCmdLine(buffer);
        if(line->argCount == 1 && !strcmp(line->args[0], "quit")) //quits the shell
        {
            freeCmdLine(line);
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
            freeCmdLine(line);
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
            freeCmdLine(line);
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
            freeCmdLine(line);
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
            freeCmdLine(line);
            line = NULL;
        }
        else if(line->argCount == 1 && !strcmp(line->args[0], "procs")) //prints processes
        {
            printProcessList(process_list);
            freeCmdLine(line);
            line = NULL;
        }
        else if(line->argCount == 1 && !strcmp(line->args[0], "hist"))  //history impl
        {
            enqueue(queue, buffer);
            printStrQueue(queue);
            freeCmdLine(line);
            continue;
        }
        
        else if(!strcmp(line->args[0], "!!"))
        {
            char *nonHist = NULL;
            if(line->argCount == 1) nonHist = getNonHistLast(queue);
            else fprintf(stderr, "USER ERROR: Wrong number of arguments for !! command\n");
            freeCmdLine(line);
            buffer[0] = '\0';
            if(nonHist) 
            {
                line = parseCmdLine(nonHist);
                strcpy(buffer, nonHist);
            }
        }
        else if(sscanf(line->args[0], "!%i", &n) == 1)
        {
            char *nonHist = NULL;
            if(line->argCount == 1) nonHist = getNonHistN(queue, n);
            else fprintf(stderr, "USER ERROR: Wrong number of arguments for !n command\n");
            freeCmdLine(line);
            buffer[0] = '\0';
            if(nonHist) 
            {
                line = parseCmdLine(nonHist);
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