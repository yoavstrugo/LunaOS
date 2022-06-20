#ifndef _COMMON
#define _COMMON

#include <limits.h>

char *make_absolute(char *main, char *sub);


extern char *current_working_directory;
char *get_working_directory(char *for_whom);
void set_working_directory(char *working_directory);

#endif