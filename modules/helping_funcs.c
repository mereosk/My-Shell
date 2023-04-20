#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>


#include "helping_funcs.h"
#include "bash_interface.h"


void parse(char *inputCommandWhole){
    char *separateCommand;
    char *restSC = inputCommandWhole;
    char *token;
    char *remStr=malloc(256 * sizeof(*remStr));
    char *command = malloc(20 * sizeof(*command));
    int fd;

    strcpy(restSC, inputCommandWhole);

    // When the shell sees a semicolon then it's treated
    // as a command separator
    while((separateCommand = strtok_r(restSC, ";", &restSC) )) {
        printf("Separate command is %s\n",separateCommand);

        strcpy(remStr, separateCommand);
        token = strtok(separateCommand, ">");
        token = trim_whitespace(token);

        // If the token is a substring of separateCommand then
        // the redirection exists
        if(strcmp(token, remStr)) {
            strcpy(command, token);
            printf("Command is %s\n",command);
        }
            token = strtok(0, ">");
            if(token == NULL)
                printf("IM HERE");
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