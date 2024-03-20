# HShell
Command history via up/down arrows<br />
this required me to have a DS for the history, but also need to somehow store edited versions of history commands, and retain them all

Output redirection with '>'

Built-in handling for 'cd', 'pwd', 'exit', 'jobs'

Uses fork() and execvp() to run commands

why execvp?

background execution using '&' token at end of command; had to handle SIGCHLD signals, mask signals as needed to ensure output was accurate, etc. for commands related to background processes, the terminal output on start and finish of jobs was similar to that of zsh. required reciving SIGCHLD signals and handling them accordingly