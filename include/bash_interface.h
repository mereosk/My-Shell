#pragma once // #include once

#include "ADTMap.h"
#include "ADTVector.h"

extern enum RedirectOptions {
    Both,
    InOnly,
    OutOnly
};


void execute_redirection(char *command, char *inFile, char *outFile, Vector vecArg, bool redirectionFlag);

// Creates an alias
bool create_alias(Map map, char *command);

void destroy_alias(Map map, char *command);

void execute_command(char *command, Vector vecArg);