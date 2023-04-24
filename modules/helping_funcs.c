#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>


#include "helping_funcs.h"
#include "bash_interface.h"

int starts_with(Pointer a,Pointer b){
    return strncmp(a, b, strlen(b));
}

void parse(char *inputCommandWhole , Vector historyVector){
    char *separateCommand, *token, *firstWord, *SCommandCopy;
    char *designator;
    char *savePtrFW;
    char *kwCreateAlias="createalias";
    char *kwDestroyAlias="destroyalias";
    char *kwHistory="history";
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
          // If the command has the form "!something" then its an event designator
          char *subValue=NULL;
          printf("print a specific command\n");
          // Take string that follows the ! 
          char *eventAfter=&separateCommand[1];
          printf("String is %s\n",eventAfter);
          // Check what the designator is
          if((intDesignator=begins_with_number(eventAfter))!=0) {
            printf("int designator is %d\n",intDesignator);
            if(intDesignator>0) {
              // !n refers to command line n

              subValue = vector_get_at(historyVector, intDesignator-1);
              if(subValue != NULL) {
                printf("COMMAND IS %s\n", subValue);
                // Substitude the last command that is !n with the command in line n
                // However if the command is the same than the previous one, keep only one
                if(strcmp(subValue, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
                  vector_remove_last(historyVector);
                else
                  vector_set_at(historyVector, vector_size(historyVector)-1, strdup(subValue));
              }
              else {
                // If it is NULL then there isnt an event in command line n
                printf("\n-mysh: !%d: event not found\n", intDesignator);
                // Delete it from the history
                vector_remove_last(historyVector);
              }
            }
            else if(intDesignator<0){
              // !-n refers to command n lines back

              subValue = (char *)vector_get_at(historyVector, vector_size(historyVector)-abs(intDesignator)-1);
              if(subValue != NULL) {
                printf("COMMAND IS %s\n",subValue);
                // Substitude the last command that is !-n with the command n lines back
                // However if the command is the same than the previous one, keep only one
                if(strcmp(subValue, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
                  vector_remove_last(historyVector);
                else
                  vector_set_at(historyVector, vector_size(historyVector)-1, strdup(subValue));
              }
              else {
                // If it is NULL then there isnt an event n lines back
                printf("\n-mysh: !%d: event not found\n", intDesignator);
                // Delete it from the history
                vector_remove_last(historyVector);
              }
            }
          }
          else if(eventAfter[0]=='!') {
            // !! refer to the previous command, same as !-1
            subValue = (char *)vector_get_at(historyVector, vector_size(historyVector)-2);
            // !-n refers to command n lines back
            printf("COMMAND IS %s\n",subValue);
            // Substitude the last command that is !-n with the command n lines back
            // However if the command is the same than the previous one, keep only one
            if(strcmp(subValue, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
              vector_remove_last(historyVector);
            else
              vector_set_at(historyVector, vector_size(historyVector)-1, strdup(subValue));
          }
          else {
            // !string refers to the most recent command preceding the current
            // position in the history list starting with 'string'

            // Firstly take the first string using strtok
            char *strToSearch = strtok(eventAfter, " ");
            printf("The string that im searching is %s\n", strToSearch);
            subValue=(char *)reverse_vector_find(historyVector, strToSearch, starts_with);
            if(subValue != NULL) {
              printf("WE FOUND THE STIRNG %s\n",subValue);
              // Substitude the last command that is !-n with the command n lines back
              // However if the command is the same than the previous one, keep only one
              if(strcmp(subValue, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
                vector_remove_last(historyVector);
              else
                vector_set_at(historyVector, vector_size(historyVector)-1, strdup(subValue));
            }
            else {
              // If it is NULL then there isnt an event that starts with 'string'
              printf("\n-mysh: !%s: event not found\n", strToSearch);
              // Delete it from the history
              vector_remove_last(historyVector);
            }
          }
          
          // printf("SUBVALUE IS %s and %s\n", subValue, separateCommand);
          // // If the substitution went through concatenate the subbed command
          // // with the files
          
          // if(subValue != NULL) {
          //   char *testStr=calloc((256),sizeof(*testStr));
          //   strncpy(testStr, subValue, strlen(subValue));
          //   strcat(testStr, separateCommand);
          //   printf("WHOLE NEW VALUE IS %s\n", testStr);
          // }
          
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

int begins_with_number(char *str){
  int len = strlen(str);
  int i, retNumber;
  char *copyStr=calloc((len+1),sizeof(*copyStr));

  if (len<=0)
    return 0;

  int j=0,k=0;

  // If the first character is - then skip
  if(str[0]=='-') {
    j++;
    copyStr[k++]='-';
  }

  // Skip the zeros in the start
  while(str[j]=='0'){
    j++;
  }
  
  for(i=j; i<len ; i++,k++){
    if(isdigit(str[i]))
      copyStr[k]=str[i];
    else
      break;
  }
  
  if(strlen(copyStr)>0) {
    retNumber = atoi(copyStr);
    free(copyStr);
    return retNumber;
  }
  free(copyStr);
  return 0;
}