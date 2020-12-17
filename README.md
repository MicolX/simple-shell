---- 12.16.2020 ----
1. start the project
2. use getopt to handle two options, -c and -x
3. ignore SIGINT, SIGQUIT, SIGTSTP
4. sish can execute one command with option -c and return the correct status code
5. two built-in implemented: exit and cd
6. -x mode implemented
7. make a makefile

---- 12.17.2020 ----
1. now echo can be redirected to a file, works for both '>' and '>>'
2. need to fix execute(), it doesn't take arguments
3. -x mode also need to modified
4. now I know why we need to tokenize the string, but still not sure about the
right way to do it
