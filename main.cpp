#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow wn_main;

    QPushButton *btn = new QPushButton(&wn_main);

    btn->connect(btn, &QPushButton::clicked, [=, &wn_main] () {
        wn_main.startDowload();
    });

    btn->show();
    wn_main.log("ok");

    qApp->setStyleSheet(
    "QWidget { background-color: black; color: gray; }"      // All windows
    "QPushButton { "
    "   background-color: gray; "
    "   color: white; "
    "   border: none; "
    "   min-width: 100px; "
    "   max-width: 200px; "
    "   min-height: 25px; "
    "   padding: 5px; "
    "}"                                                     // All buttons
    "QPushButton:hover { background-color: darkgray; }"    // При наведении
    );


    wn_main.show();

    return a.exec();
}
