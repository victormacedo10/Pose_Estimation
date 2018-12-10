#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <iostream>
#include <fstream>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->centralWidget);
    setFocusPolicy(Qt::NoFocus);
    initial_screen();
}

void MainWindow::initial_screen(){

    video.release();
    scene = new QGraphicsScene(this);
    QString blue = "background-color: #afc7ed";
    ui->graphicsView->setStyleSheet(blue);
    ui->graphicsView->setScene(scene);
    QGraphicsTextItem *text = scene->addText("Escolha uma das opcoes");
    printf("Init\n");
}

void MainWindow::getName(){
    scene = new QGraphicsScene(this);
    QString color = "background-color: #afc7ed";
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    QGraphicsTextItem *text = scene->addText("Digite seu nome");
    printf("Screen 1\n");
}

void MainWindow::getPhoto(){
    name = ui->NameIn->text().toUtf8().constData();
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    if(video.isOpened())
    {
        video.release();
    }
    video.open(0);
    Mat frame;
    stop = 0;
    while(menu == 2)
    {
        video >> frame;
        flip(frame,frame,1);

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
    video.release();
    imageAddr = "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Pose_Estimation-master/Photos/" + name;
    imwrite(imageAddr + ".jpg", frame);
}

void MainWindow::Calibrate(){

    const int POSE_PAIRS[17][2] =
    {
        {1,2}, {1,5}, {2,3},
        {3,4}, {5,6}, {6,7},
        {11,8}, {8,9}, {9,10},
        {1,11}, {11,12}, {12,13},
        {1,0}, {0,14},
        {14,16}, {0,15}, {15,17}
    };

    string protoFile = "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Models/pose/coco/pose_deploy_linevec.prototxt";
    string weightsFile = "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Models/pose/coco/pose_iter_440000.caffemodel";

    scene = new QGraphicsScene(this);
    QString color = "background-color: #afc7ed";
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    QGraphicsTextItem *text = scene->addText("Carregando ...");

    int nPoints = 17;
    vector<Point> points(nPoints);

    Mat frame = imread(imageAddr + ".jpg");
    while(!frame.data){
        cout <<  "Could not open or find the image" << endl;
        frame = imread(imageAddr + ".jpg");
    }
    cout <<  "Image opened" << endl;

    Net net = readNetFromCaffe(protoFile, weightsFile);

    int frameWidth = frame.cols;
    int frameHeight = frame.rows;

    int inWidth = 368;
    int inHeight = 368;
    float thresh = 0.1;

    Mat inpBlob = blobFromImage(frame, 1.0 / 255, Size(inWidth, inHeight), Scalar(0, 0, 0), false, false);

    net.setInput(inpBlob);

    Mat output = net.forward();

    int H = output.size[2];
    int W = output.size[3];

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

    for (int n=0; n < nPoints; n++){
        cout << points[n].x << " ";
        cout << points[n].y << endl;
    }

    int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);
    int i = 0;
    name = ui->NameIn->text().toUtf8().constData();
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    Mat frameCopy = frame.clone();

    while(menu == 4){
        frame = frameCopy.clone();

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

            line(frame, partA, partB, Scalar(0,0,0), 5);
            if(POSE_PAIRS[n][0]==i)
                circle(frame, partA, 5, Scalar(0,255,255), -1);
            else
                circle(frame, partA, 5, Scalar(0,0,255), -1);
            if(POSE_PAIRS[n][1]==i)
                circle(frame, partB, 5, Scalar(0,255,255), -1);
            else
                circle(frame, partB, 5, Scalar(0,0,255), -1);

            if(key==1){
                points[i].x -= 2;
                key = 0;
            }
            else if(key==3){
                points[i].x += 2;
                key = 0;
            }
            else if(key==4){
                points[i].y += 2;
                key = 0;
            }
            else if(key==2){
                points[i].y -= 2;
                key = 0;
            }
            else if(key==5){
                if(i<nPoints-1)
                    i++;
                else
                    i=0;
                key = 0;
            }
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

    ofstream ip;
    string txtAddr = imageAddr + ".txt";
    const char* charAddr = txtAddr.c_str();
    ip.open(charAddr);
    for (int n=0; n < nPoints; n++){
        ip << points[n].x << " ";
        ip << points[n].y << endl;
    }
    ip.close();

    scene = new QGraphicsScene(this);
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    text = scene->addText("Carregado!!!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    int current = 0;
    QString focused = "background-color: #f10; color:#fff;";
    QString unfocused = "background-color: #fff; color:#000;";
    if (event->key() == Qt::Key_A){
        key = 1;
    }
    else if (event->key() == Qt::Key_W){
        key = 2;
    }
    else if (event->key() == Qt::Key_D){
        key = 3;
    }
    else if (event->key() == Qt::Key_S){
        key = 4;
    }
    else if (event->key() == Qt::Key_Q){
        key = 5;
    }
}

void MainWindow::on_RightBtn_clicked()
{
    if(menu == 1){
        menu = 2;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Tirar Foto");
        getPhoto();
    }
    else if(menu == 2){
        menu = 3;
        ui->LeftBtn->setText("Tirar outra");
        ui->RightBtn->setText("Confirmar");
    }
    else if(menu==3){
        menu = 4;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Salvar Pontos");
        Calibrate();
    }
    else if(menu==4){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
}

void MainWindow::on_LeftBtn_clicked()
{
    if(menu == 0){
        menu = 1;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Ok");
        getName();
    }
    else if(menu == 1){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
    else if(menu == 2){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
    else if(menu == 3){
        menu = 2;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Tirar Foto");
        getPhoto();
    }
    else if(menu == 4){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(video.isOpened())
    {
        video.release();
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
