#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;
using namespace cv::dnn;

const int POSE_PAIRS[17][2] = 
{   
    {1,2}, {1,5}, {2,3},
    {3,4}, {5,6}, {6,7},
    {1,8}, {8,9}, {9,10},
    {1,11}, {11,12}, {12,13},
    {1,0}, {0,14},
    {14,16}, {0,15}, {15,17}
};

string protoFile = "../../Models/pose/coco/pose_deploy_linevec.prototxt";
string weightsFile = "../../Models/pose/coco/pose_iter_440000.caffemodel";

int nPoints = 18;

int main(int argc, char **argv)
{
    int cnt = 0;
    string name = "victor";
    string videoFile = "../Videos/" + name + ".mp4";
    // Take arguments from commmand line
    if (argc == 2)
    {   
      videoFile = argv[1];
    }

    int inWidth = 368;
    int inHeight = 368;
    float thresh = 0.01;    

    cv::VideoCapture cap(videoFile);

    int nframes = (int) cap.get(CAP_PROP_FRAME_COUNT);

    int fps = cap.get(CAP_PROP_FPS);
    cout << fps << endl;
    cout << nframes << endl;

    Mat frame, frameCopy;
    int frameWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int frameHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
    
    Net net = readNetFromCaffe(protoFile, weightsFile);
    double t=0;

    ofstream ip;
    ip.open("../Test_files/" + name + ".txt");

    while( waitKey(1) < 0)
    {       
        double t = (double) cv::getTickCount();

        cap >> frame;
        Mat inpBlob = blobFromImage(frame, 1.0 / 255, Size(inWidth, inHeight), Scalar(0, 0, 0), false, false);

        net.setInput(inpBlob);

        Mat output = net.forward();

        int H = output.size[2];
        int W = output.size[3];

        // find the position of the body parts
        vector<Point> points(nPoints);
        for (int n=0; n < nPoints; n++)
        {
            // Probability map of corresponding body's part.
            Mat probMap(H, W, CV_32F, output.ptr(0,n));

            Point2f p(-1,-1);
            Point maxLoc;
            double prob;
            minMaxLoc(probMap, 0, &prob, 0, &maxLoc);
            if (prob > thresh)
            {
                p = maxLoc;
                p.x *= (float)frameWidth / W ;
                p.y *= (float)frameHeight / H ;
            }
            points[n] = p;
            ip << p.x << " ";
            ip << p.y << " ";
        }
        cnt++;
        cout << "Progress: " << cnt << "/" << nframes << endl;
    }
    cap.release();
    ip.close();
    return 0;
}