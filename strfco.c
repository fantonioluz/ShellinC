#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include "comando.h"
#include "strfco.h"

#define MAX_ARGUMENTS 4096



void tokenizeString(char *input, char **tokens, int *numTokens) {
    char *token = strtok(input, ";");
    *numTokens = 0;
    while (token != NULL && *numTokens < MAX_ARGUMENTS) {
        tokens[(*numTokens)++] = token;
        token = strtok(NULL, ";");
    }
    tokens[*numTokens] = NULL;
}

void tokenizepipe(char *input, char **tokens, int *numTokens) {
    char *token = strtok(input, "|");
    *numTokens = 0;
    while (token != NULL && *numTokens < MAX_ARGUMENTS) {
        tokens[(*numTokens)++] = token;
        token = strtok(NULL, "|");
    }
    tokens[*numTokens] = NULL;
}