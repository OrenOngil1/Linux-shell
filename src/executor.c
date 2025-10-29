#include "executor.h"
#include "history.h"
#include "process.h"
#include "sig.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int get_exit_code(int status)
{
    if(WIFEXITED(status)) {
        return WEXITSTATUS(status);

    } else if(WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

int is_normal_exit(int status) {
    return get_exit_code(status) == 0;
}

int is_builtin(char **args)
{
    return is_cd(args[0]) || is_procs_list(args[0]) || is_signal(args[0]) || is_history(args[0]);
}

int execute_builtin(char **args, int num ,Env *env)
{
    if(is_cd(args[0]) && num == 2) {
        return execute_cd(args[1]);

    } else if(is_procs_list(args[0]) && num == 1) {
        print_process_list(env->process_list);
        return 0;

    } else if(is_signal(args[0]) && num == 2) {
        return send_signal(args[0], atoi(args[1]), env);

    } else if(is_history(args[0]) && num == 1) {
        return execute_history(args[0], env);

    }

    fprintf(stderr, "wrong number of arguments for %s\n", args[0]);
    return 1;
}

void close_pipe(int p[2], int close_write_end) {
    if(p[0] != -1 && close(p[0]) == -1) {
        perror("close error of read end");
        exit(1);
    } else if(p[0] != -1) {
        p[0] = -1;
    }

    if(close_write_end && p[1] != -1 && close(p[1]) == -1) {
        perror("close error of write end");
        exit(1);

    } else if(close_write_end && p[1] != -1) {
        p[1] = -1;
    }

}

int execute_executable(char **args, int num, int should_wait ,Env *env)
{
    int status;
    pid_t pid;
    Cmd *temp;
    
    if(*args != NULL && is_builtin(args)) {
        return execute_builtin(args, num, env);

    } else {
        pid = fork();
        if(pid == -1) {
            perror("fork error during command execution");
            exit(1);

        } else if(pid == 0) {
            close_pipe(env->process_pipe, 1);
            execvp(*args, args);
            fprintf(stderr, "Executer error: Could not execute\n");
            exit(127);

        } else if(should_wait) {
            waitpid(pid, &status, 0);
            //printf("%s executed\n", args[0]);
            return get_exit_code(status);
        }
    }
    //printf("%s executed\n", args[0]);
    temp = init_executble(args, num);
    add_process_record(pid, temp, env->process_pipe);
    free(temp);
    return 0;
}

int execute_io_redirect(IO_Type io_type, char *filename, Cmd *cmd, Env *env)
{
    int fd, old_fd, target_fd, exit_code;

    switch(io_type) {
    case IO_INPUT:
        target_fd = 0;

        fd = open(filename, O_RDONLY);
        if(fd == -1) {
            perror("open error during input redirection");
            exit(1);
        }

        break;
    
    case IO_OUTPUT:
        target_fd = 1;

        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd == -1) {
            perror("open error during output redirection");
            exit(1);
        }

        break;

    default:
        fprintf(stderr, "Executor error: Unhandled I/O redirection command\n");
        exit(1);
    }

    old_fd = dup(target_fd);
    if(old_fd == -1) {
        perror("dup error during I/O redirection");
        exit(1);
    }

    if(dup2(fd, target_fd) == -1) {
        perror("dup2 error during I/O redirection");
        exit(1);
    }

    if(close(fd) == -1) {
        perror("close error during I/O redirection");
        exit(1);
    }

    exit_code = execute(cmd, env);

    if(dup2(old_fd, target_fd) == -1) {
        perror("dup2 error during I/O redirection");
        exit(1);
    }

    if(close(old_fd) == -1) {
        perror("close error during I/O redirection");
        exit(1);
    }

    return exit_code;

}

int open_subshell(Cmd *cmd, Env *env)
{
    int status;
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork error during subshell execution");
        exit(1);
    }

    if(pid == 0) {
        close_pipe(env->process_pipe, 0);
        exit(execute(cmd, env));

    } else {
        waitpid(pid, &status, 0);
        return get_exit_code(status);
    }

    return 1;
}

int execute_pipe(Cmd **cmds, int num, Env *env)
{
    pid_t pids[num];
    int status, prev_pipe, exit_code = 0;
    int p[2];

    for(int i = 0; i < num; i++) {

        //creating pipe for new current child
        if(i < num - 1 && pipe(p) == -1) {
            perror("pipe error");
            exit(1);
        }

        pids[i] = fork();

        if(pids[i] == -1) {
            perror("fork error during pipe execution");
            exit(1);

        //child process, redirects I/O and continues execution of current command
        } else if(pids[i] == 0) {

            //if not first process in pipe, redirects standard input to read end of previous pipe
            if(i > 0) {
                if(dup2(prev_pipe, 0) == -1) {
                    perror("dup2 error on input redirection in child");
                    exit(1);
                }

                if(close(prev_pipe) == -1) {
                    perror("close error on write end in child");
                    exit(1);
                }
            }

            //if not last process, redirects output to write end of the newly created pipe
            if(i < num - 1) {

                if(close(p[0]) == -1) {
                    perror("close error on read end of pipe in child");
                    exit(1);
                }

                if(dup2(p[1], 1) == -1) {
                    perror("dup2 error on output redirection in child");
                    exit(1);
                }

                if(close(p[1]) == -1) {
                    perror("close error on write end in child");
                    exit(1);
                }
            }

            close_pipe(env->process_pipe, 0);

            exit(execute(cmds[i], env));
        }

        //parent process: closes write end of pipe and previous read end, sets up the read end for the next process
        if(i > 0 && close(prev_pipe) == -1) {
            perror("close error on previous pipe in parent");
            exit(1);
        }

        if(i < num - 1 && close(p[1]) == -1) {
            perror("close error on write end in parent");
            exit(1);
        }
        prev_pipe = p[0];
    }

    //waits for all children to terminate, but returns the exit code of the first child to fail, if there is one
    for(int i = 0 ; i < num; i++) {
        waitpid(pids[i], &status, 0);
        if(exit_code == 0 && !is_normal_exit(status)) {
            exit_code = get_exit_code(status);
        }
    }
    return exit_code;
}

int execute_and(Cmd **cmds, int num, Env *env)
{
    int exit_code = 0;

    for(int i = 0; i < num && exit_code == 0; i++) {
        exit_code = execute(cmds[i], env);
    }

    return exit_code;
}

int execute_or(Cmd **cmds, int num, Env *env)
{
    int exit_code = 1;

    for(int i = 0; i < num && exit_code != 0; i++) {
        exit_code = execute(cmds[i], env);
    }

    return exit_code;
}

int execute_nonblocking(Cmd **cmds, int num, Env *env)
{
    int exit_code;

    for(int i = 0; i < num; i++) {
        if(cmds[i]->type == CMD_EXEC) {
            exit_code = execute_executable(cmds[i]->exec.args, cmds[i]->exec.num, 0, env);
        } else {
            exit_code = execute(cmds[i], env);
        }
    }
    return exit_code;
}

int execute_sequential(Cmd **cmds, int num, Env *env)
{
    int exit_code;
    
    for(int i = 0; i < num; i++) {
        exit_code = execute(cmds[i], env);
    }

    return exit_code;
}

int execute(Cmd *cmd, Env *env)
{
    switch(cmd->type) {
    
    case CMD_EXEC:
        return execute_executable(cmd->exec.args, cmd->exec.num, 1, env);

    case CMD_IO_REDIRECT:
        return execute_io_redirect(cmd->io_redirect.io_type, cmd->io_redirect.file_name, cmd->io_redirect.cmd, env);

    case CMD_SUBSHELL:
        return open_subshell(cmd->unary_cmd, env);

    case CMD_NONBLOCKING:
        return execute_nonblocking(cmd->binary_cmd.cmds, cmd->binary_cmd.num, env);

    case CMD_SEQUENTIAL:
        return execute_sequential(cmd->binary_cmd.cmds, cmd->binary_cmd.num, env);

    case CMD_PIPE:
        return execute_pipe(cmd->binary_cmd.cmds, cmd->binary_cmd.num, env);

    case CMD_AND:
        return execute_and(cmd->binary_cmd.cmds, cmd->binary_cmd.num, env);

    case CMD_OR:
        return execute_or(cmd->binary_cmd.cmds, cmd->binary_cmd.num, env);
     
    default:
        fprintf(stderr, "Executor error: Unhandled command type\n");
        exit(126);
    }
}

int is_cd(char *arg)
{
    return strcmp(arg, "cd") == 0;
}

int execute_cd(char *dest) {
    if(chdir(dest) == -1) {
        perror("chdir error");
        return 1;
    }

    return 0;
}

int is_procs_list(char *arg)
{
    return strcmp(arg, "procs") == 0;
}
