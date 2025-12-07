#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>    
#include <sys/wait.h>

 int generate_tokens(char *str, char *argv[]){
    int argc = 0;
    char *token = strtok(str, " ");

    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
 }
int main() {
    char *line = NULL;   
    size_t len = 0;     
    char *argv[50];
    while (1) {
        printf("mini-shell> Please type your command: ");
        fflush(stdout);  

        ssize_t nread = getline(&line, &len, stdin);
        if (nread == -1) {
            printf("\n");
            break;
        }

        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        if (line[0] == '\0') {
            continue;
        }
        int argc = generate_tokens(line,argv);
        
        if(argc==0)
         continue;
        if(strcmp(argv[0],"exit")==0)
         break;
        if(strcmp(argv[0],"cd")==0){
            char *next= argv[1];
            if(next==NULL){
                next=getenv("HOME");
            }
            else if(chdir(next)!=0)
                perror("cd");
            continue;   
        }
        pid_t val = fork();
        if(val<0){
            perror("fork");
            continue;
        }
        else if(val==0){
            execvp(argv[0],argv);
            perror("execvp");
        }
        else{
            int status;
            if(waitpid(val,&status,0)<0){
                perror("waitpid");
            }
        }
        printf("your command is : %s\n", line);
    }

    free(line);
    return 0;
}
