#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
	init_counters();
	int ret;
	ret = fork();
	int i;
	for(i=0; i<10000; i++)
	{
		set_var(0, get_var(0)+1);  // this line has two system calls
	}
	if(ret == 0)
	{
		exit();
	}
	else
	{
		wait();
		int val;
		val = get_var(0);         // this line has one system call
		printf(1, "%d", val);
		exit();
	}
}
		
