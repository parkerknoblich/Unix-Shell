#include <stdio.h>  // printf
#include <string.h> // string
#include <unistd.h> // fork, execlp, pipe
#include <wait.h>   // wait
#include <assert.h> // assert
#include <stdlib.h>
#include <fcntl.h>

#define MAX_LINE 80

void freeMemory(char* args[], int numOfArgs) {
    int i = 0;
    while ((args[i] != NULL) && (i < numOfArgs) && (i < MAX_LINE)) {
        free(args[i++]);
    }
}


int getCommand(char* args[], int* numOfArgs, int* waitFlag) {
    char line[MAX_LINE];
    fgets(line, MAX_LINE, stdin);
    size_t length = strlen(line);
    line[length - 1] = '\0';
    if (strcmp(line, "exit") == 0) {
        return -1;
    } else if (strcmp(line, "!!") == 0) {
        if (*numOfArgs < 1) {
            printf("No commands in history.\n");
        }
        return 0;
    } else {
        freeMemory(args, *numOfArgs);
        *numOfArgs = 0;
        if (strchr(line, '&') != NULL) {
            int i, j;
            for (i = 0, j = 0; i < length; i++) {
                if (line[i] != '&') {
                    line[j] = line[i];
                    j++;
                }
            }
            line[j] = '\0';
            *waitFlag = 1;
        } else {
            *waitFlag = 0;
        }
        char* token = strtok(line, " ");
        while (token != NULL) {
            args[*numOfArgs] = strdup(token);
            token = strtok(NULL, " ");
            (*numOfArgs)++;
        }
        args[*numOfArgs] = NULL;
        return 1;
    }
}

int main() {
    char *args[MAX_LINE];
    int numOfArgs = 0;
    int pipeFlag = 0;
    int waitFlag = 0;
    while (1) {
        printf("osh>");
        fflush(stdout);
        int userInputResult = getCommand(args, &numOfArgs, &waitFlag);
        if (userInputResult == -1) {
            break;
        }
        int forkret = fork();
        if (forkret == 0) {
            int redirectionFlag = 0;
            int redirectionFile;
            if (args[1] != NULL) {
                if (strcmp(args[1], "<") == 0) {                 // input
                    redirectionFile = open(args[2], O_RDONLY);
                    dup2(redirectionFile, STDIN_FILENO);
                    args[1] = args[2] = NULL;
                    redirectionFlag = 1;
                } else if (strcmp(args[1], ">") == 0) {         // output
                    redirectionFile = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(redirectionFile, STDOUT_FILENO);
                    args[1] = args[2] = NULL;
                    redirectionFlag = 2;
                } else if (strcmp(args[1], "|") == 0) {         // pipe
                    pipeFlag = 1;
                    printf("PIPE");
                }
            }
            int executeResult = execvp(args[0], args);
            assert(executeResult >= 0);
            if (redirectionFlag == 1) {
                close(STDIN_FILENO);
            } else if (redirectionFlag == 2) {
                close(STDOUT_FILENO);
            }
            close(redirectionFile);
            return 0;
        }  else {
            int commandResult;
            if (waitFlag) {
               printf("Parent waiting\n");
                wait(&commandResult);
            }
            printf("Command returned %d\n", commandResult);
        }
    }
}

