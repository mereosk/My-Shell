#pragma once // #include once

#include "ADTMap.h"
#include "ADTVector.h"
#include "ADTList.h"

void execute_redirection(char *command, char *inFile, char *outFile, List argsList, bool redirectionFlag);

// Creates an alias
bool create_alias(Map map, char *command);

void destroy_alias(Map map, char *command);

void execute_command(char *command, List argsList);