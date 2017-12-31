#include "types.h"
#include "stat.h"
#include "user.h"

#define ARRAY_SIZE 7
char buf[8192];

int
main(int argc, char *argv[])
{	
	uint va[ARRAY_SIZE]={0,0X1000,0X2000,0X100000,0x80000000,0XFFFFFFFF,0xE000000};
	uint pa;
	int flags;
	int i;
	
	for(i=0; i<ARRAY_SIZE; i++)
	{
		if(get_va_to_pa(va[i],&pa,&flags))
			printf(1,"VA: %x   PA: %x\n",va[i],pa);
		else
			printf(1,"VA: %x   PA: -\n",va[i]);
	}
	
	exit();
}