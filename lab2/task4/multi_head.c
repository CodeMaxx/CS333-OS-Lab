#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

extern int errno;

int main(int argc, char * argv[]) {
	int i = 1;
	if (argc > 1){
		while(argv[i] != NULL) {
			pid_t child_pid = fork();

			if(child_pid) {
				wait(NULL);
			}
			else {
				execv("./my_head", (char *[]){"./my_head", "10", argv[i], NULL});
				break;
			}
			i++;
		}
	}
	else
	{
		printf("Format: ./multi_head <file> <file> ...");
	}
}