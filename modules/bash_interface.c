#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <glob.h>
#include <ctype.h>

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

void execute_pipe(List commands, List argsListAll, char *inFile, char *outFile, bool appendFlag) {
    int numOfCommands=list_size(commands);
    print_list(commands);
    print_args(argsListAll);
    printf("num of commands is %d\n", numOfCommands);
    int fds[2], input, output, pid, i, status, pidArray[numOfCommands], outputFD;
    char *command;
    List argsList;
    ListNode commandlNode, argslNode;

    // Check if the first character of inFile is \0, that means
    // that there weren't an input redirection
    if(inFile[0] == '\0') {
        // The first input will be from stdin
        input = 0;
    }
    else {
        // The first input will be from inFile file
        printf("Im here %s\n", inFile);
        if((input=open(inFile, O_RDONLY))==-1) {
            // If the inFile doesn't exist then print the errno and 
            // continue the bash (don't exit)
            fprintf(stderr,"mysh:%s:%s\n",inFile,strerror(errno));
            return;
        }
    }

    // Check if the outFile is not NULL. If that is true then 
    // an output redirection must be executed
    if(outFile != NULL) {
        // If the append flag is true there is an output redirection
        // that appends in the file instead of truncade it
        if(appendFlag)
            outputFD=open(outFile, O_WRONLY|O_APPEND|O_CREAT, 0666);
         else
            outputFD=open(outFile, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    }

    // Loop through all the commands, each command is a child

    for( i=0, command = (char *)list_node_value(commands, commandlNode=list_first(commands)),
    argsList = (List)list_node_value(argsListAll, argslNode=list_first(argsListAll)) ;\
    i<numOfCommands ; \
    i++, command = (char *)list_node_value(commands, commandlNode=list_next(commands, commandlNode)), \
    argsList = list_node_value(argsListAll, argslNode=list_next(commands, argslNode)) \
    ) {
        if((strcmp(command,"cd")==0)) {
            change_directory(argsList);

            return;
        }
        if(i<(numOfCommands-1)) {
            if(pipe(fds) == -1) { perror("pipe"); exit(1);}
            output = fds[WRITE];
        }
        else {
            if(outFile != NULL)
                output = outputFD;
            else 
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
                if(i<(numOfCommands-1)) 
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
            fprintf(stderr,"mysh:%s:%s\n",command,strerror(errno));
            return;
        }  

        // Parent
        if(output!=1) close(output);
        if(input!=0) close(input);
        input=fds[READ];
        pidArray[i] = pid;
    }
    for(int j=0; j<numOfCommands; j++) {
        if(waitpid(pidArray[j], &status, 0 ) == -1) { fprintf(stderr,"mysh:%s\n",strerror(errno));}
    }
    // sleep(100);
    return;
}

void wildcard_matching(List argsListAll) {
    List inListArg;
    char *arg;
    char **found;
    glob_t gstruct;
    int result;

    // Loop through all the arguments
    for(ListNode outlNode=list_first(argsListAll);
    outlNode != LIST_EOF;
    outlNode = list_next(argsListAll , outlNode)
    ) {
        inListArg = (List)list_node_value(argsListAll, outlNode);
    
        // Loop through the arguments of a specific command
        ListNode inlNode=list_first(inListArg);
        ListNode prevNode=LIST_BOF;
        while(inlNode != LIST_EOF) {
            // Get the argument and search for wildcards
            arg = (char *)list_node_value(inListArg, inlNode);
            printf("argument is %s\n",arg);
            if((result=glob(arg, GLOB_ERR , NULL, &gstruct)) != 0) {
                if( result!=GLOB_NOMATCH )
                    fprintf(stderr,"mysh:%s\n",strerror(errno));
            }
            
            if(gstruct.gl_pathc != 0) {
                printf("Found %zu filename matches\n", gstruct.gl_pathc);
                found = gstruct.gl_pathv;
                ListNode tempNode = prevNode;
                while(*found)
                {
                    printf("%s\n",*found);
                    // Insert the matched filenames in the list after prev list node
                    list_insert_next(inListArg, tempNode, strdup(*found));
                    // Now tempNode is the newlly created node
                    if(tempNode == LIST_BOF) {
                        tempNode=list_first(inListArg);
                    }
                    else {
                        tempNode=list_next(inListArg, tempNode);
                    }
                    printf("node value is %s\n",(char *)list_node_value(inListArg, tempNode));
                    found++;
                }

                // Remove the list node that had the wildcard
                list_remove_next(inListArg, tempNode);
                print_list(inListArg);
                prevNode=tempNode;
            }
            else 
                prevNode =inlNode;
            // Free the dynamically allocated storage from glob()
            globfree(&gstruct);
            inlNode=list_next(inListArg, prevNode);
        }
    }

}

// Check if a string is an alias
Pointer check_alias(Map map, char *str) {
    return map_find(map, str);
}


void replace_aliases(List comList, List argList, Map aliasMap) {
    List indivArgList;
    ListNode comlNode,argLNode, prevComNode=LIST_BOF;
    char *command, *result;

    //Loop through commands and their arguments
    comlNode = list_first(comList);
    argLNode = list_first(argList);
    while(comlNode != LIST_EOF) {
        indivArgList=list_node_value(argList, argLNode);
        command = list_node_value(comList, comlNode);
        if((result=check_alias(aliasMap, command))!=NULL) {
            result=trim_whitespace(result);
            printf("There is the alias of %s: %s\n",command, trim_whitespace(result));
            comlNode=insert_alias_in_lists(comList, prevComNode, indivArgList, result);
        }
        else { 
            printf("There is not any alias\n");
            comlNode=list_next(comList, comlNode);
        }

        argLNode=list_next(argList, argLNode);
    }
   
}

char *trim_whitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}


void change_directory(List directoryList) {
    if(list_size(directoryList)>1) {
        fprintf(stderr,"-mysh: cd: too many arguments");
        return;
    }
    char *directory=list_node_value(directoryList, list_first(directoryList));
    char *homeDir = "/home";

    if((directory==NULL) || (strcmp(directory,"~")==0) || (strcmp(directory,"~/")==0))
        chdir(homeDir);
    else if(chdir(directory) != 0)
        fprintf(stderr,"-mysh: cd: %s:%s",directory, strerror(errno));
}


char *replace_env_vars(char *inputCommandWhole) {
    char *envVarToReplace=calloc(50,sizeof(char));
    char *envVarToRepPlusDollar=calloc(50,sizeof(char));
    char *envVar;

    bool envVarFlag = false;
    int i=0, k=0, j=0;
    while(i<inputCommandWhole[i]) {
        if(inputCommandWhole[i] == '$') {
            envVarFlag = true;
            envVarToRepPlusDollar[j] = inputCommandWhole[i];
            j++;
        }
        else if(inputCommandWhole[i] >= 'A' && inputCommandWhole[i] <= 'Z') {
            if(envVarFlag==true) {
                envVarToReplace[k] = inputCommandWhole[i];
                envVarToRepPlusDollar[j] = inputCommandWhole[i];
                k++, j++;
            }
        }
        else {
            if(envVarFlag==true) {
                envVarFlag=false;
                envVar=getenv(envVarToReplace);
                // fprintf(stderr,"so the variable is %s and the env is %s. Whole string is %s\n", envVarToReplace, envVar, str_replace(inputCommandWhole, envVarToRepPlusDollar, envVar));
                char *toReturn=str_replace(inputCommandWhole, envVarToRepPlusDollar, envVar);
                free(envVarToReplace);free(envVarToRepPlusDollar);
                return toReturn;
            }
        }
        i++;
    }
    if(envVarFlag==true) {
        envVar=getenv(envVarToReplace);
        // fprintf(stderr,"so the variable is %s and the env is %s. Whole string is %s\n", envVarToReplace, envVar, str_replace(inputCommandWhole, envVarToRepPlusDollar, envVar));
        char *toReturn=str_replace(inputCommandWhole, envVarToRepPlusDollar, envVar);
        free(envVarToReplace);free(envVarToRepPlusDollar);
        return toReturn;
    }
    // No replaces were made so return the same string
    int len = strlen(inputCommandWhole);
    char *returnVal = calloc(len+1, sizeof(*returnVal));
    strcpy(returnVal, inputCommandWhole);
    free(envVarToReplace);free(envVarToRepPlusDollar);
    return returnVal;
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *saveOrig=orig;
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)) ; ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);

    // if(freeFlag==true)
    //     free(saveOrig);

    return result;
}