#ifndef SHELL_H
#define SHELL_H

//Reads input from user until newline character that isn't escaped.
//On success returns the length of the input string, otherwise returns -1.
int read_line(char *buffer, int len);

//Prints the current working directory.
void print_cwd();

#endif