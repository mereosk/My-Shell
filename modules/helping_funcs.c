#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>


#include "helping_funcs.h"
#include "bash_interface.h"


void parse(char *inputCommandWhole , Vector historyVector){
    char *separateCommand, *token, *firstWord, *SCommandCopy;
    char *designator;
    char *savePtrFW;
    char *kwCreateAlias="createalias";
    char *kwDestroyAlias="destroyalias";
    char *kwHistory="myHistory";
    char *restSC = inputCommandWhole;
    char *remStr=malloc(256 * sizeof(*remStr));
    char *command = malloc(20 * sizeof(*command));
    int intDesignator;
    // char *SCommandCopy = malloc(256 * sizeof(*SCommandCopy));
    int fd;


    // When the shell sees a semicolon then it's treated
    // as a command separator
    while((separateCommand = strtok_r(restSC, ";", &restSC) )) {
        printf("Separate command is %s\n",separateCommand);

        // Take the first word of the separate command(separated with semicolomn)
        int separateCommandLength = strlen(separateCommand);
        SCommandCopy = (char *)calloc(separateCommandLength+1, sizeof(char));
        strncpy(SCommandCopy, separateCommand, separateCommandLength);
        firstWord = strtok_r(SCommandCopy, " ", &savePtrFW);

        printf("The first command is %s\n", firstWord);
        printf("Separate command is %s\n",separateCommand);

        // Check if the first word is the keyword for creating an alias
        if(strcmp(firstWord, kwCreateAlias)==0){
          printf("create alias\n");
        }   
        else if(strcmp(firstWord, kwDestroyAlias)==0) {
          // Check if the first word is the keyword for destroying an alias
          printf("destroy alias\n");
        }   
        else if((strcmp(firstWord, kwHistory)==0)) {
          // Check if the first word is the keyword for showing the last 20 commands
          printf("-----------------------\nTHE HISTORY OF MYSH COMMANDS :\n");
          vector_print(historyVector);
          printf("-----------------------\n");
        }   
        else if(separateCommand[0]=='!') {
          // If the command has the form "!number" then show the number'th command
          printf("print a specific command\n");
          // Take string that follows the ! 
          char *eventAfter=&separateCommand[1];
          printf("String is %s\n",eventAfter);
          // Check what the designator is
          designator=strtok(eventAfter, " ");
          // printf("designator is %s\n",designator);
          if((intDesignator=atoi(designator))){
            printf("int designator is %d\n",intDesignator);
            // !n refers to command line n
            if(intDesignator>0) {
              printf("COMMAND IS %s\n",(char *)vector_get_at(historyVector, intDesignator-1));
            }
            else{
              // !-n refers to command n lines back
              printf("COMMAND IS %s\n",(char *)vector_get_at(historyVector, vector_size(historyVector)-abs(intDesignator)-1));
            }
          }
          else {
            printf("designator is %s\n",designator);
          }
        }

        free(SCommandCopy);

        // strcpy(remStr, separateCommand);
        // token = strtok(separateCommand, ">");
        // token = trim_whitespace(token);

        // // If the token is a substring of separateCommand then
        // // the redirection exists
        // if(strcmp(token, remStr)) {
        //     strcpy(command, token);
        //     printf("Command is %s\n",command);
        // }
        //     token = strtok(0, ">");
        //     if(token == NULL)
        //         printf("IM HERE");
        // // Checks for delimiter
        // while (token != 0) {
        //     printf("%s\n", token);

        //     // Use of strtok
        //     // go through other tokens
        //     token = strtok(0, ">");
        //     if(token == NULL)
        //         break;

        //     token = trim_whitespace(token);
        //     printf("%s\n", token);
        //     // If there are multiple redirections
        //     // open or truncade all files but use only
        //     // use the last file
        //     if((fd = creat(token, 0664)) == -1) {
        //         perror("creating");
        //         exit(1);
        //     }
        // }
        // // Now do the redirection of output
        // output_redirection(command, fd);
        
    }

    // if (token == NULL) {
    //     puts("empty string!");
    //     return 1;
    // }
    free(command);
    free(remStr);
    printf("Hello im in parse this is the stirng %s", separateCommand);
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