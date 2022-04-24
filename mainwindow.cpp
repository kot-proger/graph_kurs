#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <gstreamer.h>
#include <qfiledialog.h>

GStreamer gst;

MainWindow::MainWindow(QWidget *parent, int argc, char *argv[])
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    gst = GStreamer(argc, argv);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_select_file_btn_clicked()
{
    QString file = QFileDialog::getOpenFileName(0, "Open Dialog", "", "*");
    this->ui->coosed_file_label->setText(file);
    gst.Analyse("https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm");
}
