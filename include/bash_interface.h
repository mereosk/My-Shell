#pragma once // #include once

#include "ADTMap.h"
#include "ADTVector.h"
#include "ADTList.h"

void execute_redirection(char *command, char *inFile, char *outFile, List argsList, bool redirectionFlag);

// Creates an alias
bool create_alias(Map map, char *command);

void destroy_alias(Map map, char *command);

void execute_command(char *command, List argsList);

void execute_pipe(List commands, List argsListAll, char *inFile, char *outFile, bool appendFlag);

//  This function goes through all the arguments in the arg list
//  and finds all the wildcards. Nextly in matches them with filenames
//  and returns a list that is the old one plus the filenames that go matched
void wildcard_matching(List argsListAll);