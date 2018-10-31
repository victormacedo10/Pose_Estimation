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

vector<float> angle_measure(vector<Point> points){
    double x, y, angle;
    vector<float> thetas(nPoints);
    for (int n = 0; n < nPoints; n++){
        Point2f partA = points[POSE_PAIRS[n][0]];
        Point2f partB = points[POSE_PAIRS[n][1]];
        if((partB.x - partA.x) == 0){
            thetas[n] = 90;
        }
        else{
            x = partB.x - partA.x;
            y = partB.y - partA.y;
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
                    if (y<0){
                        angle = -90;
                    }
                }
            }
            thetas[n] = angle;
        }
    }
    return thetas;
}

vector<float> distance_measure(vector<Point> points){
    vector<float> d(nPoints);
    for (int n = 0; n < nPoints; n++){
        Point2f partA = points[POSE_PAIRS[n][0]];
        Point2f partB = points[POSE_PAIRS[n][1]];
        float Ax = (float) partA.x;
        float Ay = (float) partA.y;
        float Bx = (float) partB.x;
        float By = (float) partB.y;
        float x = pow((Ax - Bx),2);
        float y = pow((Ay - By),2);
        d[n] = (float) sqrt(x + y);
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
    while(ip.good()){
        ip>>x;
        ip>>y;
        points->push_back(Point((int) x,(int) y));
    }
}

vector<Point> resize_funtion(vector<Point> prof, vector<Point> stud){

    vector<float> d_stud(nPoints);
    vector<float> a_prof(nPoints);

    d_stud = distance_measure(stud);
    a_prof = angle_measure(prof);

    int dif[2] = {0};

    for (int n = 0; n < nPoints; n++){
        Point2f partA = prof[POSE_PAIRS[n][0]];
        Point2f partB = prof[POSE_PAIRS[n][1]];
        partB.x += dif[0];
        partB.y += dif[1];

        partB.x = d_stud[n] * cos(a_prof[n] * PI / 180) + partA.x;
        partB.y = d_stud[n] * sin(a_prof[n] * PI / 180) + partA.y;

        if(n==0 || n==3 || n==6 || n==9 || n==12 || n==15){
            dif[0] = 0;
            dif[1] = 0;
        }
        else{
            Point2f partB_p = prof[POSE_PAIRS[n][1]];
            dif[0] = partB.x - partB_p.x;
            dif[1] = partB.y - partB_p.y;
        }
        prof[POSE_PAIRS[n][1]] = partB;
    }

    vector<float> dp_prof(nPoints);
    vector<float> ap_prof(nPoints);

    dp_prof = distance_measure(prof);
    ap_prof = angle_measure(prof);

    Point2f offset = prof[1] - stud[1];

    for (int n = 0; n < nPoints+1; n++){
        Point2f p_prof = prof[n];
        p_prof -= offset;
        prof[n] = p_prof;
    }

    return prof;
}

int main(int argc, char **argv){ 

    int n_joints = 18;

    vector<Point> prof(0);
    vector<Point> stud(0);
    vector<Point> video_prof(0);

    string prof_name = "victor";
    string stud_name = "gabriel";
    string image = "../Photos/" + stud_name + ".jpeg";

    Mat frame = imread(image);

    namedWindow("Output-Skeleton", WINDOW_NORMAL);

    readfile(&stud, "../Photos/" + stud_name + ".txt");
    readfile(&prof, "../Photos/" + prof_name + ".txt");
    readfile(&video_prof, "../Test_files/" + prof_name + ".txt");

    ofstream ip;
    ip.open("../Test_files/" + prof_name + "_" + stud_name + ".txt");

    int n_frames = (int) video_prof.size()/stud.size();
    for (int j = 0; j < n_frames; j++){
        for (int n = 0; n < n_joints; n++){
            prof[n] = video_prof[n + n_joints*j];
        }

        prof = resize_funtion(prof, stud);

        for (int n=0; n < n_joints; n++){
            ip << prof[n].x << " ";
            ip << prof[n].y << endl;
        }
        cout << j + 1 << "/" << n_frames << endl;
    }
    
    ip.close();
    cout << "finished" << endl;
    return 0;


}
