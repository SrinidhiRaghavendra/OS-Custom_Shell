#include<string.h>
#include "execute.h"

int main() {
    setbuf(stdout,0);
    int stop = 0;
    char* cmd; 
    while(!stop) {
        printf("mysh>");
        
        if(scanf("%s",cmd) == EOF || strcmp("exit", cmd) == 0) {
            printf("Exiting mysh. Bye!!!");
            stop = 1;
            break;
        } else {
            char *cmdbuf=NULL;
            size_t cmdbufsize=0;
            getline(&cmdbuf,&cmdbufsize,stdin);
            strcat(cmd, " ");
            strcat(cmd, cmdbuf);
            executeCommand(cmd);
        }
    }
    return 0;
}