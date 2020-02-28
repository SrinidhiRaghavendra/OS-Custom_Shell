# OS-Custom_Shell
//This project is initially targetted for CS550 assignment Binghamton University Spring 2020
//Initial Goal:

A C program that: 
1. Writes a prompt to standard output of the form mysh>
2. Reads a command from standard input, terminated by a newline (enter on the keyboard)
3. Execute the command it just read.

Comiple and Run instructions:

Compile:
Go to the source directory containing the project and run 'make' command. This compiles the source code in ./src directory and creates an executable mysh in ./build directory.

Run:
Run './build/mysh' from the source directory.

Notes:
1. For all redirections and pipes, make sure to add white space before and after '<' or '>' or '|' symbols.
2. Does not handle * in the command line.
3. No command completion or tab controlled suggestions supported.
