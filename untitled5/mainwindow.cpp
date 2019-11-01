#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>  // template class header file has no suffix name of .hh 
#include <utility>
#include <vector>
#include <map>
#include <memory>
#include <string>

#include <ctime>
#include <QThread>

#include "tracker_main.hpp"
#include <typeinfo>

#define FREQ 3

using namespace cv;

int image_x = 0;
int image_y = 0;
int image_p_col = 0;
int image_p_row = 0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->move(55,90);
    this->resize(QSize(1000,920));          
    this->setAttribute(Qt::WA_DeleteOnClose);

    //加载样式表
    QFile qssfile(":resource/style.qss");
    qssfile.open(QFile::ReadOnly);
    QString qss;
    qss=qssfile.readAll();
    this->setStyleSheet(qss);

    //柱状图初始化
    QLinearGradient gradient(0, 0, 0, 400);    // set dark background gradient: 设置暗背景渐变
    gradient.setColorAt(0, QColor(90, 90, 90));//开始颜色为黑色
    gradient.setColorAt(0.38, QColor(200, 105, 105));//红色
    gradient.setColorAt(1, QColor(70, 70, 70));//黑色
    ui->histogram1->setBackground(QBrush(gradient));//设置图表背景（用画刷设置）

    // create empty bar chart objects: 这个就是创建柱状图了
    //新版本应该是取消了之前的AddPlottable（不是很清楚，大概这样子）
    //然后直接在new QCPBars的时候指定x，y轴就可以了

    fossil1 = new QCPBars(ui->histogram1->xAxis, ui->histogram1->yAxis);
    fossil1->setAntialiased(false);// gives more crisp, pixel aligned bar borders.取消反锯齿
    fossil1->setStackingGap(1);//设置与下方柱子的像素点间隔
    //set names and colors: 设置名字和颜色
    //fossil1->setName("exercise state");
    fossil1->setPen(QPen(QColor(111, 9, 176).lighter(170)));// >100 则返回较浅的颜色
    fossil1->setBrush(QColor(111, 9, 176));

    // prepare x axis with country labels: //设置x轴标签
    ticks << 1 << 2 << 3 << 4 << 5 << 6;
    labels << "walking" << "jogging" << "low speed" << "medium speed" << "high speed" << "sprint";
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    ui->histogram1->xAxis->setTicker(textTicker);
    ui->histogram1->xAxis->setTickLabelRotation(60);//设置标签角度旋转
    ui->histogram1->xAxis->setSubTicks(false);//设置是否显示子标签
    ui->histogram1->xAxis->setTickLength(0, 4);
    ui->histogram1->xAxis->setRange(0, 7);//设置x轴区间
    ui->histogram1->xAxis->setBasePen(QPen(Qt::white));
    ui->histogram1->xAxis->setTickPen(QPen(Qt::white));
    ui->histogram1->xAxis->grid()->setVisible(true);//设置网格是否显示
    ui->histogram1->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    ui->histogram1->xAxis->setTickLabelColor(Qt::white);//设置标记标签颜色
    ui->histogram1->xAxis->setLabelColor(Qt::white);

    // prepare y axis: //设置y轴
    ui->histogram1->yAxis->setRange(0, 5);
    yMax=5;
    ui->histogram1->yAxis->setPadding(5); // a bit more space to the left border 设置左边留空间
    ui->histogram1->yAxis->setLabel("Cumulative distance(m)");
    ui->histogram1->yAxis->setBasePen(QPen(Qt::white));
    ui->histogram1->yAxis->setTickPen(QPen(Qt::white));
    ui->histogram1->yAxis->setSubTickPen(QPen(Qt::white));//设置SubTick颜色，SubTick指的是轴上的刻度线
    ui->histogram1->yAxis->grid()->setSubGridVisible(true);
    ui->histogram1->yAxis->setTickLabelColor(Qt::white);//设置标记标签颜色（y轴标记标签）
    ui->histogram1->yAxis->setLabelColor(Qt::white);//设置标签颜色（y轴右边标签）
    ui->histogram1->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    ui->histogram1->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    //ui->histogram1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->histogram2->setBackground(QBrush(gradient));
    fossil2 = new QCPBars(ui->histogram2->xAxis, ui->histogram2->yAxis);
    fossil2->setAntialiased(false);// gives more crisp, pixel aligned bar borders.取消反锯齿
    fossil2->setStackingGap(1);//设置与下方柱子的像素点间隔
    fossil2->setPen(QPen(QColor(111, 9, 176).lighter(170)));// >100 则返回较浅的颜色
    fossil2->setBrush(QColor(111, 9, 176));
    // prepare x axis with country labels: //设置x轴标签
    ui->histogram2->xAxis->setTicker(textTicker);
    ui->histogram2->xAxis->setTickLabelRotation(60);//设置标签角度旋转
    ui->histogram2->xAxis->setSubTicks(false);//设置是否显示子标签
    ui->histogram2->xAxis->setTickLength(0, 4);
    ui->histogram2->xAxis->setRange(0, 7);//设置x轴区间
    ui->histogram2->xAxis->setBasePen(QPen(Qt::white));
    ui->histogram2->xAxis->setTickPen(QPen(Qt::white));
    ui->histogram2->xAxis->grid()->setVisible(true);//设置网格是否显示
    ui->histogram2->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    ui->histogram2->xAxis->setTickLabelColor(Qt::white);//设置标记标签颜色
    ui->histogram2->xAxis->setLabelColor(Qt::white);

    // prepare y axis: //设置y轴
    ui->histogram2->yAxis->setRange(0, 5);
    ui->histogram2->yAxis->setPadding(5); // a bit more space to the left border 设置左边留空间
    ui->histogram2->yAxis->setLabel("Cumulative distance(m)");
    ui->histogram2->yAxis->setBasePen(QPen(Qt::white));
    ui->histogram2->yAxis->setTickPen(QPen(Qt::white));
    ui->histogram2->yAxis->setSubTickPen(QPen(Qt::white));//设置SubTick颜色，SubTick指的是轴上的刻度线
    ui->histogram2->yAxis->grid()->setSubGridVisible(true);
    ui->histogram2->yAxis->setTickLabelColor(Qt::white);//设置标记标签颜色（y轴标记标签）
    ui->histogram2->yAxis->setLabelColor(Qt::white);//设置标签颜色（y轴右边标签）
    ui->histogram2->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    ui->histogram2->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    //ui->histogram2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_closeButton_clicked()
{
    close();
}


void MainWindow::on_openButton_clicked()
{

    filename =QFileDialog::getOpenFileName(this,tr("Open Video File"),".",tr("Video Files(*.avi *.mp4 *.flv *.mkv)"));

     flag=1;

//******************************************************************************
    // Opening video.
    auto video_path = filename.toStdString();
    std::unique_ptr<ImageReader> video =
        ImageReader::CreateImageReaderForPath(video_path);           

    //PT_CHECK(video->IsOpened()) << "Failed to open video: " << video_path;
    double video_fps = video->GetFrameRate();
    video_s=1000/video_fps;
    // std::cout << video_fps << std::endl;       // video_fps = 30

    typedef std::chrono::duration<double, std::ratio<1, 1000>> ms;

    float image_scale = 2.25;
    int orign_x = 1920;
    int orign_y = 1080;
    image_x = (orign_x - 544*image_scale)/2;
    image_y = orign_y - 320*image_scale;

    // image_x = 0;
    // image_y = 0;

    cv::Rect rect(image_x,image_y,544*image_scale,320*image_scale);
    // rect.x = 0;
    // rect.y = 0;
    // rect.width = 1920;
    // rect.height = 1080;
    TrackedObjects detections1;
    TrackedObject  detection1;

    //求变换矩阵
    Mat srcImg = imread(":/resource/badminton.jpg", cv::IMREAD_GRAYSCALE);

	Point2f srcPoints[4], dstPoints[4];
	
	// 变换前 1920x1280的赛场矩阵四点
	srcPoints[0] = cv::Point2f(611, 195);
	srcPoints[1] = cv::Point2f(1249, 220);
	srcPoints[2] = cv::Point2f(1444, 1006);
	srcPoints[3] = cv::Point2f(535, 983);

	//目标矩阵 四点
	dstPoints[0] = cv::Point2f(250, 110);
	dstPoints[1] = cv::Point2f(422, 110);
	dstPoints[2] = cv::Point2f(422, 503);
	dstPoints[3] = cv::Point2f(250, 503);
	
	//求解变换矩阵
	m_persctiveMat = getPerspectiveTransform(srcPoints, dstPoints);
	
    for(int i=0;i<2;++i){
        for(int j=0;j<6;++j)
            state[i][j]=0;
    }
    dialog = new QDialog(this);
    dialog->setWindowTitle("trail");
    dialog->resize(QSize(400,800));
    dialog->move(1600,90);
    QLabel *label =new QLabel(dialog);
    label->resize(dialog->rect().width(),dialog->rect().height());

    cv::Mat p_frame=cv::imread("/home/dahu/Desktop/pedestrain/untitled5/resource/court1.png");  // 坐标透视转换用背景图片长宽
    image_p_col = p_frame.cols;
    image_p_row = p_frame.rows;

     std::ifstream fp("/home/dahu/Desktop/pedestrain/pedestrain_tracker/det_out1.txt",std::ios::in);
    // std::ofstream fp("/home/dahu/Desktop/pedestrain/pedestrain_tracker/det_out1.txt",std::ios::out);

    QPixmap person0,person1;
    person0.load(":resource/0.jpg");
    person1.load(":resource/1.jpg");
    person0=person0.scaled(ui->head0->width(),ui->head0->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->head0->setPixmap(person0);
    ui->head0->setAlignment(Qt::AlignLeft);
    person1=person1.scaled(ui->head1->width(),ui->head1->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->head1->setPixmap(person1);
    ui->head1->setAlignment(Qt::AlignLeft);

 
    int time_count = 1;
    auto start_time = std::chrono::high_resolution_clock::now(); // get current time stamp
    auto end_time = std::chrono::high_resolution_clock::now();
    std::string time_text;
    cv::Point origin = cv::Point(0,60);

    auto start_time1 = std::chrono::high_resolution_clock::now(); // get current time stamp
    auto end_time1 = std::chrono::high_resolution_clock::now();
    ms total_time1 = std::chrono::duration_cast<ms>(end_time1 - start_time1);
    std::string time_text1;
    cv::Point origin1 = cv::Point(0,30);

    //显示视频后窗口大小和背景变化
    this->resize(QSize(1480,900));
    QFile qssfile(":resource/style1.qss");
    qssfile.open(QFile::ReadOnly);
    QString qss;
    qss=qssfile.readAll();
    this->setStyleSheet(qss);

    for(;;){
        auto pair = video->Read();
        frame = pair.first;     // first is a image 
        if (frame.empty()) break;

        start_time1 = std::chrono::high_resolution_clock::now();
         tracker_work_fr(pair,rect,video_fps,fp);
        // tracker_work_fw(pair,rect,video_fps,fp);
        // tracker_work(pair,rect,video_fps);

        // display the result on image
        
        // Drawing all detected objects on a frame by BLUE COLOR

        // 画检测框
        // for (const auto &detection : detections) {
        //     detection1 = detection;
        //     detections1.emplace_back(detection1);
        //     cv::rectangle(frame, detection1.rect, cv::Scalar(255, 0, 0), 3);
        // }

        // Drawing tracked detections only by RED color and print ID and detection
        // confidence level.

        // int i=0;
        // 画跟踪框
        for (const auto &detection : tracker->TrackedDetections()) {

            detection1 = detection;

            //detection.object_id = 0;
            //  if(detection1.object_id>1)
            //      continue;
            cv::rectangle(frame, detection1.rect, cv::Scalar(0, 0, 255), 3);
            // std::string text = std::to_string(detection1.object_id) +
            //     " conf: " + std::to_string(detection1.confidence);
            std::string tmp=std::to_string(detection1.object_id);
            std::string text;
            if(tmp=="0")
                text="A";
            else
                text="B";
            cv::putText(frame, text, detection1.rect.tl(), cv::FONT_HERSHEY_COMPLEX,
                        1.3, cv::Scalar(0, 0, 255), 3);

            //calculate
            calculate(detection1,detection1.object_id);
            // if(i==0) ++i;
            }
        end_time1 = std::chrono::high_resolution_clock::now();
        total_time1 += std::chrono::duration_cast<ms>(end_time1 - start_time1);
//****************************************************************

        //QThread::msleep(20);    // 注意这里修改

        // 跟踪计算的时间
        
        // ms total = std::chrono::duration_cast<ms>(end_time - start_time);
        // fps_time = 1000.0/total.count();
        // cv::Point origin = cv::Point(0,30);
        
        // 设置字符串显示精度
        // std::stringstream ss;
        // ss << std::setprecision(4)<<fps_time;
        // std::string time_text = "FPS:" + ss.str();
        // cv::putText(frame, time_text, origin, cv::FONT_HERSHEY_COMPLEX,
        //             1.0, cv::Scalar(0, 0, 255), 2);

        Mat frame_out = frame;
        // frame.copyTo(frame_out);  // 一样的

        // 画轨迹
        if(trail_on){
            QImage img;
            trail_frame = tracker->DrawActiveTracks(frame_out);
            cvtColor(trail_frame, trail_frame, CV_BGR2RGB );
            img = QImage( (const unsigned char*)(trail_frame.data), trail_frame.cols, trail_frame.rows, QImage::Format_RGB888 );
            label->setScaledContents(true);
            label->setPixmap(QPixmap::fromImage(img));
            dialog->show();
        }

        //柱状图
        //update();
        
        //显示计算两个运动员的参数
        ui->p0label->setText(det[0].t0);
        ui->attr0->setText(det[0].t1);
        ui->attr1->setText(det[1].t1);
        ui->p1label->setText(det[1].t0);

        //DelayMS(delay);

        //if(count<5) MainWindow::resize(Qimg.width(),Qimg.height());
        ui->label->setScaledContents(true);

        
        // 计算帧率
        if(time_count==5){
        // 跟踪计算时间
        float fps_time1 = 1000.0/total_time1.count();
        std::stringstream ss_1;
        ss_1 << std::setprecision(4)<<fps_time1*5;
        time_text1 = "FPS:" + ss_1.str();
        total_time1 -= total_time1;

        // 总时间
        end_time = std::chrono::high_resolution_clock::now();
        ms total = std::chrono::duration_cast<ms>(end_time - start_time);
        fps_time = 1000.0/total.count();
    
        std::stringstream ss;
        ss << std::setprecision(4)<<fps_time*5;
        time_text = "Total_FPS:" + ss.str();

        start_time = std::chrono::high_resolution_clock::now();
        time_count = 0;

        }
 
        // 显示跟踪时间
        cv::putText(frame, time_text1, origin1, cv::FONT_HERSHEY_COMPLEX,
                    1.0, cv::Scalar(0, 0, 255), 2);
        // 显示总时间
        cv::putText(frame, time_text, origin, cv::FONT_HERSHEY_COMPLEX,
                    1.0, cv::Scalar(0, 0, 255), 2);
        cv::resize(frame, frame, cv::Size(), 0.5, 0.5);    // 图片缩小一半
        char k = cv::waitKey(delay);
        if (k == 27)
            break; 
        ++count;
        cvtColor(frame, frame, CV_BGR2RGB );
        Qimg = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(Qimg));
        time_count++;
    }

    fp.close();
    varInit();
}

void MainWindow::calculate(TrackedObject  detection, int i){
    
    i = i%2;    // 暂时这么处理

    if(count%FREQ==0){  // !
        double tmp=0;
        det[i].a=det[i].b;
        det[i].b=Point2f(detection.rect.x+detection.rect.width/2,detection.rect.y+detection.rect.height/2);
        //trans
        det[i].b = cvt_pos(det[i].b, m_persctiveMat);
        if(det[i].a!=Point2f(0,0)){
            det[i].dx=qAbs(det[i].b.x-det[i].a.x); //qAbs绝对值
            det[i].dy=qAbs(det[i].b.y-det[i].a.y);
            tmp=sqrt(det[i].dx*det[i].dx+det[i].dy*det[i].dy);

            //temporary speed and max speed
            //double sspeed_tmp=0.03*tmp/(FREQ*fps_time)*1000;
            double sspeed_tmp=0.03*tmp/(FREQ*video_s)*1000;
            if(sspeed_tmp<=9)
                det[i].sspeed=sspeed_tmp;
            if(det[i].mspeed<det[i].sspeed)
                det[i].mspeed=det[i].sspeed;

            //state of run
            //d为距离，1像素点等于3厘米
            if(det[i].sspeed>=8.27){
                det[i].rstate="sprint";
                state[i][5]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else if(det[i].sspeed>=5.78){
                det[i].rstate="high speed running";
                state[i][4]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else if(det[i].sspeed>=4.32){
                det[i].rstate="medium speed running";
                state[i][3]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else if(det[i].sspeed>=3.22){
                det[i].rstate="low speed running";
                state[i][2]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else if(det[i].sspeed>=2.11){
                det[i].rstate="jogging";
                state[i][1]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else if(det[i].sspeed>=1.2){
                det[i].rstate="walking";
                state[i][0]+=0.03*tmp;
                det[i].d+=0.03*tmp;
            }

            else{
                det[i].rstate="static";
            }

            update();//柱状图
            //det[i].aspeed=det[i].d/(count*fps_time)*1000;//average speed
            det[i].aspeed=det[i].d/(count*video_s)*1000;//average speed
        }

    }
    QString str;
    if(i==0)
        str="A";
    else 
        str="B";
    // std::cout<<detection.rect.x<<"   "<<detection.rect.y<<std::endl;
//std::cout <<typeid(total2) <<typeid(total2.count())<<"\n"<<endl;

    det[i].t0=str+"\nDISTANCE(m): "+"\nAVERAGE SPEED(m/s): "+"\nSPOT SPEED(m/s): "
                +"\nMAX SPEED(m/s): "+"\nSTATE: ";
    det[i].t1="\n"+QString::number(det[i].d,'f',2)+"\n"+QString::number(det[i].aspeed,'f',2)+"\n"
            +QString::number(det[i].sspeed,'f',2)+"\n"+QString::number(det[i].mspeed,'f',2)+"\n"+det[i].rstate;


}


void MainWindow::DelayMS(unsigned int ms)
{
    QTime Timer = QTime::currentTime().addMSecs(ms);
    while( QTime::currentTime() < Timer )
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}

void MainWindow::varInit(){
    int i;
    for(i=0;i<2;++i){
        det[i].a=Point2f(0,0);
        det[i].b=det[i].a;
        det[i].d=0;
        det[i].aspeed=0;
        det[i].sspeed=0;
        det[i].dx=0;
        det[i].dy=0;
        det[i].t0="";
        det[i].t1="";
        det[i].rstate="";
    }
    count=0;
}

void MainWindow::on_Trail_clicked()
{   if(trail_on==0)
        trail_on=1;  
    else{
        trail_on=0; 
        dialog->close();  
    }

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


double MainWindow::max(QVector<double> v){
    double tmp=0;
    for(int i=0;i<v.size();++i){
        if(tmp<v.at(i)){
            tmp=v.at(i);
        }
    }
    return tmp;
}

void MainWindow::update(){

    // Add data:添加数据
    QVector<double> fossilData;
    for(int i=0;i<6;++i)
        fossilData  << state[0][i];
    //setData(QVector<double> , QVector<double>) 第一个参数是指定哪条bar
    //y轴缩放
    auto max1 = max(fossilData);

    fossil1->setData(ticks, fossilData);
    ui->histogram1->replot(QCustomPlot::rpQueuedReplot);//参数避免重复调用replot和提高性能

    QVector<double> fossilData2;
    for(int i=0;i<6;++i)
        fossilData2  << state[1][i];
    auto max2 = max(fossilData2);
    auto max=(max1>max2)?max1:max2;
    if(max > yMax){
        yMax=max*2;
        ui->histogram1->yAxis->setRange(0, yMax);
        ui->histogram2->yAxis->setRange(0, yMax);      
    }
    //setData(QVector<double> , QVector<double>) 第一个参数是指定哪条bar
    fossil2->setData(ticks, fossilData2);
    ui->histogram2->replot(QCustomPlot::rpQueuedReplot);
}