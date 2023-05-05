#pragma once // #include once

#include "ADTVector.h"
#include "ADTMap.h"
#include "bash_interface.h"
#include "ADTList.h"

// This function parses the string
void parse(char *inputStr, Vector historyVector, Map aliasMap);

int begins_with_number(char *str);

char *rest_args(char *str);

// Classic compare function for strings
int str_compare(Pointer a,Pointer b);
