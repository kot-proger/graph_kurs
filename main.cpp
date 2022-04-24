#include "mainwindow.h"
#include "gstreamer.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(nullptr, argc, argv);
    w.show();
    return a.exec();
}
