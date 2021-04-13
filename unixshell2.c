#include <stdio.h>  // printf
#include <string.h> // string
#include <unistd.h> // fork, execlp, pipe
#include <wait.h>   // wait
#include <assert.h> // assert
#include <stdlib.h> // free
#include <fcntl.h>  // open

#define MAX_LINE 80 // max amount of command arguments

// frees memory from command arguments
void freeMemory(char* args[], int numOfArgs) {
    int i = 0;
    while ((args[i] != NULL) && (i < numOfArgs) && (i < MAX_LINE)) {
        free(args[i++]);
    }
}

// reads command from user and formats/stores appropriately for execution
int getCommand(char* args[], int* numOfArgs, int* waitFlag) {
    char line[MAX_LINE];                                          // holds user input
    fgets(line, MAX_LINE, stdin);                                 // stores user input in line
    size_t length = strlen(line);                                 // length of user input
    line[length - 1] = '\0';                                      // remove new line character at end of user input
    if (strcmp(line, "exit") == 0) {                              // case user input = "exit"
        return 1;                                                 // return 1 if user inputs "exit"
    } else if (strcmp(line, "!!") == 0) {                         // case user input = "!!"
        if (*numOfArgs < 1) {                                     // check if a previous command has been executed
            printf("No commands in history.\n");           // if not, print error message
        }
        return 0;
    } else {                                                      // case user input is a normal command
        freeMemory(args, *numOfArgs);                             // free memory
        *numOfArgs = 0;                                           // set number of arguments to 0
        if (strchr(line, '&') != NULL) {                       // check if user inputted a "&" and remove if they did
            int i, j;
            for (i = 0, j = 0; i < length; i++) {
                if (line[i] != '&') {
                    line[j] = line[i];
                    j++;
                }
            }
            line[j] = '\0';
            *waitFlag = 1;                                        // set wait flag if they inputted a "&"
        }
        char* token = strtok(line, " ");                    // pointer that walks through user input, delimited by " "
        while (token != NULL) {                                   // walk through user input
            args[*numOfArgs] = strdup(token);                     // add each token to argument string
            token = strtok(NULL, " ");
            (*numOfArgs)++;                                       // increment number of arguments
        }
        args[*numOfArgs] = NULL;                                  // set last index in argument string to NULL
        return 0;
    }
}

// main function
int main() {
    char *args[MAX_LINE];
    int numOfArgs = 0;
    int pipeFlag = 0;
    int waitFlag = 0;
    while (1) {
        printf("osh>");
        fflush(stdout);
        int userInputResult = getCommand(args, &numOfArgs, &waitFlag);
        if (userInputResult == 1) {
            break;
        }
        int mainForkReturn = fork();
        if (mainForkReturn == 0) {
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
                    printf("PIPE\n");
                    pipeFlag = 1;
                    int fd[2];
                    pipe(fd);
                    int pipeForkReturn = fork();
                    if (pipeForkReturn == 0) {  // pipe child 1
                        close(fd[0]);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                        char* commandOneArgs[2];
                        commandOneArgs[0] = args[0];
                        commandOneArgs[1] = NULL;
                        execvp(commandOneArgs[0], commandOneArgs);
                    } else {
                        wait(NULL);
                        close(fd[1]);
                        dup2(fd[0], STDIN_FILENO);
                        close(fd[0]);
                        char* commandTwoArgs[2];
                        commandTwoArgs[0] = args[2];
                        commandTwoArgs[1] = NULL;
                        execvp(commandTwoArgs[0], commandTwoArgs);
                    }
                    close(fd[0]);
                    close(fd[1]);
                } else if (strcmp(args[1], "whoami") == 0) {
                    int whoAmIForkReturn = fork();
                    if (whoAmIForkReturn == 0) {
                        char* commandOneArgs[2];
                        commandOneArgs[0] = args[1];
                        commandOneArgs[1] = NULL;
                        execvp(commandOneArgs[0], commandOneArgs);
                    } else {
                        wait(NULL);
                        char* commandTwoArgs[2];
                        commandTwoArgs[0] = args[0];
                        commandTwoArgs[1] = NULL;
                        execvp(commandTwoArgs[0], commandTwoArgs);
                    }
                }
            }
            if (pipeFlag == 0) {
                int executeResult = execvp(args[0], args);
                assert(executeResult >= 0);
                if (redirectionFlag == 1) {
                    close(STDIN_FILENO);
                } else if (redirectionFlag == 2) {
                    close(STDOUT_FILENO);
                }
                close(redirectionFile);
                return 0;
            }
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