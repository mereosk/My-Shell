#pragma once // #include once

#include "ADTMap.h"

void output_redirection(char *command, int outputFD);

// Creates an alias
bool create_alias(Map map, char *command);