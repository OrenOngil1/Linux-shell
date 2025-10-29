#ifndef SCANNER_H
#define SCANNER_H

//Definition of scanner functions

//Returns an array of tokens: strings of operators, special characters, names of built-in functions, general use words etc.
char **scan(char *line);

//Frees all memory allocated to tokens.
void free_tokens(char **tokens);

//Prints out tokens in an array format.
void print_tokens(char **tokens);

#endif