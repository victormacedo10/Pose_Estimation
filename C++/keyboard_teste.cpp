#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>

using namespace std;

char key = 'x';

void* get_keys(void *arg){
	while(1){
		key = getchar();
		cout << key << endl;
		usleep(1000000);
	}
	return NULL;
}

int main(int argc, char *argv[]){

	pthread_t keys;
	int i = 0;

	pthread_create(&keys, NULL, get_keys, NULL);

    while(1)
    {
        i++;
        if(key == 'a'){
        	cout << i << endl;
        }
        	
    }
    return(0);
}
