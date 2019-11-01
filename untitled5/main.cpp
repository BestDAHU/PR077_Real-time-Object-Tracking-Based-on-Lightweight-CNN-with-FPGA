#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
#include <tracker_main.hpp>

int main(int argc, char *argv[])
{
    tracker_init(argc, argv);
    QApplication a(argc, argv);   // create a instance; GUI program
    // tracker_main(argc,argv);
    // QLabel label("tracker");
    // label.show();
    MainWindow *w=new MainWindow;
    QApplication::setQuitOnLastWindowClosed(false);
    w->show();
    
    return a.exec();              // a.exec():turn on the event loop
}
