#include "dialog.h"
#include "ui_dialog.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    img.load(":/2.jpg");
        for(i=0;i<1000;++i){
            b[i].rx()=i;
            b[i].ry()=i;

        }
        for(i=0;i<100;++i){

            a[i].rx()=-1;
            a[i].ry()=-1;
        }
                k=0;t=0;
           // while(1){
                //DelayMS(40);
                    //update();
                    if(t==0&&k==100){
                        k=0;
                        t=1;
                    }

               // }

//    QLabel *label =new QLabel(dialog);
//    label->resize(dialog->rect().width(),dialog->rect().height());
//    Mat s=imread("/home/cheng/Desktop/untitled5/resource/2.jpg");
//        cvtColor( s, s, CV_BGR2RGB );
//        QImage img = QImage( (const unsigned char*)(s.data), s.cols, s.rows, QImage::Format_RGB888 );
//                    QPainter paint(&img);
//                    paint.setPen(QPen(QColor(0,0,255)));
//                    paint.drawPoints(b,1000);

                    ui->label->setScaledContents(true);
            ui->label->setPixmap(QPixmap::fromImage(img));

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::DelayMS(unsigned int ms)
{
    QTime Timer = QTime::currentTime().addMSecs(ms);
    while( QTime::currentTime() < Timer )
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}

void Dialog::paintEvent(QPaintEvent *){
    QPainter paint(&img);
    paint.setPen(QPen(QColor(0,0,255)));
//            a[k]=b[k];
//            if(t==0)
//                paint.drawPoints(a,k+1);
//            else
//                paint.drawPoints(a,100);

//            ++k;
    paint.drawPoints(b,1000);


}
