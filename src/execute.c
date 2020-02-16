#include "execute.h"
//getCmdLine is copied from the cmdLine.c file given in the assignment.
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
	while( (this_token= strsep(&cmd, " \t\v\f\n\r")) !=NULL) {
		if (*this_token=='\0') continue;
		tokens[n]=this_token;
		n++;
		if (n>=numTokens) { // increase the size of the tokens
			numTokens *=2; // Double the size
			assert( (tokens = realloc(tokens,sizeof(char *) * numTokens)) != NULL);
		}
	}
	tokens[n]=NULL;
	return tokens;
}

int executeCommand(char* cmd) {
    char** tokens=getCmdLine(cmd);
    pid_t pid = fork();
    if(pid == 0) {
        //exec tthe program
        int status = execvp(tokens[0], tokens);
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
    return 0;
}
