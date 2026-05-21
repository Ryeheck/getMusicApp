#include "mainwindow.h"
#include "logView.h"
#include "downloadManager.h"

#include <QString>
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
    inputFolder->setPlaceholderText("Enter folder... (default: system): ");
    inputURL->setPlaceholderText("Enter url... (only youtube)");

    logs = new logView(this);
    manager = new downloadManager(this);

    lyricsButton = new QPushButton("Lyrics dowload(s)", this);
    musicButton = new QPushButton("Music dowload(s)", this);
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

    layoutButtonsHOne->addWidget(musicButton, 4);
    layoutButtonsHOne->addWidget(lyricsButton, 4);
    layoutButtonsHTwo->addWidget(titleButton, 4);
    
    layoutButtonsHOne->addStretch();
    layoutButtonsHOne->addWidget(selectAllButton);
    layoutButtonsHOne->addWidget(deselectAllButton);

    layoutButtonsHTwo->addStretch();
    layoutButtonsHTwo->addWidget(stopButton);
    layoutButtonsHTwo->addWidget(clearListButton);
    layoutButtonsHTwo->addWidget(stopForNextButton);

    layoutButtons->addLayout(layoutButtonsHOne);
    layoutButtons->addLayout(layoutButtonsHTwo);

    layoutMain->addLayout(layoutButtons);

    setupConnections();

    /*
    ПРИМЕР:
    
    QPushButton *newbtn = new QPushButton("delete");
    logs->addItem("nirvana", "40vm", "deleted", newbtn);

    connect(newbtn, &QPushButton::clicked, [this] () {
        logs->removeAlso(0);
    });
    */
}

void MainWindow::setupConnections()
{
    connect(titleButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(titleButton->text()));
        logs->log("Wait...");
        
        manager->getSongs(inputURL->text());

        setupBeforeDownload(false);
        stopButton->show();
    }); 
    
    connect(musicButton, &QPushButton::clicked, [this] () {
        handleDownload(musicButton, false);
    });

    connect(selectAllButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(selectAllButton->text()));

        logs->setSelectAllItem();
    });    

    connect(deselectAllButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(deselectAllButton->text()));

        logs->setDeselectAllItem();
    });

    connect(clearListButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(clearListButton->text()));

        logs->clearAll();
        manager->clearSongs();
    });  

    connect(stopButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(stopButton->text()));

        manager->stopDownload();
        setupBeforeDownload(false);
    });

    connect(stopForNextButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(stopForNextButton->text()));

        manager->setIsStopped(true);
        setupBeforeDownload(false);
    });

    connect(lyricsButton, &QPushButton::clicked, [this] () {
        handleDownload(lyricsButton, true);
    });

    connect(manager, &downloadManager::progressBarRequested, logs, &logView::updateProgressBar);
    connect(manager, &downloadManager::setupDownloadRequested, this, &MainWindow::setupBeforeDownload);
    connect(manager, &downloadManager::logMessageRequested, logs, &logView::log);

    connect(manager, &downloadManager::updateStatusRequested, this, 
            [this] (const songInfo &song) {
        int row = logs->findRowById(song.id);
        logs->updateStatus(row, song.status);
    });

    connect(manager, &downloadManager::songAdded, this, 
            [this] (const songInfo *song) {
        logs->addItem(song);
    });
}

void MainWindow::handleDownload(QPushButton *button, bool isLyrics)
{
    logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(button->text()));
    logs->log("Wait...");

    QString url = inputURL->text();
    QString folder = inputFolder->text();

    if (url.isEmpty())  url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ";

    manager->setIsStopped(false);

    for(QTableWidgetItem *item : logs->getItemsFromColumn(0)) 
    {
        QString id = item->data(Qt::UserRole).toString();
        bool isChecked = item->checkState() == Qt::Checked;

        manager->updateSongCheckState(id, isChecked);
    }

    if (!logs->getTableWidgetCount())
        manager->getSongs(url, folder, true, isLyrics);
    else                         
        manager->startDownload(folder, isLyrics);
}

void MainWindow::setupBeforeDownload(bool set)
{
    if (set) {
        clearListButton->hide();
        selectAllButton->hide();
        deselectAllButton->hide();

        stopButton->show();
        stopForNextButton->show();
    } else {
        stopButton->hide();
        stopForNextButton->hide();

        clearListButton->show();
        selectAllButton->show();
        deselectAllButton->show();
    }
}

MainWindow::~MainWindow()
{
    qDebug() << "ok";
}

