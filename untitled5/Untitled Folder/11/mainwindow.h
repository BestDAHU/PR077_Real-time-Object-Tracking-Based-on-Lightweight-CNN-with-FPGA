#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <QImage>
#include <qdebug.h>
#include <QTime>
#include<QTimer>
#include<QPainter>
#include<QEventLoop>
#include <QFileDialog>
#include "dialog.h";
#include <histogram.h>
using namespace cv;

extern int state[7];
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    //void paintEvent(QPaintEvent *);
    ~MainWindow();


private slots:
    void on_openButton_clicked();

    void on_closeButton_clicked();

    void on_pauseButton_clicked();

    void DelayMS(unsigned int ms);

    void draw();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
//    QDialog *dialog;
    VideoCapture capture;
    QString filename;
    Mat src;
    QImage Qimg,zm;
    int flag=-1;
    int delay;
    int count=0;
    int x=0,y=0;
    QPainter paint;
    QPen pen;
    QPoint a={0,0},b={0,0};
    double d=0,aspeed=0,sspeed=0;
    QString rstate;
    QEventLoop eventloop;

};

#endif // MAINWINDOW_H
