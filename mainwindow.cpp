#include "mainwindow.h"
#include "logView.h"
#include "downloadManager.h"
#include "settingDialog.h"

#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layoutMain        = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget);
    layoutButtons     = new QVBoxLayout();
    layoutButtonsHOne = new QHBoxLayout();
    layoutButtonsHTwo = new QHBoxLayout();

    inputFolder = new QLineEdit(this);
    inputURL    = new QLineEdit(this);
    inputFolder->setPlaceholderText("Enter folder... (default: system): ");
    inputURL->setPlaceholderText("Enter url... (only youtube)");

    logs    = new logView(this);
    manager = new downloadManager(this);
    
    settingDialog diag(this);
    manager->setFormats(diag.getAudioFormat(), diag.getVideoFormat(), diag.getLyricsFormat(),
                        diag.getVideoQuality(), diag.getAudioQuality());
    manager->setCookies(diag.getCookiesBrowser());

    lyricsButton  = new QPushButton("Lyric download(s)", this);
    musicButton   = new QPushButton("Music download(s)", this);
    titleButton   = new QPushButton("Playlist", this);
    settingButton = new QPushButton("Setting", this);
    videoButton   = new QPushButton("Video download(s)", this);

    clearListButton   = new QPushButton("Clear all", this);
    selectAllButton   = new QPushButton("Select All", this);
    deselectAllButton = new QPushButton("Deselect All", this);

    clearListButton->hide();
    selectAllButton->hide();
    deselectAllButton->hide();

    stopButton        = new QPushButton("Stop", this);
    stopForNextButton = new QPushButton("Stop for next", this);

    stopForNextButton->hide();
    stopButton->hide();

    layoutButtons->setSpacing(3);

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    layoutMain->addWidget(logs);

    layoutButtonsHOne->addWidget(videoButton, 4);
    layoutButtonsHOne->addWidget(musicButton, 4);
    layoutButtonsHOne->addWidget(lyricsButton, 4);

    layoutButtonsHTwo->addWidget(settingButton, 4);
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
        logs->log("Wait...");
        
        manager->getMedia(inputURL->text());

        setupBeforeDownload(false);
        stopButton->show();
    }); 
    
    connect(settingButton, &QPushButton::clicked, [this] () {
        settingDialog diag(this);

        if(diag.exec() == QDialog::Accepted) {
            manager->setFormats(diag.getAudioFormat(), diag.getVideoFormat(), diag.getLyricsFormat(),
                                diag.getVideoQuality(), diag.getAudioQuality());
            manager->setCookies(diag.getCookiesBrowser());
        }
    });

    connect(musicButton, &QPushButton::clicked, [this] () {
        handleDownload(true);
    });

    connect(videoButton, &QPushButton::clicked, [this] () {
        handleDownload();
    });

    connect(selectAllButton, &QPushButton::clicked, [this] () {
        logs->setSelectAllItem();
    });    

    connect(deselectAllButton, &QPushButton::clicked, [this] () {
        logs->setDeselectAllItem();
    });

    connect(clearListButton, &QPushButton::clicked, [this] () {
        logs->clearAll();
        manager->clearMedia();
    });  

    connect(stopButton, &QPushButton::clicked, [this] () {
        manager->stopDownload();
        setupBeforeDownload(false);
    });

    connect(stopForNextButton, &QPushButton::clicked, [this] () {
        manager->setIsStopped(true);
        setupBeforeDownload(false);
    });

    connect(lyricsButton, &QPushButton::clicked, [this] () {
        handleDownload(false, true);
    });

    connect(manager, &downloadManager::progressBarRequested, logs, &logView::updateProgressBar);
    connect(manager, &downloadManager::logMessageRequested, logs, &logView::log);

    connect(manager, &downloadManager::activeTasksCountChanged, this, 
            [this] (const int count) {
        if (count > 0) {
            setupBeforeDownload(true);
        } else if (count == 0) {
            setupBeforeDownload(false);
            logs->log("All Done!");   
        }
    });

    connect(manager, &downloadManager::updateStatusRequested, this, 
            [this] (const QString &id, const QString &status) {
        int row = logs->findRowById(id);
        logs->updateStatus(row, status);
    });

    connect(manager, &downloadManager::mediaAdded, this, 
            [this] (const mediaInfo *media) {
        logs->addItem(media);
    });
}

void MainWindow::handleDownload(bool isSongs, bool isLyrics)
{
    logs->log("Wait...");

    QString url = inputURL->text();
    QString folder = inputFolder->text();

    if (url.isEmpty())  url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ";
    if (folder.isEmpty() && isSongs)  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    else if (folder.isEmpty())        folder = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    manager->setIsStopped(false);
    
    for(QTableWidgetItem *item : logs->getItemsFromColumn(0)) 
    {
        QString id = item->data(Qt::UserRole).toString();
        bool isChecked = item->checkState() == Qt::Checked;

        manager->updateSongCheckState(id, isChecked);
    }

    if (!logs->getTableWidgetCount())
        manager->getMedia(url, folder, true, isSongs, isLyrics);
    else                         
        manager->startDownload(folder, isSongs, isLyrics);
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

