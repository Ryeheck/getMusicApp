#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QListWidgetItem>
#include <QListWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow windowMain;
    windowMain.resize(1280, 900);
    
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
    
    windowMain.show();

    return a.exec();
}
