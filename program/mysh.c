#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ADTList.h"

int main(int argc, char** argv) {

    List mylist = list_create(NULL);
    char yo[50];
    while(1) {
        printf("\nin-mysh-now:>");
        scanf("%s", yo);
    }

    list_destroy(mylist);
}