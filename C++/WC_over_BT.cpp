#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
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
using namespace cv;
using namespace cv::dnn;

const int POSE_PAIRS[17][2] = 
{   
    {1,2}, {1,5}, {2,3},
    {3,4}, {5,6}, {6,7},
    {11,8}, {8,9}, {9,10},
    {1,11}, {11,12}, {12,13},
    {1,0}, {0,14},
    {14,16}, {0,15}, {15,17}
};

int j = 0;
int fps = 20;
int play = 0;
int f_len = 0;
int m=1, i=0;

int xc=0, yc=0, xs=0, ys=0;

float size = 1;

double x, y;

void* playback(void* dummy_ptr)
{
	int delay = (int) 1e6/(fps);
	// Imprime '1' continuamente em stderr.
	while(1)
	{
		if(play==1){
			if(j<f_len){
				j+=18;
			}
			else{
				j=0;
				play=0;
			}
		}
		usleep(delay);
	}
	return NULL;
}
int key = 0;

void read_btn(int sig){
	int nPoints = 18;
	ifstream ip_in;
	
	while (key==0){
		ip_in.open("btn_status.txt");
		ip_in >> key;	
		ip_in.close();
	}
	cout << "Pai: " << key << endl;
	if(key==3){
		for (int n = 0; n < nPoints; n++){
			x -= m;
		}
		key=0;
	}
	else if(key==4){
		for (int n = 0; n < nPoints; n++){
			x += m;
		}
		key=0;
	}
	else if(key==2){
		for (int n = 0; n < nPoints; n++){
			y += m;
		}
		key=0;
	}
	else if(key==1){
		for (int n = 0; n < nPoints; n++){
			y -= m;
		}
		key=0;
	}
	else if(key==7){
		play = 1;
		key=0;
	}
	else if(key==6){
		if(size<5)
			size+=0.02;
			cout << "size = " << size << endl;
			cout << "{xc, yc} = ";
			cout << xc << " " << yc << endl;
			cout << "{x, y}a = ";
			cout << x << " " << y  << endl;
			xs = xc*(1-size);
			ys = yc*(1-size);
			cout << "{x, y}d = ";
			cout << x  << " " << y << endl;
		key=0;
	}
	else if(key==5){
		if(size>0){
			size-=0.02;
			cout << "size = " << size << endl;
			cout << "{xc, yc} = ";
			cout << xc << " " << yc << endl;
			cout << "{x, y}a = ";
			cout << x << " " << y  << endl;
			xs = xc*(1-size);
			ys = yc*(1-size);
			cout << "{x, y}d = ";
			cout << x  << " " << y << endl;
		}
		key=0;
	}
	ofstream ip_o;
	ip_o.open("btn_status.txt");
	ip_o.clear();
	ip_o << 0;
	ip_o.close();
}

int main(int argc, char **argv){
	
	int nPoints = 18;

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
				
		while (1){
			int n = read(device, buffer, sizeof(char));
			if (n>0){
//					printf("lido\n");
				ip_in.open("btn_status.txt");
				ip_in >> key;
				ip_in.close();
				if (key == 0){
					if (atoi(buffer)!=0){
						key = atoi(buffer);
						ip_o.open("btn_status.txt");
						ip_o.clear();
						cout << "filho: " << key << endl;
						ip_o << key;
						ip_o.close();
						kill(pid_pai, SIGUSR1);
						usleep(100000);
					}
				}
				
			}
		}			
		close(device);
	}

	else{ // pai recebe o que foi lido pelo filho via arquivo em uma função
		  // de signal
		printf("Pai inicializado\n");
		signal(SIGUSR1, read_btn);
	
		pthread_t thread_id1;
		pthread_create (&thread_id1, NULL, &playback, NULL);

		int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);

		VideoCapture cap(0);
		if (!cap.isOpened()) {
			cerr << "ERROR: Unable to open the camera" << endl;
			return 0;
		}

		int fps = cap.get(CAP_PROP_FPS);
	    cout << fps << endl;

		Mat frame;
		cout << "Start grabbing, press space on Live window to terminate" << endl;
		
		ifstream ip;
	    ip.open("../Test_files/gabriel_victor.txt");
	    if(!ip.is_open()){
	        cout << "file does not exists:" << '\n';
	    }
	    vector<Point> points(0);
		ip>>x;
	    ip>>y;
	    points.push_back(Point((int) x,(int) y));
	    while(ip.good()){
		    ip>>x;
		    ip>>y;
		    points.push_back(Point((int) x,(int) y));
		    f_len++;
		}
		cout << f_len << endl;

		x=0;
		y=0;

		while(key==0) {
			cap >> frame;
			flip(frame,frame,1);

			if (frame.empty()) {
				cerr << "ERROR: Unable to grab from the camera" << endl;
				break;
			}

			for (int n = 0; n < nPoints-5; n++){

				Point2f partA = points[j+POSE_PAIRS[n][0]];
				Point2f partB = points[j+POSE_PAIRS[n][1]];

				partA.x = (partA.x + x)*size + xs;
				partA.y = (partA.y + y)*size + ys;
				partB.x = (partB.x + x)*size + xs;
				partB.y = (partB.y + y)*size + ys;

				if (n == 9){
					Point2f partC = points[j+8];

					partC.x = (partC.x + x)*size + xs;
					partC.y = (partC.y + y)*size + ys;

					partB.x = (partB.x + partC.x)/2;
					partB.y = (partB.y + partC.y)/2;

					xc = partB.x;
					yc = partB.y;
				}

				if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
							    continue;

				line(frame, partA, partB, Scalar(0,0,0), 3);
				circle(frame, partA, 3, Scalar(0,0,255), -1);
				circle(frame, partB, 3, Scalar(0,0,255), -1);
			}

			imshow("Live",frame);
			cv::waitKey(1);
		}
		cout << "Saiu" << endl;
		cout << "Closing the camera" << endl;
		cap.release();
		destroyAllWindows();
		cout << "bye!" <<endl;
		cout << frame.size();
		return 0;
	}
}
