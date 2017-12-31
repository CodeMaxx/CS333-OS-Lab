#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void sigintHandler(int signum){
	printf("%s\n", "Do you really want to quit (y|n)?");
	char c;
    scanf("%s", &c);
	if(!strcmp(&c, "Y") || !strcmp(&c, "y"))
		exit(signum);
}

int main() {
	signal(SIGINT, sigintHandler);

	while(1) {
		;
	}
}
