#include "types.h"
#include "stat.h"
#include "user.h"

void delay(int count)
{
	int i;
	int j,k;
	int *data;

	data=(int *)malloc(sizeof(int)*1024*10);
	if(data<=0)
		printf(1,"Error on memory allocation \n");

	for(i=0;i<count;i++)
	{
		for(k=0;k<5700;k++)
			for(j=0;j<1024*10;j++)
				data[j]++;
	}

}
int
main(int argc, char *argv[])
{
	if(argc<2)
	{
		printf(1," test-case <number-of-children>");
		exit();
	}
	int N=atoi(argv[1]);

	int pids[N];
	int rets[N];
	setprio(1000);
	printf(1,"Priority of parent process = %d\n",getprio());

	for(int i=0;i<N;i++)
	{
		// * Set process priority
		// * Change priority of children in different order 
		//   and verify your implementations !!!

		int ret=fork2(i+1);
		if(ret==0)
		{
			delay(5);
			exit();
		}
		else if(ret>0)
		{
			pids[i]=ret;
		}
		else
		{
			printf(1,"fork error \n");
			exit();
		}
	}

	for(int i=0;i<N;i++)
	{
		rets[i]=wait();
	}

	printf(1,"\nAll children completed\n");
	for(int i=0;i<N;i++)
		printf(1,"Child %d.    pid %d\n", i,pids[i]);

	printf(1,"\nExit order \n");
	for(int i=0;i<N;i++)
		printf(1,"pid %d\n", rets[i]);

	exit();
}