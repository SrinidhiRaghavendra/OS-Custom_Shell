#include "execute.h"
#include <sys/stat.h>
#include <fcntl.h>
//getCmdLine is copied from the cmdLine.c file given in the assignment.

int hasMultipleCommands = 0;
int hasInputRedirection = 0;
int hasOutputRedirection = 0;

char** getCmdLine(char* cmd) {
	// getline will reallocate cmdbuf to be large enough to fit the next line from stdin
    int numTokens=32;
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
	while( (this_token= strsep(&cmd, " \t\v\f\n\r")) != NULL) {
		if (*this_token=='\0') continue;
        if(*this_token == '|') hasMultipleCommands = 1;
        if(*this_token == '<') hasInputRedirection = 1;
        if(*this_token == '>') hasOutputRedirection = 1;
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

int executeMultipleCommands(char *cmd, char** tokens) {
    // Tokenize based on pipe command
    return 0;
}

int executeWithoutPipes(char *cmd) {
    char *inputDelim = "<";
    char *outputDelim = ">";
    char *copy = (char *)malloc(strlen(cmd));
    strcpy(copy, cmd);
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
        back = inputFile + strlen(inputFile) - 1;
        *back = '\0';
        fdInputFile = open(inputFile, O_RDONLY);
    }
    if(hasOutputRedirection) {
        if(outputFile[0] == ' ') {
            outputFile += 1;
        }
        back = outputFile + strlen(outputFile) - 1;
        *back = '\0';
        fdOutputFile = open(outputFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    }
    localTokens = getCmdLine(actualCmd);
    if(strcmp(localTokens[0], "cd") == 0) {
        chdir(localTokens[1]);
        return 0;
    }
    pid_t pid = fork();
    if(pid == 0) {
        //exec tthe program
        if(hasInputRedirection) {
            close(STDIN_FILENO);
            dup2(fdInputFile, STDIN_FILENO);
            close(fdInputFile);
        }
        if(hasOutputRedirection) {
            close(STDOUT_FILENO);
            dup2(fdOutputFile, STDOUT_FILENO);
            close(fdOutputFile);
        }
        int status = execvp(localTokens[0], localTokens);
        if (status == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        int ret = waitpid(pid, &status,  0);
        if (ret < 0) {
            perror("waitpid failed:");
            exit(2); 
        }
    }
    free(copy);
    return 0;
}

int executeCommand(char* cmd) {
    hasInputRedirection = hasOutputRedirection = 0;
    hasMultipleCommands = (strchr(cmd, '|') != NULL);
    if(hasMultipleCommands)  {
        char *copy = (char *)malloc(strlen(cmd));
        strcpy(copy, cmd);
        char** tokens=getCmdLine(copy);
        executeMultipleCommands(copy, tokens);
        free(copy);
    }
    else {
        executeWithoutPipes(cmd);
    }
    return 0;
}
