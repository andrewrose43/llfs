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


