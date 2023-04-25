#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "bash_interface.h"

#define READ 0
#define WRITE 1

void output_redirection(char *command, int outputFD){
    int pid, status;

    // Call fork system call
    if((pid=fork()) == -1) {
        perror("fork");
        exit(2);
    }

    // Parent . This is where the bash command
    // will be executed
    if(pid !=0) {
        printf("Im the parent process");
        // Wait for the child
        if(wait(&status) != pid) {
            perror("wait");
            exit(3);
        }
        printf("Child terminated");
    }
    else {      // Child is the reader
        fflush(stdout);
        dup2(outputFD,1);
        close(outputFD);
        execlp(command, command, NULL);
        perror("execlp");
        exit(4);
    }
}

bool create_alias(Map map, char *command) {
    char *tempStr = command;
    char *key, *value;

    // First character is space (from parsing)
    tempStr++;
    printf("MY COMMAND IS %s %s\n", command, tempStr);
    // First token is the key
    key = strtok(tempStr, "\"");
    // printf("length is %d",strlen(key));
    if(key[strlen(key)-1]=' ')
        key[strlen(key)-1]='\0';

    value = strtok(NULL, "\"");
    // Value mush have "" wrapping its name
    // if(value[0]!='\"' || value[strlen(value-1)]!='\"') {
    //     printf("\n-mysh: createalias: Correct usage: 'createalias command \"cd changed command\"'\n");
    //     return false;
    // }
    printf("Value is %s\n", value);
    // Remove the quotes
    // value++; value[strlen(value-1)]='\0';
    // Insert into the alias vector
    map_insert(map, strdup(key), strdup(value));

    //printf("\n-mysh: !%d: event not found\n", intDesignator);
    return true;
}