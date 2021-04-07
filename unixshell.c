#include "unixshell.h"
#include <stdio.h>    // printf
#include <unistd.h>   // fork, execlp, pipe
#include <assert.h>   // assert
#include <wait.h>     // wait
#include <string.h>   // string

#define MAX_LINE 80   // maximum length command

int main() {
    char args[MAX_LINE / 2 + 1];
    int should_run = 1;

    if (should_run) {
        printf("osh>");
        fgets(args, MAX_LINE, stdin);

        int i = 0;
        char *ptr = strtok(args, " ");
        char *splitArgs[MAX_LINE / 2 + 1];
        while (ptr != NULL) {
            splitArgs[i++] = ptr;
            ptr = strtok(NULL, " ");
        }
        splitArgs[i] = NULL;
        splitArgs[i - 1][strcspn(splitArgs[i - 1], "\n")] = 0;
        int waitFlag = 0;
        printf("%s", splitArgs[i - 1]);
        if (strcmp(splitArgs[i - 1], "&") == 0) {
            printf("Hello World\n");
        }
        int forkResult = fork();
        assert(forkResult != -1);

        printf("%s", splitArgs[i - 1]);

        if (forkResult == 0) {    // child
            int executeResult = execvp(splitArgs[0], splitArgs);
            assert(executeResult >= 0);
            return 0;
        } else {                  // parent
            printf("\nParent waiting\n");
            int psret;
//            if (splitArgs[i - 1] == "&") {
                wait(&psret);
//            }
            printf("ps command returned %d \n", psret);
        }

    }
    return 0;
}
