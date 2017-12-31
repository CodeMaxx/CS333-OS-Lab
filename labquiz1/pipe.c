#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h> 


#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

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


void exec_builtin(char** tokens) {
	if(!strcmp(tokens[0], "exit")) {
		exit(0);
	}

	pid_t childpid = fork();

	if(childpid) {
		wait(NULL);
	}
	else {
		int ret = execvp(tokens[0], tokens);

		if(ret == -1) {
			printf("Command not found\n");
			exit(1);
		}
		exit(0);
	}

}

void main(void)
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;

    while (1) {           
       
        printf("moshmaxx>");     
        bzero(line, MAX_INPUT_SIZE);
//        gets(line);           
	fgets(line, MAX_INPUT_SIZE, stdin);           
        // printf("Got command %s\n", line);
        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);
   
        //do whatever you want with the commands, here we just print them

        int pipes = 0, location = 0;

        if(tokens[0] == NULL)
        	continue;

        for(i=0;tokens[i]!=NULL;i++){
            if(!strcmp(tokens[i], "|")) {
            	pipes = 1;
            	location = i;
            	break;
            }
        }

        

        if(pipes) {
        	char **command1 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
        	char **command2 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
        	for(i=0; tokens[i]!=NULL; i++) {
        		if(i < location) {
        			command1[i] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                	strcpy(command1[i], tokens[i]);
                }
                else if (i > location) {
                	command2[i - location - 1] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                	strcpy(command2[i - location - 1], tokens[i]);
                }
        	}
        	// TODO

        	int pipefd[] = {3, 4};

        	if(pipe(pipefd) == -1) {
        		printf("Error in pipe\n");
        	}
        	

        	pid_t childpid1 = fork();
        	

			if(childpid1) {
				
				pid_t childpid2 = fork();

				if(childpid2) {
					// printf("The hell\n");
					close(pipefd[1]);
					waitpid(childpid2, NULL, 0);
					// printf("Out of hell\n");
				}
				else {
					int stdin_bak = dup(0);
					close(pipefd[1]);
					dup2(pipefd[0], 0);
					int ret = execvp(command2[0], command2);
					close(pipefd[0]);
					dup2(stdin_bak, 0);
					if(ret == -1) {
						printf("Command not found\n");
						exit(1);
					}
					exit(0);
				}
				waitpid(childpid1, NULL, 0);
				// printf("Woohooo\n");
			}
			else {
				int stdout_bak = dup(1);
				if(dup2(pipefd[1], 1) == -1)
					printf("Error\n");;

				close(pipefd[0]);
				int ret = execvp(command1[0], command1);
				close(pipefd[1]);
				dup2(stdout_bak, 1);
				if(ret == -1) {
					printf("Command not found\n");
					exit(1);
				}

				exit(0);

			}

        }
        else
        	exec_builtin(tokens);

        
       
        // Freeing the allocated memory	
        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
     

}
