#include "sig.h"
#include "process.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>

int is_signal(char *arg)
{
    return strcmp(arg, "halt") == 0 || strcmp(arg, "wakeup") == 0 || strcmp(arg, "ice") == 0;
}

int send_signal(char *signal, pid_t pid, Env *env)
{
    if(strcmp(signal, "halt") == 0) {

        if(kill(pid, SIGSTOP) == -1) {
            perror("Executor error: Could not pause process");
            return 1;
        }
        update_process_status(*env->process_list, pid, SUSPENDED);
        return 0;

    } else if(strcmp(signal, "wakeup") == 0) {

        if(kill(pid, SIGCONT) == -1) {
            perror("Executor error: Could not continue process");
            return 1;
        }
        update_process_status(*env->process_list, pid, RUNNING);
        return 0;

    } else if(strcmp(signal, "ice") == 0) {

        if(kill(pid, SIGTERM) == -1) {
            perror("Executor error: Could not terminate process");
            return 1;
        }
        update_process_status(*env->process_list, pid, TERMINATED);
        return 0;
    }

    //unhandled signals
    fprintf(stderr, "Executor error: Cannot send unhandled signal\n");
    return 1;
}
