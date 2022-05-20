#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <gstreamer.h>
#include <qfiledialog.h>
#include <video.h>

GStreamer gst;
Video video;
bool getVideo = false;

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
    QString file = QFileDialog::getOpenFileName(0, tr("Open video"), "", tr("Video (*.mp4 *.mov *.wmv *.avi *.avihd *.flv *.f4v *.swf *.mkv *.webm *.mpg *.peg *.mp2 *.mpe *.mpv)"));
    if (file != "")
    {
        this->ui->coosed_file_label->setText(file);
        video = gst.Analyse(file);
        getVideo = true;
        this->ui->width_label->setText("Width: " + QString::number(video.getWidth()));
        this->ui->FPS_label->setText("FPS: " + QString::number(video.getFPS()));
        this->ui->Heigth_label->setText("Heigth: " + QString::number(video.getHeigth()));
        this->ui->aspect_label->setText("Aspect Ratio: " + video.getAspectRatio());
    }
}

void MainWindow::on_processBtn_clicked()
{
    if(getVideo) {
        gst.Process(video, 30, 1080, 1920, "16:9");
    }
}
