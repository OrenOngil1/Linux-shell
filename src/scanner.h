#ifndef SCANNER_H
#define SCANNER_H

#include <stdlib.h>

//Definition of scanner functions

//Returns an array of tokens: strings of operators, special characters, names of built-in functions, general use words etc.
char **scan(char *line);

//Frees all memory allocated to tokens.
void free_tokens(char **tokens);

//Detectes if an operator occurs at the begging of line. 
//If so, returns a pointer to a duplicate string, else returns NULL.
char *get_op(char *line);

//Prints out tokens in an array format.
void print_tokens(char **tokens);

#endif