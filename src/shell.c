#include "shell.h"
#include "scanner.h"
#include "parser.h"
#include "executor.h"
#include "history.h"
#include <linux/limits.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int read_line(char *buffer, int len)
{
    char *res;
    int n = 0;

    while((res = fgets(&buffer[n], len - n, stdin)) != NULL) {

        //always assumes buffer ends with newline
        //n + 1 is number of bytes read
        n = strcspn(buffer, "\n");
        buffer[n] = '\0';

        if(n > 0 && buffer[n - 1] == '\\') {
            buffer[--n] = '\0';
            printf("> ");
            fflush(stdout);
            continue;

        } else {
            break;
        }
    }
    
    //removes comments
    //here, n is number of bytes read
    if(res != NULL){
        n = strcspn(buffer, "#");
        buffer[n] = '\0';
        return n;
    }

    return -1;
}

void print_cwd()
{
    char cwd[PATH_MAX];
    if(getcwd(cwd, PATH_MAX) == NULL)
    {
        perror("Error on getting current working directory's name");
        exit(1);
    }

    printf("%s> ", cwd);
    fflush(stdout);
}

int main()
{
    char **tokens;
    Cmd *cmd;

    Process *first_p = NULL;
    
    Env global_env = {init_queue(), &first_p, {0, 0}};

    char buffer[ARG_MAX] = {0};

    while(1) {

        if(pipe(global_env.process_pipe) == -1) {
            perror("pipe error in main shell");
            exit(1);
        }

        print_cwd();

        if(read_line(buffer, ARG_MAX) < 0 || strcmp(buffer, "quit") == 0) {
            break;
        }
        //printf("line read\n");

        tokens = scan(buffer);
        //printf("line scanned\n");

        cmd = parse(tokens);
        //printf("line parsed\n");

        if(cmd && execute(cmd, &global_env) == 0) {
            archive(global_env.history, (void *)strdup(buffer));
        }
        //printf("line executed\n");

        if(close(global_env.process_pipe[1]) == -1) {
            perror("close error on write end in main");
            exit(1);
        }

        update_processes(global_env.process_list, global_env.process_pipe);
        //printf("added to process list\n");

        if(close(global_env.process_pipe[0]) == -1) {
            perror("close error on read end in main");
            exit(1);
        }

        free_tokens(tokens);
        free_cmd(cmd);
        
    }

    free_queue(global_env.history);
    free_process_list(global_env.process_list);

    return 0;
}