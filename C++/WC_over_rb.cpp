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

int j = 0;
int fps = 20;
int play = 0;
int f_len = 0;

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

int main(int argc, char **argv)
{ 

	pthread_t thread_id1;
	pthread_create (&thread_id1, NULL, &playback, NULL);

	int key, m=1, i=0;

	int xc=0, yc=0, xs=0, ys=0;

	float size = 1;

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

	while(1) {

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
		waitKey(1);
	}

	cout << "Closing the camera" << endl;
	cap.release();
	destroyAllWindows();
	cout << "bye!" <<endl;
	cout << frame.size();
	return 0;
}
