#include "../io/File.c"

int main(){
	
	// Create the virtual disk
	FILE* disk = fopen("vdisk", "wb+");
	// Clear vdisk
	char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
	fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	
	return 0;
}
