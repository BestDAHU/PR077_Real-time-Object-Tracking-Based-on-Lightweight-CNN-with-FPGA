#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <QImage>
#include <qdebug.h>
#include <QTime>
#include<QTimer>
#include<QPainter>
#include<QEventLoop>
#include <QFileDialog>

using namespace cv;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void paintEvent(QPaintEvent *);
    void DelayMS(unsigned int ms);

private:
    Ui::Dialog *ui;
    QPoint a[100],b[1000];
    int k=0,t=0,i=0;
    QImage img;

};

#endif // DIALOG_H
