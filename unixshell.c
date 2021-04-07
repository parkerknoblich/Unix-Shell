#include "unixshell.h"
#include <stdio.h>    // printf
#include <unistd.h>   // fork, execlp, pipe
#include <assert.h>   // assert
#include <wait.h>     // wait
#include <string.h>   // string

#define MAX_LINE 80   // maximum length command

int main() {
    char args[MAX_LINE / 2 + 1];                 // holds raw user input
    int should_run = 1;                          // flag to continue or exit

    if (should_run) {
        printf("osh>");                   // print prompt
        fgets(args, MAX_LINE, stdin);            // get user input
        int i = 0;                               // set up index counter
        char *ptr = strtok(args, " ");     // set up pointer to loop through tokens
        char *splitArgs[MAX_LINE / 2 + 1];       // holds split user input on space
        while (ptr != NULL) {                    // loop through raw user input and split on space
            splitArgs[i++] = ptr;                // add token to updated string
            ptr = strtok(NULL, " ");    // move pointer to next token
        }
        splitArgs[i - 1][strcspn(splitArgs[i - 1], "\n")] = 0;  // remove new line character from last user token
        int waitFlag = 0;    // flag to indicate wait
        if (strcmp(splitArgs[i - 1], "&") == 0) {  // check if user inputted "&"
            splitArgs[i - 1] = NULL;   // set the index of "&" to NULL
            waitFlag = 1;              // set wait flag to true
        }
        int forkResult = fork();   // fork child from parent
        assert(forkResult != -1);  // check for fork error
        if (forkResult == 0) {     // child process
            int executeResult = execvp(splitArgs[0], splitArgs);  // execute fork
            assert(executeResult >= 0);  // check for execute error
            return 0;                    // return
        } else if (waitFlag == 1) {               // parent process (only go in on wait)
            printf("\nParent waiting\n");  // print wait message
            int commandReturn;                            // return value from command
            wait(&commandReturn);                         // wait
            printf("command returned %d \n", commandReturn);  // print success
        }
    }
    return 0;
}
