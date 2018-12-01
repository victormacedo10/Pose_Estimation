#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
}

MainWindow::~MainWindow()
{
    delete ui;
}

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

string protoFile = "../../../Models/pose/coco/pose_deploy_linevec.prototxt";
string weightsFile = "../../../Models/pose/coco/pose_iter_440000.caffemodel";

int j = 0;
int fps = 20;
int play = 0;
int f_len = 0;
int cal_state = 0;

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


void MainWindow::on_Cal_pressed()
{
    int key, m=1, i=0;
    string name = "stick";
    string imageFile = "../../Photos/" + name + ".jpeg";
    string imageAddr = "../../Photos/" + name + ".txt";
    // Take arguments from commmand line
    int nPoints = 17;
    int inWidth = 368;
    int inHeight = 368;
    float thresh = 0.1;

    int cameraIndex = 0;
    if(cal_state>2){
        cal_state = 0;
    }
    else{
        cal_state++;
    }

    Mat frame2;
    if(video.isOpened())
    {
        ui->Cal->setText("Processing");
        video.release();
    }
    if(cal_state==1){
        ui->Cal->setText("Capture");
        if(!video.open(cameraIndex))
        {
            QMessageBox::critical(this,
                                  "Camera Error",
                                  "Make sure you entered a correct camera index,"
                                  "<br>or that the camera is not being accessed by another program!");
            return;
        }
        while(video.isOpened())
        {
            video >> frame2;
            flip(frame2,frame2,1);

            if(!frame2.empty())
            {
                QImage qimg(frame2.data,
                            frame2.cols,
                            frame2.rows,
                            frame2.step,
                            QImage::Format_RGB888);
                pixmap.setPixmap( QPixmap::fromImage(qimg.rgbSwapped()) );
                ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
            }

            qApp->processEvents();
            imwrite(imageFile, frame2);
        }
    }
    else if(cal_state==2){
        ui->Cal->setText("Adjusted");
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

        if(1)
        {
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
            if(!frame.empty())
            {
                QImage qimg(frame.data,
                            frame.cols,
                            frame.rows,
                            frame.step,
                            QImage::Format_RGB888);
                pixmap.setPixmap( QPixmap::fromImage(qimg.rgbSwapped()) );
                ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
            }

            qApp->processEvents();
        }
    }
    video.release();
    ui->Cal->setText("Calibration");
}

void MainWindow::on_startBtn_pressed()
{

    pthread_t thread_id1;
    pthread_create(&thread_id1, NULL, &playback, NULL);

    int key, m=1, i=0;

    int xc=0, yc=0, xs=0, ys=0;

    float size = 1;

    int nPoints = 18;
    double x, y;

    if(video.isOpened())
    {
        ui->startBtn->setText("Start");
        video.release();
        return;
    }
    bool isCamera = 1;
    int cameraIndex = 0;
    if(!video.open(cameraIndex))
    {
        video.release();
    }

    ui->startBtn->setText("Stop");

    string prof_name = "victor";
    string resize_addr = "cd ../../C++/ && ./resize_video_general " + prof_name;
    const char* resize_addr1 = resize_addr.c_str();
    system(resize_addr1);

    int fps = video.get(CAP_PROP_FPS);
    cout << fps << endl;

    //Mat frame2;
    cout << "Start grabbing, press space on Live window to terminate" << endl;

    ifstream ip;
    string file_addr = "../../Test_files/" + prof_name + "_gabriel.txt";
    const char* file_addr1 = file_addr.c_str();
    ip.open(file_addr1);
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
    ip.close();

    x=0;
    y=0;

    Mat frame;
    while(video.isOpened())
    {
        video >> frame;
        flip(frame,frame,1);

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

        if(!frame.empty())
        {
            QImage qimg(frame.data,
                        frame.cols,
                        frame.rows,
                        frame.step,
                        QImage::Format_RGB888);
            pixmap.setPixmap( QPixmap::fromImage(qimg.rgbSwapped()) );
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
        play = 1;
        qApp->processEvents();
    }

    ui->startBtn->setText("Start");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(video.isOpened())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Stop the video before closing the application!");
        video.release();
        event->ignore();
    }
    else
    {
        event->accept();
    }
}
