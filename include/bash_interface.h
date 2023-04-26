#pragma once // #include once

#include "ADTMap.h"
#include "ADTVector.h"

void execute_redirection(char *command, int outputFD);

// Creates an alias
bool create_alias(Map map, char *command);

void destroy_alias(Map map, char *command);

void execute_command(char *command, Vector vecArg);