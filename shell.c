#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>    
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_ARGS 64
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
void run_pipeline(char *argv[], int argc, int pipe_pos) {
    char *argv_cmd1[MAX_ARGS];
    char *argv_cmd2[MAX_ARGS];
    int i;
    int cmd1 = 0;
    for (i = 0; i < pipe_pos; i++) {
        argv_cmd1[cmd1++] = argv[i];
    }
    argv_cmd1[cmd1] = NULL;
    int cmd2 = 0;
    for (i = pipe_pos + 1; i < argc; i++) {
        argv_cmd2[cmd2++] = argv[i];
    }
    argv_cmd2[cmd2] = NULL;
    if (cmd1 == 0 || cmd2 == 0) {
        printf("Syntax error: something should present in left or right for pipeline\n");
        return;
    }
    char *file_in_cmd1 = NULL;
    char *file_out_cmd1 = NULL;
    char *file_in_cmd2 = NULL;
    char *file_out_cmd2 = NULL;
    cmd1 = redirection_handling(argv_cmd1,cmd1,&file_in_cmd1,&file_out_cmd1);
    cmd2 = redirection_handling(argv_cmd2,cmd2,&file_in_cmd2,&file_out_cmd2);
    if(cmd1 < 0 || cmd2 < 0){
        return;
    }
    if(file_out_cmd1!= NULL || file_in_cmd2 != NULL){
        printf("Invalid command");
        return;
    }
    if (cmd1 == 0 || cmd2 == 0) {
        printf("Syntax error: something should present in left or right for redirections\n");
        return;
    }

    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    if (pid1 == 0) {
        if(file_in_cmd1 != NULL){
            int fd_in = open(file_in_cmd1, O_RDONLY);
            if (fd_in < 0) {
                perror("open (cmd1 input)");
                exit(1);
            }
            if (dup2(fd_in, 0) < 0) {
                perror("dup2 (cmd1 input)");
                close(fd_in);
                exit(1);
            }
            close(fd_in);
        }
            if (dup2(fd[1], 1) < 0) {
                perror("dup2 cmd1");
                exit(EXIT_FAILURE);
            }
            close(fd[0]); 
            close(fd[1]); 

            execvp(argv_cmd1[0], argv_cmd1);
            perror("execvp cmd1");
            exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return;
    }

    if (pid2 == 0) {
        if (dup2(fd[0], 0) < 0) {
            perror("dup2 cmd2");
            exit(EXIT_FAILURE);
        }
        close(fd[1]); 
        close(fd[0]); 
         if(file_out_cmd2 != NULL){
            int fd_out = open(file_out_cmd2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open (cmd2 output)");
                exit(1);
            }
            if (dup2(fd_out, 1) < 0) {
                perror("dup2 (cmd2 output)");
                close(fd_out);
                exit(1);
            }
            close(fd_out);
        }
        execvp(argv_cmd2[0], argv_cmd2);
        perror("execvp cmd2");
        exit(EXIT_FAILURE);
    }

    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
}
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) printf("mini-shell:%s $ ", cwd);
    else printf("$ ");
    fflush(stdout);
}
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int check_background(char *argv[], int argc) {
    if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
        argv[argc - 1] = NULL;
        return 1;
    }
    return 0;
}

int main() {
    char *line = NULL;   
    size_t len = 0;     
    char *argv[50];
    char *file_in=NULL;
    char *file_out=NULL;
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, SIG_IGN);
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
        if(argc<=0){
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
        int pipeline_position=-1;
        for(int i=0;i<argc;i++){
            if(strcmp(argv[i],"|")==0){
                pipeline_position=i;
                break;
            }
        }
        if(pipeline_position!=-1){
            run_pipeline(argv,argc,pipeline_position);
            continue;
        }

        argc = redirection_handling(argv,argc,&file_in,&file_out);
        int background = check_background(argv, argc);
        if(argc<=0){
            continue;
        }
        pid_t val = fork();
        if(val<0){
            perror("fork");
            continue;
        }
        else if(val==0){
            signal(SIGINT, SIG_DFL);
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
            if(background){
                printf("[bg] started pid %d\n", val);
            }
            else{
                int status;
                if (waitpid(val, &status, 0) < 0) {
                    perror("waitpid");
                }
            }
        }
    }

    free(line);
    return 0;
}
