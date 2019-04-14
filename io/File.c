#include "File.h"

/*Reads one 512-byte block from disk into buffer
 * @param buffer: a chunk of dynamic memory to read into
 * @param blockNum: which block, from 0 to 4095, is being read from
 * @param disk: the virtual disk
 */
void readBlock(char* buffer, int blockNum, FILE* disk){
	fseek(disk, blockNum*BLOCK_SIZE, SEEK_SET);
	fread(buffer, BLOCK_SIZE, 1, disk);
}

/* 
 * Reads some particular number of bytes from the disk into a buffer.
 * @param buffer: a chunk of dynamic memory to read into
 * @param blockNum: which block, from 0 to 4095, is being read from
 * @param disk: the virtual disk
 * @param size: the number of bytes to read
 * @param offset: the byte offset within the block at which the read will begin
 */
void readBytes(char* buffer, int blockNum, FILE* disk, int size, int offset){
	fseek(disk, blockNum*BLOCK_SIZE+offset, SEEK_SET);
	fread(buffer, size, 1, disk);
}

/* Writes one 512-byte block into memory
 * Overwrites the ENTIRE BLOCK!
 * @param disk: the virtual disk
 * @param blockNum: which block, from 0 to 4095, is being written to
 * @param data: the data to be written
 * @param size: the number of bytes occupied by @param data
 */
void writeBlock(FILE* disk, int blockNum, char* data, int size){
	//First, write the contents
	fseek(disk, blockNum*BLOCK_SIZE, SEEK_SET); //Find the block to write to
	fwrite(data, size, 1, disk); //Write it!
	//Then clear the rest of the block
	fseek(disk, blockNum*BLOCK_SIZE+size, SEEK_SET);
	char* zeroes = calloc(BLOCK_SIZE-size, 1);
	fwrite(zeroes, BLOCK_SIZE-size, 1, disk);
	free(zeroes);
}

/* Writes any number of bytes into memory
 * Does NOT overwrite an entire block. This is a surgical strike
 * @param disk: the virtual disk
 * @param blockNum: which block, from 0 to 4095, is being written to
 * @param data: the data to be written
 * @param size: the number of bytes occupied by @param data
 * @param offset: the byte offset within the block at which the write will begin
 */
void writeBytes(FILE* disk, int blockNum, char* data, int size, int offset){
	fseek(disk, blockNum*BLOCK_SIZE+offset, SEEK_SET); //Find the location to write to
	fwrite(data, size, 1, disk); //Write it!
}

//TODO: gotta write to the file size field of the inode (and, well, the entire i-node's contents) whenever the file is written to
//TODO: this method must make an entry in the directory, which means it must be provided with a) the name of the new file and b) the directory
//TODO: this method must give the new i-node its file flag
//It will need to do some kind of while()-based repeated function call to go through as many layers of directory as are necessary to make it work
//This function assumes that we're working exclusively in the root directory. No sub-directories.
void createFile(FILE* disk, FILE* stream, char* filename){

	//Open up that incoming file!
	FILE* newFile = fopen(filename, "rb");
	
	int inodeBlockNum = findFreeInode(disk);
	int inodeNum = inodeBlockNum - ZONE_OFFSET_INODES; //necessary to fit inodeNum into the byte provided to it in a directory
	setInodeAvailability(disk, inodeBlockNum, 1); //Mark the correct i-node as occupied

	int dataBlockNum = findFreeDataBlock(disk);
	setDataBlockAvailability(disk, dataBlockNum, 1); //Mark the correct data block as occupied
	printf("dataBlockNum is %d\n", dataBlockNum);

	char rootBlock[4]; //to read that block number
	readBytes(rootBlock, ZONE_OFFSET_DATA_FREELIST, disk, 2, 0); //Read the block number of the root directory into rootBlock

	int* intRootBlock = (int*) rootBlock; //Cast that pointer as an int pointer to pass it to readBlock
	
	char* buffer = (char*)malloc(BLOCK_SIZE);
	readBlock(buffer, *intRootBlock, disk); //Read the directory into the buffer

	//Now let's find the first empty slot in the directory!
	for (int i = 0; i < 16; i++){
		if (buffer[32*i]) continue; //move to next slot if this one is occupied (0 indicates unoccupied slot)
		//if we made it past the if statement, we've found our empty slot!
		buffer[32*i] = inodeNum;
		strncpy((char*)&buffer[32*i+1], &filename[1], MAX_NAME_LENGTH); //32*i+1 because it is stored one bit on from the inode number; filename[1] to remove the slash; MAX_NAME_LENGTH to only copy what is needed
		writeBlock(disk, *intRootBlock, buffer, BLOCK_SIZE); //Write the buffer to disk!
		free(buffer);
		return; //you've copied in that file name; now shoo!
	}
	free(buffer);
	fprintf(stderr, "Directory out of space!\n");
	exit(1);
}

/*
 * Makes a new directory.
 * Incomplete, obviously.
 */
void createDir(FILE* disk, char* dirName){
	int blockNum = findFreeDataBlock(disk);
}

// Returns the block number of the first available i-node
int findFreeInode(FILE* disk){
	printf("Finding free inode...\n");
	char* buffer = (char*)malloc(BLOCK_SIZE);
	readBlock(buffer, ZONE_OFFSET_INODE_FREELIST, disk); //reads the freelist vector!
	for (int iblock = ZONE_OFFSET_INODES; iblock < ZONE_OFFSET_DATA; iblock++) {//Traverse the blocks that are part of the i-node zone!
		int whichByte = (iblock-(iblock%8))/8; //determines which byte of the bit vector corresponds to any block
		int arrVal = buffer[whichByte]; //arrVal = byte of bit vector currently being looked at
		int block = 0;
		int a = arrVal;

		for (int shift = 7; shift >= 0; shift--){
			int b = a>>shift;
			if ((~b)&1){
				block = (whichByte*8)+(7-shift);
				free(buffer);
				printf("Free inode found at %d\n", block);
				return block;
			}
		}
	}
}

// Returns the block number of the first available data block
int findFreeDataBlock(FILE* disk){
        char* buffer = (char*)malloc(BLOCK_SIZE);
        readBlock(buffer, ZONE_OFFSET_DATA_FREELIST, disk); //reads the freelist vector!
        for (int iblock = ZONE_OFFSET_DATA; iblock < NUM_BLOCKS; iblock++){//Traverse the blocks that are part of the data zone!
                int whichByte = (iblock-(iblock%8))/8; //determines which byte of the bit vector corresponds to any block
                int arrVal = buffer[whichByte]; //arrVal = byte of bit vector currently being looked at
                int block = 0;
                int a = arrVal;

                for (int shift = 7; shift >= 0; shift--){
                        int b = a>>shift;
                        if ((~b)&1){
                                block = (whichByte*8)+(7-shift);
                                free(buffer);
                                return block;
                        }
                }
        }
}

/* Marks an i-node block free or occupied
 * @param block: the block being freed or occupied
 * @param av: 0 = being freed; 1 = being occupied
 */
void setInodeAvailability(FILE* disk, int block, int av){
	printf("Setting inode availability\n");
	char *buffer = calloc(BLOCK_SIZE, 1);
	readBlock(buffer, ZONE_OFFSET_INODE_FREELIST, disk);
	int whichByte = (block-(block%8))/8;
	int bit = 7 - (block%8); //the bit to be set/reset
	unsigned char byte = buffer[whichByte]; //unsigned to avoid registering a negative number
	int x = byte;
	if (av==0){
		x = x & (255-(1<<bit));
	}
	else if (av==1){
		x = x | (1<<bit);
	}
	byte = x;
	buffer[whichByte] = byte;
	writeBlock(disk, ZONE_OFFSET_INODE_FREELIST, buffer, BLOCK_SIZE);
	free(buffer);
}

/* Marks a data block free or occupied
 * @param block: the block being freed or occupied
 * @param av: 0 = being freed; 1 = being occupied
 */
void setDataBlockAvailability(FILE* disk, int block, int av){
        char *buffer = calloc(BLOCK_SIZE, 1);
        readBlock(buffer, ZONE_OFFSET_DATA_FREELIST, disk);
        int whichByte = (block-(block%8))/8;
        int bit = 7 - (block%8); //the bit to be set/reset
        unsigned char byte = buffer[whichByte]; //unsigned to avoid registering a negative number
        int x = byte;
        if (av==0){
                x = x & (255-(1<<bit));
        }
        else if (av==1){
                x = x | (1<<bit);
        }
        byte = x;
        buffer[whichByte] = byte;
        writeBlock(disk, ZONE_OFFSET_DATA_FREELIST, buffer, BLOCK_SIZE);
        free(buffer);
}

// Get this party started
FILE* InitLLFS(){


	// CREATE AND CLEAR THE DISK

	// Create the virtual disk
	FILE* disk = fopen("../disk/vdisk", "wb+");
	// Clear virtual disk
	// (this also clears the freelist bit vectors, which need to be set to 0 to indicate they are free)
        char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
        fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	free(init);


	// SET BOUNDARIES ON THE FREELIST VECTORS

	// Next, we need to mark blocks 0 and 1 as occupied so that the i-node vector does not register itself and the data block vector as free space for i-nodes to use up
	setInodeAvailability(disk, 0, 1);
	setInodeAvailability(disk, 1, 1);

	// Then do the same for block 256 in the data block vector - data must not overwrite i-nodes.
	setDataBlockAvailability(disk, 256, 1);

	// Finally, mark the first 7 bits of the data block zone as occupied so that i-nodes are not written there. (A similar protection is not needed at the end of the data block zone because the data block zone ends at the end of the file.)
	setInodeAvailability(disk, 257, 1);
	setInodeAvailability(disk, 258, 1);
	setInodeAvailability(disk, 259, 1);
	setInodeAvailability(disk, 260, 1);
	setInodeAvailability(disk, 261, 1);
	setInodeAvailability(disk, 262, 1);
	setInodeAvailability(disk, 263, 1);


	// CREATE THE ROOT DIRECTORY

	// First, create the directory's i-node
	int rootINodeBlock = findFreeInode(disk);
	// Mark that i-node as occupied
	setInodeAvailability(disk, rootINodeBlock, 1); //Mark the correct i-node as occupied
	int rootDirBlock = findFreeDataBlock(disk); //the block number of the root dir
	setDataBlockAvailability(disk, rootDirBlock, 1); // Mark that data block as occupied


	// Write the size of the directory to its inode (BLOCK_SIZE, of course)
	// This requires some casting trickery.
	char tmpchars[4]; //4 bytes in an array
	int *intTmpChars = (int*)tmpchars;
	*intTmpChars = BLOCK_SIZE;
	// Now, casting tmpchars (or the disk location it writes to) as an int* will get you BLOCK_SIZE!

	// Write the file size (BLOCK_SIZE) to disk
	writeBytes(disk, rootINodeBlock, tmpchars, INODE_OFFSET_FILE_TYPE_FLAG - INODE_OFFSET_FILE_SIZE, INODE_OFFSET_FILE_SIZE);

	// Write the directory flag to disk
	*intTmpChars = INODE_FLAG_DIRECTORY;
        writeBytes(disk, rootINodeBlock, tmpchars, INODE_OFFSET_FIRST_DATA_BLOCK - INODE_OFFSET_FILE_TYPE_FLAG, INODE_OFFSET_FILE_TYPE_FLAG);

	// Write the directory's block number to disk
	*intTmpChars = rootDirBlock;
	printf("The directory's contents will be stored in block %d\n", rootDirBlock);
	writeBytes(disk, rootINodeBlock, tmpchars, INODE_OFFSET_PER_DATA_BLOCK, INODE_OFFSET_FIRST_DATA_BLOCK);

	// I stored the block number in an inode for professionalism's sake, but won't use it. To navigate to the directory in the future, it will be easier to simply access its block number at the beginning of the free data block vector.
	writeBytes(disk, ZONE_OFFSET_DATA_FREELIST, tmpchars, INODE_OFFSET_PER_DATA_BLOCK, 0);

	// ALL DONE!

	return disk;
}
