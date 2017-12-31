#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

pid_t main_pid;

void get_max(int A[], int L, int R) {
	if(R-L+1 < 10) {
		int max = A[R];
		for(int i = L; i < R; i++) {
			if(A[i] > max)
				max = A[i];
		}

		pid_t pid = getpid(), ppid = getppid();

		printf("%d %d %d %d %d\n", pid, ppid, L, R, max);
		if(pid == main_pid)
			printf("%d\n", max);
		exit(max);
	}
	else {
		pid_t childpid1 = fork();

		if(childpid1) {
			int status1, status2;
			
			pid_t childpid2 = fork();

			if(childpid2) {
				waitpid(childpid1, &status1, 0);
				waitpid(childpid2, &status2, 0);
				int m1 = WEXITSTATUS(status1);
				int m2 = WEXITSTATUS(status2);
				int max = m1 > m2 ? m1 : m2;

				printf("%d %d %d %d %d\n", getpid(), getppid(), L, R, max);

				if(getpid() == main_pid)
					printf("%d\n", max);

				exit(max);
			}
			else {
				get_max(A, (L+R)/2+1, R);
			}

		}
		else
		{
			get_max(A, L, (L+R)/2);
		}
		
	}
}


int main() {
	main_pid = getpid();

	int num;
	if(!scanf("%d", &num)) {
		printf("Could not read number of elements in array\n");
	}

	int *array = malloc(num * sizeof(int));

	for(int i = 0; i < num; i++) {
		int var;
		if(!scanf("%d", &var)) {
			printf("Could not read input number\n");	;
		}
		array[i] = var;
	}

	get_max(array, 0, num - 1);
}
