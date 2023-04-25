#pragma once // #include once

#include "ADTVector.h"
#include "ADTMap.h"
#include "bash_interface.h"

// This function parses the string
void parse(char *inputStr, Vector historyVector, Map aliasMap);

char *trim_whitespace(char *str);

int begins_with_number(char *str);

char *rest_args(char *str);

// Classic compare function for strings
int str_compare(Pointer a,Pointer b);
