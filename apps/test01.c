#include "../io/File.h"

int main(){
	
	// Create the virtual disk
	FILE* disk = fopen("../disk/vdisk", "wb+");
	// Clear vdisk
	char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
	fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	
	printf("It's working\n");
	return 0;
}
