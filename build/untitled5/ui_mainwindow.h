/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QLabel *p1label;
    QLabel *p0label;
    QLabel *label;
    QLabel *head0;
    QLabel *head1;
    QLabel *attr0;
    QLabel *attr1;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *openButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *pauseButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *Trail;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *closeButton;
    QSpacerItem *horizontalSpacer_5;
    QCustomPlot *histogram1;
    QCustomPlot *histogram2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->setEnabled(true);
        MainWindow->resize(1631, 968);
        MainWindow->setFocusPolicy(Qt::NoFocus);
        MainWindow->setAcceptDrops(false);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        p1label = new QLabel(centralWidget);
        p1label->setObjectName(QStringLiteral("p1label"));
        p1label->setGeometry(QRect(1120, 432, 165, 140));
        p1label->setMinimumSize(QSize(165, 140));
        p0label = new QLabel(centralWidget);
        p0label->setObjectName(QStringLiteral("p0label"));
        p0label->setGeometry(QRect(1120, -3, 165, 140));
        p0label->setMinimumSize(QSize(165, 140));
        p0label->setMaximumSize(QSize(273, 392));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(3, 0, 1024, 768));
        label->setMinimumSize(QSize(712, 0));
        head0 = new QLabel(centralWidget);
        head0->setObjectName(QStringLiteral("head0"));
        head0->setGeometry(QRect(1030, 10, 88, 121));
        head0->setMinimumSize(QSize(88, 101));
        head1 = new QLabel(centralWidget);
        head1->setObjectName(QStringLiteral("head1"));
        head1->setGeometry(QRect(1030, 445, 88, 121));
        head1->setMinimumSize(QSize(88, 121));
        attr0 = new QLabel(centralWidget);
        attr0->setObjectName(QStringLiteral("attr0"));
        attr0->setGeometry(QRect(1295, -3, 161, 140));
        attr0->setMinimumSize(QSize(150, 140));
        attr1 = new QLabel(centralWidget);
        attr1->setObjectName(QStringLiteral("attr1"));
        attr1->setGeometry(QRect(1295, 432, 161, 140));
        attr1->setMinimumSize(QSize(150, 140));
        horizontalLayoutWidget = new QWidget(centralWidget);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(0, 770, 1021, 121));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        openButton = new QPushButton(horizontalLayoutWidget);
        openButton->setObjectName(QStringLiteral("openButton"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(openButton->sizePolicy().hasHeightForWidth());
        openButton->setSizePolicy(sizePolicy);
        openButton->setMinimumSize(QSize(150, 50));
        openButton->setMaximumSize(QSize(150, 50));
        openButton->setBaseSize(QSize(0, 0));

        horizontalLayout->addWidget(openButton);

        horizontalSpacer = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pauseButton = new QPushButton(horizontalLayoutWidget);
        pauseButton->setObjectName(QStringLiteral("pauseButton"));
        pauseButton->setMinimumSize(QSize(150, 50));
        pauseButton->setMaximumSize(QSize(150, 50));

        horizontalLayout->addWidget(pauseButton);

        horizontalSpacer_2 = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        Trail = new QPushButton(horizontalLayoutWidget);
        Trail->setObjectName(QStringLiteral("Trail"));
        Trail->setMinimumSize(QSize(150, 50));
        Trail->setMaximumSize(QSize(150, 50));

        horizontalLayout->addWidget(Trail);

        horizontalSpacer_3 = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        closeButton = new QPushButton(horizontalLayoutWidget);
        closeButton->setObjectName(QStringLiteral("closeButton"));
        sizePolicy.setHeightForWidth(closeButton->sizePolicy().hasHeightForWidth());
        closeButton->setSizePolicy(sizePolicy);
        closeButton->setMinimumSize(QSize(150, 50));
        closeButton->setMaximumSize(QSize(150, 50));

        horizontalLayout->addWidget(closeButton);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);

        histogram1 = new QCustomPlot(centralWidget);
        histogram1->setObjectName(QStringLiteral("histogram1"));
        histogram1->setGeometry(QRect(1030, 137, 425, 281));
        histogram2 = new QCustomPlot(centralWidget);
        histogram2->setObjectName(QStringLiteral("histogram2"));
        histogram2->setGeometry(QRect(1030, 572, 425, 281));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1631, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        p1label->setText(QString());
        p0label->setText(QString());
        label->setText(QString());
        head0->setText(QString());
        head1->setText(QString());
        attr0->setText(QString());
        attr1->setText(QString());
        openButton->setText(QApplication::translate("MainWindow", "open", 0));
        pauseButton->setText(QApplication::translate("MainWindow", "pause", 0));
        Trail->setText(QApplication::translate("MainWindow", "trail", 0));
        closeButton->setText(QApplication::translate("MainWindow", "close", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
