#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef UTIL_H
#define UTIL_H

#define ERROR 127

extern int status;

#endif

char *getinput(char *, size_t);
int execute(char *);
int cd(char *);
int echo(char *);
