#include <sys/param.h>
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
#define MAXTOKENS 128
#define UMASK 0664

extern int status;

#endif

char *getinput(char *, size_t);
int execute(char *, int, int);
int cd(char *);
int echo(char *);
int getparam(char *, char **, int, const char *);
