/********************************************************************************
** Form generated from reading UI file 'form.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_H
#define UI_FORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include "form.h"

QT_BEGIN_NAMESPACE

class Ui_histogram
{
public:
    QGridLayout *gridLayout;
    QCustomPlot *customPlot;
    QCustomPlot *customPlot1;

    void setupUi(QWidget *form)
    {
        if (form->objectName().isEmpty())
            form->setObjectName(QStringLiteral("form"));
        form->resize(1493, 787);
        gridLayout = new QGridLayout(form);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        customPlot = new QCustomPlot(form);
        customPlot->setObjectName(QStringLiteral("customPlot"));

        gridLayout->addWidget(customPlot, 0, 0, 1, 1);

        customPlot1 = new QCustomPlot(form);
        customPlot1->setObjectName(QStringLiteral("customPlot1"));

        gridLayout->addWidget(customPlot1, 0, 1, 1, 1);


        retranslateUi(form);

        QMetaObject::connectSlotsByName(form);
    } // setupUi

    void retranslateUi(QWidget *form)
    {
        form->setWindowTitle(QApplication::translate("histogram", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class histogram: public Ui_histogram {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_H
