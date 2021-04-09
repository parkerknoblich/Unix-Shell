#include <stdio.h>  // printf
#include <string.h> // string
#include <unistd.h> // fork, execlp, pipe
#include <wait.h>   // wait
#include <assert.h> // assert
#include <stdlib.h>

#define MAX_LINE 80

int main(){
    char line[MAX_LINE];
    char* args[MAX_LINE];
    char* prevArgs[MAX_LINE];
    prevArgs[0] = "\0";
    int waitFlag = 0;

    while(1) {
        printf("osh>");
        fgets(line, MAX_LINE, stdin);
        size_t length = strlen(line);
        line[length - 1] = '\0';
        if(strcmp(line, "exit") == 0) {
            break;
        } else if (strcmp(line, "!!") == 0) {
            *args = strdup(*prevArgs);
            for (int b = 0; b < strlen(*prevArgs); b++) {
                printf("%s", args[b]);
            }
            if (strcmp(prevArgs[0], "\0") == 0) {
                printf("No commands in history.");
            }
        } else {
            if (strchr(line, '&') != NULL) {
                int i, j;
                for (i = 0, j = 0; i < length; i++) {
                    if (line[i] != '&') {
                        line[j] = line[i];
                        j++;
                    }
                }
                line[j] = '\0';
                waitFlag = 1;
            } else {
                waitFlag = 0;
            }
            char *token;
            token = strtok(line," ");
            int i = 0;
            while(token != NULL){
                args[i] = token;
                token = strtok(NULL," ");
                i++;
            }
            args[i] = NULL;
        }

        int forkret = fork();
        if (forkret == 0) {               // Child
            int executeResult = execvp(args[0], args);
            assert(executeResult >= 0);
            return 0;
        } else {                    // Parent
            int commandResult;
            if (waitFlag) {
                printf("Parent waiting\n");
                wait(&commandResult);
            }
            printf("Command returned %d\n", commandResult);
        }
        *prevArgs = strdup(*args);
        for (int b = 0; b < strlen(*args); b++) {
            printf("%s", prevArgs[b]);
        }
    }
}