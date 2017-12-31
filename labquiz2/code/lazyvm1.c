#include "types.h"
#include "stat.h"
#include "user.h"

#define ARRAY_SIZE 2048
int buf[ARRAY_SIZE];

#define EXTRA_SPACE 8182

int
main(int argc, char *argv[])
{	
	int sz, vp, pp;
	buf[ARRAY_SIZE-1] = 1234;

	sz = getNumPages(&vp, &pp);
	printf(1, "sz: %d, vp: %d, pp: %d\n", sz, vp, pp);

	int * ptr = (int *)&buf[ARRAY_SIZE-1];
	printf(1, "addr: %d, value: %d\n", ((int)ptr), *ptr);

	sbrk(EXTRA_SPACE);

	sz = getNumPages(&vp, &pp);
	printf(1, "sz: %d, vp: %d, pp: %d\n", sz, vp, pp);

	ptr = (int *)20484;
	printf(1, "addr: %d, value: %d\n", ((int)ptr), *ptr);

	sz = getNumPages(&vp, &pp);
	printf(1, "sz: %d, vp: %d, pp: %d\n", sz, vp, pp);

	ptr = (int *)28658;
	printf(1, "addr: %d, value: %d\n", ((int)ptr), *ptr);

	sz = getNumPages(&vp, &pp);
	printf(1, "sz: %d, vp: %d, pp: %d\n", sz, vp, pp);

	ptr = (int *)28676;
	printf(1, "addr: %d\n", ((int)ptr));
	printf(1, "value: %d\n", *ptr);

	sz = getNumPages(&vp, &pp);
	printf(1, "sz: %d, vp: %d, pp: %d\n", sz, vp, pp);
	
	exit();
}