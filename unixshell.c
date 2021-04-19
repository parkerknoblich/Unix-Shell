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
    while ((args[i] != NULL) && (i < numOfArgs) && (i < MAX_LINE / 2  + 1)) {
        free(args[i++]);
    }
}

// checks to see the type of wait condition the user inputted ("&" (don't wait) or ";" (wait) or nothing (wait))
int checkForWait(char line[], int length, char character, int* waitFlag) {
    if (strchr(line, character) != NULL) {                        // check if user inputted a "&" or ";" and remove if they did
        int i, j;
        for (i = 0, j = 0; i < length; i++) {
            if (line[i] != character) {
                line[j] = line[i];
                j++;
            }
        }
        line[j] = '\0';
        if (character == '&') {
            *waitFlag = 1;                                        // set wait flag if they inputted a "&"
        } else if (character == ';') {
            *waitFlag = 0;                                        // don't set wait flag if they inputted a ";"
        }
    }
    return *waitFlag;
}

// reads command from user and formats/stores appropriately for execution
int getCommand(char* args[], char prevArgs[], int* numOfArgs, int* waitFlag) {
    char line[MAX_LINE / 2 + 1];                                  // holds user input
    fgets(line, MAX_LINE / 2 + 1, stdin);                      // stores user input in line
    size_t length = strlen(line);                                 // length of user input
    line[length - 1] = '\0';                                      // remove new line character at end of user input
    if (strcmp(line, "exit") == 0) {                              // case: user input = "exit"
        return 1;
    } else if (strcmp(line, "!!") == 0) {                         // case: user input = "!!"
        if (*numOfArgs < 1) {                                     // check if a previous command has been executed
            printf("No commands in history.\n");           // if not, print error message
        } else {                                                  // else, print previous command to screen
            printf("Running previous command: %s\n", prevArgs);
        }
        return 0;
    } else {                                                      // case: user input is a normal command
        strcpy(prevArgs, line);
        freeMemory(args, *numOfArgs);                             // free memory
        *numOfArgs = 0;
        checkForWait(line, length, '&', waitFlag);       // check for type of wait
        checkForWait(line, length, ';', waitFlag);
        char* token = strtok(line, " ");
        while (token != NULL) {                                   // walk through user input and add each token to argument string
            args[*numOfArgs] = strdup(token);
            token = strtok(NULL, " ");
            (*numOfArgs)++;
        }
        args[*numOfArgs] = NULL;                                  // null-terminate end of arguments
        return 0;
    }
}

// runs the users command
int runCommand(char* args[], int hasExecuted, int waitFlag) {
    int mainForkReturn = fork();                                  // fork a child process
    if (mainForkReturn == 0) {                                    // child
        int redirectionFlag = 0;                                  // indicates which direction the text is being redirected, if applicable
        int redirectionFile;                                      // the user-specified text file, if applicable
        if (args[1] != NULL) {                                    // checks if multiple arguments are present
            if (strcmp(args[1], "<") == 0) {                      // case: user inputted ">"
                redirectionFile = open(args[2], O_RDONLY);
                dup2(redirectionFile, STDIN_FILENO);
                args[1] = args[2] = NULL;
                redirectionFlag = 1;
                hasExecuted = 0;
            } else if (strcmp(args[1], ">") == 0) {               // case: user inputted "<"
                redirectionFile = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(redirectionFile, STDOUT_FILENO);
                args[1] = args[2] = NULL;
                redirectionFlag = 2;
                hasExecuted = 0;
            } else if (strcmp(args[1], "|") == 0) {               // case: user inputted "|"
                int fd[2];
                pipe(fd);
                int pipeForkReturn = fork();                      // create child process
                if (pipeForkReturn == 0) {                        // pipe child
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                    char* commandOneArgs[2];
                    commandOneArgs[0] = args[0];
                    commandOneArgs[1] = NULL;
                    execvp(commandOneArgs[0], commandOneArgs);
                } else {                                          // pipe parent
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
                hasExecuted = 1;
            } else if (strstr(args[1], "-") == NULL) {// case: user inputted a second command
                int whoAmIForkReturn = fork();                      // create child process
                if (whoAmIForkReturn == 0) {                        // child process
                    char* commandOneArgs[2];
                    commandOneArgs[0] = args[1];
                    commandOneArgs[1] = NULL;
                    execvp(commandOneArgs[0], commandOneArgs);
                } else {                                            // parent process
                    wait(NULL);
                    char* commandTwoArgs[2];
                    commandTwoArgs[0] = args[0];
                    commandTwoArgs[1] = NULL;
                    execvp(commandTwoArgs[0], commandTwoArgs);
                }
                hasExecuted = 1;
            }
        }
        if (!hasExecuted) {                                         // check if execution has taken place
            int executeResult = execvp(args[0], args);          // if not, execute command
            assert(executeResult >= 0);
            if (redirectionFlag == 1) {                             // check redirectionFlag
                close(STDIN_FILENO);                                // close standard input on input
            } else if (redirectionFlag == 2) {
                close(STDOUT_FILENO);                               // close standard output on output
            }
            close(redirectionFile);                                 // close text file
            return 0;
        }
    }  else {                                                       // parent process
        int commandResult;
        if (!waitFlag) {                                             // wait if user inputted "&"
            printf("Parent waiting\n");
            wait(&commandResult);
        }
        printf("Command returned %d\n", commandResult);
    }
}

// main function
int main() {
    char* args[MAX_LINE / 2 + 1];                                    // holds command arguments that user inputted
    char prevArgs[MAX_LINE / 2 + 1];
    int numOfArgs = 0;                                               // tracks number of arguments
    int hasExecuted = 0;                                             // indicates if execution has taken place
    int waitFlag = 0;                                                // indicates if user inputted "&"
    while (1) {                                                      // run while user doesn't input "exit"
        printf("osh>");
        fflush(stdout);
        int userInputResult = getCommand(args, prevArgs, &numOfArgs, &waitFlag); // get user command
        if (userInputResult == 1) {                                   // leave on "exit"
            break;
        }
        runCommand(args, hasExecuted, waitFlag);                      // run user command
    }
}