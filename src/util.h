#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_LINE 1024

void    trim_newline(char *s);
char   *read_first_line(const char *path);
char   *read_value_from_proc(const char *path, const char *prefix);
char   *exec_cmd(const char *cmd);
int     count_digits(int n);
char   *make_bar(int pct, int use_color);

#endif
