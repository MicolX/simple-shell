#include "util.h"

typedef enum Flag {
	OVERWRITE,
	APPEND,
	NONE
} Flag;

int status;

char *
getinput(char *buffer, size_t buflen) {
	printf("sish$ ");
	return fgets(buffer, buflen, stdin);
}

void
dollar(char **str) {
	char temp[BUFSIZ];

	if (strlen(*str) == 2 && strncmp(*str, "$?", 2) == 0) {
		snprintf(temp, BUFSIZ, "%d", status);
	} else if (strlen(*str) == 2 && strncmp(*str, "$$", 2) == 0) {
		snprintf(temp, BUFSIZ, "%d", getpid());
	} else {
		return;
	}

	*str = strdup(temp);
}

int
getparam(char *command, char **argv, int len, const char *sep) {
	if (command == NULL || strlen(command) == 0) return 0;

	char *p;
	int i = 0;
	
	for ((p = strtok(command, sep)); p; (p = strtok(NULL, sep)), i++) {
		if (i < len - 1) {
			while (p[0] == ' ' || p[0] == '\t') p++;
			while (p[strlen(p) - 1] == ' ') p[strlen(p) - 1] = '\0';
			argv[i] = p;
		} else {
			return -1;
		}
	}
	argv[i] = NULL;
	return i;
}

int
redirect(char *command, char *infile, char *outfile, char **argv, size_t len, Flag* flag) {
	char *p = command, *q = command;
	int i = 0, isin = 0, isout = 0;
	infile[0] = '\0';
	outfile[0] = '\0';

	while (p[0] != '\0' && q[0] != '\0' && i < (signed int)len) {
		if (q[0] == '<') {
			isin = 1;
			q[0] = '\0';

			if (strlen(p) > 0) {
				while (p[0] == ' ' || p[0] == '\t') p++;
				while (p[strlen(p) - 1] == ' ') p[strlen(p) - 1] = '\0';
			}

			if (strlen(p) > 0) {
				if (isout) {
					(void)strlcpy(outfile, p, strlen(p) + 1);
					isout = 0;
				} else {
					argv[i++] = p;
				}
			}
			p = q + 1;
		} else if (q[0] == '>') {
			isout = 1;
			q[0] = '\0';

			if (strlen(p) > 0) {
				while (p[0] == ' ' || p[0] == '\t') p++;
				while (p[strlen(p) - 1] == ' ') p[strlen(p) - 1] = '\0';
			} else {
				if (isin) return -1;
			}

			if (strlen(p) > 0) {
				if (isin) {
					(void)strlcpy(infile, p, strlen(p) + 1);
					isin = 0;
				} else {
					argv[i++] = p;
				}
			} else {
				if (isin) return -1;
			}

			if (q+1 != NULL) {
				if (q[1] == '>') {
					*flag = APPEND;
					q++;
				} else {
					*flag = OVERWRITE;
				}
			} else {
				return -1;
			}
			p = q + 1;
		} else if (q[0] == ' ') {
			q[0] = '\0';

			if (strlen(p) > 0) {
				while (p[0] == ' ' || p[0] == '\t') p++;
				while (p[strlen(p) - 1] == ' ') p[strlen(p) - 1] = '\0';
			}

			if (strlen(p) > 0) {
				if (isout) {
					(void)strlcpy(outfile, p, strlen(p) + 1);
					isout = 0;
				} else if (isin) {
					(void)strlcpy(infile, p, strlen(p) + 1);
					isin = 0;
				} else {
					argv[i++] = p;
				}
			}
			p = q + 1;	
		}
		
		q++;
	}

	if (isout + isin == 2) return -1;

	if (strlen(p) > 0) {
		if (isin) {
			(void)strlcpy(infile, p, strlen(p) + 1);
		} else if (isout) {
			(void)strlcpy(outfile, p, strlen(p) + 1);
		} else {
			argv[i++] = p;
		}
	}

	argv[i] = NULL;
	return i;
}



int
execute(char *command, int in, int out) {
	pid_t pid;
	char *args[MAXTOKENS], infile[MAXPATHLEN], outfile[MAXPATHLEN];
	int infd = -1, outfd = -1, oflag, argc, wopt = WALLSIG;
	Flag flag = NONE;

	if ((argc = redirect(command, infile, outfile, args, MAXTOKENS, &flag)) == -1) {
		perror("Invalid syntax for redirection\n");
		return EXIT_FAILURE;
	}

	if (strcmp(args[argc - 1], "&") == 0) {
		wopt = WNOHANG;
		args[--argc] = NULL;
	}

	if ((pid = fork()) == -1) {
		fprintf(stderr, "sish: can't fork: %s\n", strerror(errno));
	} else if (pid == 0) {
		if (strlen(infile) > 0) {
			if ((infd = open(infile, O_RDONLY)) == -1) {
				fprintf(stderr, "failed to open %s: %s\n", infile, strerror(errno));
				return ERROR;
			}

			if (dup2(infd, in) == -1) {
				fprintf(stderr, "failed to dup: %s\n", strerror(errno));
				return EXIT_FAILURE;
			}

			(void)close(infd);
		}

		if (strlen(outfile) > 0) {
			oflag = O_CREAT|O_WRONLY;
			if (flag == APPEND) {
				oflag = oflag|O_APPEND;
			} else {
				oflag = oflag|O_TRUNC;
			}

			if ((outfd = open(outfile, oflag, UMASK)) == -1) {
				fprintf(stderr, "failed to open %s: %s\n", infile, strerror(errno));
				return ERROR;
			}

			if (dup2(outfd, out) == -1) {
				fprintf(stderr, "failed to dup: %s\n", strerror(errno));
				return EXIT_FAILURE;
			}

			(void)close(outfd);
		}

		if (in != STDIN_FILENO) {
			if (dup2(in, STDIN_FILENO) == -1) {
				perror("dup failed during execution\n");
				exit(ERROR);
			}
			(void)close(in);
		}

		if (out != STDOUT_FILENO) {
			if (dup2(out, STDOUT_FILENO) == -1) {
				perror("dup failed during execution\n");
				exit(ERROR);
			}
			(void)close(out);
		}
		
		if (wopt == WNOHANG) {
			if (setpgid(0, 0) == -1) {
				err(EXIT_FAILURE, "failed to run in background: %s\n", strerror(errno));
			}
		}

		execvp(args[0], args);
		fprintf(stderr, "sish: couldn't exec %s: %s\n", command, strerror(errno));
		exit(ERROR);
	}

	if (waitpid(pid, &status, wopt) == -1) {
		perror("failed at wait\n");
		status = ERROR;
	}

	if (status != EXIT_SUCCESS) {
		status = ERROR;
	}

	return status;
}

int
cd(char *path) {
	struct passwd *pw;

	if (path == NULL) {
		if ((pw = getpwuid(getuid())) == NULL) {
			fprintf(stderr, "failed to get passwd\n");
			return ERROR;
		}

		if (chdir(pw->pw_dir) != 0) {
			fprintf(stderr, "cd: can't cd to %s\n", pw->pw_dir);
			return ERROR;
		}
	} else {
		if (chdir(path) != 0) {
			fprintf(stderr, "cd: can't cd to %s\n", path);
			return ERROR;
		}
	}

	return EXIT_SUCCESS;
}

int
echo(char *str) {
	char *left;
	char message[BUFSIZ];

	if (strchr(str, '>') != NULL) {
		int fd;
		int oflag = O_WRONLY|O_CREAT;	

		left = strsep(&str, ">");

		while (left[strlen(left) - 1] == ' ') left[strlen(left) - 1] = '\0';
		while (left[0] == ' ') left++;

		(void)dollar(&left);

		if (str == NULL) {
			fprintf(stderr, "Syntax error: newline unexpected\n");
			return EXIT_FAILURE;
		}

		if (str[0] == '>') {
			str++;
			oflag = oflag|O_APPEND;
		} else {
			oflag = oflag|O_TRUNC;
		}
		
		if (str == NULL) {
			fprintf(stderr, "Syntax error: newline unexpected\n");
			return EXIT_FAILURE;
		}

		while (str != NULL && str[0] == ' ') str++;
		
		if (str == NULL) {
			fprintf(stderr, "Syntax error: newline unexpected\n");
			return EXIT_FAILURE;
		}

		if ((fd = open(str, oflag, 0664)) == -1) {
			fprintf(stderr, "failed to open %s: %s\n", str, strerror(errno));
			return EXIT_FAILURE;
		}
	
		(void)snprintf(message, BUFSIZ, "%s\n", left);
		if (write(fd, message, strlen(message)) != (signed int)strlen(message)) {
			fprintf(stderr, "error writing to %s: %s\n", str, strerror(errno));
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	(void)dollar(&str);
	printf("%s\n", str);

	return EXIT_SUCCESS;
}
