#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "logView.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layoutMain = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget);
    layoutButtons = new QVBoxLayout();
    layoutButtonsHOne = new QHBoxLayout();
    layoutButtonsHTwo = new QHBoxLayout();

    inputFolder = new QLineEdit(this);
    inputURL = new QLineEdit(this);
    inputFolder->setPlaceholderText("Enter folder... (default: Music): ");
    inputURL->setPlaceholderText("test 1");

    logs = new logView(this);

    startButton = new QPushButton("Start dowload(s)", this);
    titleButton = new QPushButton("Playlist", this);

    clearListButton = new QPushButton("Clear all", this);
    selectAllButton = new QPushButton("Select All", this);
    deselectAllButton = new QPushButton("Deselect All", this);

    clearListButton->hide();
    selectAllButton->hide();
    deselectAllButton->hide();

    stopButton = new QPushButton("Stop", this);
    stopForNextButton = new QPushButton("Stop for next", this);

    stopForNextButton->hide();
    stopButton->hide();
    
    layoutButtons->setSpacing(3);

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    layoutMain->addWidget(logs);

    layoutButtonsHOne->addWidget(startButton, 4);
    layoutButtonsHTwo->addWidget(titleButton, 4);

    layoutButtonsHOne->addStretch();
    layoutButtonsHOne->addWidget(selectAllButton);
    layoutButtonsHOne->addWidget(deselectAllButton);

    layoutButtonsHTwo->addStretch();
    layoutButtonsHTwo->addWidget(clearListButton);

    layoutButtonsHOne->addWidget(stopForNextButton, 0, Qt::AlignRight);
    layoutButtonsHTwo->addWidget(stopButton, 0, Qt::AlignRight);

    layoutButtons->addLayout(layoutButtonsHOne);
    layoutButtons->addLayout(layoutButtonsHTwo);

    layoutMain->addLayout(layoutButtons);

    setupConnections();
}

void MainWindow::setupConnections()
{
    QString message;

    connect(titleButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", titleButton->text());
        logs->log(message);
        logs->log("Wait...");
        
        logs->getTitle(inputURL->text());

        stopButton->hide();
        stopForNextButton->hide();

        clearListButton->show();
        selectAllButton->show();
        deselectAllButton->show();
    }); 
    
    connect(startButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", startButton->text());
        logs->log(message);
        logs->log("Wait...");

        stopButton->show();
        stopForNextButton->show();

        clearListButton->hide();
        selectAllButton->hide();
        deselectAllButton->hide();

        QString url = inputURL->text();
        QString folder = inputFolder->text();

        if (folder.isEmpty())  folder = "Music/songs";
        if (url.isEmpty())     url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ";

        logs->setIsStoppedForNext(false);

        if (!logs->getItemsCount())  logs->getTitle(url, true, folder);
        else                         logs->startDowload(folder);
    });

    connect(selectAllButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", selectAllButton->text());
        logs->log(message);

        logs->setSelectAllItem();
    });    

    connect(deselectAllButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", deselectAllButton->text());
        logs->log(message);

        logs->setDeselectAllItem();
    });

    connect(clearListButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", clearListButton->text());
        logs->log(message);

        logs->clearAll();
    });  

    connect(stopButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", stopButton->text());
        logs->log(message);

        stopButton->hide();
        stopForNextButton->hide();

        logs->stopDowload();
    });

    connect(stopForNextButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", stopForNextButton->text());
        logs->log(message);

        stopButton->hide();
        stopForNextButton->hide();

        logs->setIsStoppedForNext(true);
    });
}

MainWindow::~MainWindow()
{
    qDebug() << "ok";
}

