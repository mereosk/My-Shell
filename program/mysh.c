#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ADTList.h"
#include "helping_funcs.h"
#include "ADTVector.h"
#include "ADTMap.h"

void cyan() {
  printf("\033[1;36m");
}

void reset() {
  printf("\033[0m");
}

int main(int argc, char** argv) {

    int myshHistoryDF;
    List mylist = list_create(NULL);
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

            /* writing content to stdout */
            puts(trimmedInputBuffer);
            // printf("size of the array is %d", sizeof(inputBuffer[0]));
            // Parse the input
            parse(trimmedInputBuffer, historyVector, aliasMap);
        }
    }

    vector_print(historyVector);

    // Free the allocated spaces left
    free(inputBuffer);
    map_destroy(aliasMap);
    list_destroy(mylist);
    vector_destroy(historyVector);
}