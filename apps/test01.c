#include "../io/File.h"

int main(){
	// Create the virtual disk
	FILE* disk = InitLLFS();
	
	//A little testing...
	FILE* stream = fopen("./darthplagueis.txt", "rb");
	char filename[MAX_NAME_LENGTH+1] = "/darthplagueis.txt"; //+1 because the slash gets trimmed off
	createFile(disk, stream, filename);

	return 0;
}
