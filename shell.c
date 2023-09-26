#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "comando.h"
#include "strfco.h"


#define MAX_ARGUMENTS 4096

char lastCommand[MAX_ARGUMENTS];


int main(int argc, char *argv[]) {
    char command[MAX_ARGUMENTS];
    system("clear");
    int fimDoArquivo = 0;

    if (argc == 2 && fimDoArquivo == 0) {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        int style = 0;  
        while (1) {
            fgets(command, sizeof(command), file);
            size_t len = strlen(command);
            if (len > 0 && command[len - 1] == '\n') {
                command[len - 1] = '\0';
            }
            if (strcmp(command, "exit") == 0 || feof(stdin)) {
                exit(EXIT_SUCCESS);
            }

            if (strcmp(command, "style parallel") == 0) {
                style = 1;
                continue; 
            }
            if (strcmp(command, "style sequential") == 0) {
                style = 0;
                continue;
            }


            if (strcmp(command, "!!") == 0) {
                if (strlen(lastCommand) > 0) {
                    if(style == 0){
                        
                        executeCommandSeq(lastCommand);
                    }
                    else if(style == 1){
                        executeComandPar(lastCommand);
                    }
                    
                } else {
                    printf("No previous command to execute.\n");
                }
             }            


            else if(style == 0){
                executeCommandSeq(command);
            }
            else if(style == 1){
                executeComandPar(command);
            }
            strcpy(lastCommand, command);
            if(feof(file)){
                fimDoArquivo = 1;
                printf("Fim do arquivo, finalizando sua sessão...\n");
                break;
            }

        }

        fclose(file);
    } 
    
    else if (argc == 1 || fimDoArquivo == 1) {

        int style = 0;
        while (1) {
            if (style == 0) {
                printf("falf seq> ");
            } else if (style == 1) {
                printf("falf par> ");
            }

            fgets(command, sizeof(command), stdin);

            size_t len = strlen(command);
            if (len > 0 && command[len - 1] == '\n') {
                command[len - 1] = '\0';
            }

            if (strcmp(command, "exit") == 0 || feof(stdin)) {
                break;
            }

            if (strcmp(command, "style parallel") == 0) {
                style = 1;
                continue; 
            }
            if (strcmp(command, "style sequential") == 0) {
                style = 0;
                continue;
            }


            if (strcmp(command, "!!") == 0) {
                if (strlen(lastCommand) > 0) {
                    if(style == 0){
                        
                        executeCommandSeq(lastCommand);
                    }
                    else if(style == 1){
                        executeComandPar(lastCommand);
                    }
                    
                } else {
                    printf("No previous command to execute.\n");
                }
             }            


            else if(style == 0){
                executeCommandSeq(command);
            }
            else if(style == 1){
                executeComandPar(command);
            }
            strcpy(lastCommand, command);

        }

    }

    return 0;
}
