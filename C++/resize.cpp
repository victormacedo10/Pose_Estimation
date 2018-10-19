#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <math.h>

#define PI 3.14159265

using namespace std;
using namespace cv;
using namespace cv::dnn;

const int POSE_PAIRS[17][2] = 
{   
    {1,2}, {2,3}, {3,4},
    {1,5}, {5,6}, {6,7},
    {1,11}, {11,12}, {12,13},
    {11,8}, {8,9}, {9,10},
    {1,0}, {0,14},
    {14,16}, {0,15}, {15,17}
};

int nPoints = 17;

void angle_measure(vector<Point> points, vector<Point> *thetas){
    double x, y, angle;

    for (int n = 0; n < nPoints; n++){
        Point2f partA = points[POSE_PAIRS[n][0]];
        Point2f partB = points[POSE_PAIRS[n][1]];
        if((partB.x - partA.x) == 0){
            thetas->push_back(Point(90.0));
        }
        else{
            y = partB.x - partA.x;
            x = partB.y - partA.y;
            angle = atan(y/x) * 180 / PI;
            if(angle > 0){
                if(x<0 && y<0){
                    angle += 180;
                }
            }
            else if(angle < 0){
                if(x<0 && y>0){
                    angle +=180;
                }
            }
            else{
                if(x>0){
                    angle = 0;
                }
                else if (x<0){
                    angle = 180;
                }
                else{
                    if (y>0){
                        angle = 90;
                    }
                    else{
                        angle = -90;
                    }
                }
            }
            thetas->push_back(Point(angle));
        }
    }
}

vector<float> distance_measure(vector<Point> points){
    vector<float> d(nPoints-1);
    for (int n = 0; n < nPoints; n++){
        Point2f partA = points[POSE_PAIRS[n][0]];
        Point2f partB = points[POSE_PAIRS[n][1]];
        double Ax = partA.x;
        double Bx = partB.x;
        double Ay = partA.y;
        double By = partA.y;
        d[n] = sqrt(pow((Ax - Bx),2) + pow((Ay - By),2));
    }
    return d;
}

void readfile(vector<Point> *points, string file_path){
    double x, y;

    ifstream ip;
    ip.open(file_path);
    if(!ip.is_open()){
        cout << "file does not exists:" << '\n';
    }

    ip>>x;
    ip>>y;
    points->push_back(Point((int) x,(int) y));
    while(ip.good()){
        ip>>x;
        ip>>y;
        points->push_back(Point((int) x,(int) y));
    }
}

int main(int argc, char **argv){ 

    vector<Point> prof(0);
    vector<Point> stud(0);

    string prof_name = "men";
    string stud_name = "leandro";
    string image = "../Photos/" + stud_name + ".jpg";

    Mat frame = imread(image);

    namedWindow("Output-Skeleton", WINDOW_NORMAL);

    readfile(&stud, "../Photos/" + stud_name + ".txt");
    readfile(&prof, "../Photos/" + prof_name + ".txt");

    for (int n = 0; n < nPoints; n++){

        Point2f partA = stud[POSE_PAIRS[n][0]];
        Point2f partB = stud[POSE_PAIRS[n][1]];
        Point2f partC = stud[8];
        if (n == 6){
            partC.x = (partB.x + partC.x)/2;
            partC.y = (partB.y + partC.y)/2;
            line(frame, partA, partC, Scalar(0,0,0), 3);
        }
        else{
            line(frame, partA, partB, Scalar(0,0,0), 3);
            circle(frame, partA, 3, Scalar(0,0,255), -1);
            circle(frame, partB, 3, Scalar(0,0,255), -1);
        }
        circle(frame, partC, 3, Scalar(0,0,255), -1);
    }

    //cout << prof << endl;

    vector<float> d_prof(nPoints-1);
    d_prof = distance_measure(prof);

    for (int n = 0; n < nPoints; n++){
        //cout << d_prof[n] << endl;
    }

    for (int n = 0; n < nPoints; n++){

        Point2f partA = prof[POSE_PAIRS[n][0]];
        Point2f partB = prof[POSE_PAIRS[n][1]];
        Point2f partC = prof[8];
        if (n == 6){
            partC.x = (partB.x + partC.x)/2;
            partC.y = (partB.y + partC.y)/2;
            line(frame, partA, partC, Scalar(0, 255, 255), 3);
        }
        else{
            line(frame, partA, partB, Scalar(0, 255, 255), 3);
            circle(frame, partA, 8, Scalar(0,0,255), -1);
            circle(frame, partB, 8, Scalar(0,0,255), -1);
        }
        circle(frame, partC, 8, Scalar(0,0,255), -1);
    }
    cout << "imshow" << endl;
    imshow("Output-Skeleton",frame);
    waitKey();
    return 0;
}
