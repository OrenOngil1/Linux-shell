#ifndef SIG_H
#define SIG_H

#include <sys/types.h>
#include "executor.h"

//Return 1 if arg is a signal keyword, 0 otherwise.
int is_signal(char *arg);

//Sends the signal detemined by the keyword signal to the process who has pid. 
int send_signal(char *signal, pid_t pid, Env *env);

#endif