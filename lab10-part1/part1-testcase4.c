
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "emufs.h"

#define EMULATED_DISK_SIZE 42

char smalltext[512] = "START-This is a small text consisting 50 bytes-END";
int main(int argc, char *argv[])
{
	int ret;
	char *readBuf;
	if(argc < 2)
	{
		printf("Error: <DISK-NAME> \n");
		exit(1);
	}

	printf("Disk name : %s \n",argv[1]);
	ret=opendevice(argv[1], EMULATED_DISK_SIZE);
	if(ret < 0)
	{
		// Error
		exit(1);
	}
	else if (ret == 1)
	{	
		// File system not found
		create_file_system();
	}
	else if (ret == 2)// File system not found
	{	
		// File system found
	}

	printf("\n");
	fsdump();


	struct file_t *f1 = eopen("file1");
	struct file_t *f2 = eopen("file2");

	ewrite(f1, smalltext, BLOCKSIZE);
	ewrite(f2, smalltext, BLOCKSIZE);
	
	fsdump();

	ewrite(f1, smalltext, BLOCKSIZE);
	fsdump();
	ewrite(f1, smalltext, BLOCKSIZE);

	fsdump();

	printf("Truncating 1 block from file2. current offset: %d\n",f2->offset);
	ret=etruncate(f2,1);
	printf("%d blocks truncated from file2, offset: %d\n\n",ret, f2->offset);

	fsdump();

	printf("Writing %d bytes to file1, offset: %d.\n",BLOCKSIZE,f1->offset); 
	ewrite(f1, smalltext, BLOCKSIZE);
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	fsdump();

	printf("Truncating all blocks from file1. current offset: %d\n",f1->offset);
	ret=etruncate(f1,10);
	printf("%d blocks truncated from file1, offset: %d\n\n",ret, f1->offset);

	printf("Truncating all blocks from file2. current offset: %d\n",f2->offset);
	ret=etruncate(f2,10);
	printf("%d blocks truncated from file2, offset: %d\n\n",ret, f2->offset);

	fsdump();

	if (closedevice()<0)
		exit(1);
	printf("Device Closed \n");

	return 0;
}
