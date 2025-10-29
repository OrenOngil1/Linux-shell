#ifndef OPERATOR_H
#define OPERATOR_H

//Operator header, containing the operator tables,
//its entries being pairs of the oprerator and its binding powers as tuples in the hierarchy.

//Definition of an Operator.
typedef struct operator {
    char *str;
    int l_bp;
    int r_bp; 
} Operator;

//Operator table of operators that do not require a special format.
extern const Operator simple_ops[];

//Returns a duplicate of the operator at the start of this line, or NULL if none found.
char *dup_op(char *line);

//Returns true if token is an operator, false otherwise.
int is_op(char *token);

//Returns Operator view of the op.
Operator get_op(char *op);

//Retunrs 1 if op is a terminator token, 0 otherwise.
int is_terminator(char *op);

//Retunrs 1 if op is a close bracket token, 0 otherwise.
int is_close_bracket(char *op);

#endif