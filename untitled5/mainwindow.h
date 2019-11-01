#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/types_c.h>
#include <QImage>
#include <qdebug.h>
#include <QTime>
#include<QTimer>
#include<QPainter>
#include<QEventLoop>
#include <QFileDialog>
#include "tracker_main.hpp"
#include <qcustomplot.h>

using namespace cv;
//extern int state[2][7];//在外部类使用该数组需要外部定义
//class Histogram; //用到外部类要前置声明forward declaration，比#include快，外部类变化时引用方不需要重新编译
typedef std::chrono::duration<double, std::ratio<1, 1000>> ms;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void update();

private slots:
    void on_openButton_clicked();

    void on_closeButton_clicked();

    void on_pauseButton_clicked();

    void on_Trail_clicked();

    void DelayMS(unsigned int ms);
    // cv::Mat DrawTracks(const cv::Mat &frame);
    // cv::Point Center(const cv::Rect& rect);
    // std::unordered_map<size_t, std::vector<cv::Point>> GetActiveTracks();
    // std::set<size_t> active_track_ids();

    void calculate(TrackedObject detection, int i);
    void varInit();

    double max(QVector<double> v);//柱状图

private:
    Ui::MainWindow *ui;
    QDialog* dialog;
    VideoCapture capture;
    cv::Mat frame;
    QString filename;
    //cv::Mat frame;
    QImage Qimg;
    int flag=-1;    //pause
    int delay=0;
    int count=0;
    //int x=0,y=0;
    QPainter paint;
    QPen pen;
    //TrackedObject a,b;
    //double d=0,aspeed=0,sspeed=0;
    float fps_time;
    Mat m_persctiveMat;
    QEventLoop eventloop;

    struct det{
        Point2f a={0,0},b={0,0};
        QString t0,t1;
        double d=0; //distance
        double aspeed=0,sspeed=0,mspeed=0;   //average speed , spot speed
        double dx=0,dy=0;
        QString rstate;
    }det[2];
    cv::Mat trail_frame;
    int trail_on=0;

    // std::vector<cv::Scalar> colors_;
    // std::set<size_t>active_track_ids_;
    //柱状图相关变量
    double state[2][6];
    QCPBars *fossil1,*fossil2;
    QVector<double> ticks;
    QVector<QString> labels;
    double yMax;
    double video_s;
};

#endif // MAINWINDOW_H
