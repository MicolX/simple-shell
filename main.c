#include "main.h"

char *
getinput(char *buffer, size_t buflen) {
	printf("sish$ ");
	return fgets(buffer, buflen, stdin);
}


int
main(int argc, char **argv) {
	char buf[BUFSIZ], opt;
	pid_t pid;
	int opt_c, opt_x, status;
	struct passwd *pw;

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGINT signal\n");
	}

	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGQUIT signal\n");
	}

	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGTSTP signal\n");
	}

	while ((opt = getopt(argc, argv, "c:x")) != -1) {
		switch (opt) {
			case 'c':
				opt_c = 1;
			
				if (optarg == NULL) {
					printf("sish [-x] [-c command]\n");
				}
				break;
			
			case 'x':
				opt_x = 1;
				break;

			default:
				err(EXIT_FAILURE, "%s: invalid option '%s'", argv[0], optarg);
		}
	}

	if (opt_c && optarg != NULL) {
		if ((pid = fork()) == -1) {
			err(EXIT_FAILURE, "sish: can't fork: %s\n", strerror(errno));
		} else if (pid == 0) {
			execlp(optarg, optarg, (char *)0);
			err(EXIT_FAILURE, "sish: couldn't exec %s: %s\n", optarg, strerror(errno));
			exit(127);
		}

		if (wait(&status) == -1) {
			perror("failed at wait\n");
		}

		if (status != EXIT_SUCCESS) {
			exit(127);
		}

		exit(EXIT_SUCCESS);
	}

	while (getinput(buf, BUFSIZ)) {
		
		buf[strlen(buf)-1] = '\0';

		if (opt_x == 1) {
			fprintf(stderr, "+ %s\n", buf);
		}

		if (strlen(buf) == 4 && strncmp(buf, "exit", 4) == 0) {
			exit(EXIT_SUCCESS);
		}

		if (strncmp(buf, "cd", 2) == 0) {
			char *cd = strdup(buf);
			(void)strsep(&cd, " \t");
			if (cd == NULL) {
				if ((pw = getpwuid(getuid())) == NULL) {
					fprintf(stderr, "failed to get passwd\n");
				}
				if (chdir(pw->pw_dir) != 0) {
					fprintf(stderr, "cd: can't cd to %s\n", pw->pw_dir);
				}
			} else {
				if (chdir(cd) != 0) {
					fprintf(stderr, "cd: can't cd to %s\n", cd);
				}
			}
			continue;
		}

		if ((pid = fork()) == -1) {
			err(EXIT_FAILURE, "sish: can't fork: %s\n", strerror(errno));
		} else if (pid == 0) {
			execlp(buf, buf, (char *)0);
			err(EXIT_FAILURE, "sish: couldn't exec %s: %s\n", buf, strerror(errno));
			exit(127);
		}
		
		if (wait(&status) == -1) {
			perror("failed at wait\n");
		}

		if (!WIFEXITED(status)) {
			exit(127);
		}
	}

	exit(EXIT_SUCCESS);
}
				

