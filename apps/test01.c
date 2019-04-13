#include "../io/File.h"

int main(){
	// Create the virtual disk
	FILE* disk = InitLLFS();
	printf("It's working\n");
	//do_other_things_?
	
	//A little testing...
	createFile(disk);
	
	return 0;
}
