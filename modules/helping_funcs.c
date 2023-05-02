#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "helping_funcs.h"

#define START 0
#define HAVECOMMAND 1
#define OUTREDIRECT 2
#define INREDIERECT 3
#define PIPE 4

// Compare function that returns true if sting a starts with b
int starts_with(Pointer a,Pointer b){
    return strncmp(a, b, strlen(b));
}



// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
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
    return result;
}

void parse(char *inputCommandWhole , Vector historyVector, Map aliasMap){
    char *separateCommand, *firstWord, *SCommandCopy, *separateCommandParse;
    char *savePtrFW;
    char *kwCreateAlias="createalias";
    char *kwDestroyAlias="destroyalias";
    char *kwHistory="history";
    char *kwAlias="alias";
    char *kwChDir="cd";
    char *restSC = inputCommandWhole;
    char *remStr=malloc(256 * sizeof(*remStr));
    char *wholeStr=calloc((256),sizeof(*wholeStr));
    char *command = malloc(20 * sizeof(*command));
    int intDesignator;
    // char *SCommandCopy = malloc(256 * sizeof(*SCommandCopy));
    int fd;


    // When the shell sees a semicolon then it's treated
    // as a command separator
    while((separateCommand = strtok_r(restSC, ";", &restSC) )) {
        printf("Separate command is %s\n",separateCommand);
        separateCommand = trim_whitespace(separateCommand);

        // Take the first word of the separate command(separated with semicolumn)
        int separateCommandLength = strlen(separateCommand);
        SCommandCopy = (char *)calloc(separateCommandLength+1, sizeof(char));
        strncpy(SCommandCopy, separateCommand, separateCommandLength);
        firstWord = strtok_r(SCommandCopy, " ", &savePtrFW);

        printf("The first command is %s\n", firstWord);
        // Check if the first word is an alias
        // if(check_alias(aliasMap, firstWord)) {
        //   char *sepCommandReplaced = str_replace(separateCommand, firstWord, map_node_value(aliasMap, map_find_node(aliasMap, firstWord)));
        //   printf("IT IS AN ALIAS and the replaced string is %s\n", sepCommandReplaced);
        // }
        printf("Separate command is %s\n",separateCommand);

        // Check if the first word is the keyword for creating an alias
        if(strcmp(firstWord, kwCreateAlias)==0){
          printf("create alias the separate command is %s\n", rest_args(separateCommand));
          if(rest_args(separateCommand) == NULL) 
            printf("\n-mysh: createalias: Correct usage: 'createalias command \"cd changed command\"'\n");
          else
            create_alias(aliasMap, rest_args(separateCommand));
          free(SCommandCopy);
          continue;
        }   
        else if(strcmp(firstWord, kwDestroyAlias)==0) {
          // Check if the first word is the keyword for destroying an alias
          if(rest_args(separateCommand) == NULL) 
            printf("\n-mysh: destroyalias: Correct usage: 'destroyalias myhome'\n");
          else
            destroy_alias(aliasMap, rest_args(separateCommand));
          printf("destroy alias\n");
          free(SCommandCopy);
          continue;
        }   
        else if(strcmp(firstWord, kwAlias)==0) {
          // Check if the first word is the keyword for destroying an alias
          printf("Alias\n");
          map_print(aliasMap);
          free(SCommandCopy);
          continue;
        }   
        // else if(strcmp(firstWord, kwChDir)==0) {
        //   printf("Now we change directory");
        //   // First word is the keyword for changing directories
        //   char *restArgs=rest_args(separateCommand);
        //   // Run through the rest arguments and check how many spaces are there
        //   // If there are more that 1 then there are too many arguments
        //   char *copyRestArgs=restArgs;
        //   int spaceCount=0;
        //   if(restArgs!=NULL) {
        //     restArgs=trim_whitespace(restArgs);
        //     for(int i=0;i<=copyRestArgs[i];i++)
        //       if(copyRestArgs[i]==' ')
        //         spaceCount++;
        //   }

        //   if(spaceCount>1) {
        //     fprintf(stderr,"-mysh: cd: too many arguments");
        //   }
        //   else {
        //     printf("rest arguments is %s\n", restArgs);
        //     change_directory(restArgs);
        //   }
        //   free(SCommandCopy);
        //   continue;
        // }
        else if((strcmp(firstWord, kwHistory)==0)) {
          // Check if the first word is the keyword for showing the last 20 commands
          printf("-----------------------\nTHE HISTORY OF MYSH COMMANDS :\n");
          vector_print(historyVector);
          printf("-----------------------\n");
          free(SCommandCopy);
          continue;
        }   
        else if(separateCommand[0]=='!') {
          // If the command has the form "!something" then its an event designator
          char *subValue=NULL;
          // Take string that follows the ! 
          char *eventAfter=strdup(&separateCommand[1]);
          // Check what the designator is
          if((intDesignator=begins_with_number(eventAfter))!=0) {
            if(intDesignator>0) {
              // !n refers to command line n

              subValue = vector_get_at(historyVector, intDesignator-1);
              if(subValue == NULL) {
                // If it is NULL then there isnt an event in command line n
                printf("\n-mysh: !%d: event not found\n", intDesignator);
                // Delete it from the history
                vector_remove_last(historyVector);
                free(SCommandCopy);
                free(eventAfter);
                continue;
              }
            }
            else if(intDesignator<0){
              // !-n refers to command n lines back

              subValue = (char *)vector_get_at(historyVector, vector_size(historyVector)-abs(intDesignator)-1);
              if(subValue == NULL) {
                // If it is NULL then there isnt an event n lines back
                printf("\n-mysh: !%d: event not found\n", intDesignator);
                // Delete it from the history
                vector_remove_last(historyVector);
                free(SCommandCopy);
                free(eventAfter);
                continue;
              }
            }
          }
          else if(eventAfter[0]=='!') {
            // !! refer to the previous command, same as !-1
            subValue = (char *)vector_get_at(historyVector, vector_size(historyVector)-2);
            if(subValue == NULL) {
                // If it is NULL then there isnt an event n lines back
                printf("\n-mysh: !%d: event not found\n", intDesignator);
                // Delete it from the history
                vector_remove_last(historyVector);
                free(SCommandCopy);
                free(eventAfter);
                continue;
            }
          }
          else {
            // !string refers to the most recent command preceding the current
            // position in the history list starting with 'string'

            // Firstly take the first string using strtok
            char *strToSearch = strtok(eventAfter, " ");
            subValue=(char *)reverse_vector_find(historyVector, strToSearch, starts_with);
            if(subValue == NULL) {
              // If it is NULL then there isnt an event that starts with 'string'
              printf("\n-mysh: !%s: event not found\n", strToSearch);
              // Delete it from the history
              vector_remove_last(historyVector);
              free(SCommandCopy);
              free(eventAfter);
              continue;
            }
          }
          
          // If the substitution went through concatenate the subbed command
          // with the rest arguments
          if(subValue != NULL) {
            strncpy(wholeStr, subValue, strlen(subValue));
            char *restArgs = rest_args(separateCommand);
            // If rest arguments is not NULL then it hace arguments and we 
            // have to concatenate them
            if(restArgs != NULL) {
              strcat(wholeStr, restArgs);
            }
            // If restArgs is NULL it doesn't have other arguments
            printf("whole str is %s\n", wholeStr);
            // Substitude the last command (in history) that is !n/!-n/!!/!string 
            // with the command in subValue plus the rest arguments
            // However if the command is the same than the previous one, keep only one
            if(strcmp(wholeStr, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
              vector_remove_last(historyVector);
            else
              vector_set_at(historyVector, vector_size(historyVector)-1, strdup(wholeStr));
          }
          free(eventAfter);
        }
        free(SCommandCopy);
        // Now we will parse the command using the logic of
        // state machines.
        printf("NOW STARTS THE PARSTING\n\n");
        if(wholeStr[0] != 0)
          separateCommandParse=wholeStr;
        else
          separateCommandParse=separateCommand;

        int k=0, indivStr=0;
        // This flag is turning true when it sees ">>"
        bool appendFlag=false;
        char *strKeeper=calloc(256, sizeof(*strKeeper));
        char *commandSave=calloc(256, sizeof(*commandSave));
        // infileSave is where the infile will be saved so that
        // later will be used to do the redirection
        char *infileSave=calloc(256, sizeof(*infileSave));
        // The state of the machine starts expecting a command
        int state = START;
        // List will be for keeping the arguments of a specific command
        List argsList = list_create(free);
        // List will be for keeping the all the arguments (List of lists)
        List argsListAll = list_create(list_destroy);
        // List will be for keeping all the commands
        List comList = list_create(free);
        bool pipeFlag=false;
        while(1) {
          // printf("char %c%s", separateCommand[k], separateCommand);
          strKeeper[indivStr]=separateCommandParse[k];
          // com1 
          if(separateCommandParse[k] == '\0') {
            // If the state is command it is -mysh command
            if(state == START) {
              printf("IMG HERE\n");
              // Insert the command in the command list
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              replace_aliases(comList, argsListAll, aliasMap);
              if(pipeFlag==true) {
                 printf("Im actually here\n");
                               
                 execute_pipe(comList, argsListAll, infileSave, NULL, appendFlag);
              }
              else {
                // execute_command(strKeeper, argsList);
                execute_pipe(comList, argsListAll, infileSave, NULL, appendFlag);
              }
              printf("came here\n");
              // sleep(100);
              break;
            }
            else if(state == HAVECOMMAND) {
              printf("argument have command\n");
              // Insert the last argument in the list and execute the command
              list_insert_next(argsList, list_last(argsList), strdup(strKeeper));
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              wildcard_matching(argsListAll);
              replace_aliases(comList, argsListAll, aliasMap);
              if(pipeFlag==true) {
                printf("Im actually here\n");
                execute_pipe(comList, argsListAll, infileSave, NULL, appendFlag);
              }
              else {
                printf("CAME HEEEEEEEEEEEEEEERE\n");
                // execute_command(commandSave, argsList);
                execute_pipe(comList, argsListAll, infileSave, NULL, appendFlag);
                printf("CAME HEEEEEEEEEEEEEEERE\n");
              }
              break;
            }
            else if(state == OUTREDIRECT) {
              // execute_redirection(commandSave, infileSave, strKeeper, argsList, appendFlag);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_pipe(comList, argsListAll, infileSave, strKeeper, appendFlag);
              break;
            }
            else if(state == INREDIERECT) {
              printf("STR IS %s\n", strKeeper);
              // execute_redirection(commandSave, strKeeper, NULL, argsList, appendFlag);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_pipe(comList, argsListAll, strKeeper, NULL, appendFlag);
              break;
            }
          }
          else if(separateCommandParse[k] == ' ') {
            // Without the space
            strKeeper[indivStr] = '\0';
            // Check if the stirng in strKeeper is a command
            if(state == START) {
              // Save the command and search for arguments
              strcpy(commandSave, strKeeper);
              printf("Command save is %s\n", commandSave);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = HAVECOMMAND;
              // Skip the spaces
              while(separateCommandParse[k] == ' ') {
                k++;
              }
            }
            else if(state == HAVECOMMAND) {
              // Insert the argument in the list
              list_insert_next(argsList, list_last(argsList), strdup(strKeeper));
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              // Skip the spaces
              while(separateCommandParse[k] == ' ') {
                k++;
              }
            }
            else if(state == OUTREDIRECT) {
              // Just skip the spaces and continue
              printf("STRKEEPER %s\n",strKeeper);
              while(separateCommandParse[++k] == ' ');
              continue;
            }
            else if(state == INREDIERECT) {
              // Just skip the spaces and continue
              printf("STRKEEPER %s\n",strKeeper);
              while(separateCommandParse[++k] == ' ');
              continue;
            }
            
            // Make the counter 0 in order to continue with the arguments
            // if they exist
            indivStr=0;
            continue;
          }
          else if(separateCommandParse[k] == '>') {
            strKeeper[indivStr] = '\0';
            appendFlag=false;
            if(state==START) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              printf("Command save is %s\n", commandSave);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==HAVECOMMAND && separateCommandParse[k-1] == ' ') {
              state=OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==HAVECOMMAND && separateCommandParse[k-1] != ' ') {
              printf("argumentssssss\n");
              // Insert the argument in the list
              list_insert_next(argsList, list_last(argsList), strdup(strKeeper));
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==OUTREDIRECT) {
              printf("STR KEEPER IS %s", strKeeper);
              // There are multiple redirections so
              // open or truncade all files but use only
              // use the last file
              if((fd = creat(strKeeper, 0666)) == -1) {
                  perror("creating");
                  exit(1);
              }
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
            else if(state==INREDIERECT) {
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              printf("Command save is %s\n", infileSave);
              state = OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
            
            // Check if the next character is ">>" in order to redirect
            // insertsion in an existing file (append)
            // Also notice that k is the next k because we skipped at least
            // one character above
            if(separateCommandParse[k]=='>') {
              printf("I DO APPEND\n");
              appendFlag=true;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
            // Make the counter 0 in order to continue with the arguments
            // if they exist
            indivStr=0;
            continue;
          }
          else if(separateCommandParse[k] == '<') {
            strKeeper[indivStr] = '\0';
            if(state==START) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              printf("Command save is %s\n", commandSave);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==HAVECOMMAND && separateCommandParse[k-1] == ' ') {
              state=INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==HAVECOMMAND && separateCommandParse[k-1] != ' ') {
              printf("argumentssssss\n");
              // Insert the argument in the list
              list_insert_next(argsList, list_last(argsList), strdup(strKeeper));
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==INREDIERECT) {
              printf("STR KEEPER IS %s", strKeeper);
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              printf("Command save is %s\n", infileSave);
              // There are multiple redirections so
              // open or truncade all files but use only
              // use the last file
              if((fd = open(strKeeper, O_RDONLY)) == -1) {
                  perror("creating");
                  break;
              }
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
             // Make the counter 0 in order to continue with the arguments
            // if they exist
            indivStr=0;
            continue;
          }
          else if(separateCommandParse[k] == '|') {
            strKeeper[indivStr] = '\0';
            pipeFlag=true;
            if(state==START) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              printf("Command save is %s\n", commandSave);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = START;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==HAVECOMMAND && separateCommandParse[k-1] == ' ') {
              state=START;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==HAVECOMMAND && separateCommandParse[k-1] != ' ') {
              printf("argumentssssss\n");
              // Insert the argument in the list
              list_insert_next(argsList, list_last(argsList), strdup(strKeeper));
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=START;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==INREDIERECT) {
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              printf("Command save is %s\n", infileSave);
              state = START;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
             // Make the counter 0 in order to continue with the arguments
            // if they exist
            indivStr=0;
            continue;
          }

          k++; indivStr++;
        }
        free(commandSave);free(strKeeper);free(infileSave);list_destroy(argsList);list_destroy(comList);list_destroy(argsListAll);
        
    }

    free(command);
    free(remStr);
    free(wholeStr);
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

char *rest_args(char *str) {
  char *strToReturn = str;
  int len = strlen(strToReturn);

  int count=0;
  // Skip the first characters
  while(*strToReturn++ != ' '){
    // If the count is greater than length, the command does not
    // have arguments so return NULL
    if(count>len)
      return NULL;
    count++;
  }
  printf("Str is %s\n", strToReturn);
  // Skip spaces
  while(*strToReturn++ == ' ');
  // Insert the space and the first char of the argument in the return string
  strToReturn-=2;

  return strToReturn;
}

int str_compare(Pointer a,Pointer b){
    return strcmp(a, b);
}