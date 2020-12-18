#include "util.h"


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
execute(char *command, int in, int out) {
	pid_t pid;
	char *args[MAXTOKENS];

    if (getparam(command, args, MAXTOKENS, " ") == -1) {
		perror("Invalid syntax: too many arguments\n");
		return ERROR;
	}

	if ((pid = fork()) == -1) {
		fprintf(stderr, "sish: can't fork: %s\n", strerror(errno));
	} else if (pid == 0) {
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

		execvp(args[0], args);
		fprintf(stderr, "sish: couldn't exec %s: %s\n", command, strerror(errno));
		exit(ERROR);
	}

	if (wait(&status) == -1) {
		perror("failed at wait\n");
		status = ERROR;
	}
//	if (out == STDOUT_FILENO) {
//		if (wait(&status) == -1) {
//			perror("failed at wait\n");
//			status = ERROR;
//		}
//	}

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

	if (strchr(str, '>') != NULL) {
		int fd;
		int oflag = O_WRONLY|O_CREAT;	

		left = strsep(&str, ">");

		while (left[strlen(left) - 1] == ' ') left[strlen(left) - 1] = '\0';
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
	
		(void)strlcat(str, "\n", 2);
		if (write(fd, left, strlen(left)) != (signed int)strlen(left)) {
			fprintf(stderr, "error writing to %s: %s\n", str, strerror(errno));
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	(void)dollar(&str);
	printf("%s\n", str);

	return EXIT_SUCCESS;
}
