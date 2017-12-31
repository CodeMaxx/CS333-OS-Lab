#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// There is some problem with `exit` command. If you're not able to exit

char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line); i++){

        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0){
                tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0; 
            }
        } 
        else {
            token[tokenIndex++] = readChar;
        }
    }
 
    free(token);
    tokens[tokenNo] = NULL ;
    return tokens;
}


void sigintHandler(int signum){
    ;
}


void exec_builtin(char ** tokens) {
    pid_t child_pid = fork();

        if(child_pid == -1) {
            printf("Could not fork.\n");
        }
        else {
            if(child_pid) {
                wait(NULL);
            }
            else {
                int ret = execvp(tokens[0], tokens);
                if(ret = -1)
                    printf("Could not find command: %s\n", tokens[0]);
                }
        }
        return;
}

void execute_one(char** tokens, int redirect, int k) {
    if(strcmp("exit", tokens[0]) == 0) {
            if(tokens[1] != NULL) {
                printf("Bad Format\n");
            }
            else
                exit(0);
        }

    if(tokens[0] == NULL)
            return;

        if(strcmp(tokens[0], "cd") == 0) {
            if(tokens[1] == NULL)
            {
                printf("Wrong number of arguments\n");
                return;
            }
            else if(tokens[2] != NULL) {
                printf("Wrong number of arguments\n");
                return;
            }
            int ret = chdir(tokens[1]);
            if(ret == -1)
                printf("Error: Directory not found\n");
            return;
        }

        if (redirect) {
            if(tokens[k+1] == NULL)
            {
                printf("Wrong number of arguments\n");
                return;
            }
            else if(tokens[k+2] != NULL) {
                printf("Wrong number of arguments\n");
                return;
            }
            int stdoutback = dup(1);
            int fd = open(tokens[k+1], O_WRONLY|O_CREAT, 00777);
            dup2(fd, 1);
            free(tokens[k]);
            free(tokens[k+1]);
            tokens[k] = NULL;
            exec_builtin(tokens);
            dup2(stdoutback, 1);
        }
        else {
            exec_builtin(tokens);
        }
}


int main() {
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;

    signal(SIGINT, sigintHandler);

    while (1) {           
       
        printf("moshmaxx>");     
        bzero(line, MAX_INPUT_SIZE);
        gets(line);           
       ;// printf("Got command %s\n", line);
        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);

        for(i=0;tokens[i]!=NULL;i++){
          ;//  printf("found token %s\n", tokens[i]);
        }
   
        //do whatever you want with the commands, here we just print them
        int redirect = 0, k = 0, sequential = 0, f = 0;
        int seqlist[64];
        char **tokens_copy = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
        char *token_copy = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));

        if(tokens[0] == NULL) {
            printf("\n");
            continue;
        }

        for(i = 0; i < 64; i++) {
            seqlist[i] = -1;
        }

        for(i=0;tokens[i]!=NULL;i++){
            if(strcmp(tokens[i], ">")== 0) {
                k = i;
                redirect = 1;
            }

            if(strcmp(tokens[i], ";;") == 0) {
                seqlist[f] = i;
                f++;
                sequential = 1;
            }
        }

        if(sequential == 1) {
            i = 0;
            for(int j = 0; j <= f; j++) {
                int tno = 0;
                int p = 0;
                if(j != f)
                    p = seqlist[j];
                else
                {
                    for(;tokens[p]!=NULL;p++){
                        ;
                    }
                }
                for(; i < p; i++) {
                    tokens_copy[tno] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                    strcpy(tokens_copy[tno], tokens[i]);
                    if(strcmp(tokens_copy[tno], ">")== 0) {
                        k = tno;
                        redirect = 1;
                    }
                    tno++;
                }
                tokens_copy[tno] = NULL;
                execute_one(tokens_copy, redirect, k);
                i++;
                redirect = 0;
                for(int b=0;tokens_copy[b]!=NULL;b++){
                    ;// printf("found token %s\n", tokens_copy[b]);
                }
                // for(int b=0;tokens_copy[b]!=NULL;b++){
                //     free(tokens_copy[b]);
                // }
            }
        }
        else
            execute_one(tokens, redirect, k);

        
        // Freeing the allocated memory 
        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
}