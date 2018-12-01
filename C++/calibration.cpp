#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <string>

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

string protoFile = "../../Models/pose/coco/pose_deploy_linevec.prototxt";
string weightsFile = "../../Models/pose/coco/pose_iter_440000.caffemodel";

int nPoints = 17;

int main(int argc, char **argv)
{
    int key, m=1, i=0;
    string name = "stick";
    string imageFile = "../Users/" + name + ".jpeg";
    string imageAddr = "../Users/" + name + ".txt";
    // Take arguments from commmand line

    int inWidth = 368;
    int inHeight = 368;
    float thresh = 0.1;    

    Mat frame2;

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "ERROR: Unable to open the camera" << endl;
        return 0;
    }

    while(1) {

        cap >> frame2;
        flip(frame2,frame2,1);

        if (frame2.empty()) {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }

        imshow("Live",frame2);
        key = cv::waitKey(1);
        if (key==120){
            break;
        }
    }
    imwrite(imageFile, frame2);

    Mat frame = imread(imageFile);
    Mat frameCopy = frame.clone();
    int frameWidth = frame.cols;
    int frameHeight = frame.rows;

    double t = (double) cv::getTickCount();
    Net net = readNetFromCaffe(protoFile, weightsFile);

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
    }

    int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);
        
    while(1){

        imshow("Output-Skeleton", frame);

        for (int n = 0; n < nPoints-4; n++){
            // lookup 2 connected body/hand parts
            Point2f partA = points[POSE_PAIRS[n][0]];
            Point2f partB = points[POSE_PAIRS[n][1]];

            if (n == 9){
                Point2f partC = points[8];
                partB.x = (partB.x + partC.x)/2;
                partB.y = (partB.y + partC.y)/2;
            }

            if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
                continue;

            line(frame, partA, partB, Scalar(0,0,0), 2);
            if(POSE_PAIRS[n][0]==i)
                circle(frame, partA, 2, Scalar(0,255,255), -1);
            else
                circle(frame, partA, 2, Scalar(0,0,255), -1);
            if(POSE_PAIRS[n][1]==i)
                circle(frame, partB, 2, Scalar(0,255,255), -1);
            else
                circle(frame, partB, 2, Scalar(0,0,255), -1);
        }

        key = cv::waitKey(3);
        if (key==120){
            break;
        }
        else if(key>=49 && key<=57){
            m = 2*(key - 48);
        }
        else if(key==81){
            points[i].x -= m;
        }
        else if(key==83){
            points[i].x += m;
        }
        else if(key==84){
            points[i].y += m;
        }
        else if(key==82){
            points[i].y -= m;
        }
        else if(key==32){
            if(i<nPoints-1)
                i++;
            else
                i=0;
        }
    }

    ofstream ip;
    ip.open(imageAddr);
    for (int n=0; n < nPoints; n++){
        ip << points[n].x << " ";
        ip << points[n].y << endl;
    }
    ip.close();

    return 0;
}