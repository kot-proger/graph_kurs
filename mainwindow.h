#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    MainWindow(QWidget *parent, int argc, char *argv[]);
private slots:
    void on_select_file_btn_clicked();

    void on_processBtn_clicked();

    void on_comboBox_3_currentIndexChanged(int index);

private:

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
