#include<stdio.h>
#include<string.h>

int main() {
    int stop = 0;
    char* cmd; 
    while(!stop) {
        printf("mysh>");
        
        if(scanf("%s",cmd) == EOF || strcmp("exit", cmd) == 0) {
            printf("Exiting mysh. Bye!!!");
            stop = 1;
            break;
        }
        printf("your command: %s\n",cmd);
    }
    return 0;
}