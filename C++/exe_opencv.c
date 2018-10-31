#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, const char *argv[]){
	char command[150];
	strcpy(command, "g++ $(pkg-config --libs --cflags opencv) -o ");
	//strcat(command, argv[1]);
	/*strcat(command, " ");
	strcat(command, argv[1]);
	strcat(command, ".cpp");
	strcat(command, "-ldthread");*/
	
	printf("%s", command);
	return 0;
}