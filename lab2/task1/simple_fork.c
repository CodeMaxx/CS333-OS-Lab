#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
	pid_t child_pid = fork();

	if(child_pid) {
		printf("Parent : The child's process ID is : %u\n", child_pid);
		wait(NULL);
		printf("%s\n", "Parent : The child has terminated.");
	}
	else {
		printf("Child : The child's process ID is : %u\n", getpid());
	}
}