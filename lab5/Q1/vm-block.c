#include "types.h"
#include "stat.h"
#include "user.h"

#define KERNBASE 0x80000000
#define PGSIZE 4096
#define PTE_W 0x002 // Writeable

void print_vm_block()
{
	int start = 0;
	int end = 0;
	int start_flag = 0;
	int flag;
	int prev_flag = 0;
	uint pa;
	for(uint i = 0; i < KERNBASE + (KERNBASE - 1); i += PGSIZE) {
		int ret = get_va_to_pa(i, &pa, &flag);
		flag = flag & PTE_W;
		if(ret && !start_flag ) {
			start = i;
			start_flag = 1;
		}
		else if((!ret && start_flag) || i == 0xFFFFE000 || (start_flag && flag != prev_flag))
		{
			end = i;
			start_flag = 0;

			if(prev_flag)
				printf(1, "[%x %x] rw %d bytes\n", start, end - 1, end - start);
			else
				printf(1, "[%x %x] r %d bytes\n", start, end - 1, end - start);

			if(ret)
			{
				start = i;
				start_flag = 1;
			}
		}

		prev_flag = flag;

		if(i == 0xFFFFE000)
			break;
	}

	printf(1, "\n");
}

int
main(int argc, char *argv[])
{
	char *buf1, *buf2, *buf3;

	print_vm_block();

	buf1=sbrk(4096);
	buf1[0]='\0';
	print_vm_block();

	buf2=sbrk(4096*3);
	buf2[0]='\0';
	print_vm_block();

	buf3=sbrk(4096*11);
	buf3[0]='\0';
	print_vm_block();

	exit();
}
