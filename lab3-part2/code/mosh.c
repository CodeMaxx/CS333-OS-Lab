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


pid_t current_process, // pid of current foreground process
        bg_pgid; // process group id for background processes
int ctrlc = 0; // Check if Ctrl+C has been pressed. If this is 1, next commands in the sequence are not executed


// Given function - Tokenizes the command input by user
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


// Handles SIGINT for parent
void sigintHandler(int signum){
    ctrlc = 1;
}


// Handles SIGCHLD for parent
void sigchldHandler(int signum) {
    int status;
    pid_t back;
    // If a background process terminates, print Background process completed
    while ((back = waitpid(-1, &status, WNOHANG)) > 0) {
        if(back != current_process)
            write(STDOUT_FILENO, "Background process completed\n", sizeof "Background process completed\n" -1);
    }
}


// Forks the process. Executes commands.
void exec_builtin(char ** tokens, int parallel) {
    pid_t child_pid = fork();

        if(child_pid == -1) {
            printf("Could not fork.\n");
        }
        else {
            if(child_pid) {
                if(!parallel) { // Waits if foreground process.
                    current_process = child_pid;
                    wait(NULL);
                }
            }
            else {
                if(parallel)
                    setpgid(getpid(), bg_pgid); // Puts background processes in background process group
                int ret = execvp(tokens[0], tokens);
                if(ret == -1){
                    // printf("%d\n", parallel);
                    printf("Could not find command: %s\n", tokens[0]);
                    exit(0);
                }
                }
        }
        return;
}


void change_dir(char** tokens) { // Change directory command; checks for proper format as well.
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
    if(ret == -1){
        printf("Error: Directory not found\n");
        return;
    }

    // Print working directory after cd-ing

    // sample_mosh does this but not in README outputs so I'm keeping this commented out.

    // char cwd[1024];
    // if (getcwd(cwd, sizeof(cwd)) != NULL)
    //     printf("%s\n", cwd);
    // else
    //     printf("getcwd() error");
    // return;
}


// Execute single command present in tokens
void execute_one(char** tokens, int redirect, int k, int parallel) {
    if(tokens[0] == NULL)
            return;

    if(strcmp("exit", tokens[0]) == 0) {
        if(tokens[1] != NULL) { // Check format for exit
            printf("Bad Format\n");
        }
        else {
            killpg(bg_pgid, SIGINT); // Kill background processes if "exit" command
            exit(0);
        }
    }

    if (redirect == 1) {
        // Check if redirection format is proper
        if(tokens[k+1] == NULL)
        {
            printf("Wrong number of arguments\n");
            return;
        }
        else if(tokens[k+2] != NULL) {
            printf("Wrong number of arguments\n");
            return;
        }
        int stdoutback = dup(1); // Backup for stdout
        int fd = open(tokens[k+1], O_WRONLY|O_CREAT, 00777); // File to write to
        dup2(fd, 1); // Change file descriptor of file to 1
        free(tokens[k]);
        free(tokens[k+1]);
        tokens[k] = NULL;
        exec_builtin(tokens, parallel);
        dup2(stdoutback, 1); // Restore STDOUT file descriptor
    }
    else {
        exec_builtin(tokens, parallel);
    }
}


int main() {
    char  line[MAX_INPUT_SIZE];
    char  **tokens;
    int i;

    pid_t bg_while = fork(); // Set up a background loop; leader of background process group

    if(bg_while == -1) {
            printf("Could not fork.\n");
        }
        else {
            if(!bg_while) {
                setpgid(getpid(), getpid());
                while(1) {;}
            }
            else {
                bg_pgid = bg_while;
            }
        }

    pid_t parent_pid = getpid();
    setpgid(parent_pid, parent_pid);

    // Override handlers for SIGINT and SIGCHLD
    signal(SIGINT, sigintHandler);
    signal(SIGCHLD, sigchldHandler);


    while (1) {
        current_process = parent_pid;
        printf("moshmaxx>");
        bzero(line, MAX_INPUT_SIZE);
        gets(line);

        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);

        int redirect = 0, // 1 when command involves redirecting to a file
            k = 0, // Index of `>` in the corresponding redirect command
            sequential = 0, // 1 when command involves sequential execution
            f = 0;  // Stores number of commands in sequence
        int seqlist[64]; // Stores which index have ;;

        int g = 0, // Stores number of commands in parallel
            parallel = 0; // 1 when command involves parallel execution
        int parlist[64]; // Stores which index have &&

        // If nothing is given as command, loop again
        if(tokens[0] == NULL) {
            printf("\n");
            continue;
        }

        // Initialising
        for(i = 0; i < 64; i++) {
            seqlist[i] = -1;
            parlist[i] = -1;
        }

        // Initialise variables for seqential or parallel running
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

            if(strcmp(tokens[i], "&&") == 0) {
                parlist[g] = i;
                g++;
                parallel = 1;
            }
        }

        // Command can't have sequential and parallel mixture
        if(sequential == 1 && parallel == 1) {
            printf("Bad Format\n");
            continue;
        }

        if(sequential == 1) {
            i = 0;
            for(int j = 0; j <= f; j++) {
                // tokens_copy has the individual commands to be executed
                char **tokens_copy = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
                int tno = 0; // Token number in tokens_copy
                int p = 0; // Stores (last + 1)th index of each command
                redirect = 0;
                // For last command set p to the end of the tokens array
                if(j != f)
                    p = seqlist[j];
                else
                {
                    for(;tokens[p]!=NULL;p++){
                        ;
                    }
                }

                // Put command in token_copy
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

                execute_one(tokens_copy, redirect, k, parallel);
                i++;

                // Freeing the allocated memory
                for(int b=0;tokens_copy[b]!=NULL;b++){
                    free(tokens_copy[b]);
                }
                free(tokens_copy);

                // Skip rest of the commands if Ctrl+C is pressed
                if(ctrlc) {
                    ctrlc = 0;
                    break;
                }
            }
        }
        else if(parallel == 1) {
            i = 0;
            for(int j = 0; j <= g; j++) {
                // tokens_copy has the individual commands to be executed
                char **tokens_copy = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
                int tno = 0; // Token number in tokens_copy
                int p = 0; // Stores (last + 1)th index of each command
                redirect = 0;
                // For last command set p to the end of the tokens array
                if(j != g)
                    p = parlist[j];
                else
                {
                    for(;tokens[p]!=NULL;p++){
                        ;
                    }
                }

                // Put command in token_copy
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

                execute_one(tokens_copy, redirect, k, parallel);
                i++;

                // Freeing the allocated memory
                for(int b=0;tokens_copy[b]!=NULL;b++){
                    free(tokens_copy[b]);
                }
                free(tokens_copy);
            }
        }
        else {
            if(strcmp("cd", tokens[0]) == 0)
                change_dir(tokens);
            else
                execute_one(tokens, redirect, k, parallel); // Normal command execution
        }


        // Freeing the allocated memory
        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
}