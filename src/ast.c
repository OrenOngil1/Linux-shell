#include "ast.h"
#include "stack.h"
#include "operator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LITERAL_LENGTH 100

void print_str_arr(char **arr)
{
    printf("[");
    for(char **ptr = arr; *ptr != NULL; ptr++) {
        printf("%s", *ptr);

        if(*(ptr + 1) != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

int is_terminator_op(char *op) {
    return strcmp(op, ";") == 0 || strcmp(op, "&") == 0;
}

Cmd *init_empty() {
    char **args = malloc(sizeof(char *));
    if(args == NULL) {
        perror("calloc error on creating empty command");
        exit(1);
    }
    args[0] = NULL;
    return init_executble(args, 0);
}

int is_same_binary(char *op, Cmd_type type) {
    return (strcmp(op, "|") == 0 && type == CMD_PIPE)
            || (strcmp(op, "&&") == 0 && type == CMD_AND)
            || (strcmp(op, "||") == 0 && type == CMD_OR)
            || (strcmp(op, "&") == 0 && type == CMD_NONBLOCKING)
            || (strcmp(op, ";") == 0 && type == CMD_SEQUENTIAL);
}

Cmd *extend_binary_cmd(Cmd *cmd1, Cmd *cmd2) {
    Cmd **new_cmds = realloc(cmd1->binary_cmd.cmds, (cmd1->binary_cmd.num + 1) * sizeof(Cmd *));
    if(new_cmds == NULL) {
        perror("realloc error on creating extending binary command");
        exit(1);
    }

    cmd1->binary_cmd.cmds = new_cmds;
    cmd1->binary_cmd.cmds[cmd1->binary_cmd.num++] = cmd2;

    return cmd1;
}


//Returns a pointer to an exec Cmd from the tokens at the beginning of tokens array.
//n is for counting how many tokens are part of said exec, and thus, how much tokens have been read.
Cmd *get_exec(char **tokens, int *n)
{
    char **args;
    char **args_start = malloc((MAX_LITERAL_LENGTH + 1) * sizeof(char *));
    char **curr_args = args_start;
    *n = 0;

    while(*tokens != NULL && !is_op(*tokens)) {

        if(*n + 1 > MAX_LITERAL_LENGTH) {
            fprintf(stderr, "arguments count overflow\n");
            exit(1);
        }

        *curr_args = strdup(*tokens);
        ++*n;
        tokens++;
        curr_args++;
    }

    if(*n == 0) {
        free(args_start);
        return NULL;
    }

    args = realloc(args_start, sizeof(char *) * (*n + 1));
    if(args == NULL) {
        perror("realloc error in executable node creation\n");
        exit(1);
    }
    args[*n] = NULL;

    return init_executble(args, *n);
}

int pop_op_create_and_push_cmd(Stack *op_stack, Stack *cmd_stack)
{
    Cmd *cmd;
    Cmd *operand1, *operand2;
    int is_unary_ans, is_binary_ans, is_terminator_ans;

    char *op = (char *)pop(op_stack);

    is_unary_ans = is_unary(op);
    is_binary_ans = is_binary(op);
    is_terminator_ans = is_terminator_op(op);

    if(is_unary_ans && cmd_stack->size >= 1) {
        cmd = init_unary(op, (Cmd *)pop(cmd_stack));

    } else if(is_unary_ans && cmd_stack->size < 1) {
        fprintf(stderr, "Syntax error: unary operator %s must have one operand\n", op);
        return 0;

    } else if(is_binary_ans && cmd_stack->size >= 2) {
        operand2 = (Cmd *)pop(cmd_stack);
        operand1 = (Cmd *)pop(cmd_stack);
        cmd = init_binary(op, operand1, operand2);

    } else if(is_binary_ans && cmd_stack->size == 1 && is_terminator_ans) {
        operand1 = (Cmd *)pop(cmd_stack);
        cmd = init_binary(op, operand1, init_empty());

    } else if(is_binary_ans && cmd_stack->size < 2 && !is_terminator_ans) {
        fprintf(stderr, "Syntax error: binary operator %s must have two operands\n", op);
        return 0;

    } else {
        fprintf(stderr, "Syntax error: unhandled operator %s\n", op);
        return 0;
    }

    push(cmd_stack, (void *)cmd);
    return 1;
}

Cmd *pop_for_subshell(Stack *op_stack, Stack *cmd_stack) {

    char *op;

    while(op_stack->size > 0) {
        op = (char *)peek(op_stack);

        if(strcmp(op,"(") == 0) {
            pop(op_stack);

            if(cmd_stack->size > 0) {
                return init_subshell((Cmd *)pop(cmd_stack));
                
            } else {
                return init_subshell(init_empty());
            }

        } else if(!pop_op_create_and_push_cmd(op_stack, cmd_stack)) {
            return NULL;
        }
    }

    fprintf(stderr, "Syntax error: no opening bracket '(' found for closing bracket ')'\n");
    return NULL;
}

Cmd *parse(char **tokens) 
{
    int n;
    char *op;
    Cmd *cmd;
    
    Stack *cmd_stack = init_stack();
    Stack *op_stack = init_stack();

    while(tokens != NULL && *tokens != NULL) {

        //current token is a literal, or the literal detection fails
        cmd = get_exec(tokens, &n);
        if(cmd != NULL) {
            push(cmd_stack, (void *)cmd);
            tokens += n;
            
        //operator cases    

        //case one: open round bracket; push to operator stack and continue
        } else if(strcmp(*tokens, "(") == 0) {
            push(op_stack, (void *)*tokens);
            tokens++;
        
        //case two: close round brackets: pop current command until open bracket and wrap it
        } else if(strcmp(*tokens, ")") == 0) {
            cmd = pop_for_subshell(op_stack, cmd_stack);
            if(cmd == NULL) {
                free_stack(cmd_stack);
                free_stack(op_stack);
                return NULL;
            }

            push(cmd_stack, (void *)cmd);
            tokens++;

        //case three: i/o redirection operator
        } else if(is_io(*tokens) && cmd_stack->size >= 1) {
            
            //exits if no source ore destination file name provided
            if(*(tokens + 1) == NULL) {
                fprintf(stderr, "Syntax error: I/O redirection operator %s requires file name\n", *tokens);
                free_stack(cmd_stack);
                free_stack(op_stack);
                return NULL;
            }

            cmd = init_io_redirect((Cmd *)pop(cmd_stack), get_io_type(*tokens), strdup(*(tokens + 1)));
            push(cmd_stack, (void *)cmd);
            tokens += 2;

        //compare precendeces
        } else {

            //enqueuing all operators with higher(= smaller) or the same precedence which aren't pranthesses or io redirection,
            while(op_stack->size > 0){
                op = (char *)peek(op_stack);

                if(strcmp(op, "(") != 0 && precedence(*tokens) >= precedence(op)) {
                    pop_op_create_and_push_cmd(op_stack, cmd_stack);

                } else { 
                    break;
                }
            }
            push(op_stack, (void *)*tokens);
            tokens++;
        }

    }

    //empties stack after passage over tokens
    while(op_stack->size > 0) {
        pop_op_create_and_push_cmd(op_stack, cmd_stack);
    }

    //case of empty commands
    if(cmd_stack->size > 0) {
        cmd = pop(cmd_stack);

    } else {
        cmd = init_empty();
    }

    free_stack(cmd_stack);
    free_stack(op_stack);

    return cmd;
}

Cmd *init_unary(char *op, Cmd *operand) {
    // tempoprary until we add more unary operators
    fprintf(stderr, "Syntax error: Cannot read unknow unary operator: %s\n", op);
    return NULL;
}

//temporary until we add unary operations
int is_unary(char *op) {
    return 0;
}

Cmd *init_binary(char *op, Cmd *operand1, Cmd *operand2)
{
    Cmd **cmd_arr;
    int num = 2;

    //previously computed node is of the same type as this
    if(is_same_binary(op, operand1->type)) {
        return extend_binary_cmd(operand1, operand2);
    }

    //creating an array of operands for binary command
    cmd_arr = malloc(sizeof(Cmd *) * num);
    if(cmd_arr == NULL) {
        perror("realloc error for pipe");
        return NULL;
    }

    cmd_arr[0] = operand1;
    cmd_arr[1] = operand2;
    
    if(strcmp(op, "|") == 0) {
        return init_pipe(cmd_arr, num);

    } else if(strcmp(op, "&&") == 0) {
        return init_and(cmd_arr, num);

    } else if(strcmp(op, "||") == 0) {
        return init_or(cmd_arr, num);

    } else if(strcmp(op, "&") == 0) {
        return init_nonblocking(cmd_arr, num);

    } else if(strcmp(op, ";") == 0) {
        return init_sequential(cmd_arr, num);
    }

    fprintf(stderr, "Syntax error: Unhandled binary operator: %s\n", op);
    return NULL;
}

int is_binary(char *op) {
    return strcmp(op, "|") == 0 
            || strcmp(op, "&&") == 0
            || strcmp(op, "||") == 0
            || strcmp(op, "&") == 0
            || strcmp(op, ";") == 0;
}

int is_io(char *op) {
    return strcmp(op, "<") == 0 || strcmp(op, ">") == 0;
}

int is_empty(Cmd *exec_cmd)
{
    return exec_cmd->exec.args[0] == NULL;
}

IO_Type get_io_type(char *io_op) {
    if(strcmp(io_op, "<") == 0) {
        return IO_INPUT;

    } else if(strcmp(io_op, ">") == 0) {
        return IO_OUTPUT;
    }

    fprintf(stderr, "Syntax error: unhandled I/O redirection operator: %s\n", io_op);
    return -1;
}

Cmd *init_executble(char **args, int num)
{
    Cmd *exec_cmd = malloc(sizeof (Cmd));
    if(exec_cmd == NULL) {
        perror("malloc error for executable node");
        exit(1);
    }

    exec_cmd->type = CMD_EXEC;
    exec_cmd->exec.args = args;
    exec_cmd->exec.num = num;

    return exec_cmd;
}

Cmd *init_nonblocking(Cmd **cmds, int num)
{
    Cmd *nonblocking = malloc(sizeof(Cmd));
    if(nonblocking == NULL) {
        perror("malloc error for non-blocking node");
        exit(1);
    }

    nonblocking->type = CMD_NONBLOCKING;
    nonblocking->binary_cmd.cmds = cmds;
    nonblocking->binary_cmd.num = num;

    return nonblocking;
}

Cmd *init_sequential(Cmd **cmds, int num)
{
    Cmd *nonblocking = malloc(sizeof(Cmd));
    if(nonblocking == NULL) {
        perror("malloc error for sequential node");
        exit(1);
    }

    nonblocking->type = CMD_SEQUENTIAL;
    nonblocking->binary_cmd.cmds = cmds;
    nonblocking->binary_cmd.num = num;

    return nonblocking;
}

Cmd *init_subshell(Cmd *cmd)
{
    Cmd *subshell = malloc(sizeof(Cmd));
    if(subshell == NULL) {
        perror("malloc error for subshell node");
        exit(1);
    }

    subshell->type = CMD_SUBSHELL;
    subshell->unary_cmd = cmd;

    return subshell;
}

Cmd *init_pipe(Cmd **cmds, int num)
{
    Cmd *pipe = malloc(sizeof(Cmd));
    if(pipe == NULL) {
        perror("malloc error for pipe node");
        exit(1);
    }

    pipe->type = CMD_PIPE;
    pipe->binary_cmd.cmds = cmds;
    pipe->binary_cmd.num = num;

    return pipe;
}

Cmd *init_and(Cmd **cmds, int num)
{
    Cmd *and = malloc(sizeof(Cmd));
    if(and == NULL) {
        perror("malloc error for and node");
        exit(1);
    }

    and->type = CMD_AND;
    and->binary_cmd.cmds = cmds;
    and->binary_cmd.num = num;

    return and;
}

Cmd *init_or(Cmd **cmds, int num)
{
    Cmd *or = malloc(sizeof(Cmd));
    if(or == NULL) {
        perror("malloc error for or node");
        exit(1);
    }

    or->type = CMD_OR;
    or->binary_cmd.cmds = cmds;
    or->binary_cmd.num = num;

    return or;
}

Cmd *init_io_redirect(Cmd *cmd, IO_Type io_type, char *file_name)
{
    Cmd *redirect = malloc(sizeof(Cmd));
    if(redirect == NULL) {
        perror("malloc error for io redirection node");
        exit(1);
    }

    redirect->type = CMD_IO_REDIRECT;
    redirect->io_redirect.cmd = cmd;
    redirect->io_redirect.io_type = io_type;
    redirect->io_redirect.file_name = file_name;

    return redirect;
}

void free_exectble_cmd(Cmd *cmd)
{
    for(char **arg_ptr = cmd->exec.args; *arg_ptr != NULL; arg_ptr++) {
        free(*arg_ptr);
    }

    free(cmd->exec.args);
}

void free_unary_cmd(Cmd *cmd)
{
    free_cmd(cmd->unary_cmd);
}

void free_binary_cmd(Cmd *cmd)
{
    for(int i = 0; i < cmd->binary_cmd.num; i++) {
        free_cmd(cmd->binary_cmd.cmds[i]);
    }
    free(cmd->binary_cmd.cmds);
}

void free_io_redirect_cmd(Cmd *cmd)
{
    free(cmd->io_redirect.file_name);
    free_cmd(cmd->io_redirect.cmd);
}

void free_cmd(Cmd *cmd)
{
    if(cmd == NULL) return;

    switch (cmd->type) {
    case CMD_EXEC:
        free_exectble_cmd(cmd);
        break;

    case CMD_SUBSHELL:
        free_unary_cmd(cmd);
        break;
    
    case CMD_NONBLOCKING:
    case CMD_SEQUENTIAL:
    case CMD_PIPE:
    case CMD_AND:
    case CMD_OR:
        free_binary_cmd(cmd);
        break;

    case CMD_IO_REDIRECT:
        free_io_redirect_cmd(cmd);
        break;

    default:
        fprintf(stderr, "Error: Cannot free unhandled type of command\n");
        exit(1);
    }
    
    free(cmd);
}

void indent(int level)
{
    for(int i = 0; i < level; i++) {
        printf("\t");
    }
}

void print_unary_cmd(Cmd *cmd, char *type, int level)
{
    indent(level);
    printf("{type: %s}---\n", type);
    print_cmd_internal(cmd->unary_cmd, ++level);
}

void print_binary_cmd(Cmd *cmd, char *type, int level)
{
    indent(level);
    printf("{type: %s}---\n", type);

    for(int i = 0; i < cmd->binary_cmd.num; i++) {
        print_cmd_internal(cmd->binary_cmd.cmds[i], level + 1);
    }
}

void print_io_redirect(Cmd *cmd, int level)
{
    char *io_type;

    switch(cmd->io_redirect.io_type) {
    
    case IO_INPUT:
        io_type = "input redirection";
        break;

    case IO_OUTPUT:
        io_type = "output redirection";
        break;

    default:
        fprintf(stderr, "Error: unhandled I/O node in printing\n");
        exit(1);
    }

    indent(level);
    printf("{type: %s, new source/destination: %s}---\n", io_type, cmd->io_redirect.file_name);
    print_cmd_internal(cmd->io_redirect.cmd, ++level);
}

void print_cmd_internal(Cmd *cmd, int level)
{
    switch(cmd->type) {

    case CMD_EXEC:
        indent(level++);
        printf("{type: exec}---");
        print_str_arr(cmd->exec.args);
        break;

    case CMD_SUBSHELL:
        print_unary_cmd(cmd, "subshell", level);
        break;

    case CMD_IO_REDIRECT:
        print_io_redirect(cmd, level);
        break;

    case CMD_NONBLOCKING:
        print_binary_cmd(cmd, "non-blocking", level);
        break;

    case CMD_SEQUENTIAL:
        print_binary_cmd(cmd, "sequential", level);
        break;

    case CMD_PIPE:
        print_binary_cmd(cmd, "pipe", level);
        break;
    
    case CMD_AND:
        print_binary_cmd(cmd, "and", level);
        break;
    
    case CMD_OR:
        print_binary_cmd(cmd, "or", level);
        break;
    
    default:
        fprintf(stderr, "Error on printing command of unknown type\n");
        exit(1);
    }
}

Cmd *dup_exec(Cmd *executable_cmd)
{
    if(executable_cmd == NULL) {
        return NULL;
    }

    int n = executable_cmd->exec.num;
    char **args = malloc(sizeof(char *) * (n + 1));
    if(args == NULL) {
        perror("malloc error during command duplication");
        exit(1);
    }

    for(int i = 0; i < n; i++) {
        args[i] = strdup(executable_cmd->exec.args[i]);
    }
    args[n] = NULL;

    return init_executble(args, n);   
}