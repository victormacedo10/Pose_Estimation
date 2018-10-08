#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <ctime>

using namespace std;
using namespace cv;
using namespace cv::dnn;

int main(int argc, const char * argv[])
{
	int nPoints = 18;
	int i;
	char string[100];
	int x, y;

	time_t current_time;

	ifstream ip;
	ip.open("../Test_files/bikefit1.txt");
	vector<Point> points(0);

	if(!ip.is_open()){
        cout << "file does not exists:" << '\n';
    }
    ip>>x;
    ip>>y;
    points.push_back(Point(x,y));
    for(i=0;i<2*nPoints;i++){
        ip>>x;
        ip>>y;
        points.push_back(Point(x,y));
    }
    ip.close();

    cout << points << endl;

    ofstream ip1;
    ip1.open("../Test_files/bikefit2.txt");

    if(!ip1.is_open()){
        cout << "file does not exists:" << '\n';
    }
    for(i=0;i<2*nPoints;i++){
    	ip1 << points[i].x << " ";
    	ip1 << points[i].y << " ";
    }
    ip1.close();

	return 0;
}
