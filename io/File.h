#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// SYSTEM CONSTANTS
// These values represent the byte and block offsets by which critical data fields and disk zones may be found within the virtual disk.

// zone offsets (in units of 512B blocks)
#define ZONE_OFFSET_INODE_FREELIST 0
#define ZONE_OFFSET_DATA_FREELIST 1
#define ZONE_OFFSET_INODES 2
#define ZONE_OFFSET_DATA 258

// i-node section boundaries (bytes)
#define INODE_OFFSET_FILE_SIZE 0
#define INODE_OFFSET_FILE_TYPE_FLAG 4
#define INODE_OFFSET_FIRST_DATA_BLOCK 5
#define INODE_OFFSET_PER_DATA_BLOCK 2
#define INODE_OFFSET_SINGLE_INDIRECT_BLOCK 25
#define INODE_OFFSET_DOUBLE_INDIRECT_BLOCK 27

// The flag bytes which indicate whether an inode is for a file or a directory
#define INODE_FLAG_FILE 255
#define INODE_FLAG_DIRECTORY 0

// block size (bytes)
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define INODE_SIZE 29 //the amount of space an i-node actually uses in its block

// Function declarations!

void readBlock(char* buffer, int blockNum, FILE* disk);
void writeBlock(FILE* disk, int blockNum, char* data, int size);
void writeBytes(FILE* disk, int blockNum, char* data, int size, int offset);
char* createEmptyInode();
void createFile(FILE* disk);
int findFreeInode(FILE* disk);
int findFreeDataBlock(FILE* disk);
void setInodeAvailability(FILE* disk, int block, int av);
void setDataBlockAvailability(FILE* disk, int block, int av);
FILE* InitLLFS();


