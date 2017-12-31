#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "emufs.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/*
	----------------DEVICE------------------
*/
int EMULATED_DISK_FD=-1;

int opendevice(char *device_name, int size)
{

	/*
		* It opens the the emulated deive.
		* If emulated device file is not exit
			* Create emulated disk file.
			* Creates superblocks and writes to disk.
			* file system type on newly created superblock will be null or '0'
		* It returns 2, if a file sytem exits on the deivce. Other wise return 1.
		* In case of any error, it returns -1.
	*/

	if(size > MAX_BLOCKS || size<2)
	{
		printf("Invalid Disk Size\n");
		return -1;
	}

	FILE *fp;
	int fd;
	fp = fopen(device_name, "r");
	struct superblock_t *superblock;

	if(!fp)
	{
		//	Creating the device
		printf("Creating disk. \n");

		superblock = (struct superblock_t*)malloc(sizeof(struct superblock_t));
		strcpy(superblock->name,device_name);
		strcpy(superblock->fstype,"0");
		superblock->disk_size=size;

		fp = fopen(device_name, "w+");
		if(!fp)
		{
			printf("Error : Unable to create device. \n");
			free(superblock);
			return -1;
		}
		fd = fileno(fp);

		// Make size of the disk as the total size
        fseek(fp, size*BLOCKSIZE, SEEK_SET);
        fputc('\0', fp);

		fseek(fp, 0, SEEK_SET);
		char tempBuf[BLOCKSIZE];
		memcpy(tempBuf, superblock, sizeof(struct superblock_t));
		write(fd, tempBuf, BLOCKSIZE);

		printf("New disk created\n");
		EMULATED_DISK_FD = fd;	// Set the current device as the file descriptor of the disk file
		free(superblock);
		return 1;		// File system does not exist.

	}
	else
	{
		fclose(fp);
		fd = open(device_name, O_RDWR);
		printf("Disk exists. \n");
		EMULATED_DISK_FD = fd;

		char tempBuf[BLOCKSIZE];
		superblock = (struct superblock_t*)malloc(sizeof(struct superblock_t));

		read(EMULATED_DISK_FD, tempBuf,BLOCKSIZE);
		memcpy(superblock,tempBuf,sizeof(struct superblock_t));
		if(strcmp(superblock->fstype,"0")==0)
		{
			// File system does not exist.
			printf("File system not found on this disk \n");
			free(superblock);
			return 1;
		}
		else
		{
			printf("File system on the disk : %s \n",superblock->fstype);
			free(superblock);
			return 2;
		}
	}


}

int writedevice(int block, char * buf)
{

	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	int offset=block * BLOCKSIZE;
	lseek(EMULATED_DISK_FD, offset, SEEK_SET);
	int ret=write(EMULATED_DISK_FD, buf, BLOCKSIZE);
	if(ret!=BLOCKSIZE)
	{
		printf("Error: Disk write error. \n");
		return -1;
	}
	return 1;
}

int readdevice(int block, char * buf)
{
	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	int offset=block * BLOCKSIZE;
	lseek(EMULATED_DISK_FD, offset, SEEK_SET);
	int ret=read(EMULATED_DISK_FD, buf, BLOCKSIZE);
	if(ret!=BLOCKSIZE)
	{
		printf("Error: Disk read error. \n");
		return -1;
	}
	return 1;
}

int closedevice()
{
	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	close(EMULATED_DISK_FD);
	EMULATED_DISK_FD = -1;

	return 1;
}

struct metadata_t* readMetadata();

void fsdump()
{
	metadata_t* meta = readMetadata();

	printf("NAME \tSIZE \t[BLOCKS] \tLAST MODIFIED\n");

	for(int i = 0; i < MAX_FILES; i++) {
		inode_t* inode = &meta->inodes[i];
		if(inode->status == USED) {
			printf("%s \t%d \t[%d %d %d %d] \t%s", inode->name, inode->file_size, inode->blocks[0], inode->blocks[1], inode->blocks[2], inode->blocks[3], asctime(localtime(&inode->modtime)));
		}
	}
	printf("\n");
}


/*
	----------------HELPER FUNCTIONS------------------
*/

struct superblock_t* readSuperblock()
{
	char* buf = calloc(BLOCKSIZE, sizeof(char));
	if(readdevice(0, buf) == -1)
		printf("ERROR: Could not read superblock\n");

	superblock_t* super = malloc(sizeof(superblock_t));
	memcpy(super, buf, sizeof(superblock_t));

	return super;
	/*
		* Read 0th block from the device into a blocksize buffer
		* Create superblock_t variable and fill it using reader buffer
		* Return the superblock_t variable
	*/
}

int writeSuperblock(struct superblock_t *superblock)
{
	char* buf = calloc(BLOCKSIZE, sizeof(char));
	memcpy(buf, superblock, sizeof(superblock_t));
	if(writedevice(0, buf) == -1) {
		printf("ERROR: Could not write superblock\n");
		return -1;
	}
	return 0;
	/*
		* Read the 0th block from device into a buffer
		* Write the superblock into the buffer
		* Write back the buffer into block 0
	*/
}

struct metadata_t* readMetadata()
{
	char* buf = calloc(BLOCKSIZE, sizeof(char));
	if(readdevice(1, buf) == -1)
		printf("ERROR: Could not read metadata\n");

	metadata_t* meta = malloc(sizeof(metadata_t));
	memcpy(meta, buf, sizeof(metadata_t));

	return meta;
	// Same as readSuperBlock(), but it is stored on block 1
}

int writeMetadata(struct metadata_t *metadata)
{
	char* buf = calloc(BLOCKSIZE, sizeof(char));
	memcpy(buf, metadata, sizeof(metadata_t));
	if(writedevice(1, buf) == -1) {
		printf("ERROR: Could not write metadata\n");
		return -1;
	}
	return 0;
	// Same as writeSuperblock(), but it is stored on block 1
}

/*
	----------------FILE SYSTEM API------------------
*/

int create_file_system()
{
	struct superblock_t* super;
	super = readSuperblock();
	strcpy(super->fstype, "emufs");
	super->bitmap[0] = super->bitmap[1] = '1';
	for(int i = 2; i < MAX_BLOCKS; i++) {
		if(i <= super->disk_size)
			super->bitmap[i] = '0';
		else
			super->bitmap[i] = 'x';
	}

	if(writeSuperblock(super) == -1)
		return -1;
	printf("Superblock initialised\n");

	metadata_t* meta = calloc(1, sizeof(metadata_t));

	if(writeMetadata(meta) == -1)
		return -1;
	printf("Metadata initialised\n");
	printf("File system created.\n");
	return 0;
	/*
	   	* Read the superblock.
	    * Set file system type on superblock as 'emufs'
		* Clear the bitmaps.  values on the bitmap will be either '0', or '1', or'x'.
		* Create metadata block in disk
		* Write superblocka nd metadata block back to disk.
	*/
}

struct file_t* eopen(char * filename)
{
	metadata_t* meta = readMetadata();
	int inode_num = -1;
	for(int i = 0; i < MAX_FILES; i++){
		if(!strcmp(filename, meta->inodes[i].name) && meta->inodes[i].status == USED) {
			inode_num = i;
			break;
		}
	}

	file_t* file = calloc(1, sizeof(file_t));

	if(inode_num == -1) {
		for(int i = 0; i < MAX_FILES; i++) {
			if(meta->inodes[i].status == UNUSED) {
				inode_num = i;
				break;
			}
		}

		if(inode_num == -1) {
			printf("ERROR: No space on disk! Can't create new file\n");
			return NULL;
		}

		inode_t* inode = &meta->inodes[inode_num];
		inode->status = USED;
		strcpy(inode->name, filename);
		inode->file_size = 0;
		for(int i = 0; i < 4; i++) {
			inode->blocks[i] = -1;
		}
		time(&inode->modtime);
	}

	file->offset = 0;
	file->inode_number = inode_num;
	writeMetadata(meta);
	printf("File Opened\n");
	return file;
	/*
		* If file exist, get the inode number. inode number is the index of inode in the metadata.
		* If file does not exist,
			* find free inode.
			* allocate the free inode as USED
			* if free id not found, print the error and return -1
		* Create the file hander (struct file_t)
		* Initialize offsetin the file hander
		* Return file handler.
	*/
}

int ewrite(struct file_t* file, char* data, int size)
{
	metadata_t* meta = readMetadata();
	superblock_t* super = readSuperblock();
	inode_t* inode = &meta->inodes[file->inode_number];

	int starting_block_no = file->offset/BLOCKSIZE;
	int num_blocks_to_write = size/BLOCKSIZE;

	if(starting_block_no + num_blocks_to_write > 4) {
		printf("ERROR: Maximum File Size Exceeded\n");
		return -1;
	}

	for(int i = starting_block_no; i < starting_block_no + num_blocks_to_write; i++) {
		int block_no = inode->blocks[i];

		if(block_no == -1) {
			for(int j = 2; j < super->disk_size; j++) {
				if(super->bitmap[j] == '0') {
					super->bitmap[j] = '1';
					inode->blocks[i] = j;
					block_no = j;
					break;
				}
			}
		}

		if(block_no == -1) {
			printf("ERROR: Can't write to file. No space on disk.\n");
			return -1;
		}

		if(writedevice(block_no, &data[(i - starting_block_no)*BLOCKSIZE]) == -1) {
			printf("ERROR: Can't write to device. Filename: %s\n", inode->name);
			return -1;
		}
	}

	inode->file_size = file->offset + size;
	eseek(file, file->offset + size);
	time(&inode->modtime);
	writeMetadata(meta);
	writeSuperblock(super);

	return size;

	// You do not need to implement partial writes in case file exceeds 4 blocks
	// or no free block is available in the disk.
}

int eread(struct file_t* file, char* data, int size)
{
	char* buf = calloc(size, sizeof(char));
	metadata_t* meta = readMetadata();
	inode_t* inode = &meta->inodes[file->inode_number];
	// if(size + file->offset > inode.file_size) {
	// 	printf("Can't read")
	// }
	int starting_block_no = file->offset/BLOCKSIZE;
	int num_blocks_to_read = size/BLOCKSIZE;

	if(inode->blocks[starting_block_no + num_blocks_to_read - 1] == -1) {
		printf("ERROR: Can't read from file. File too small\n");
			return -1;
	}

	for(int i = starting_block_no; i < starting_block_no + num_blocks_to_read; i++) {
		int block_no = inode->blocks[i];
		if(readdevice(block_no, &buf[(i - starting_block_no)*BLOCKSIZE]) == -1) {
			printf("ERROR: Cannot read from device. Filename: %s\n", inode->name);
			return -1;
		}
	}

	eseek(file, file->offset + size);
	strcpy(data, buf);
	writeMetadata(meta);
	return size;
	// NO partial READS.
}

void eclose(struct file_t* file)
{
	free(file);
	// free the memory allocated for the file handler 'file'
}

int eseek(struct file_t *file, int offset)
{
	if(!file)
		return -1;

	metadata_t* meta = readMetadata();
	inode_t* inode = &meta->inodes[file->inode_number];
	if(offset < 0)
		return -1;
	file->offset = offset;
	if(writeMetadata(meta) == -1)
		return -1;
	return 0;
	// Chnage the offset in file hanlder 'file'
}

int etruncate(struct file_t *file, int n)
{
	if(!file)
		return -1;

	metadata_t* meta = readMetadata();
	superblock_t* super = readSuperblock();
	inode_t* inode = &meta->inodes[file->inode_number];
	int file_blocksize = inode->file_size/BLOCKSIZE;
	int truncate = MIN(file_blocksize, n);

	int i;
	for(i = 0; i < truncate; i++) {
		int blockno = inode->blocks[file_blocksize - 1 - i];
		super->bitmap[blockno] = '0';
		inode->blocks[file_blocksize - 1 - i] = -1;
	}

	eseek(file, file->offset - i*BLOCKSIZE);
	time(&inode->modtime);
	writeMetadata(meta);
	writeSuperblock(super);

	return i;
	/*
		* Truncate last n blocks allocated a file referred by the file hanlder 'file'
		* If n is greater than the blocks allocated, remove all blocks.
		* It should return the actual number of blocks truncated.

		* Read the metadata & superblock from disk.
		* Remove the last n blocks allocated in inode of a file
		* Add the those truncated blocks into free list(Update bitmap)
		* Write metadata and superblock back to disk
	*/
}

