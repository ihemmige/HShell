# HShell
This project is a shell built from scratch in C++, which has implemented both basic functionality to run common Unix-like commands, as well as more advanced features of a zsh/bash-like shell. TLDR cool features: background jobs, command history, output redirection.

First, user's commands are read in at the terminal. For running basic commands (like 'ls', 'echo', etc.), the execvp function is used. I chose execvp over other options in the exec family, as execvp will search the PATH variable for executables (the p) and the command arguments can be passed as an array, as opposed to a variable number of arguments for each part of the command (the v). When a command is entered, a new child process is created using fork(), and execvp is run from the child process with the given command and arguments.

There were also a number of built-in commands that I implemented. Here, "built-in" means that entering these commands wouldn't result in a new forked process, but rather the commands would be run directly from the parent process. These included 'cd', 'pwd', 'exit', and 'jobs' (for listing the currently running background jobs, discussed more later).

I also implemented command history, enabling the user to go through past commands using the up-down arrow keys. However, to allow this, I had to be able to detect key presses. Thus, I implemented a custom getch() function that modified terminal settings and used the 'read' system call to take chars one at a time from the terminal without the enter key needing to be pressed. This allowed me to handle all regular letter key presses, but also detect when the arrow keys or Enter were pressed. For command history, a couple different data structures were needed: first was a simple deque, that would store past entered commands; second was an unordered_map that, when history commands were edited, they could be restored upon the next Enter key press. Thus, the user could use up-down arrows to reuse previously entered commands, and the behavior of those commands stored closely resembles that of zsh/bash.

I implemented output redirection

Output redirection with '>'

background execution using '&' token at end of command; had to handle SIGCHLD signals, mask signals as needed to ensure output was accurate, etc. for commands related to background processes, the terminal output on start and finish of jobs was similar to that of zsh. required reciving SIGCHLD signals and handling them accordingly
