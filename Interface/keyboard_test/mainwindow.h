#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QtGui>
#include <QtCore>

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>

#include "opencv2/opencv.hpp"
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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initial_screen();
    void getName();
    void getPhoto();
    void Calibrate();
    void getVid();
    void resizePoints();
    void overlayStick();

    int menu = 0, stop = 0;

    string imageAddr;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_RightBtn_clicked();
    void on_LeftBtn_clicked();

    void on_OpenVid_clicked();

    void on_OpenStu_clicked();

private:
    Ui::MainWindow *ui;
    void keyPressEvent(QKeyEvent * event);
    QGraphicsScene *scene;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
    string name, prof;
    int key, play = 0;
};

#endif // MAINWINDOW_H
