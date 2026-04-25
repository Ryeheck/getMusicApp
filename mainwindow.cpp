#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "logView.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layoutMain = new QVBoxLayout(centralWidget);

    inputFolder = new QLineEdit(this);
    inputURL = new QLineEdit(this);
    inputFolder->setPlaceholderText("Enter folder... (default: Music/songs): ");
    inputURL->setPlaceholderText("Enter URL (youtube)");

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    logs = new logView(this);
    layoutMain->addWidget(logs);

    startButton = new QPushButton("Start dowload(s)", this);
    titleButton = new QPushButton("Playlist", this);

    layoutMain->addWidget(startButton);
    layoutMain->addWidget(titleButton);

    clearListButton = new QPushButton("Clear all", this);
    selectAllButton = new QPushButton("Select All", this);
    deselectAllButton = new QPushButton("Deselect All", this);

    clearListButton->hide();
    selectAllButton->hide();
    deselectAllButton->hide();

    layoutMain->addWidget(selectAllButton, 0, Qt::AlignRight);
    layoutMain->addWidget(deselectAllButton, 0, Qt::AlignRight);
    layoutMain->addWidget(clearListButton, 0, Qt::AlignRight);

    setupConnections();
}

void MainWindow::setupConnections()
{
    QString message;
    connect(titleButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", titleButton->text());
        logs->log(message);

        logs->getTitle(inputURL->text());
        clearListButton->show();
        selectAllButton->show();
        deselectAllButton->show();

    }); 
    
    connect(startButton, &QPushButton::clicked, [this, &message] () {
        message = QString("<span style='color:%1;'>: You clicked to %2</span>").arg("white", startButton->text());
        logs->log(message);

        selectAllButton->hide();
        deselectAllButton->hide();

        QString folder = inputFolder->text();
        if (folder.isEmpty())  logs->startDowload();
        else                   logs->startDowload(folder);
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
}

MainWindow::~MainWindow()
{
    qDebug() << "ok";
}

