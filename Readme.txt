
Dhruv Khattar
201402087

To run the shell:

1. gcc shell.c
2. ./a.out

Shell Features:

- The directory from which the shell is invoked will be the home directory of the shell represented by ~.
- Supports semi-colon seperated list of commands.
- 'pwd' , 'cd' and 'echo' are built in the shell itself (builtin commands)
- All other commands are executed by forking parent process.
- Piping various commands.
- Input and Output redirection.
- Background Processes.

- Commands Used : 
  - fork : To run a child process under the parent process.
  - execvp : Used to execute a command along with the arguments.
  - strtok : Used to tokenize the command.
  - wait : Used to wait for the child process to end.
  - perror : Used for various errors.
  - getcwd : Used for current directory.
  - getlogin_r : Used to get login name of the user.
  - gethostname : Used to get host name of the OS.
