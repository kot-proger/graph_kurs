#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qfiledialog.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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
}
