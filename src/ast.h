#ifndef AST_H
#define AST_H

#define print_cmd(cmd) print_cmd_internal(cmd, 0)
//
//Defenition of the context free grammer which defines the shell's syntax.
//
//<io-op> ::= < | >                                                                     ;; enum IO_Type
//<exec> ::= args*                                                                      ;; Exec(char **args)
//<non-blocking> ::= <cmd> (& <cmd>)+ | <cmd> (& <cmd>)* &                              ;; nonblocking(Cmd **cmds, int num)
//<sequential> ::= <cmd> (; <cmd>)+ | <cmd> (; <cmd>)* ;                                ;; sequential(Cmd **cmds, int num)
//<io-redirect> ::= <cmd> <io-op> <file_name>                                           ;; IO_Redirect(Cmd *cmd, io_op *op, file_name) 
//<subshell> ::= (<cmd>)                                                                ;; subshell(Cmd *cmd)
//<and> ::= <cmd> (&& <cmd>+)+                                                          ;; and(Cmd **cmds, int num)
//<or> ::= <cmd> (|| <cmd>+)+                                                           ;; or(Cmd **cmds, int num)
//<pipe> ::= <cmd> (| <cmd>+)+                                                          ;; pipe(Cmd **cmds, int num)
//
//
//<cmd> ::= <exec> 
//          | <blocking>
//          | <io-redirect>
//          | <subshell>
//          | <pipe>
//          | <and>
//          | <or>
//
//
//

//Allows for expansion to more I/O redirection operators.
typedef enum io_type {
    IO_INPUT,
    IO_OUTPUT
} IO_Type;
//
//
//Types of commands in AST.
typedef enum {
    CMD_EXEC,
    CMD_NONBLOCKING,
    CMD_SEQUENTIAL,
    CMD_SUBSHELL,
    CMD_IO_REDIRECT,
    CMD_PIPE,
    CMD_AND,
    CMD_OR
} Cmd_type;


//The atomic type, describes argument to be handed to an exec() function.
typedef struct exec_cmd {
    char **args;
    int num;
} Exec_cmd;


//A command which needs one or more operands
typedef struct binary_cmd{
    struct cmd **cmds;
    int num; 
} Binary_cmd;


//A command to redirect I/O.
typedef struct io_redirect{
    IO_Type io_type;
    struct cmd *cmd;
    char *file_name; 
} IO_Redirect;


//Commands; the nodes ot the AST.
typedef struct cmd {
    Cmd_type type;

    union {
        Exec_cmd exec;

        struct cmd *unary_cmd; 

        Binary_cmd binary_cmd;

        IO_Redirect io_redirect;
    };

} Cmd;



//Main parsing function
Cmd *parse(char **tokens);

//Utility functions for initiating Cmd nodes in the AST.

//Initializes an unary Cmd.
Cmd *init_unary(char *op, Cmd *operand);

//Returns 1 if op is an unary operation or 0 otherwise.
int is_unary(char *op);

//Initializes a binary Cmd.
Cmd *init_binary(char *op, Cmd *operand1, Cmd *operand2);

//Returns 1 if op is a binary operation or 0 otherwise.
int is_binary(char *op);

//Returns 1 if op is an I/O redirection operation or 0 otherwise.
int is_io(char *op);

//Returns 1 if the exec type exec_cmd holds no arguments or 0 otherwise.
int is_empty(Cmd *exec_cmd);

//Retunrs the IO_type of io_op.
IO_Type get_io_type(char *io_op);

//Initializes an executable Cmd.
Cmd *init_executble(char **args, int num);

//Initializes a nonblocking Cmd.
Cmd *init_nonblocking(Cmd **cmd, int num);

//Initializes a sequential Cmd.
Cmd *init_sequential(Cmd **cmds, int num);

//Initializes a subshell Cmd.
Cmd *init_subshell(Cmd *cmd);

//Initializes a pipline Cmd.
Cmd *init_pipe(Cmd **cmds, int num);

//Initializes an and Cmd.
Cmd *init_and(Cmd **cmds, int num);

//Initializes an or Cmd.
Cmd *init_or(Cmd **cmds, int num);

//Initializes an I/O redirection Cmd.
Cmd *init_io_redirect(Cmd *cmd, IO_Type io_type , char *file_name);

//Initializes an empty executable Cmd.
Cmd *init_empty();

//Frees the memory allocated to cmd and its args array.
void free_exectble_cmd(Cmd *cmd);

//Frees the memory allocated to an unary Cmd.
void free_unary_cmd(Cmd* cmd);

//Frees the memory allocated to a binary Cmd.
void free_binary_cmd(Cmd *cmd);

//Frees the memory allocated to an I/O redirection Cmd.
void free_io_redirect_cmd(Cmd *cmd);

//Frees the memory allocated to cmd.
void free_cmd(Cmd *cmd);

//Internal printing function, for user: use the print_cmd macro.
void print_cmd_internal(Cmd *cmd, int level);

//Duplicates executable_cmd, assumes it's of type CMD_EXEC.
Cmd *dup_exec(Cmd *executable_cmd);

#endif