#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <errno.h>

#include "ADTList.h"
#include "parsing.h"
#include "ADTVector.h"
#include "ADTMap.h"

void cyan() {
  printf("\033[1;36m");
}

void reset() {
  printf("\033[0m");
}

void catchInterupt(int signo) {
  // Do not do anything
}

int main(int argc, char** argv) {
    int status, kidpid;

    static struct sigaction act;
    act.sa_handler=catchInterupt;
    sigfillset(&(act.sa_mask));

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);

    // Initialise the history vector
    Vector historyVector = vector_create(0,  free);
    // Initialise the map that will keep the aliases
    Map aliasMap = map_create(str_compare, free, free);
    map_set_hash_function(aliasMap, hash_string);

    char *inputBuffer = malloc(256 * sizeof(*inputBuffer));

    while(1) {
        cyan();
        printf("\nin-mysh-now:> ");
        reset();
        // Read 256 bytes and put them in inputBuffer
        if( fgets (inputBuffer, 256, stdin) != NULL ) {
            // First trim all the whitespace
            char *trimmedInputBuffer = trim_whitespace(inputBuffer);
            // Pass the command into the history vector. However don't 
            // insert it if the previous command is the same (There's no reason)
            if(vector_get_at(historyVector, vector_size(historyVector)-1)==NULL || \
            strcmp(trimmedInputBuffer, vector_get_at(historyVector, vector_size(historyVector)-1))!=0)
                vector_insert_last(historyVector, strdup(trimmedInputBuffer));
               
            
            // Check if the user wants to exit the shell
            if(strcmp(trimmedInputBuffer,"exit")==0)
              break;
            // Check before parsing if any child has finished
            while ((kidpid = waitpid(-1, &status, WNOHANG)) > 0) ;
            // Parse the input
            parse(trimmedInputBuffer, historyVector, aliasMap);
        }
    }

    vector_print(historyVector);

    // Free the allocated spaces left
    free(inputBuffer);
    map_destroy(aliasMap);
    vector_destroy(historyVector);
}