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

    MainWindow wn_main;
    //QListWidget *listWidget = new QListWidget(&wn_main);
    /*
    // Создаем элемент списка
    QListWidgetItem* item = new QListWidgetItem("Rick Astley - Never Gonna Give You Up", listWidget);

    // Включаем возможность ставить галочку
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); 

    // Устанавливаем начальное состояние (снята)
    item->setCheckState(Qt::Unchecked); 

    // Можно добавить цвет, как мы обсуждали раньше
    item->setForeground(Qt::blue); 
    */

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
    //listWidget->show();
    
    wn_main.show();

    return a.exec();
}
