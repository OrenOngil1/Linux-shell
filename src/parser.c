#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"


#define Free(x) if(x) free((void *)(x))

/* array of string representations of operators for the parser */
//The longer the operator, the more at the beginnig it must be!
const char *OPERATORS[] = {"<", ">", "&", "|", NULL}; 

char *isOperator(char *str)
{
    const char * const *operator_ptr = OPERATORS;
    const char *operator;
    int n;
    while((operator = *operator_ptr))
    {
        n = strlen(operator);
        if(!strncmp(str, operator, n))
        {
            char* op = calloc(strlen(operator) + 1, sizeof(char));
            strcpy(op, operator);
            return op;
        }
        operator_ptr++;
    }
    return NULL;
}

int skipWhiteSpaces(char *str, int i, int j)
{
    while(i < j && str[i] != '\0' && str[i] == ' ') i++;
    return i;
}

char *firstWord(char *line)
{
    int beginIndex = skipWhiteSpaces(line, 0, strlen(line));
    int endIndex = beginIndex;
    char *temp = NULL;
    while(line[endIndex] != '\0' && line[endIndex] != ' ' && !(temp = isOperator(&line[endIndex])))
    {
        endIndex++;
    }
    if(endIndex == beginIndex)
    {
        return NULL;
    }
    free(temp);
    char *word = calloc(endIndex - beginIndex + 1, sizeof(char));
    strncpy(word, &line[beginIndex], endIndex - beginIndex);
    return word;
}


int parseNext(ParsedCmdLine *pLine, char *line)
{
    pLine->next = parseCmdLine(line);
    if(!pLine->next) return -1;
    return 1;
}

void freeCmdLine(ParsedCmdLine *pCmdLine)
{
    if(!pCmdLine) return;

    for(int i = 0; i < pCmdLine->argCount; i++)
    {
        free(pCmdLine->args[i]);
    }
    Free(pCmdLine->inputRedirect);
    Free(pCmdLine->outputRedirect);
    freeCmdLine(pCmdLine->next);
    free(pCmdLine->str);
    free(pCmdLine);
}

char *unparseCmdLine(ParsedCmdLine *pCmdLine)
{
    int i = skipWhiteSpaces(pCmdLine->str, 0, strlen(pCmdLine->str));
    return &pCmdLine->str[i];
}

int isInvalidInput(char *line)
{
    return !line || line[0] == '\0' || strlen(line) > ARG_MAX;
}

char *makeStr(char *line, int endIndex)
{
    int beginIndex = 0;
    char *str = calloc(endIndex + 1 - beginIndex, sizeof(char));
    strncpy(str, &line[beginIndex], endIndex - beginIndex);
    return str;
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
    return 0;
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
    return 0;
}

int setNonBlocking(ParsedCmdLine *pLine, char *line)
{
    pLine->blocking = 0;
    return 0;
}

int applyOperator(ParsedCmdLine *pLine, char *line)
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
        return setNonBlocking(pLine, line + 1);
    }
    return 0;
}

ParsedCmdLine *parseCmdLine(char *line)
{
    if(isInvalidInput(line))
    {
        return NULL;
    }

    ParsedCmdLine *pLine = calloc(1, sizeof(ParsedCmdLine));
    pLine->argCount = 0;
    pLine->inputRedirect = NULL;
    pLine->outputRedirect = NULL;
    pLine->blocking = 1;
    pLine->next = NULL;

    int i = 0;
    int j;
    char *arg;
    while(pLine->argCount < MAX_ARGS && line[i] != '\0')
    {
        i = skipWhiteSpaces(line, i, strlen(line));
        j = 0;
        if((arg = isOperator(&line[i]))) 
        {
            if(strcmp(arg, "|"))
            {
                pLine->args[pLine->argCount++] = arg;
            }
            else {
                free(arg);
            }
            j = applyOperator(pLine, &line[i]);
            if(!j) 
            {
                i += strlen(arg);
                continue;
            }
        }
        
        if(j == 0)
        {
            arg = firstWord(&line[i]);
            if(arg) 
            {
                pLine->args[pLine->argCount++] = arg;
                i += strlen(arg);
            }
            else if(line[i] == '\0')
            {
                break;
            }
            else {
                i++;
            }
        }
        else if(j == -1)
        {
            freeCmdLine(pLine);
            return NULL;
        }
        else if(j == 1)
        {
            break;
        }
    }

    pLine->str = makeStr(line, i);
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
