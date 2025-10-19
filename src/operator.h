#ifndef OPERATOR_H
#define OPERATOR_H

//Operator header, containing the operator tables,
//its entries being pairs of the oprerator and its precedence in the hierarchy.

//Type of Entry in operator tables.
typedef struct entry {
    char *key;
    int value;
} Entry;

//Operator table of operators that do not require a special format.
extern const Entry simple_ops[];

//Returns a duplicate of the operator at the start of this line, or NULL if none found.
char *get_op(char *line);

//Returns true if token is an operator, false otherwise.
int is_op(char *token);

//Returns the precedence of the operator in the hierarchy.
int precedence(char *op);

#endif