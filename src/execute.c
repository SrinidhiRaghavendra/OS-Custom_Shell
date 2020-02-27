#include "execute.h"
#include <sys/stat.h>
#include <fcntl.h>
//getCmdLine is copied from the cmdLine.c file given in the assignment.

int hasMultipleCommands = 0;
int hasInputRedirection = 0;
int hasOutputRedirection = 0;
int numTokens = 0;

char** getCmdLine(char* cmd, char *seperator) {
	// getline will reallocate cmdbuf to be large enough to fit the next line from stdin
    numTokens=32;
    char** tokens=malloc(sizeof(char *) * numTokens);
	if (!cmd) {
		if (feof(stdin)) {
			tokens[0]="exit";
			tokens[1]=NULL;
			return tokens;
		}
		perror("getCmdLine invocation of getline: ");
		tokens[0]=NULL;
		return tokens;
	}
	// Break up cmd based on white space
	int n=0;
	char *this_token;
	while( (this_token= strsep(&cmd, seperator)) != NULL) {
		if (*this_token=='\0') continue;
		tokens[n]=this_token;
		n++;
		if(n>=numTokens) { // increase the size of the tokens
			numTokens *=2; // Double the size
			assert( (tokens = realloc(tokens,sizeof(char *) * numTokens)) != NULL);
		}
	}
	tokens[n]=NULL;
    numTokens = n;
	return tokens;
}

int executeWithoutPipes(char *cmd, int inputFd, int outputFd, int pipeCmdNumber) {
    char *inputDelim = "<";
    char *outputDelim = ">";
    char *copy = (char *)malloc(strlen(cmd));
    strcpy(copy, cmd);
    free(copy);
    if(strchr(copy, '<') != NULL) {
        hasInputRedirection = 1;
    } if(strchr(copy, '>') != NULL) {
        hasOutputRedirection = 1;
    }
    char *actualCmd, *inputFile, *outputFile, *back;
    char **localTokens;
    int fdInputFile = 0;
    int fdOutputFile = 0;
    if(hasInputRedirection && !hasOutputRedirection) {
        actualCmd = strtok(copy, inputDelim);
		inputFile = strtok(NULL, inputDelim);
    } else if(hasOutputRedirection && !hasInputRedirection) {
        actualCmd = strtok(copy, outputDelim);
		outputFile = strtok(NULL, outputDelim);
    }else if(hasOutputRedirection && hasInputRedirection){
        char *cmdPart1 = strtok(copy, outputDelim);
        char *cmdPart2 = strtok(NULL, outputDelim);
        if(strchr(cmdPart1, '<') != NULL) {
            //input redirection is here, split part 1 on "<" to get cmd and input file in thst order
            actualCmd = strtok(cmdPart1, "<");
            inputFile = strtok(NULL, "<");
            outputFile = cmdPart2;
        } else {
            //input redirection is here, split part 1 on ">" to get output file and input file in that order
            actualCmd = cmdPart1;
            outputFile = strtok(cmdPart2, "<");
            inputFile = strtok(NULL, "<");
        }
    } else {
        actualCmd = cmd;
    }
    if(hasInputRedirection) {
        if(inputFile[0] == ' ') {
            inputFile += 1;
        }
        if(inputFile[strlen(inputFile) -1] == ' ' || inputFile[strlen(inputFile) -1] == '\n') {
            back = inputFile + strlen(inputFile) - 1;
            *back = '\0';
        }
        fdInputFile = open(inputFile, O_RDONLY);
    }
    if(hasOutputRedirection) {
        if(outputFile[0] == ' ') {
            outputFile += 1;
        }
        if(outputFile[strlen(outputFile) -1] == ' ' || outputFile[strlen(outputFile) -1] == '\n') {
            back = outputFile + strlen(outputFile) - 1;
            *back = '\0';
        }
        fdOutputFile = open(outputFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | S_IREAD);
    }
    localTokens = getCmdLine(actualCmd, " \t\v\f\n\r");
    //pid_t pid = fork();
    int stdout = 0;
    //if(pid == 0) {
        //exec tthe program
        if(hasInputRedirection || inputFd != -1) {
            //close(STDIN_FILENO);
            if(inputFd != -1) {
                fdInputFile = inputFd;
            }
            dup2(fdInputFile, STDIN_FILENO);
            //close(fdInputFile);
            if(pipeCmdNumber > 0 && hasInputRedirection) {//second one onwards cannot havbe an input redirection
                perror("Has INPUT redirection in the middle of a piped command\n");
                exit(EXIT_FAILURE);
            }
        }
        if(hasOutputRedirection || outputFd != -1) {
            //close(STDOUT_FILENO);
            if(outputFd != -1) {
                fdOutputFile = outputFd;
            }
            dup2(fdOutputFile, STDOUT_FILENO);
            //close(fdOutputFile);
            if(pipeCmdNumber != -1 && hasOutputRedirection) {// All except last command cannot have an output redirection
                perror("Has output redirection in the middle of a piped command\n");
                exit(EXIT_FAILURE);
            }
        }
        int status = execvp(localTokens[0], localTokens);
        if (status == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    //}
    return stdout;
}

int executeMultipleCommands(char *cmd, int cmdNum, int inputFd) {
    char *copy = (char *)malloc(strlen(cmd));
    strcpy(copy, cmd);
    char **tokens = getCmdLine(copy, "|");
    free(copy);
    int localCopyNumTokens = numTokens;
    if(cmdNum >= localCopyNumTokens) {
        return 0;
    }
    int pfds[2];
    int status;
    pipe(pfds);
    pid_t pid = fork();
    if(pid == 0) {
        close(pfds[0]);
        executeWithoutPipes(tokens[cmdNum], (cmdNum == 0 ? -1 : inputFd), (cmdNum == (localCopyNumTokens - 1) ? -1 : pfds[1]), (cmdNum == (localCopyNumTokens-1) ? -1 : cmdNum));
    }
    else {
        close(pfds[1]);
        int status;
        int ret = waitpid(pid, &status,  0);
        if (ret < 0) {
            perror("waitpid failed:");
            exit(2); 
        }
        executeMultipleCommands(cmd, cmdNum + 1, pfds[0]);
        close(pfds[0]);
    }
    return 0;
}

int executeCommand(char* cmd) {
    hasInputRedirection = hasOutputRedirection = 0;
    hasMultipleCommands = (strchr(cmd, '|') != NULL);
    if(hasMultipleCommands)  {
        char *copy = (char *)malloc(strlen(cmd));
        strcpy(copy, cmd);
        executeMultipleCommands(copy, 0, -1);
        free(copy);
    }
    else {
        int status;
        pid_t pid = fork();
        if(pid == 0) {
            executeWithoutPipes(cmd, -1, -1, -1);
        } else {
            int status;
            int ret = waitpid(pid, &status,  0);
            if (ret < 0) {
                perror("waitpid failed:");
                exit(2); 
            }
        }
    }
    return 0;
}
