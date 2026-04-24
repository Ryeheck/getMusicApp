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
    "QProcess { color: green; }"
    );

    wn_main.log("ok");
    wn_main.show();

    return a.exec();
}
