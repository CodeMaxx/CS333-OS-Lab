#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BLOCKSIZE 512
#define MAX_BLOCKS 64
#define MAX_FILES 8

typedef struct superblock_t {
	char name[16];
	int total_blocks;
	int block_size;
	int free_blocks;
	int num_files;
	char status[MAX_BLOCKS];
// 16 bytes: filesystem name
// 	 4 bytes: total blocks on device
//  4 bytes: block size
//  4 bytes: number of blocks free
//  4 bytes: number of files on disk
// 64 bytes: status of each block (used/unused)
//           0: unused, 1: used
} superblock_t;

typedef struct inode_t {
	int inode_number;
	char filename[12];
	int file_size;
	int start_block;
	int end_block;
	//  4 bytes: inode number
// 12 bytes: file name
//  4 bytes: file size
//  4 bytes: start block
//  4 bytes: end block
} inode_t;

typedef struct metadata_t
{
	int num_files;
	int start_block;
	int FAT[MAX_BLOCKS];
	inode_t inodes[MAX_FILES];

} metadata_t;

// format(ordered from byte 512):  
//   4 bytes: number of files on disk
//   4 bytes: starting block for data
// 256 bytes: FAT file-allocation table
//            each entry holds pointer to next block.
//            -1 is end-of-file

// 224 bytes: inodes[MAXFILES];

superblock_t* read_superblock(int fd) {
	superblock_t* super = calloc(1, sizeof(superblock_t));
	lseek(fd, 0, SEEK_SET);
	int r = read(fd, (void*)super, sizeof(superblock_t));
	if(r != sizeof(superblock_t)) {
		printf("Could only read %d bytes from superblock.\n", r);
	}

	return super;
}

metadata_t* read_metadata(int fd) {
	metadata_t* meta = calloc(1, sizeof(metadata_t));
	lseek(fd, BLOCKSIZE, SEEK_SET);
	int r = read(fd, (void*)meta, sizeof(metadata_t));
	if(r != sizeof(metadata_t)) {
		printf("Could only read %d bytes from metadata.\n", r);
	}

	return meta;
}

int main(int argc, char** argv) {
	if(argc < 3) {
		printf("Format: %s [disk-name] [options]\n", argv[0]);
	}

	int fd = open(argv[1], O_RDONLY);

	for(int i = 2; i < argc; i++) {

		if(!strcmp(argv[i], "-s")) {
			printf("Disk Read \n");
			superblock_t* super = read_superblock(fd);
			printf("PRINTING SUPERBLOCK INFO\n");
			printf("----------------------------------------------------------------\n");
			printf("Size of superblock_t : %ld \n", sizeof(superblock_t));
			printf("Size of inode_t : %ld\n", sizeof(inode_t));
			printf("Size of metadata_t : %ld\n\n", sizeof(metadata_t));

			printf("Disk details: \n"); 
			printf("----------------------\n");
			printf("File system name : %s\n", super->name);
			printf("Total blocks on disk : %d\n", super->total_blocks);
			printf("Block size : %d\n", BLOCKSIZE);
			printf("Number of blocks free : %d\n", super->free_blocks);
			printf("Number of files on disk : %d\n", super->num_files);
			printf("Free blocks ('.' is used, block number is unused) : \n");
			printf("[");
			for(int i = 0; i < super->total_blocks; i++) {
				if(super->status[i] == '0') {
					printf("%d ", i);
				}
				else {
					printf(".  ");
				}
				if(i % 9 == 8)
					printf("\n ");
			}
			printf("]\n----------------------------------------------------------------\n");
			free(super);
		}
		else if(!strcmp(argv[i], "-f")) {
			metadata_t* meta = read_metadata(fd);
			printf("PRINTING FILES INFO\n");

			printf("----------------------------------------------------------------\n");
			printf("Files on Disk:\n"); 
            printf("----------------------\n");
            printf("\tName\tinode#\tsize\tblocks\n");
            for(int i = 0; i < MAX_FILES; i++) {
            	inode_t* in = &meta->inodes[i];
            	if(in->inode_number != -1) {
	            	printf("\t%s\t%d\t%d\t[", in->filename, in->inode_number, in->file_size);
	            	int k = in->start_block;
	            	int lastblock;
	        		while(lastblock != in->end_block) {
	        			printf("%d ", k);
	        			lastblock = k;
	        			k = meta->FAT[k];

	        			if(k == -1 && lastblock != in->end_block) {
	        				printf("-1 ");
	        				break;
	        			}
	        		}
	        		printf("]\n");
	        	}
            }
            printf("----------------------------------------------------------------\n");
            free(meta);
		}
		else if(!strcmp(argv[i], "-c")) {
			printf("RUNNING FSCK\n");
			printf("----------------------------------------------------------------\n");

			int err = 0;

			superblock_t* super = read_superblock(fd);
			metadata_t* meta = read_metadata(fd);
			int checked[MAX_BLOCKS];

			for(int i = 0; i < MAX_BLOCKS; i++) {
				checked[i] = 0;
			}

			if(super->status[0] != '1') {
				printf("Bitmap has wrong value for superblock block status\n");
				err = 1;
			}

			checked[0] = 1;
			
			if(super->status[1] != '1') {
				printf("Bitmap has wrong value for metadata block status\n");
				err = 1;
			}

			checked[1] = 1;

			for(int i = 0; i < MAX_FILES; i++) {
            	inode_t* in = &meta->inodes[i];
            	if(in->inode_number != -1) {
            		int cnt = 0;
            		int lastblock;
	            	int k = in->start_block;
	        		while(k != -1) {
	        			cnt ++;
	        			lastblock = k;
	        			if(super->status[lastblock] != '1') {
	        				printf("Block number %d has incompatible FAT entry\n", lastblock);
	        				err = 1;
	        			}

	        			checked[lastblock] = 1;

	        			k = meta->FAT[k];
	        		}
					if(cnt*BLOCKSIZE != in->file_size) {
						printf("Filesize in inode doesn't match FAT. Filename: %s Inode number: %d\n", in->filename, in->inode_number);
					}
					if(lastblock != in->end_block) {
						printf("Last block in FAT doesn't match inode. Filename: %s Inode number: %d\n", in->filename, in->inode_number);
					}
	        	}
            }

            for(int i = 0; i < MAX_BLOCKS; i++) {
            	if(checked[i] == 0) {
            		if(super->status[i] != '0') {
            			printf("Block number %d is not in FAT but marked 1\n", i);
            		}
            	}
            }

            if(err){
            	printf("FILE CHECK : ERRORS FOUND\n");
            }
            else {
            	printf("FILE CHECK : NO ERRORS FOUND\n");
            }

			printf("----------------------------------------------------------------\n");
			free(super);
			free(meta);
		}
		else {
			printf("Option not recognised.\n");
		}
		printf("End  !\n");
	}
}

