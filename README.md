# HShell
This project is a shell built from scratch in C++, which has implementation of both basic functionality to run common commands, as well as more advanced features of a zsh/bash-like shell. TLDR cool features: background jobs, command history, output redirection.

First, user's commands are read in at the terminal. For running basic commands (like 'ls', 'echo', etc.), the execvp function is used. I chose execvp over other options in the exec family, as execvp will search the PATH variable for executables (the p) and the command arguments can be passed as an array, as opposed to a variable number of arguments for the command (the v). When a command is entered, a new child process is created using fork(), and execvp runs from the child process with given command and arguments.

There were also a number of built-in commands that I implemented. Here, "built-in" means that these commands don't result in a new forked process, but rather the commands would be run directly from the parent process. These included 'cd', 'pwd', 'exit', 'jobs' (for listing the currently running background jobs, discussed more later), and 'history'.

**Background processes** In bash/zsh, the '&' token at the end of a command indicates that the command should be run in the background, allowing the user to immediately regain control of the shell program. In order to implement this, of course first the '&' token had to be detected. After that, the steps to start the command were similar to the foreground case, except after the fork(), a background process would use setsid() to create a new session and disconnect itself from the terminal. Meanwhile, the main change in the parent process (the main shell program) was that instead of simply using waitpid to wait for the child to terminate, the job and its pid would be stored in a data structure. The terminal output on the start and finish of jobs, as well as the 'jobs' command, was modeled after that of zsh. In the background case, since waitpid was not being used (so the user could continue entering commands), there had to be a way to detect when jobs completed in the background; for this, I used a signal handler for SIGCHLD that would wait for the child, update the jobs table, and output that the job had completed. One consequence of using signals in the workflow and allowing concurrent process running was that terminal output could be inconsistent (owing to asynchronicity). To resolve this, I used sigprocmask to mask the SIGCHLD when creating a new job, keeping output consistent and clean.

**Command history** Enabling the user to go through past commands using up-down arrow keys. However, to allow this, I had to be able to detect key presses. Thus, I implemented a custom getch() function that modified terminal settings and used the 'read' system call to take chars one at a time from the terminal without the Enter key needing to be pressed. This allowed me to handle all regular letter key presses, but also detect when the arrow keys or Enter were pressed. For command history, a couple different data structures were needed: first was a simple deque, that would store the history; second was an unordered_map that, when history commands were edited, they could be restored upon the next Enter key press. Thus, the user could use up-down arrows to reuse previously entered commands, and the behavior of those commands stored closely resembles that of zsh/bash. The 'history' built-in command was implemented to provide the most recent commands.

**Output redirection** with the '>' operator. This process involved checking each token in a command for the arrow operator, and then looking for a file location after that arrow. In the case output redirection was necessary, the original STDOUT file descriptor (the terminal the program was running from) was saved, and then was pointed to a newly opened FD for the file. These were both done using the dup2 system call. Finally, after the user's command had been carried out, the original file descriptors for input and output were restored.

Other notes:
- Used mutexes when modifying the jobs table/map since signals introduced asynchronicity
- There were a number of data members (job table, command, mutex, job number, etc) that had to be global variables to allow their access within the static signal handler functions
