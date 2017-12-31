#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

extern int errno;

int main(int argc, char * argv[]) {
	if(argc < 4) {
		printf("%s\n", "Format: ./write_head <int> <input_file> <output_file>");
		return 0;
	}

	int fd = open(argv[2], O_RDONLY|O_CREAT);

	pid_t child_pid = fork();

	if(!child_pid) {
		close(STDOUT_FILENO);
		fd = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, 00777);
		if(fd == -1) {
			printf("Could not open %s. Error No: %i\n", argv[3], errno);
		}
		execv("./my_head", (char *[]){"./my_head", argv[1], argv[2], NULL});
	}
}