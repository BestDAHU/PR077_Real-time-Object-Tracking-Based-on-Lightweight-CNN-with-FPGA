#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>


using namespace cv;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    QFile qssfile(":style.qss");
    qssfile.open(QFile::ReadOnly);
    QString qss;
    qss=qssfile.readAll();
    this->setStyleSheet(qss);

}

MainWindow::~MainWindow()
{
    delete ui;
    
}


//void QDialog::paintEvent(QPaintEvent *){

////    QPainterPath path;
////    path.setFillRule(Qt::WindingFill);
////    path.addRect(10, 10, this->width()-20, this->height()-20);

////    QPainter painter(this);
////    painter.setRenderHint(QPainter::Antialiasing, true);
////    painter.fillPath(path, QBrush(Qt::white));

////    QColor color(0, 0, 0, 50);
////    for(int i=0; i<10; i++)
////    {
////        QPainterPath path;
////        path.setFillRule(Qt::WindingFill);
////        path.addRect(10-i, 10-i, this->width()-(10-i)*2, this->height()-(10-i)*2);
////        color.setAlpha(150 - i*50);
////        painter.setPen(color);
////        painter.drawPath(path);
////    }

//    int k=0,t=0;
//    while(k<100){
//        a[k]=b[k];
//        while(a[k].rx()!=-1){
//            if(t==0)
//              paint.drawPoints(a,k+1);
//             else
//                paint.drawPoints(a,100);

//        }

//        ++k;
//        if(k==100){
//            k=0;
//            t=1;
//        }
//    }
//}
void MainWindow::on_closeButton_clicked()
{


//    dialog = new QDialog(this);
//    dialog->resize(QSize(800,600));

//    dialog->setObjectName("dialog");
//    dialog->setStyleSheet("#dialog{border-image:url(:/2.jpg);}");

//    QLabel *label =new QLabel(dialog);
//    label->resize(dialog->rect().width(),dialog->rect().height());
//    QImage img;
//    img.load(":/badminton.jpg");
//    label->setScaledContents(true);
//    label->setPixmap(QPixmap::fromImage(img));
//        img.load(":/2.jpg");
//        label->setScaledContents(true);
//        label->setPixmap(QPixmap::fromImage(img));

//    cv::Mat s=imread("/home/cheng/Desktop/untitled5/resource/2.jpg",IMREAD_COLOR);



//    for(i=0;i<1000;++i){
//        b[i].rx()=i;
//        b[i].ry()=i;
//        a[i].rx()=-1;
//        a[i].ry()=-1;
//    }

//    QPainter paint(dialog);
//    update();


//    i=0;
//    while(i<1000){

//        cv::line(s,a[i],a[i+1],cv::Scalar(0,0,255),10,CV_AA);

//    cvtColor( s, s, CV_BGR2RGB );
//    img = QImage( (const unsigned char*)(s.data), s.cols, s.rows, QImage::Format_RGB888 );
//        label->setScaledContents(true);
//        label->setPixmap(QPixmap::fromImage(img));
//        ++i;
//     }

//    dialog->show();
//    //close();
    Dialog *d=new Dialog;
//    d->resize(QSize(800,600));
//    QLabel *label =new QLabel(d);
//    label->resize(d->rect().width(),d->rect().height());

//    cv::Mat s=imread("/home/cheng/Desktop/untitled5/resource/2.jpg",IMREAD_COLOR);
//        cvtColor( s, s, CV_BGR2RGB );
//        QImage img = QImage( (const unsigned char*)(s.data), s.cols, s.rows, QImage::Format_RGB888 );
//            label->setScaledContents(true);
//            label->setPixmap(QPixmap::fromImage(img));
       d->show();
}


void MainWindow::on_openButton_clicked()
{

    filename =QFileDialog::getOpenFileName(this,tr("Open Video File"),".",tr("Video Files(*.avi *.mp4 *.flv *.mkv)"));
    src = capture.open(filename.toStdString());
    if(!capture.isOpened()){
        qDebug() << QString::fromLocal8Bit("视频未成功打开!");
        return;
    }

    flag=1;
    double rate = capture.get(CV_CAP_PROP_FPS);
    delay =1000 / rate;

    count=0;
    d=sspeed=aspeed=0;

    while (capture.read(src)){
        if(src.empty()){
            qDebug() << QString::fromLocal8Bit("视频未成功读取!");
            return;
        }

        ++count;
        cvtColor( src, src, CV_BGR2RGB );
        Qimg = QImage( (const unsigned char*)(src.data), src.cols, src.rows, QImage::Format_RGB888 );
        //if(count<5) MainWindow::resize(Qimg.width(),Qimg.height());
        zm.load(":/timg.jpg");
        draw();



        //显示图片
        DelayMS(delay);
        ui->label->setScaledContents(true);
        ui->label->setPixmap( QPixmap::fromImage(Qimg));

        QPixmap person0,person1;
        person0.load(":/0.jpg");
        person1.load(":/1.jpg");
        person0=person0.scaled(ui->head0->width(),ui->head0->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        ui->head0->setPixmap(person0);
        ui->head0->setAlignment(Qt::AlignCenter);
        person1=person1.scaled(ui->head1->width(),ui->head1->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        ui->head1->setPixmap(person1);
        ui->head1->setAlignment(Qt::AlignCenter);
      }


}

void MainWindow::draw(){

    x+=2;
    y=x;
    double dx=0,dy=0;
    //process data
    //shuiping38.5  chuizhi 30.2  danwei:frame/m
    //chu li Qimg
    if(count%10==0){
        a=b;
        b=QPoint(x,y);
        //distance
        dx=qAbs(b.rx()-a.rx());
        dy=qAbs(b.ry()-a.ry());
        dx=dx/38.5;
        dy=dy/30.2;
        double tem=sqrt(dx*dx+dy*dy);
        d+=tem;
        //average speed and temporary speed
        aspeed=d/(count*delay)*1000;
        sspeed=tem/(10*delay)*1000;

        //state of run
        if(sspeed>=8.27)
            rstate="冲刺跑";
        else if(sspeed>=5.78)
            rstate="高速跑";
        else if(sspeed>=4.32)
            rstate="中速跑";
        else if(sspeed>=3.22)
            rstate="低速跑";
        else if(sspeed>=2.11)
            rstate="慢跑";
        else
            rstate="慢跑以下";
    }

    //display pic and data
    QPen pen1;
    paint.begin(&Qimg);
//    pen.setColor(QColor(255,0,0));
//    pen.setWidth(2);
//    paint.setPen(pen);
//    paint.drawRect(x,y,200,200);


    paint.setPen(pen1);
    const QRect rect = QRect(Qimg.width()-300,-10, 300, 150);
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.setOpacity(0.8);
    paint.drawImage(rect,zm);



        QString t0,t1;

//        const QRect r0 = QRect(Qimg.width()+ui->dataLabel->x()+ui->dataLabel->width()/3,-10, 200, 330);
//        const QRect r1 = QRect(Qimg.width()+ui->dataLabel->x()+ui->dataLabel->width()/3,ui->dataLabel->height()/2, 200, 330);
//        const QRect data0 = QRect(ui->p0label->x(),ui->p0label->y(), ui->p0label->width(), ui->p0label->height());
//        const QRect data1 = QRect(ui->p1label->x(),ui->p1label->y(), ui->dataLabel->width(), ui->dataLabel->height()/2-r1.height());

//        paint.drawImage(r0,person0);
//        paint.drawImage(r1,person1);
        t0="\n   DISTANCE: "+QString::number(d,'f',2)+"\n   AVERAGE SPEED: "+QString::number(aspeed,'f',2)+"\n   SPOT SPEED:"
                +QString::number(sspeed,'f',2)+"\n   STATE:"+rstate;
        t1="\n   DISTANCE: "+QString::number(d,'f',2)+"\n   AVERAGE SPEED: "+QString::number(aspeed,'f',2)+"\n   SPOT SPEED:"
                        +QString::number(sspeed,'f',2)+"\n   STATE:"+rstate;


        ui->p0label->setText(t0);
        ui->p1label->setText(t1);




    QString t="\n"+QString::number(Qimg.width())+"\n   DISTANCE: "+QString::number(d,'f',2)+"\n   AVERAGE SPEED: "+QString::number(aspeed,'f',2)+"\n   SPOT SPEED:"
                +QString::number(sspeed,'f',2)+"\n   STATE:"+rstate;
    pen.setColor(QColor(210,210,210));
    paint.setFont(QFont("Arial",16));
    paint.setPen(pen);
    paint.drawText(rect,t);

//        paint.drawText(data0,t0);
//        paint.drawText(data1,t1);

    paint.end();
}


void MainWindow::DelayMS(unsigned int ms)
{
    QTime Timer = QTime::currentTime().addMSecs(ms);
    while( QTime::currentTime() < Timer )
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}



void MainWindow::on_pauseButton_clicked()
{

    if(flag==1){
        flag=0;
        eventloop.exec();
    }
    else if(flag==0){
        flag=1;
        eventloop.quit();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    Form *form=new Form;
    form->show();
}
