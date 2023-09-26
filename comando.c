#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "comando.h"
#include "strfco.h"


#define MAX_ARGUMENTS 4096

struct BackgroundProcess {
    pid_t pid;
    pthread_t tid;
    int active;
    int activeThread; 
    int position;
};

struct BackgroundProcess backgroundProcesses[MAX_ARGUMENTS];
int numBackgroundProcesses = 0;
int countbg = 0;



void executeComandPipeSeq(char *command) {
    char *args[MAX_ARGUMENTS];
    int numTokens;
    tokenizepipe(command, args, &numTokens);

    int pipech[2];
    if (pipe(pipech) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2;
    pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        close(pipech[0]);
        dup2(pipech[1], STDOUT_FILENO);
        close(pipech[1]);
        char *args1[MAX_ARGUMENTS];
        int i = 0;
        char *token = strtok(args[0], " ");
        while (token != NULL) {
            args1[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args1[i] = NULL;
        execvp(args1[0], args1);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        pid2 = fork();

        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            close(pipech[1]);
            dup2(pipech[0], STDIN_FILENO);
            close(pipech[0]);
            char *args2[MAX_ARGUMENTS];
            int i = 0;
            char *token = strtok(args[1], " ");
            while (token != NULL) {
                args2[i] = token;
                i++;
                token = strtok(NULL, " ");
            }
            args2[i] = NULL;
            execvp(args2[0], args2);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            close(pipech[0]);
            close(pipech[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}



void processRedirection(char *command, FILE **input_stream, FILE **output_stream) {
    *input_stream = stdin;   
    *output_stream = stdout; 

    char *input_redirect = strchr(command, '<');
    if (input_redirect != NULL) {
        *input_redirect = '\0'; 
        char *input_file = strtok(input_redirect + 1, " ");
        while (*input_file == ' ') {
            input_file++; 
        }
        FILE *file_in = fopen(input_file, "r");
        if (file_in == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        *input_stream = file_in;
    }

    char *output_redirect = strchr(command, '>');
    if (output_redirect != NULL) {
        *output_redirect = '\0'; 
        char *output_file = strtok(output_redirect + 1, " ");
        while (*output_file == ' ') {
            output_file++; 
        }
        FILE *file_out = NULL;
        if (output_redirect[0] == '>' && output_redirect[1] == '>') {
            file_out = fopen(output_file, "a");
        } else {
            file_out = fopen(output_file, "w");
        }
        if (file_out == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        *output_stream = file_out;
    }
}



void executeCommandSeq(char *command) {
    char *args[MAX_ARGUMENTS];
    int numTokens;
    tokenizeString(command, args, &numTokens);

    

    for (int j = 0; j < numTokens; j++) {
        char *pipe = strchr(args[j], '|');
        if (pipe != NULL) {
            executeComandPipeSeq(args[j]);
            continue;
        }

        FILE *input_stream;
        FILE *output_stream;

        processRedirection(args[j], &input_stream, &output_stream);

        if(strchr(args[j], 'f') != NULL && strchr(args[j], 'g') != NULL){
            bringProcessToForeground(args[j]);
            continue;
        }

        int background = 0;
        char *last_char = args[j] + strlen(args[j]) - 1;
        if (*last_char == '&') {
            background = 1;
            countbg++;
            *last_char = '\0';
        }

        if (background) {
            pid_t pid = fork();
            
            if (pid == 0) {
                if (input_stream != stdin) {
                    dup2(fileno(input_stream), STDIN_FILENO);
                    fclose(input_stream);
                }
                if (output_stream != stdout) {
                    dup2(fileno(output_stream), STDOUT_FILENO);
                    fclose(output_stream);
                }

                char *command = strtok(args[j], " ");
                char *cmd_args[10];
                int k = 0;
                cmd_args[k++] = command;
                char *arg = strtok(NULL, " ");
                while (arg != NULL) {
                    cmd_args[k++] = arg;
                    arg = strtok(NULL, " ");
                }
                cmd_args[k] = NULL;
                execvp(cmd_args[0], cmd_args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork");
            } else {
                printf("[%d] %d\n", countbg, pid);
                backgroundProcesses[numBackgroundProcesses].pid = pid;
                backgroundProcesses[numBackgroundProcesses].active = 1;
                backgroundProcesses[numBackgroundProcesses].position = countbg;
                numBackgroundProcesses++;
            }
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                if (input_stream != stdin) {
                    dup2(fileno(input_stream), STDIN_FILENO);
                    fclose(input_stream);
                }
                if (output_stream != stdout) {
                    dup2(fileno(output_stream), STDOUT_FILENO);
                    fclose(output_stream);
                }

                char *command = strtok(args[j], " ");
                char *cmd_args[10];
                int k = 0;
                cmd_args[k++] = command;
                char *arg = strtok(NULL, " ");
                while (arg != NULL) {
                    cmd_args[k++] = arg;
                    arg = strtok(NULL, " ");
                }
                cmd_args[k] = NULL;
                execvp(cmd_args[0], cmd_args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork");
            } else {
                waitpid(pid, NULL, 0);
            }
        }
    }
}



void bringProcessToForeground(char *command) {
    char *args[10];
    int k = 0;
    char *token = strtok(command, " ");
    while (token != NULL) {
        args[k++] = token;
        token = strtok(NULL, " ");
    }
    args[k] = NULL;

    if (k != 2 || strcmp(args[0], "fg") != 0) {
        printf("Usage: fg [index]\n");
        return;
    }

    int position = atoi(args[1]);
    if (position >= 1 && position <= numBackgroundProcesses) {
        pid_t pid = backgroundProcesses[position - 1].pid;
        backgroundProcesses[position - 1].active = 0;
        numBackgroundProcesses--;
        if (waitpid(pid, NULL, 0) < 0) {
            perror("waitpid");
        }
    } else {
        printf("Invalid background process index\n");
    }
    countbg--;
}



void *executeCommand(void *arg) {
    char *command = (char *)arg;
    char *args[MAX_ARGUMENTS];
    int i = 0;



    char *token = strtok(command, " ");
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;



    FILE *input_stream = NULL;
    FILE *output_stream = NULL;

    processRedirection(command, &input_stream, &output_stream);
    

    int fd_in = fileno(input_stream);
    int fd_out = fileno(output_stream);

    if (input_stream != stdin) {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (output_stream != stdout) {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    if(system(args[0]) != 0){
        perror("system");
    }

    return NULL;
}



void executeComandPipePar(char *command) {
    char *args[MAX_ARGUMENTS];
    int numTokens;
    tokenizepipe(command, args, &numTokens);

    int pipech[2];
    if (pipe(pipech) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2;
    pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        close(pipech[0]);
        dup2(pipech[1], STDOUT_FILENO);
        close(pipech[1]);
        char *args1[MAX_ARGUMENTS];
        int i = 0;
        char *token = strtok(args[0], " ");
        while (token != NULL) {
            args1[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args1[i] = NULL;
        execvp(args1[0], args1);
        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        pid2 = fork();

        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            close(pipech[1]);
            dup2(pipech[0], STDIN_FILENO);
            close(pipech[0]);
            char *args2[MAX_ARGUMENTS];
            int i = 0;
            char *token = strtok(args[1], " ");
            while (token != NULL) {
                args2[i] = token;
                i++;
                token = strtok(NULL, " ");
            }
            args2[i] = NULL;
            execvp(args2[0], args2);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            close(pipech[0]);
            close(pipech[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}



void *executeComandPar(char *comand) {
    char *args[MAX_ARGUMENTS];
    int background = 0;
    int numTokens;
    tokenizeString(comand, args, &numTokens);

    pthread_t threads[numTokens];

    for(int i = 0; i < numTokens; i++){
        if(strchr(args[i], '&') != NULL){
            printf("background\n");
            background = 1;
            countbg++;
        }

        if(strchr(args[i], 'f') != NULL && strchr(args[i], 'g') != NULL){
            bringProcessToForeground(args[0]);
            return NULL;
        }

        FILE *input_stream = NULL;
        FILE *output_stream = NULL;

        if(background){
            pid_t pidbg = fork();
            if(pidbg == 0){
                if (input_stream != stdin) {
                    dup2(fileno(input_stream), STDIN_FILENO);
                    fclose(input_stream);
                }
                if (output_stream != stdout) {
                    dup2(fileno(output_stream), STDOUT_FILENO);
                    fclose(output_stream);
                }

                char *command = strtok(args[i], " ");
                char *cmd_args[10];
                int k = 0;
                cmd_args[k++] = command;
                char *arg = strtok(NULL, " ");
                while (arg != NULL) {
                    cmd_args[k++] = arg;
                    arg = strtok(NULL, " ");
                }
                cmd_args[k] = NULL;
                pthread_create(&threads[i], NULL, executeCommand, cmd_args[0]);
            } else if (pidbg < 0) {
                perror("fork");
            } else {
                printf("[%d] %d\n", countbg, pidbg);
                backgroundProcesses[numBackgroundProcesses].pid = pidbg;
                backgroundProcesses[numBackgroundProcesses].active = 1;
                backgroundProcesses[numBackgroundProcesses].position = countbg;
                numBackgroundProcesses++;
                background = 0;
            }

            
            }
    
    }


    pid_t pid = fork();
    if (pid == 0) {
        for (int j = 0; j < numTokens; j++) {
            char *pipe = strchr(args[j], '|');
            if (pipe != NULL) {
                executeComandPipePar(args[j]);
                continue;
            }else{
                pthread_create(&threads[j], NULL, executeCommand, (void *)args[j]);
            }   
        }
        for (int j = 0; j < numTokens; j++) {
            if (pthread_join(threads[j], NULL)) {
                printf("Error joining thread %d\n", j);
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
            pthread_exit(NULL);
        }
        
    } else if (pid < 0) {
        perror("fork");
    } else if(backgroundProcesses[numBackgroundProcesses].activeThread == 0){
        waitpid(pid, NULL, 0);
    }
    return NULL;
}



