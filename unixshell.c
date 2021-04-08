#include "unixshell.h"
#include <stdio.h>    // printf
#include <unistd.h>   // fork, execlp, pipe
#include <assert.h>   // assert
#include <wait.h>     // wait
#include <string.h>   // string

#define MAX_LINE 80   // maximum length command

int main() {
    char args[MAX_LINE / 2 + 1];                                           // holds raw user input
    char *splitArgs[MAX_LINE / 2 + 1];                                     // holds split user input on space
    char *history[MAX_LINE / 2 + 1];                                       // previous command
    history[0] = "\0";                                                     // set previous command to null
    int should_run = 1;
    int haveForked = 0;
    int prevSize = 0;

    while (should_run) {                                                   // run while user doesn't exit
        printf("osh>");                                             // print prompt
        fgets(args, MAX_LINE, stdin);                                      // get user input
        int i = 0;                                                         // set up index counter
        char *ptr = strtok(args, " ");                               // set up pointer to loop through tokens
        while (ptr != NULL) {                                              // loop through raw user input and split on space
            splitArgs[i++] = ptr;                                          // add token to updated string
            ptr = strtok(NULL, " ");                              // move pointer to next token
        }
        splitArgs[i - 1][strcspn(splitArgs[i - 1], "\n")] = 0;    // remove new line character from last user token
        printf("%d\n", i);
        char *argsToExecute[i + 1];                                        // arguments to execute
        if ((strcmp(splitArgs[0], "!!") == 0)) {
            if (haveForked == 0) {
                printf("No commands in history.\n");
            } else {
                printf("Current History");
                for (int j = 0; j < prevSize; j++) {
                    //argsToExecute[j] = history[j];
                    printf("%s", history[j]);
                }
            }
        } else {
            haveForked = 1;
            for (int j = 0; j < i; j++) {                                      // copy from splitArgs to argsToExecute
                argsToExecute[j] = splitArgs[j];
            }
        }
        if (haveForked) {
            argsToExecute[i] = NULL;                                           // set last element to NULL
            int waitFlag = 0;                                                  // flag to indicate wait
            if (strcmp(argsToExecute[i - 1], "&") == 0) {                          // check if user inputted "&"
                argsToExecute[i - 1] = NULL;                                       // set the index of "&" to NULL
                waitFlag = 1;                                                  // set wait flag to true
            }
            int forkResult = fork();                                           // fork child from parent
            assert(forkResult != -1);                                          // check for fork error
            if (forkResult == 0) {                                             // child process
                int executeResult = execvp(argsToExecute[0], argsToExecute);       // execute fork
                assert(executeResult >= 0);                                    // check for execute error
                return 0;                                                      // return
            } else {                                        // parent process (only go in on wait)
                printf("\nParent waiting\n");                           // print wait message
                int commandReturn;                                             // return value from command
                if (waitFlag == 1) {
                    wait(&commandReturn);
                }
                printf("command returned %d \n", commandReturn);        // print success
            }
            prevSize = i;
            //memcpy(history, argsToExecute, (MAX_LINE / 2 + 1) * sizeof (char*));    // set current command equal to previous command
            printf("PREVIOUS HISTORY\n");
            for (int j = 0; j <= i; j++) {
                history[j] = argsToExecute[j];
                printf("Element %d: ", j);
                printf("%s", history[j]);
                printf("\n");
            }
            memset(args, 0, strlen(args));
            memset(splitArgs, 0, strlen(splitArgs));
        }
        should_run = 1;
    }
    return 0;
}