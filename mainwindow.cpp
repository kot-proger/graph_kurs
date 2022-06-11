#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <gstreamer.h>
#include <qfiledialog.h>
#include <video.h>

GStreamer gst;
Video video;
bool getVideo = true;

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
        gst.Process(video, ui->comboBox_4->currentText().toInt(), ui->comboBox->currentData().toInt(), ui->comboBox_2->currentData().toInt(), ui->comboBox_3->currentText());
    }
}

void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    printf("%dccc\n", ui->comboBox_2->currentIndex());

    if (0 == index) {
        ui->comboBox_2->addItems({"1536", "1200", "1080", "768"});

        for (int i = 0; i<ui->comboBox_2->count(); i++) {
            ui->comboBox_2->removeItem(i);
            printf("%d\n",i);
        }
    }

    if (1 == index) {
        ui->comboBox_2->addItems({"1080", "900", "810", "720"});

        for (int i = 0; i<4; i++) {
            ui->comboBox_2->removeItem(i);
        }
    }
}
