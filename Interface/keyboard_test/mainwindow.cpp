#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>

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

void MainWindow::getVid(){
    scene = new QGraphicsScene(this);
    QString color = "background-color: #afc7ed";
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    QGraphicsTextItem *text = scene->addText("Escolha o exercicio");
    printf("Screen 5\n");
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
    text = scene->addText("Calibrado");
}

void MainWindow::resizePoints(){
    prof = ui->VidIn->text().toUtf8().constData();
    scene = new QGraphicsScene(this);
    QString color = "background-color: #afc7ed";
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    QGraphicsTextItem *text = scene->addText("Rescalando Pontos ...");
    string resize_addr = "cd '/home/victor/Materiais UnB/9 semestre/Embarcados/Pose_Estimation-master/C++' && ./resize_video_general " + prof + " " + name;
    const char* resize_addr1 = resize_addr.c_str();
    system(resize_addr1);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setStyleSheet(color);
    ui->graphicsView->setScene(scene);
    text = scene->addText("Video pronto!");
}

void MainWindow::overlayStick(){

//    pthread_t thread_id1;
//    pthread_create (&thread_id1, NULL, &playback, NULL);
    int j = 0, fps, f_len;

    const int POSE_PAIRS[17][2] =
    {
        {1,2}, {1,5}, {2,3},
        {3,4}, {5,6}, {6,7},
        {11,8}, {8,9}, {9,10},
        {1,11}, {11,12}, {12,13},
        {1,0}, {0,14},
        {14,16}, {0,15}, {15,17}
    };

    double x, y;
    int xc=0, yc=0;
    int nPoints = 18;
    fps = video.get(CAP_PROP_FPS);
    cout << fps << endl;

    ifstream ip;
    string addr = "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Pose_Estimation-master/Videos/" + prof + "_" + name+ ".txt";
    const char* str_addr = addr.c_str();
    ip.open(str_addr);
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

    x=0;
    y=0;

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
    float s = 1;
    int width_frame = video.get(CAP_PROP_FRAME_WIDTH);
    int height_frame = video.get(CAP_PROP_FRAME_HEIGHT);

    Point2f part_m = points[11];
    int xs = int(width_frame/2) - part_m.x;
    int ys = int(height_frame/2) - part_m.y;

    xc = xs;
    yc = ys;

    for(int k=0; k<10; k++){
        cout << points[k] << endl;
    }

    while(menu == 7)
    {
        video >> frame;
        flip(frame,frame,1);

        if(play==1){
            if(j<f_len-250){
                j+=18;
            }
            else{
                j=0;
                play=0;
            }
        }

        for (int n = 0; n < nPoints-5; n++){
            Point2f partA = points[j+POSE_PAIRS[n][0]];
            Point2f partB = points[j+POSE_PAIRS[n][1]];

            partA.x = (partA.x)*s + xs;
            partA.y = (partA.y)*s + ys;
            partB.x = (partB.x)*s + xs;
            partB.y = (partB.y)*s + ys;

            if (n == 9){
                Point2f partC = points[j+8];

                partC.x = (partC.x)*s + xs;
                partC.y = (partC.y)*s + ys;

                partB.x = (partB.x + partC.x)/2;
                partB.y = (partB.y + partC.y)/2;

                xc = partB.x;
                yc = partB.y;
            }

            if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
                            continue;

            line(frame, partA, partB, Scalar(0,0,0), 5);
            circle(frame, partA, 5, Scalar(0,0,255), -1);
            circle(frame, partB, 5, Scalar(0,0,255), -1);
        }
        qApp->processEvents();

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
        if(key==1){
            xs -= 2;
            key = 0;
        }
        else if(key==3){
            xs += 2;
            key = 0;
        }
        else if(key==4){
            ys += 2;
            key = 0;
        }
        else if(key==2){
            ys -= 2;
            key = 0;
        }
        else if(key==5){
            play = 1;
        }
        else if(key==6){
            if(s<5)
                s+=0.02;
                xs = xc*(1-s);
                ys = yc*(1-s);
            key = 0;
        }
        else if(key==7){
            if(s>0){
                s-=0.02;
                xs = xc*(1-s);
                ys = yc*(1-s);
            }
            key = 0;
        }

    }
    video.release();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
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
    else if (event->key() == Qt::Key_X){
        play = 1;
    }
    else if (event->key() == Qt::Key_P){
        key = 6;
    }
    else if (event->key() == Qt::Key_O){
        key = 7;
    }
}



void MainWindow::on_RightBtn_clicked()
{
    if(menu == 0){
        menu = 5;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Ok");
        getVid();
    }
    else if(menu == 1){
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
    else if(menu==5){
        menu = 6;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Continuar");
        resizePoints();
    }
    else if(menu == 6){
        menu = 7;
        ui->LeftBtn->setText("Voltar");
        ui->RightBtn->setText("Ok");
        overlayStick();
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
    else if(menu == 5){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
    else if(menu == 6){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
    else if(menu == 7){
        menu = 0;
        ui->LeftBtn->setText("Calibrar");
        ui->RightBtn->setText("Executar movimento");
        initial_screen();
    }
}

void MainWindow::on_OpenVid_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Pose_Estimation-master/Videos/",
                "Text File (*.txt)"
                );
    QStringList list = filename.split('.');
    QStringList list1 = list[0].split('/');
    filename = list1[list1.size()-1];
    prof = filename.toUtf8().constData();
    cout << "prof: " << prof << endl;
    ui->VidIn->setText(filename);
}

void MainWindow::on_OpenStu_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/home/victor/Materiais\ UnB/9\ semestre/Embarcados/Pose_Estimation-master/Photos/",
                "Text File (*.txt)"
                );
    QStringList list = filename.split('.');
    QStringList list1 = list[0].split('/');
    filename = list1[list1.size()-1];
    name = filename.toUtf8().constData();
    cout << "name: " << name << endl;
    menu == 0;
    ui->NameIn->setText(filename);
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
