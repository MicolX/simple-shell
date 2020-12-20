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
5. move some code into the util

---- 12.19.2020 ----
1. Through so many hours, I finally got the pipes done!!!! Exciting!!

---- 12.20.2020 ----
1. implemented redirect and it works fine with pipelines
2. left to be done: 
	- return correct status code
	- backgrounding

---- 12.21.2020 ----
1. implemented backgrounding
2. fix a bug that would cause zombie process after a background task. 
   I don't wait and set a SIGCHLD handler when executing background command, 
   in the handler, I reap all the child processes that are not waited, and 
   then set SIG_IGN to SIGCHLD. Actually, I'm not really sure about setting 
   SIG_IGN to SIGCHLD. I used to set SIG_DFL, but that would generate zombie
   in some cases. After I set SIG_IGN, no zombie any more. I know that in BSD, 
   if I specifically set SIG_CHLD to SIG_IGN, the zombies would automatically 
   reaped by the system(i guess it's init).
3. properly get the exit code of the child process now

--- tests that I tried ---

apart from the test cases in the manual page, I also tried the following tests:

- combination of redirect and pipelines, e.g. " < file command | cmd "
- try commands with redirect, but shuffle the order, or seperate each component
  with multiple spaces. 
  e.g. " <     file     >> file1     command" or "> file1     command <file"
- quickly input several commands after a time-consuming backgroud task
