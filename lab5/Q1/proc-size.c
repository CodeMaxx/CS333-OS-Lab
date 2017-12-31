#include "types.h"
#include "stat.h"
#include "user.h"

#define KERNBASE 0x80000000
#define PHYSTOP 0xE000000
#define PGSIZE 4096

int getpusz()
{
	int size = 0;
	int flag;
	uint pa;
	for(uint i = 0; i < KERNBASE; i += PGSIZE) {
		if(get_va_to_pa(i, &pa, &flag))
			size += PGSIZE;
	}

	return size;
}

int getpksz()
{
	int size = 0;
	int flag;
	uint pa;
	for(uint i = KERNBASE; i < KERNBASE + (KERNBASE - 1); i += PGSIZE) {
		if(get_va_to_pa(i, &pa, &flag))
			size += PGSIZE;
		if(i == 0xFFFFE000)
			break;
	}

	return size;
}

int getptsz()
{
	return getpksz() + getpusz();
}

int
main(int argc, char *argv[])
{
	char *buf;

	printf(1,"\ngetpusz: %d bytes \n",getpusz());
	printf(1,"getpksz: %d bytes\n",getpksz());
	printf(1,"getptsz: %d bytes\n",getptsz());


	buf=sbrk(4096);
	buf[0]='\0';
	printf(1,"\ngetpusz: %d bytes \n",getpusz());
	printf(1,"getpksz: %d bytes\n",getpksz());
	printf(1,"getptsz: %d bytes\n",getptsz());


	buf=sbrk(4096*7);
	printf(1,"\ngetpusz: %d bytes \n",getpusz());
	printf(1,"getpksz: %d bytes\n",getpksz());
	printf(1,"getptsz: %d bytes\n",getptsz());

	exit();
}