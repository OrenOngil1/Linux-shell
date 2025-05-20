#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"


#define Free(x) if(x) free((void *)(x))

/* array of string representations of operators for the parser */
//The longer the operator, the more at the beginnig it must be!
const char *OPERATORS[] = {"<", ">", "&", "|", NULL}; 

int isOperator(char *str)
{
    const char * const *operator_ptr = OPERATORS;
    char *operator;
    int n;
    while((operator = *operator_ptr))
    {
        n = strlen(operator);
        if(!strncmp(str, operator, n))
        {
            return 1;
        }
        operator_ptr++;
    }
    return 0;
}

char *firstWord(char *line)
{
    int n = 0;
    while(line[n] != '\0' && !isOperator(&line[n]))
    {
        n++;
    }
    if(!n) return NULL;
    char *word = calloc(n + 1, sizeof(char));
    strncpy(word, line, n);
    word[n] = '\0';
    return word;
}

int parseNext(ParsedCmdLine *pLine, char *line)
{
    pLine->next = parseCmdLine(line);
    if(!pLine->next) return -1;
    return -2;
}

void freeParsedCmdLine(ParsedCmdLine *pCmdLine)
{
    if(!pCmdLine) return;

    for(int i = 0; i < pCmdLine->argCount; i++)
    {
        free(pCmdLine->args[i]);
    }
    Free(pCmdLine->inputRedirect);
    Free(pCmdLine->outputRedirect);
    freeParsedCmdLine(pCmdLine->next);
    free(pCmdLine);
}

int redirectInput(ParsedCmdLine *pLine, char *line)
{
    Free(pLine->inputRedirect);
    pLine->inputRedirect = NULL;
    pLine->inputRedirect = firstWord(line);
    if(!pLine->inputRedirect || pLine->inputRedirect[0] == '\0')
    {
        return -1;
    }
    return strlen(pLine->inputRedirect);
}

int redirectOutput(ParsedCmdLine *pLine, char *line)
{
    Free(pLine->outputRedirect);
    pLine->outputRedirect = NULL;
    pLine->outputRedirect = firstWord(line);
    if(!pLine->outputRedirect || pLine->outputRedirect[0] == '\0')
    {
        return -1;
    }
    return strlen(pLine->outputRedirect);
}

int makeBlocking(ParsedCmdLine *pLine, char *line)
{
    pLine->blocking = 1;
    return 1;
}

int checkForOperator(ParsedCmdLine *pLine, char *line)
{
    if(line[0] == '|')
    {
        return parseNext(pLine, line + 1);
    }
    else if(line[0] == '<')
    {
        return redirectInput(pLine, line + 1);
    }
    else if(line[0] == '>')
    {
        return redirectOutput(pLine, line + 1);
    }
    else if(line[0] == '&')
    {
        return makeBlocking(pLine, line + 1);
    }
    return 0;
}

ParsedCmdLine *parseCmdLine(char *line)
{
    if(!line || line[0] == '\0' || strlen(line) > ARG_MAX)
    {
        return NULL;
    }

    ParsedCmdLine *pLine = malloc(sizeof(ParsedCmdLine));
    pLine->argCount = 0;
    pLine->inputRedirect = NULL;
    pLine->outputRedirect = NULL;
    pLine->blocking = 0;
    pLine->next = NULL;

    int i = 0;
    int j;
    char *arg;
    while(pLine->argCount < MAX_ARGS && line[i] != '\0')
    {

        while(line[i] == ' ') i++;

        j = checkForOperator(pLine, &line[i]);
        
        if(j > 0)
        {
            arg = firstWord(&line[i]);
            if(arg) 
            {
                pLine->args[pLine->argCount++] = arg;
                i += strlen(arg);
            }
            else i++;
        }
        else if(j == -1)
        {
            freeParsedCmdLine(pLine);
            return NULL;
        }
        else if(j == -2)
        {
            break;
        }
    }

    pLine->str = calloc(i + 1, sizeof(char));
    strncpy(pLine->str, line, i);
    pLine->args[pLine->argCount] = NULL;
    return pLine;
}

int LineLengthWithPipe(ParsedCmdLine *pCmdLine)
{
    int n = 0;
    ParsedCmdLine *curr = pCmdLine;
    while(curr)
    {
        n += strlen(curr->str);
        curr = curr->next;
        if(curr) n++;
    }
    return n;
}

char *unparseFullCmdLine(ParsedCmdLine *pCmdLine)
{
    int length = LineLengthWithPipe(pCmdLine);
    char* str = calloc(length + 1, sizeof(char));
    ParsedCmdLine *curr = pCmdLine;
    while(curr)
    {
        strcat(str, curr->str);
        curr = curr->next;
        if (curr) strcat(str, "|");
    }
    return str;
}
