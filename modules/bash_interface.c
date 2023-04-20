#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

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