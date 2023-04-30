#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "bash_interface.h"

#define READ 0
#define WRITE 1

// char ** args_construct(Vector argsList, char *command) {
//     int sizeVec=vector_size(argsList);
//     int i;
//     int size = 2+sizeVec;
//     // Var argv is where the arguments will be placed
//     char *argv[size];

//     // Check if it has arguments
//     if(sizeVec == 0) {
//         // It has no arguments so execute just the command
//         argv[0] = command;
//         argv[1] = NULL;
//     }
//     else {
//         // It has arguments so execute the command with them
//         argv[0] = command;
//         for(i=0 ; i<sizeVec ; i++) {
//             argv[i+1] = vector_get_at(argsList, i);
//             printf("%s\n", argv[i+1]);
//         }
//         argv[i+1] = NULL;
//     }

//     return argv;
// }

void execute_redirection(char *command, char *inFile, char *outFile, List argsList, bool appendFlag){
    int pid, status, outputFD, inputFD;

    printf("infile %s and outfile %s\n",inFile, outFile);

    int sizeVec=list_size(argsList);
    int i=0;
    int size = 2+sizeVec;
    // Var argv is where the arguments will be placed
    char *argv[size];

    // Check if it has arguments
    if(sizeVec == 0) {
        // It has no arguments so execute just the command
        argv[0] = command;
        argv[1] = NULL;
    }
    else {
        // It has arguments so execute the command with them
        argv[0] = command;
        for(ListNode lNode = list_first(argsList);
        lNode != LIST_EOF;
        lNode=list_next(argsList, lNode)) {
            argv[i+1] = (char *)list_node_value(argsList, lNode);
            printf("%s\n", argv[i+1]);
            i++;
        }
        argv[i+1] = NULL;
    }

    
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
        
        if(outFile == NULL || (outFile != NULL && inFile[0] != '\0')) {
            printf("Im here\n");
            inputFD=open(inFile, O_RDONLY);
            close(0);
            dup2(inputFD,0);
            close(inputFD);
        }   
        if(inFile[0] == '\0' || (outFile != NULL && inFile[0] != '\0')) {
            printf("and here\n");
            if(appendFlag)
                outputFD=open(outFile, O_WRONLY|O_APPEND|O_CREAT, 0666);
            else
                outputFD=open(outFile, O_WRONLY|O_TRUNC|O_CREAT, 0666);
            close(1);
            dup2(outputFD,1);
            close(outputFD);
        }
        
            
        execvp(command, argv);
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
    if(key[strlen(key)-1] = ' ')
        key[strlen(key)-1]='\0';

    value = strtok(NULL, "\"");

    printf("Value is %s\n", value);

    // Insert into the alias vector
    map_insert(map, strdup(key), strdup(value));

    return true;
}

void destroy_alias(Map map, char *command) {
    char *tempStr = command;
    char *key;

    // First character is space (from parsing)
    tempStr++;

    // Check if it has 1 argument like it should
    key = strtok(tempStr, " ");
    printf("key is %s\n",key );
    if(strtok(0, " ")!=NULL) {
        printf("\n-mysh: destroyalias: Correct usage: 'destroyalias myhome'\n");
        return;
    }

    // Remove the key from the map (if it exists)
    if(map_remove(map, key)==false)
        printf("-mysh: unalias: %s not found\n", key);
}

void execute_command(char *command, List argsList) {
    printf("Im in execute the command is %s\n", command);
    int sizeVec=list_size(argsList);
    int i=0;
    int size = 2+sizeVec;
    // Var argv is where the arguments will be placed
    char *argv[size];

    // Check if it has arguments
    if(sizeVec == 0) {
        // It has no arguments so execute just the command
        argv[0] = command;
        argv[1] = NULL;
    }
    else {
        // It has arguments so execute the command with them
        argv[0] = command;
        for(ListNode lNode = list_first(argsList);
        lNode != LIST_EOF;
        lNode=list_next(argsList, lNode)) {
            argv[i+1] = (char *)list_node_value(argsList, lNode);
            printf("%s\n", argv[i+1]);
            i++;
        }
        argv[i+1] = NULL;
    }

    int pid, status;

    // Call fork system call
    if((pid=fork()) == -1) {
        perror("fork");
        exit(2);
    }
    if(pid !=0) {
        printf("Im the parent process");
        // Wait for the child
        if(wait(&status) != pid) {
            perror("wait");
            exit(4);
        }
        printf("Child terminated");
    }
    else {      // Child is the reader
        execvp(command, argv);
        perror("execvp");
        exit(5);
    }
}

void execute_pipe(List commands, List argsListAll) {
    int numOfCommands=list_size(commands);
    print_list(commands);
    print_args(argsListAll);
    printf("num of commands is %d\n", numOfCommands);
    int fds[2], input=0, output, pid, i, status, pidArray[numOfCommands];
    char *command;
    List argsList;
    ListNode commandlNode, argslNode;
    
    // int sizeVec=list_size(argsList);
    // int size = 2+sizeVec;
    // // Var argv is where the arguments will be placed
    // char *argv[size];

    // Loop through all the commands, each command is a child

    for( i=0, command = (char *)list_node_value(commands, commandlNode=list_first(commands)),
    argsList = (List)list_node_value(argsListAll, argslNode=list_first(argsListAll)) ;\
    i<numOfCommands ; \
    i++, command = (char *)list_node_value(commands, commandlNode=list_next(commands, commandlNode)), \
    argsList = list_node_value(argsListAll, argslNode=list_next(commands, argslNode)) \
    ) {
        // if(i==0) {
        //     commandlNode=list_first(commands);
        //     command = (char *)list_node_value(commands, commandlNode);
        // }
        // else { 
        //     command = (char *)list_node_value(commands, commandlNode=list_next(commands, commandlNode) );
        //     fprintf(stderr, "trying to change the com %s \n", command);
        // }
        if(i<(numOfCommands-1)) {
            if(pipe(fds) == -1) { perror("pipe"); exit(1);}
            output = fds[WRITE];
        }
        else {
            output = 1;
        }
        
        if((pid = fork()) == -1) { perror("fork"); exit(1); }

        // Children
        if(pid == 0) {
            fprintf(stderr, "im in child with command %s \n", command);
            if(input != 0) {
                dup2(input, 0);
                close(input);
            }
            if(output != 1) {
                dup2(output,1);
                close(output);
                close(fds[READ]);
            }
            // Construct the argv
            int listSize=list_size(argsList);
            fprintf(stderr, "SIZE IS %d\n\n", listSize);
            int j=0;
            int size = 2+listSize;
            // Var argv is where the arguments will be placed
            char *argv[size];
            // char *argv[2];
            // Check if it has arguments
            if(listSize == 0) {
                // It has no arguments so execute just the command
                argv[0] = command;
                argv[1] = NULL;
                fprintf(stderr, "THAP REPEEEEEEEE\n");
            }
            else {
                // It has arguments so execute the command with them
                print_list(argsList);
                argv[0] = command;
                for(ListNode lNode = list_first(argsList);
                lNode != LIST_EOF;
                lNode=list_next(argsList, lNode)) {
                    argv[j+1] = (char *)list_node_value(argsList, lNode);
                    fprintf(stderr,"%s\n", argv[j+1]);
                    j++;
                }
                argv[j+1] = NULL;
            }
            execvp(command, argv);
            perror("execvp");
            exit(5);
        }  

        // Parent
        if(output!=1) close(output);
        if(input!=0) close(input);
        input=fds[READ];
        pidArray[i] = pid;
    }
    for(int j=0; j<numOfCommands; j++) {
        if(waitpid(pidArray[j], &status, 0 ) == -1) { perror("waitpid"); exit(1);}
    }
    // sleep(100);
    return;
}