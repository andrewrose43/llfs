#include "File.h"

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

// block size (bytes)
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define INODE_SIZE 29 //the amount of space an i-node actually uses in its block
