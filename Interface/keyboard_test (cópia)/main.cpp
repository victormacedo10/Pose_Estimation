#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    //w.show();
    MainWindow *w = new MainWindow();

    /////////////////////////

    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    w->resize(screenGeometry.width(), screenGeometry.height());
    w->showNormal();

    return a.exec();
}
