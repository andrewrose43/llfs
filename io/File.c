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
//TODO: this method must make an entry in the directory, which means it must be provided with the directory
//TODO: this method must give the new i-node its file flag
//It will need to do some kind of while()-based repeated function call to go through as many layers of directory as are necessary to make it work
void createFile(FILE* disk){
	
	int blockNum = findFreeInode(disk);
	int inodeNum = blockNum - ZONE_OFFSET_INODES; //necessary to fit inodeNum into the byte provided to it in a directory

	setInodeAvailability(disk, blockNum, 1); //Mark the correct i-node as occupied

	//TESTING BEGINS

	printf("blockNum: %d\n", blockNum);
	printf("inodeNum: %d\n", inodeNum);

	blockNum = findFreeInode(disk);
	inodeNum = blockNum - ZONE_OFFSET_INODES; //necessary to fit inodeNum into the byte provided to it in a directory

	setInodeAvailability(disk, blockNum, 1); //Mark the correct i-node as occupied

	printf("blockNum: %d\n", blockNum);
	printf("inodeNum: %d\n", inodeNum);

	//TESTING ENDS

	free(inode);
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

	// Create the virtual disk
	FILE* disk = fopen("../disk/vdisk", "wb+");
	// Clear virtual disk
	// (this also clears the freelist bit vectors, which need to be set to 0 to indicate they are free)
        char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
        fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	free(init);

	// Next, we need to mark blocks 0 and 1 as occupied so that the i-node vector does not register itself and the data block vector as free space for i-nodes to use up
	char occupado = 0xc0;
	writeBytes(disk, ZONE_OFFSET_INODE_FREELIST, &occupado, 1, 0);

	// Then do the same for blocks 256 and 257 in the data block vector - data must not overwrite i-nodes.
	printf("writing to byte %d\n", ZONE_OFFSET_DATA - ZONE_OFFSET_INODES);
	writeBytes(disk, ZONE_OFFSET_DATA_FREELIST, &occupado, 1, ZONE_OFFSET_DATA - ZONE_OFFSET_INODES);

	// Finally, mark the first 6 bits of the data block zone as occupied so that i-nodes are not written there. (A similar protection is not needed at the end of the data block zone because the data block zone ends at the end of the file.)
	occupado = 0x3f;
	writeBytes(disk, ZONE_OFFSET_INODE_FREELIST, &occupado, 1, ZONE_OFFSET_DATA - ZONE_OFFSET_INODES);


	return disk;
}
