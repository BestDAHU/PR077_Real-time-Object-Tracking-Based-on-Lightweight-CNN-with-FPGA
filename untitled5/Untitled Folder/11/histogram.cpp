#include "histogram.h"
#include "ui_form.h"
#include<qcustomplot.h>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    // set dark background gradient: 设置暗背景渐变
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));//开始颜色为黑色
    gradient.setColorAt(0.38, QColor(200, 105, 105));//红色
    gradient.setColorAt(1, QColor(70, 70, 70));//黑色
    ui->customPlot->setBackground(QBrush(gradient));//设置图表背景（用画刷设置）

    // create empty bar chart objects: 这个就是创建柱状图了
    //新版本应该是取消了之前的AddPlottable（不是很清楚，大概这样子）
    //然后直接在new QCPBars的时候指定x，y轴就可以了

    QCPBars *fossil = new QCPBars(ui->customPlot->xAxis, ui->customPlot->yAxis);
    fossil->setAntialiased(false);// gives more crisp, pixel aligned bar borders.取消反锯齿
    fossil->setStackingGap(1);//设置与下方柱子的像素点间隔
    //set names and colors: 设置名字和颜色
    //fossil->setName("exercise state");
    fossil->setPen(QPen(QColor(111, 9, 176).lighter(170)));// >100 则返回较浅的颜色
    fossil->setBrush(QColor(111, 9, 176));

    // prepare x axis with country labels: //设置x轴标签
    QVector<double> ticks;
    QVector<QString> labels;
    ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7;
    labels << "static" << "walking" << "jogging" << "low speed" << "medium speed" << "high speed" << "sprint";
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    ui->customPlot->xAxis->setTicker(textTicker);
    ui->customPlot->xAxis->setTickLabelRotation(60);//设置标签角度旋转
    ui->customPlot->xAxis->setSubTicks(false);//设置是否显示子标签
    ui->customPlot->xAxis->setTickLength(0, 4);
    ui->customPlot->xAxis->setRange(0, 8);//设置x轴区间
    ui->customPlot->xAxis->setBasePen(QPen(Qt::white));
    ui->customPlot->xAxis->setTickPen(QPen(Qt::white));
    ui->customPlot->xAxis->grid()->setVisible(true);//设置网格是否显示
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    ui->customPlot->xAxis->setTickLabelColor(Qt::white);//设置标记标签颜色
    ui->customPlot->xAxis->setLabelColor(Qt::white);

    // prepare y axis: //设置y轴
    ui->customPlot->yAxis->setRange(0, 12.1);
    ui->customPlot->yAxis->setPadding(5); // a bit more space to the left border 设置左边留空间
    ui->customPlot->yAxis->setLabel("Cumulative duration of exercise state");
    ui->customPlot->yAxis->setBasePen(QPen(Qt::white));
    ui->customPlot->yAxis->setTickPen(QPen(Qt::white));
    ui->customPlot->yAxis->setSubTickPen(QPen(Qt::white));//设置SubTick颜色，SubTick指的是轴上的刻度线
    ui->customPlot->yAxis->grid()->setSubGridVisible(true);
    ui->customPlot->yAxis->setTickLabelColor(Qt::white);//设置标记标签颜色（y轴标记标签）
    ui->customPlot->yAxis->setLabelColor(Qt::white);//设置标签颜色（y轴右边标签）
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // Add data:添加数据
    QVector<double> fossilData;
    fossilData  << 0.86*10.5 << 0.83*5.5 << 0.84*5.5 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2 << 0.67*11.2;

    //setData(QVector<double> , QVector<double>) 第一个参数是指定哪条bar
    fossil->setData(ticks, fossilData);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    if(1==1){

        ui->widget_2->setBackground(QBrush(gradient));

        QCPBars *fossil1 = new QCPBars(ui->widget_2->xAxis, ui->widget_2->yAxis);
        fossil1->setAntialiased(false);// gives more crisp, pixel aligned bar borders.取消反锯齿
        fossil1->setStackingGap(1);//设置与下方柱子的像素点间隔

        fossil1->setPen(QPen(QColor(111, 9, 176).lighter(170)));// >100 则返回较浅的颜色
        fossil1->setBrush(QColor(111, 9, 176));

        // prepare x axis with country labels: //设置x轴标签

        ui->widget_2->xAxis->setTicker(textTicker);
        ui->widget_2->xAxis->setTickLabelRotation(60);//设置标签角度旋转
        ui->widget_2->xAxis->setSubTicks(false);//设置是否显示子标签
        ui->widget_2->xAxis->setTickLength(0, 4);
        ui->widget_2->xAxis->setRange(0, 8);//设置x轴区间
        ui->widget_2->xAxis->setBasePen(QPen(Qt::white));
        ui->widget_2->xAxis->setTickPen(QPen(Qt::white));
        ui->widget_2->xAxis->grid()->setVisible(true);//设置网格是否显示
        ui->widget_2->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
        ui->widget_2->xAxis->setTickLabelColor(Qt::white);//设置标记标签颜色
        ui->widget_2->xAxis->setLabelColor(Qt::white);

        // prepare y axis: //设置y轴
        ui->widget_2->yAxis->setRange(0, 12.1);
        ui->widget_2->yAxis->setPadding(5); // a bit more space to the left border 设置左边留空间
        ui->widget_2->yAxis->setLabel("Cumulative duration of exercise state");
        ui->widget_2->yAxis->setBasePen(QPen(Qt::white));
        ui->widget_2->yAxis->setTickPen(QPen(Qt::white));
        ui->widget_2->yAxis->setSubTickPen(QPen(Qt::white));//设置SubTick颜色，SubTick指的是轴上的刻度线
        ui->widget_2->yAxis->grid()->setSubGridVisible(true);
        ui->widget_2->yAxis->setTickLabelColor(Qt::white);//设置标记标签颜色（y轴标记标签）
        ui->widget_2->yAxis->setLabelColor(Qt::white);//设置标签颜色（y轴右边标签）
        ui->widget_2->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
        ui->widget_2->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

        // Add data:添加数据
        QVector<double> fossilData1;
        fossilData1  << 0.86*10.5 << 0.83*5.5 << 0.84*5.5 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2 << 0.67*11.2;

        //setData(QVector<double> , QVector<double>) 第一个参数是指定哪条bar
        fossil1->setData(ticks, fossilData1);
        ui->widget_2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }


}

Form::~Form()
{
    delete ui;
}

