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


void MainWindow::on_startBtn_pressed()
{
    using namespace std;
    using namespace cv;
    using namespace cv::dnn;

    pthread_t thread_id1;
    pthread_create(&thread_id1, NULL, &playback, NULL);

    int key, m=1, i=0;

    int xc=0, yc=0, xs=0, ys=0;

    float size = 1;

    int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);

    int nPoints = 18;
    double x, y;

    string prof_name = "victor";
    string resize_addr = "../C++/./resize_video_general " + prof_name;
    const char* resize_addr1 = resize_addr.c_str();
    //system(resize_addr1);
    system("ls ../");

    if(video.isOpened())
    {
        ui->startBtn->setText("Start");
        video.release();
        return;
    }

    bool isCamera;
    int cameraIndex = ui->videoEdit->text().toInt(&isCamera);
    isCamera = 1;
    cameraIndex = 0;
    if(isCamera)
    {
        if(!video.open(cameraIndex))
        {
            QMessageBox::critical(this,
                                  "Camera Error",
                                  "Make sure you entered a correct camera index,"
                                  "<br>or that the camera is not being accessed by another program!");
            return;
        }
    }
    else
    {
        if(!video.open(ui->videoEdit->text().trimmed().toStdString()))
        {
            QMessageBox::critical(this,
                                  "Video Error",
                                  "Make sure you entered a correct and supported video file path,"
                                  "<br>or a correct RTSP feed URL!");
            return;
        }
    }

    ui->startBtn->setText("Stop");

    int fps = video.get(CAP_PROP_FPS);
    cout << fps << endl;

    Mat frame2;
    cout << "Start grabbing, press space on Live window to terminate" << endl;

//    ifstream ip;
//    string file_addr = "../Test_files/" + prof_name + "_stick.txt";
//    const char* file_addr1 = file_addr.c_str();
//    ip.open("../Test_files/victor_stick.txt");
//    if(!ip.is_open()){
//        cout << "file does not exists:" << '\n';
//    }
//    vector<Point> points(0);
//    ip>>x;
//    ip>>y;
//    points.push_back(Point((int) x,(int) y));
//    while(ip.good()){
//        ip>>x;
//        ip>>y;
//        points.push_back(Point((int) x,(int) y));
//        f_len++;
//    }
//    cout << f_len << endl;
//    ip.close();

    x=0;
    y=0;

    Mat frame;
    while(video.isOpened())
    {
        video >> frame;
        if(!frame.empty())
        {
            copyMakeBorder(frame,
                           frame,
                           frame.rows/2,
                           frame.rows/2,
                           frame.cols/2,
                           frame.cols/2,
                           BORDER_REFLECT);

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

    ui->startBtn->setText("Start");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(video.isOpened())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Stop the video before closing the application!");
        event->ignore();
    }
    else
    {
        event->accept();
    }
}
