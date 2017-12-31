#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

extern int errno;

int main(int argc, char * argv[]) {
	if(argc < 3) {
		printf("%s", "Format: ./my_head <int> <file>");
		return 0;
	}

	int fd = open(argv[2], O_RDONLY|O_CREAT);

	int size = atoi(argv[1]);
	char buf[size + 1];
	buf[size] = '\0';

	int w = read(fd, buf, size);
	write(STDOUT_FILENO, buf, w);
    close(fd);
}
