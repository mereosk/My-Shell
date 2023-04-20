#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ADTList.h"
#include "helping_funcs.h"

void cyan() {
  printf("\033[1;36m");
}

void reset() {
  printf("\033[0m");
}

int main(int argc, char** argv) {

    List mylist = list_create(NULL);
    char *inputBuffer = malloc(256 * sizeof(*inputBuffer));

    while(1) {
        cyan();
        printf("\nin-mysh-now:> ");
        reset();
        // Read 256 bytes and put them in inputBuffer
        if( fgets (inputBuffer, 256, stdin) != NULL ) {
            // First trim all the whitespace
            char *trimmedInputBuffer = trim_whitespace(inputBuffer);
            // Check if the user wants to exit the shell
            if(strcmp(trimmedInputBuffer,"exit")==0)
              break;

            /* writing content to stdout */
            puts(trimmedInputBuffer);
            // printf("size of the array is %d", sizeof(inputBuffer[0]));
            // Parse the input
            parse(trimmedInputBuffer);
        }
    }

    // Free the allocated spaces
    free(inputBuffer);
    list_destroy(mylist);
}