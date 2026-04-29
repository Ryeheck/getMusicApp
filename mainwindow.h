#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logView.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void allTitle();
    
private:
    void setupConnections();
    QWidget *centralWidget;
    QBoxLayout *layoutMain;
    QBoxLayout *layoutButtons;
    QHBoxLayout *layoutButtonsHOne;
    QHBoxLayout *layoutButtonsHTwo;
    logView *logs;
    QLineEdit *inputURL;
    QLineEdit *inputFolder;
    QPushButton *titleButton;
    QPushButton *startButton;
    QPushButton *clearListButton;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    QPushButton *stopButton;
    QPushButton *stopForNextButton;
};
#endif // MAINWINDOW_H