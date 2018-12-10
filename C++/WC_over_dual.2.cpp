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
int f_len1, f_len2 = 0;

void* playback1(void* dummy_ptr)
{
	int delay = (int) 1e6/(fps);
	// Imprime '1' continuamente em stderr.
	while(1)
	{
		if(play==1){
			if(j<f_len1){
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

void* playback2(void* dummy_ptr)
{
	int delay = (int) 1e6/(fps);
	// Imprime '1' continuamente em stderr.
	while(1)
	{
		if(play==1){
			if(j<f_len1){
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
	pthread_create (&thread_id1, NULL, &playback1, NULL);
	pthread_t thread_id2;
	pthread_create (&thread_id2, NULL, &playback2, NULL);

	int x_off = 100, y_off = -5;

	Mat frame;

	int key, m=1, i=0;

	int xc=0, yc=0, xs=0, ys=0;

	float size = 1;

	int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);
	
    int nPoints = 17;
    double x, y;

	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "ERROR: Unable to open the camera" << endl;
		return 0;
	}

	int fps = cap.get(CAP_PROP_FPS);
    cout << fps << endl;

	ifstream ip1;
    ip1.open("../Test_files/gabriel.txt");
    if(!ip1.is_open()){
        cout << "file does not exists:" << '\n';
    }
    vector<Point> points1(0);
	ip1>>x;
    ip1>>y;
    points1.push_back(Point((int) x,(int) y));
    while(ip1.good()){
	    ip1>>x;
	    ip1>>y;
	    points1.push_back(Point((int) x,(int) y));
	    f_len1++;
	}
	cout << f_len1 << endl;

	ifstream ip2;
    ip2.open("../Test_files/gabriel_victor.txt");
    if(!ip2.is_open()){
        cout << "file does not exists:" << '\n';
    }
    vector<Point> points2(0);
	ip2>>x;
    ip2>>y;
    points2.push_back(Point((int) x,(int) y));
    while(ip2.good()){
	    ip2>>x;
	    ip2>>y;
	    points2.push_back(Point((int) x,(int) y));
	    f_len2++;
	}
	cout << f_len2 << endl;

	x=0;
	y=0;

	while(1) {

		cap >> frame;
		flip(frame,frame,1);

		if (frame.empty()) {
			cerr << "ERROR: Unable to grab from the camera" << endl;
			break;
		}

		for (int n = 0; n < nPoints-4; n++){

			Point2f partA = points2[j+POSE_PAIRS[n][0]];
			Point2f partB = points2[j+POSE_PAIRS[n][1]];

			partA.x = (partA.x + x)*size + xs - x_off;
			partA.y = (partA.y + y)*size + ys - y_off;
			partB.x = (partB.x + x)*size + xs - x_off;
			partB.y = (partB.y + y)*size + ys - y_off;

			if (n == 9){
				Point2f partC = points2[j+8];

				partC.x = (partC.x + x)*size + xs - x_off;
				partC.y = (partC.y + y)*size + ys - y_off;

				partB.x = (partB.x + partC.x)/2;
				partB.y = (partB.y + partC.y)/2;

				xc = partB.x;
				yc = partB.y;
			}

			if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
						    continue;

			line(frame, partA, partB, Scalar(0,255,255), 3);
			circle(frame, partA, 3, Scalar(0,255,0), -1);
			circle(frame, partB, 3, Scalar(0,255,0), -1);
		}

		for (int n = 0; n < nPoints-4; n++){

			Point2f partA = points1[j+POSE_PAIRS[n][0]];
			Point2f partB = points1[j+POSE_PAIRS[n][1]];

			partA.x = (partA.x + x)*size + xs + x_off;
			partA.y = (partA.y + y)*size + ys + y_off;
			partB.x = (partB.x + x)*size + xs + x_off;
			partB.y = (partB.y + y)*size + ys + y_off;

			if (n == 9){
				Point2f partC = points1[j+8];

				partC.x = (partC.x + x)*size + xs + x_off;
				partC.y = (partC.y + y)*size + ys + y_off;

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
		else if(key==171){
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
		}
		else if(key==173){
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
		}

	}

	cout << "Closing the camera" << endl;
	cap.release();
	destroyAllWindows();
	cout << "bye!" <<endl;
	cout << frame.size();
	return 0;
}
