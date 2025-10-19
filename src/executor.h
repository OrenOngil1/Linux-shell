#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "ast.h"
#include "queue.h"
#include "process.h"

//Surronding environment of the executed commands.
typedef struct env {
    Queue *history;
    Process **process_list; 
    int process_pipe[2];
} Env;

//Temporarily holds info on commands before their documentation in process_list.
typedef struct p_record {
    pid_t pid;
    Cmd cmd;
} Process_record;

//Executes the commands of the AST rooted in cmd with the environment Env.
int execute(Cmd *cmd, Env *env);

//Returns 1 if the arguments describe a built-in function e.g. cd, or 0 otherwise.
int is_builtin(char **args);

//Executes built-in function *arg, with num arguments, given as args, with the environment Env.
int execute_builtin(char **args, int num, Env *env);

//Returns 1 if the arg is the change directory command cd, or 0 otherwise.
int is_cd(char *arg);

//Changes current working directory.
int execute_cd(char *dest);

//Returns 1 if the arg is the keyword for printing the process list, or 0 otherwise
int is_procs_list(char *arg);

//Executes *arg, with num arguments, given as args, with the environment Env.
int execute_executable(char **args, int num, int should_wait ,Env *env);

//Forks a new child in which cmd runs with the environment Env.
int open_subshell(Cmd *cmd ,Env *env);

//Opens (num - 1) pipes for cmds with the environment Env.
int execute_pipe(Cmd **cmds, int num ,Env *env);

//Executes the and command between cmds with the environment Env.
int execute_and(Cmd **cmds, int num ,Env *env);

//Executes the or command between cmds with the environment Env.
int execute_or(Cmd **cmds, int num ,Env *env);

//Executes cmds as non-blocking commands with the environment Env.
int execute_nonblocking(Cmd **cmds, int num ,Env *env);

//Executes cmds sequentially with the environment Env.
int execute_sequential(Cmd **cmds, int num ,Env *env);

#endif