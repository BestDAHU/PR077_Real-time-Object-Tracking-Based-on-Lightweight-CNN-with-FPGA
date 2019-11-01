/********************************************************************************
** Form generated from reading UI file 'histogram.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HISTOGRAM_H
#define UI_HISTOGRAM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_Histogram
{
public:
    QGridLayout *gridLayout;
    QCustomPlot *histogram0;
    QCustomPlot *histogram1;

    void setupUi(QWidget *Histogram)
    {
        if (Histogram->objectName().isEmpty())
            Histogram->setObjectName(QStringLiteral("Histogram"));
        Histogram->resize(1504, 862);
        gridLayout = new QGridLayout(Histogram);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        histogram0 = new QCustomPlot(Histogram);
        histogram0->setObjectName(QStringLiteral("histogram0"));

        gridLayout->addWidget(histogram0, 0, 0, 1, 1);

        histogram1 = new QCustomPlot(Histogram);
        histogram1->setObjectName(QStringLiteral("histogram1"));

        gridLayout->addWidget(histogram1, 0, 1, 1, 1);


        retranslateUi(Histogram);

        QMetaObject::connectSlotsByName(Histogram);
    } // setupUi

    void retranslateUi(QWidget *Histogram)
    {
        Histogram->setWindowTitle(QApplication::translate("Histogram", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class Histogram: public Ui_Histogram {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTOGRAM_H
