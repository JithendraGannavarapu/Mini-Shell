#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>    
#include <sys/wait.h>
#include <fcntl.h>


static void print_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) printf("mini-shell:%s $ ", cwd);
    else printf("$ ");
    fflush(stdout);
}

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
int redirection_handling(char *argv[], int argc,char **file_in, char **file_out){
    int result=0;
    *file_in=NULL;
    *file_out=NULL;
    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"<")==0){
            if(i+1>=argc){
                printf("ERROR::file name is not there after '<' ");
                return -1;
            }
            *file_in = argv[i+1];
            i=i+1;
        }
       else if(strcmp(argv[i],">")==0){
            if(i+1>=argc){
                printf("ERROR::file name is not there after '>' ");
                return -1;
            }
            *file_out = argv[i+1];
            i=i+1;
        }
        else{
            argv[result]=argv[i];
            result=result+1;
        }
    }
    argv[result]=NULL;
    return result;
} 
int main() {
    char *line = NULL;   
    size_t len = 0;     
    char *argv[50];
    char *file_in=NULL;
    char *file_out=NULL;
    while (1) {
        print_prompt();
 

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
        argc = redirection_handling(argv,argc,&file_in,&file_out);
        if(argc<=0){
            printf("Enter a valid command");
            continue;
        }
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

            if (file_out != NULL) {
    		int fd = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    		if (fd < 0) { perror("open"); exit(1); }
    		dup2(fd, 1);   
    		close(fd);
		}

	     if (file_in != NULL) {
    		int fd = open(file_in, O_RDONLY);
    		if (fd < 0) { perror("open"); exit(1); }
    		dup2(fd, 0);   
    		close(fd);
		}
            execvp(argv[0],argv);
            perror("execvp");
        }
        else{
            int status;
            if(waitpid(val,&status,0)<0){
                perror("waitpid");
            }
        }
        // printf("your command is : %s\n", line);
    }

    free(line);
    return 0;
}
