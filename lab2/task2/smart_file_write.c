#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

extern int errno;

int main() {
	int fd = open("output.txt", O_WRONLY|O_CREAT, 00777);

	int w = write(fd, "Hi I am the Parent\n", sizeof("Hi I am the Parent\n"));

	pid_t child_pid = fork();

	if(!child_pid) {
		w = write(fd, "Hi I am the Child\n", sizeof("Hi I am the Child\n"));
	}
}