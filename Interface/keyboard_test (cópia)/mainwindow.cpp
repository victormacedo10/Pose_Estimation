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

    for (int n=0; n < nPoints; n++){
        cout << points[n].x << " ";
        cout << points[n].y << endl;
    }

    int nPairs = sizeof(POSE_PAIRS)/sizeof(POSE_PAIRS[0]);

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
        ui->LeftBtn->setStyleSheet(focused);
        ui->RightBtn->setStyleSheet(unfocused);
        ui->LeftBtn->setFocus();
        current = 0;
        printf("left\n");
    }
    if (event->key() == Qt::Key_D){
        ui->RightBtn->setStyleSheet(focused);
        ui->LeftBtn->setStyleSheet(unfocused);
        ui->RightBtn->setFocus();
        current = 1;
        printf("right\n");
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
        Calibrate();
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Salvar Pontos");
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
        initial_screen();
    }
    else if(menu == 2){
        menu = 0;
        initial_screen();
    }
    else if(menu == 3){
        menu = 2;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Tirar Foto");
        getPhoto();
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
