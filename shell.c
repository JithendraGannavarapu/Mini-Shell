
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main() {
    char *line = NULL;   
    size_t len = 0;     

    while (1) {
        printf("mini-shell> Please type your text: ");
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

        printf("your text is : %s\n", line);
    }

    free(line);
    return 0;
}
