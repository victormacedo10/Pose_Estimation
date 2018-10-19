#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>

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

int nPoints = 18;
int j = 0;
int play = 0;

vector<Point> points(0);

void* playback(void* dummy_ptr)
{
	int fps = 20;
	int delay = (int) 1e6/(fps);

	ifstream ip;
    ip.open("../Test_files/bikefit.txt");
    if(!ip.is_open()){
        cout << "file does not exists:" << '\n';
    }
    vector<Point> points_tmp(0);
	// Imprime '1' continuamente em stderr.
	while(1)
	{
		if(play==1){
			if(j<548){
				j+=18;
			}
			else{
				
				play=0;
			}
		}
		usleep(delay);
	}
	return NULL;
}

int main(int argc, char **argv)
{ 

	pthread_t thread_id1;
	pthread_create (&thread_id1, NULL, &playback, NULL);

	int key, m=1, i=0;

	int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);
	
    int nPoints = 18;
    double x, y;

	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "ERROR: Unable to open the camera" << endl;
		return 0;
	}

	int fps = cap.get(CAP_PROP_FPS);
    cout << fps << endl;

	Mat frame2;
	cout << "Start grabbing, press space on Live window to terminate" << endl;

	x=0;
	y=0;

	while(1) {

		cap >> frame2;

		if (frame2.empty()) {
			cerr << "ERROR: Unable to grab from the camera" << endl;
			break;
		}

		for (int n = 0; n < nPairs; n++){

			Point2f partA = points[j+POSE_PAIRS[n][0]];
			Point2f partB = points[j+POSE_PAIRS[n][1]];

			if (n == 9){
				Point2f partC = points[j+8];
				partB.x = (partB.x + partC.x)/2;
				partB.y = (partB.y + partC.y)/2;
			}

			if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
			    continue;

			partA.x += x;
			partA.y += y;
			partB.x += x;
			partB.y += y;

			line(frame2, partA, partB, Scalar(0,0,0), 3);
			circle(frame2, partA, 3, Scalar(0,0,255), -1);
			circle(frame2, partB, 3, Scalar(0,0,255), -1);
		}

		imshow("Live",frame2);
		key = cv::waitKey(1);
		if (key==120){
		    break;
		}
		else if(key>=49 && key<=57){
			m = 2*(key - 48);
		}
		else if(key==81){
			for (int n = 0; n < nPoints; n++){
				x -= m;
			}
		}
		else if(key==83){
			for (int n = 0; n < nPoints; n++){
				x += m;
			}
		}
		else if(key==84){
			for (int n = 0; n < nPoints; n++){
				y += m;
			}
		}
		else if(key==82){
			for (int n = 0; n < nPoints; n++){
				y -= m;
			}
		}
		else if(key==32){
			play = 1;
		}
	}

	cout << "Closing the camera" << endl;
	cap.release();
	destroyAllWindows();
	cout << "bye!" <<endl;
	cout << frame2.size();
	cout << points << endl;
	return 0;
}
