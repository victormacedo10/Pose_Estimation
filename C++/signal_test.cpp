#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

using namespace std;

int value = 0;

void read_btn(int sig){
	FILE *fp;
	fp = fopen("btn_status.txt", "a+");
	fstream ip;
	ip.open("btn_status.txt");
	ip_in >> value;
	ip.clear();
	ip << 0;
	ip.close();
	ip.close();
}

int main(int argc, char **argv){
	
	int pid_filho;
	int pid_pai = getpid();
	ofstream ip_o;
	ip_o.open("btn_status.txt");
	ip_o.clear();
	ip_o << 0;
	ip_o.close();
	pid_filho = fork();
	
	if (pid_filho == 0){ // filho lê o bluetooth e escreve em um arquivo para que
				   // o pai consiga ver apos ser avisado por um signal
		printf("Filho inicializado!\n");
		string port = "/dev/rfcomm0";
		int device = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
		printf("Bluetooth Connected\n");
		string response;
		char buffer[1];
		int n;
		ifstream ip_in;
		ip_in.open("btn_status.txt");
		ip_o.open("btn_status.txt");
    
			while (1){
				int n = read(device, buffer, sizeof(char));
				if (n>0){
					printf("lido\n");
					ip_in >> value;
					if (value == 0){
						if (atoi(buffer)!=0){
							value = atoi(buffer);
							ip_o.clear();
							ip_o << value;
							kill(pid_pai, SIGUSR1);
							usleep(100000);
						}
					}
					
				}
			}			
		close(device);
		ip_o.close();
		ip_in.close();
	}

	
	else{ // pai recebe o que foi lido pelo filho via arquivo em uma função
		  // de signal
		printf("Pai inicializado\n");
		signal(SIGUSR1, read_btn);
		while(1){
			if (value != 0){
				printf("%d\n", value);
			}
		}
	}
}