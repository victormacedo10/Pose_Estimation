#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

using namespace std;
using namespace cv;
using namespace cv::dnn;

int main(int argc, const char * argv[])
{
	int nPoints = 18;
	int fp, i;
	char string[100];
	int x, y;
	
	ifstream ip;
	ip.open("../Test_files/bikefit1.txt");
	vector<Point> points(0);

	if(!ip.is_open()){
        std::cout << "file does not exists:" << '\n';
    }
    ip>>x;
    ip>>y;
    points.push_back(Point(x,y));
    for(i=0;i<2*nPoints;i++){
        ip>>x;
        ip>>y;
        points.push_back(Point(x,y));
    }

    cout << points << endl;

	return 0;
}
