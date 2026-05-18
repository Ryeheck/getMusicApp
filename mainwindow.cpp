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
}

void MainWindow::setupConnections()
{
    connect(titleButton, &QPushButton::clicked, [this] () {
        logs->log(QString("<span style='color:silver;'>: You clicked to %1</span>").arg(titleButton->text()));
        logs->log("Wait...");
        
        manager->getTitle(inputURL->text());

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

    connect(manager, &downloadManager::songAdded, this, [this] (QString name, QString id) {
        QListWidgetItem *item = new QListWidgetItem(name);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setForeground(Qt::white);
        item->setData(Qt::UserRole, id);

        logs->addPlaylistItem(item);
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

    for(QListWidgetItem *item : logs->getItems()) 
    {
        QString id = item->data(Qt::UserRole).toString();
        bool isChecked = item->checkState() == Qt::Checked;

        manager->updateSongCheckState(id, isChecked);
    }

    if (!logs->getItemsCount())  
        manager->getTitle(url, folder, true, isLyrics);
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

