#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
	init_counters();  // init data and locks
	int ret;
	ret = fork();
	int i;
	for(i=0; i<10000; i++)
	{
// worry about locking here
		acquire_lock(0);
		set_var(0, get_var(0)+1);
		release_lock(0);
	}
	if(ret == 0)
	{
		exit();
	}
	else
	{
		wait();
		int val;
		val = get_var(0);
		printf(1, "%d", val);
		exit();
	}
}
		
