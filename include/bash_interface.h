#pragma once // #include once

#include "ADTMap.h"
#include "ADTVector.h"
#include "ADTList.h"

// Takes a command that has the form
// [space]keyStr "value1 value2 etc"
// and it inserts it in the aliasMap, the key is keyStr
// and its value everything inside ""
bool create_alias(Map map, char *command);

// Takes a command that has the form
// [space]keyStr
// and it removes it from the alias map if it exists
void destroy_alias(Map map, char *command);

// This is the 'heart' of my shell. It uses forks and pipes in order to execute a command. But it can do a lot more
// than that. Specifically, it can use pipes, execute input and output redirection, make sure that the processes will
// be executed in the background and more. For a more thorough explanation of execute_command be sure check my README file.
void execute_command(List commands, List argsListAll, char *inFile, char *outFile, bool appendFlag, bool backgroundFlag);

//  This function goes through all the arguments in the arg list
//  and finds all the wildcards. Nextly in matches them with filenames
//  and returns a list that is the old one plus the filenames that go matched
void wildcard_matching(List argsListAll);

// This function takes the list of commands, the list of lists of arguments and the alias map,
// then it loops through all the commands and their list of args and checks if a command is a
// key in alias map. It that is true it updates the lists with the key's value in the aliasMap 
void replace_aliases(List comList, List argList, Map aliasMap);

// Takes a string and trims the whitespace
char *trim_whitespace(char *str);

// Takes a list that should have the directory in its first node value (if not cd has too many args)
// the with the help of 'chdir' it changes the current working directory 
void change_directory(List directoryList);

// Loops through inputCommandWhole str and if it meets '$UPPERCASELETTERS' it 
// replaces it with the environment variable name and returns the new string.
// If it didn't find anything to replace, it returns the input. One more thing
// to address is that the return value is dynamically allocated
char *replace_env_vars(char *inputCommandWhole);

// Replaces the 'rep' substring of the 'orig' string with the substring 'with'. Then
// it returns the new dynamically allocated string
char *str_replace(char *orig, char *rep, char *with);