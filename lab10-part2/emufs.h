#include <time.h>

/*
	----------------DEVICE------------------
*/
#define BLOCKSIZE 512
#define MAX_BLOCKS 42 	// This is superblock(1) + metadata(1) + data(40)
#define MAX_FILE_SIZE 4 // In Blocks
#define MAX_FILES 10
#define UNUSED 0
#define USED 1
#define READY 2

typedef struct superblock_t
{
	char name[20];				// name of the device
	char fstype[10];			// Type of the file system
	                            // fstype = '0'     : File system not created.
	                            //        = 'emufs' : emufs exits on the disk
	int  disk_size;				// size of the device in blocks
	char bitmap[MAX_BLOCKS];	// Bitmap: 0 = free block,
								// 		   1 = allocated,
	                            //         x = block not exist(block number is more than disk_size)
} superblock_t;


/*
	----------------FILE SYSTEM------------------
*/
typedef struct inode_t
{
	int status;			// {USED or UNUSD}
	char name[10];		// name of the file
	int file_size;		// size of the file in bytes
	int blocks[4];		// array of allocated blocks
	                    // blocks[i] = -1 : Block is not allocated.
	                    // blocks[i] >0   : block number
	time_t modtime;		// last modified time
} inode_t;

typedef struct metadata_t
{
	struct inode_t inodes[MAX_FILES];
} metadata_t;

typedef struct file_t
{
	int offset;
	int inode_number;
} file_t;


/*-----------DEVICE------------*/
int opendevice(char *device_name, int size);
int closedevice();
void fsdump();


/*-----------FILE SYSTEM------------*/
int create_file_system();
struct file_t* eopen(char * filename);
void eclose(struct file_t* file);
int eread(struct file_t* file, char* data, int size);
int ewrite(struct file_t* file, char* data, int size);
int eseek(struct file_t *, int );
int etruncate(struct file_t*, int);



