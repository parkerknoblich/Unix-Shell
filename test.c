#include <stdio.h>    // printf
#include <unistd.h>   // fork, execlp, pipe
#include <assert.h>   // assert
#include <wait.h>     // wait
#include <string.h>   // string

#define MAX_LINE 80

int main() {
    char line[MAX_LINE];
    char* args[100];
    int argCount;
    while (1) {
        printf("osh>");
        fgets(line, MAX_LINE, stdin);
        if (strcmp(line, "exit") == 0) {
            break;
        }
        char* token = strtok(line, " ");
        int i = 0;
        while (token != NULL) {
            arg[i] = token;
            token = strtok(line, " ");
            i++;
        }
        args[i] = NULL;
        int pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            return 0;
        } else {
            int commandReturn;
            wait(&commandReturn);
            printf("command returned %d \n", commandReturn);
        }
    }
}

