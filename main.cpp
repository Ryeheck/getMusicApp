#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow wn_main;

    QVBoxLayout *layout = new QVBoxLayout(&wn_main);
    
    QWidget *central = new QWidget(&wn_main);
    central->setLayout(layout);
    wn_main.setCentralWidget(central);

    wn_main.show();

    return a.exec();
}
