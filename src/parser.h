#include <linux/limits.h>

#define MAX_ARGS 512

typedef struct ParsedCmdLine {
    char *args[MAX_ARGS];           /*The parsed arguments, the command name is in args[0] */
    int argCount;                   /*how many arguments are*/
    char *inputRedirect;            /*contains name of file to redirect input from, otherwise is NULL*/
    char *outputRedirect;           /*contains name of file to redirect output into, otherwise is NULL*/
    int blocking;                   /*equals 1 if this command is meant to be blocking, otherwise is 0*/
    struct ParsedCmdLine *next;     /*holds the next ParsedCmdArg in a pipeline, if there is, otherwise is NULL */
    char *str;                      /*contains a string representation of the original cmd line up to a pipe operator, if threre's any*/
} ParsedCmdLine;


/* Parses the given parameter arg into a ParsedCmdLine */
/* Returns NULL if parsing an invalid line */
/* When sucsseful, retuns a pointer to a ParseCmdLinr */
ParsedCmdLine *parseCmdLine(char *line);

/*Frees the memory held by pCmdLine's members and pCmdLine itself*/
void freeCmdLine(ParsedCmdLine *pCmdLine);

/* Returns a reference to a string representation of pCmdLine up to the least-right pipe operator, if there's any.*/
char *unparseCmdLine(ParsedCmdLine *pCmdLine);

/* Returns pointer to a string representation of the original full line, including processes down the pipeline */
char *unparseFullCmdLine(ParsedCmdLine *pCmdLine);