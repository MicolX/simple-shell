#include "main.h"

int status;

int
main(int argc, char **argv) {
	char buf[BUFSIZ], opt, *builtin, shell[BUFSIZ];
	int opt_c, opt_x;

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGINT signal\n");
	}

	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGQUIT signal\n");
	}

	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "failed to set SIG_IGN to SIGTSTP signal\n");
	}

	snprintf(shell, BUFSIZ, "%s/%s", getcwd(NULL, MAXPATHLEN), argv[0]+2);
	if (setenv("SHELL", shell, 1) == -1) {
		err(EXIT_FAILURE, "failed to set $SHELL: %s\n", strerror(errno));
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
		if (execute(optarg) != EXIT_SUCCESS) {
			exit(ERROR);
		}

		exit(EXIT_SUCCESS);
	}

	while (getinput(buf, BUFSIZ)) {
		
		buf[strlen(buf)-1] = '\0';

		if (opt_x == 1) {
			fprintf(stderr, "+ %s\n", buf);
		}

		char *line = strdup(buf);
		builtin = strsep(&line, " \t");

		if (strlen(builtin) == 4 && strncmp(builtin, "exit", 4) == 0) {
			exit(EXIT_SUCCESS);
		} else if (strlen(builtin) == 2 && strncmp(buf, "cd", 2) == 0) {
			status = cd(line);
		} else if (strlen(builtin) == 4 && strncmp(buf, "echo", 4) == 0) {
			status = echo(line);
		} else {
			status = execute(buf);
		}
	}

	exit(EXIT_SUCCESS);
}
				

