#pragma once // #include once

#include "ADTVector.h"
#include "ADTMap.h"
#include "bash_interface.h"
#include "ADTList.h"

// This function parses the string
void parse(char *inputStr, Vector historyVector, Map aliasMap);

// This function checks if a string starts with a number and then
// returns the number as an int or returns 0 if the string does
// not starts with a number
int begins_with_number(char *str);

// rest_args takes a string as argument, skips the first word
// and it returns the rest substring. If there are no rest args
// it return NULL
char *rest_args(char *str);

// Classic compare function for strings
int str_compare(Pointer a,Pointer b);
