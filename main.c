#include "main.h"

int status;

int
main(int argc, char **argv) {
	char buf[BUFSIZ], opt, *builtin, shell[BUFSIZ];
	int opt_c, opt_x, pcount = 0, fd[2];

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

	while ((opt = getopt(argc, argv, "xc:")) != -1) {
		switch (opt) {
			case 'c':
				opt_c = 1;
			
				if (optarg == NULL) {
					printf("Usage: sish [-x] [-c command]\n");
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
		if (execute(optarg, STDIN_FILENO, STDOUT_FILENO) != EXIT_SUCCESS) {
			exit(ERROR);
		}

		exit(EXIT_SUCCESS);
	}

	while (getinput(buf, BUFSIZ)) {
		if (strlen(buf) == 1) continue;

		char *pipes[MAXTOKENS];

		buf[strlen(buf)-1] = '\0';

		if ((pcount = getparam(buf, pipes, MAXTOKENS, "|")) == -1) {
			perror("command is too long\n");
			status = ERROR;
		}

		if (opt_x == 1) {
			for (int i = 0; pipes[i]; i++) {
				fprintf(stderr, "+ %s\n", pipes[i]);
			}
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
			int in = STDIN_FILENO;
			int i = 0;
			while (i < pcount - 1) {
				if (pipe(fd) == -1) {
					fprintf(stderr, "failed to pipe in main: %s\n", strerror(errno));
					break;
				}

				if ((status = execute(pipes[i], in, fd[1])) != EXIT_SUCCESS) {
					break;
				}

				(void)close(fd[1]);
				in = fd[0];
				i++;
			}

			status = execute(pipes[i], in, STDOUT_FILENO);
		}	
	}

	exit(EXIT_SUCCESS);
}
				

