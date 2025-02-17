#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "parsing.h"

#define EXPCOMMAND 0
#define EXPARGUMENT 1
#define OUTREDIRECT 2
#define INREDIERECT 3

// Compare function that returns true if sting a starts with b
int starts_with(Pointer a,Pointer b){
    return strncmp(a, b, strlen(b));
}


void parse(char *inputCommandWhole , Vector historyVector, Map aliasMap){
    char *separateCommand, *firstWord, *SCommandCopy, *separateCommandParse;
    char *savePtrFW, *wholeCommand;
    char *kwCreateAlias="createalias";
    char *kwDestroyAlias="destroyalias";
    char *kwHistory="history";
    char *kwHistory2="myHistory";
    char *kwAlias="alias";
    char *remStr=malloc(256 * sizeof(*remStr));
    char *wholeStr=calloc((256),sizeof(*wholeStr));
    char *command = malloc(20 * sizeof(*command));
    int intDesignator, backgroundFlag=false;
    int fd;

    // This variable is how many times have we replaced a env var
    int envCount=0;
    while(1) {
      // The first time we check for we check for env vars, the inputCommandWhole
      // is not dynamically allocated, thus we need to treat it differently than 
      // the other times that it is actually dynamically alloced
      if(envCount==0) {
        wholeCommand = replace_env_vars(inputCommandWhole);
        // If they are the same there are no other env vars to replace
        if(strcmp(wholeCommand, inputCommandWhole)==0)
          break;

        inputCommandWhole=wholeCommand;
      }
      else {
        wholeCommand = replace_env_vars(inputCommandWhole);
        // If they are the same there are no other env vars to replace
        if(strcmp(wholeCommand, inputCommandWhole)==0) {
          free(inputCommandWhole);
          break;
        }
          
        free(inputCommandWhole);
        inputCommandWhole=wholeCommand;
      }
      
      envCount++;
    }
    
    // Do not pass the actual wholeCommand it the strtok_r, because there will
    // be a problem free'ing it, so make a copy of it
    int wholeCommandLength = strlen(wholeCommand);
    char *wholeCommandCopy = (char *)calloc(wholeCommandLength+1, sizeof(char));
	  char *save=wholeCommandCopy;
    strncpy(wholeCommandCopy, wholeCommand, wholeCommandLength);

    char *restSC = wholeCommandCopy;
    // When the shell sees a semicolon then it's treated
    // as a command separator
    while((separateCommand = strtok_r(restSC, ";", &restSC) )) {
        separateCommand = trim_whitespace(separateCommand);

        // Take the first word of the separate command. Again as before
        // dont pass the separateCommand in strto_r but a copy of it
        int separateCommandLength = strlen(separateCommand);
        SCommandCopy = (char *)calloc(separateCommandLength+1, sizeof(char));
        strncpy(SCommandCopy, separateCommand, separateCommandLength);
        firstWord = strtok_r(SCommandCopy, " ", &savePtrFW);

        // Check if the command has to execute in the background. If so
        // mark it by turning the flag true
        if(separateCommand[strlen(separateCommand)-1] == '&')
          backgroundFlag=true;
        // Check if the first word is the keyword for creating an alias
        if(strcmp(firstWord, kwCreateAlias)==0){
          if(rest_args(separateCommand) == NULL) 
            fprintf(stderr,"\n-mysh: createalias: Correct usage: 'createalias command \"cd changed command\"'\n");
          else
            create_alias(aliasMap, rest_args(separateCommand));
          free(SCommandCopy);
          continue;
        }   
        else if(strcmp(firstWord, kwDestroyAlias)==0) {
          // Check if the first word is the keyword for destroying an alias
          if(rest_args(separateCommand) == NULL) 
            fprintf(stderr,"\n-mysh: destroyalias: Correct usage: 'destroyalias myhome'\n");
          else
            destroy_alias(aliasMap, rest_args(separateCommand));
          free(SCommandCopy);
          continue;
        }   
        else if(strcmp(firstWord, kwAlias)==0) {
          // Check if the first word is the keyword for destroying an alias
          map_print(aliasMap);
          free(SCommandCopy);
          continue;
        }   
        else if((strcmp(firstWord, kwHistory)==0) || (strcmp(firstWord, kwHistory2)==0)) {
          // Check if the first word is one of the keywords for showing the history of mysh
          // which are history and myHistory
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
                fprintf(stderr,"\n-mysh: !%d: event not found\n", intDesignator);
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
                fprintf(stderr,"\n-mysh: !%d: event not found\n", intDesignator);
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
                fprintf(stderr,"\n-mysh: !%d: event not found\n", intDesignator);
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
              fprintf(stderr,"\n-mysh: !%s: event not found\n", strToSearch);
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
            // If rest arguments is not NULL then it has arguments and we 
            // have to concatenate them
            if(restArgs != NULL) {
              strcat(wholeStr, restArgs);
            }
            // If restArgs is NULL it doesn't have other arguments
            // Substitude the last command (in history) that is !n/!-n/!!/!string 
            // with the command in subValue plus the rest arguments
            // However if the command is the same than the previous one, keep only one
            if(strcmp(wholeStr, vector_get_at(historyVector, vector_size(historyVector)-2))==0)
              vector_remove_last(historyVector);
            else
              vector_set_at(historyVector, vector_size(historyVector)-1, strdup(wholeStr));
            
            // At this point we have the form (wholeCommand)
            // !something arg1 arg2.. 
            // and we have to replace the !something with the wholeStr string that is
            // the the string the event designator gave us

            // str_replcae returns a dynamicly allocated string but parse needs a statically
            // allocated one, so copy strToBeParsed to nonDynamicStr and we are ok
            char *strToBeParsed = str_replace(wholeCommand, separateCommand, wholeStr);
            char *nonDynamicStr[strlen(strToBeParsed)+1];
            strcpy(nonDynamicStr, strToBeParsed);
            // Free and the call parse again
            free(eventAfter);free(SCommandCopy);free(command);free(remStr);free(wholeStr);
            free(wholeCommand);free(save);free(strToBeParsed);
            parse(nonDynamicStr, historyVector, aliasMap);
            return;
          }
        }
        free(SCommandCopy);
        // Now we will parse the command using the logic of
        // state machines.
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
        // outfileSave is where the outFile will be saved so that
        // later will be used to do the redirection
        char *outfileSave=calloc(256, sizeof(*outfileSave));
        // The state of the machine starts expecting a command
        int state = EXPCOMMAND;
        // List will be for keeping the all the arguments (List of lists)
        List argsListAll = list_create(list_destroy);
        // List will be for keeping all the commands
        List comList = list_create(free);
        while(1) {
          strKeeper[indivStr]=separateCommandParse[k];
          // com1 
          if(separateCommandParse[k] == '\0' || separateCommandParse[k] == '&') {
            // If the state is command it is -mysh command
            if(state == EXPCOMMAND) {

              if(separateCommandParse[k-1]!=' ' && backgroundFlag == true){
                strKeeper[strlen(strKeeper)-1]='\0';
              }
              // Insert the command in the command list and create a new arg list for this command
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              wildcard_matching(argsListAll);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_command(comList, argsListAll, infileSave, NULL, appendFlag, backgroundFlag);
              break;
            }
            else if(state == EXPARGUMENT) {
              // Insert the last argument in the list and execute the command only if it is not &
              if(backgroundFlag == false) {
                List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
                list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              }
              else {
                if(separateCommandParse[k-1]!=' ' && backgroundFlag == true){
                  strKeeper[strlen(strKeeper)-1]='\0';
                  List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
                  list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
                }
              }
              wildcard_matching(argsListAll);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_command(comList, argsListAll, infileSave, NULL, appendFlag, backgroundFlag);
              break;
            }
            else if(state == OUTREDIRECT) {
              // If there is a & and the format is -> command < infile& then the infile will be in the strKeeper but
              // if the format is -> command < infile & then the infile will be in infileSave because there is an
              // space between infile and &, thus the parser will save it when it sees the space
              char *tempOutFile = (backgroundFlag == false || separateCommandParse[k-1]!=' ') ? strKeeper : outfileSave;
              // Remove the & from the infile
              if(separateCommandParse[k-1]!=' ' && backgroundFlag == true){
                tempOutFile[strlen(tempOutFile)-1]='\0';
              }

              wildcard_matching(argsListAll);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_command(comList, argsListAll, infileSave, tempOutFile, appendFlag, backgroundFlag);
              break;
            }
            else if(state == INREDIERECT) {
              // If there is a & and the format is -> command < infile& then the infile will be in the strKeeper but
              // if the format is -> command < infile & then the infile will be in infileSave because there is an
              // space between infile and &, thus the parser will save it when it sees the space
              char *tempInFile = (backgroundFlag == false || separateCommandParse[k-1]!=' ') ? strKeeper : infileSave;
              // Remove the & from the infile
              if(separateCommandParse[k-1]!=' ' && backgroundFlag == true){
                tempInFile[strlen(tempInFile)-1]='\0';
              }

              wildcard_matching(argsListAll);
              replace_aliases(comList, argsListAll, aliasMap);
              execute_command(comList, argsListAll, tempInFile, NULL, appendFlag, backgroundFlag);
              break;
            }
          }
          else if(separateCommandParse[k] == ' ') {
            // Without the space
            strKeeper[indivStr] = '\0';
            // Check if the stirng in strKeeper is a command
            if(state == EXPCOMMAND) {
              // Save the command and search for arguments
              strcpy(commandSave, strKeeper);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = EXPARGUMENT;
              // Skip the spaces
              while(separateCommandParse[k] == ' ') {
                k++;
              }
            }
            else if(state == EXPARGUMENT) {
              // Insert the argument in the list
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              // Skip the spaces
              while(separateCommandParse[k] == ' ') {
                k++;
              }
            }
            else if(state == OUTREDIRECT) {
              // Save the outFile
              strcpy(outfileSave, strKeeper);
              // Just skip the spaces and continue
              while(separateCommandParse[++k] == ' ');
              continue;
            }
            else if(state == INREDIERECT) {
              // Save the inFile
              strcpy(infileSave, strKeeper);
              // Just skip the spaces and continue
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
            if(state==EXPCOMMAND) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==EXPARGUMENT && separateCommandParse[k-1] == ' ') {
              state=OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==EXPARGUMENT && separateCommandParse[k-1] != ' ') {
              // Insert the argument in the list
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==OUTREDIRECT) {
              // There are multiple redirections so
              // open or truncade all files but use only
              // use the last file
              if((fd = creat(strKeeper, 0666)) == -1) {
                fprintf(stderr,"-mysh: %s: %s\n", strKeeper, strerror(errno));
              }
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
            else if(state==INREDIERECT) {
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              state = OUTREDIRECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ') ;
            }
            
            // Check if the next character is ">>" in order to redirect
            // insertsion in an existing file (append)
            // Also notice that k is the next k because we skipped at least
            // one character above
            if(separateCommandParse[k]=='>') {
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
            if(state==EXPCOMMAND) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==EXPARGUMENT && separateCommandParse[k-1] == ' ') {
              state=INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==EXPARGUMENT && separateCommandParse[k-1] != ' ') {
              // Insert the argument in the list
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=INREDIERECT;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==OUTREDIRECT) {
              fprintf(stderr,"-mysh: Can't have an in redirection after an out redirection\n");
              break;
            }
            else if(state==INREDIERECT) {
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              // There are multiple redirections so
              // open or truncade all files but use only
              // use the last file
              if((fd = open(strKeeper, O_RDONLY)) == -1) {
                  fprintf(stderr,"-mysh: %s: %s\n", strKeeper, strerror(errno));
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
            if(state==EXPCOMMAND) {
              // Save the command and continue
              strcpy(commandSave, strKeeper);
              list_insert_next(comList, list_last(comList), strdup(strKeeper));
              list_insert_next(argsListAll , list_last(argsListAll), list_create(free));
              // Change state to havecommand to search for arguments
              state = EXPCOMMAND;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            // If the previous character is space then there is no need to insert
            // an argument (it was added in the previous if)
            else if(state==EXPARGUMENT && separateCommandParse[k-1] == ' ') {
              state=EXPCOMMAND;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==EXPARGUMENT && separateCommandParse[k-1] != ' ') {
              // Insert the argument in the list
              List tempList = (List)list_node_value(argsListAll, list_last(argsListAll));
              list_insert_next( tempList, list_last(tempList), strdup(strKeeper));
              state=EXPCOMMAND;
              // Skip the spaces
              while(separateCommandParse[++k] == ' ');
            }
            else if(state==OUTREDIRECT) {
              fprintf(stderr,"-mysh: Can't have a pipe after a redirect\n");
              break;
            }
            else if(state==INREDIERECT) {
              // Save the infile and continue
              strcpy(infileSave, strKeeper);
              state = EXPCOMMAND;
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
        free(commandSave);free(strKeeper);free(infileSave);free(outfileSave);list_destroy(comList);list_destroy(argsListAll);
        
    }

    free(command);
    free(remStr);
    free(wholeStr);
    free(wholeCommand);
    free(save);
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
  // It starts with the number so return it as an interger
  if(strlen(copyStr)>0) {
    retNumber = atoi(copyStr);
    free(copyStr);
    return retNumber;
  }
  free(copyStr);
  // It does not start with a number thus return 0
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
    if(count>=len)
      return NULL;
    count++;
  }
  // Skip spaces
  while(*strToReturn++ == ' ');
  // Insert the space and the first char of the argument in the return string
  strToReturn-=2;

  return strToReturn;
}

int str_compare(Pointer a,Pointer b){
    return strcmp(a, b);
}