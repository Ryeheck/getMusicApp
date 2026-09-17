#include "mainwindow.h"
#include "logView.h"
#include "../core/downloadManager.h"
#include "settingDialog.h"

#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QTranslator>
#include <QApplication>
#include <qaction.h>
#include <qmenu.h>
#include <qobject.h>
#include <qtoolbutton.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layoutMain     = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget);
    layoutBtns     = new QVBoxLayout();
    layoutBtnsHOne = new QHBoxLayout();
    layoutBtnsHTwo = new QHBoxLayout();

    inputFolder = new QLineEdit(this);
    inputURL    = new QLineEdit(this);

    logs    = new logView(this);
    manager = new downloadManager(this);
    
    settingDialog diag(this);
    manager->setFormats(diag.getAudioFormat(), diag.getVideoFormat(), diag.getLyricsFormat(),
                        diag.getVideoQuality(), diag.getAudioQuality());
    manager->setCookies(diag.getCookiesBrowser());
    manager->setJavaScript(diag.getJSRuntime());
    
    // Translator
    m_translator = new QTranslator(this);


    lyricsBtn  = new QPushButton(this);
    musicBtn   = new QPushButton(this);
    titleBtn   = new QPushButton(this);
    settingBtn = new QPushButton(this);
    videoBtn   = new QPushButton(this);

    // Menu
    toolBtn = new QToolButton(this);
    toolBtn->setPopupMode(QToolButton::InstantPopup);
    toolBtn->setFixedSize(90, 30);

    menuTool = new QMenu(this);

    
    logsAction       = menuTool->addAction(tr("Show logs"));
    logsAction->setCheckable(true);

    clearAll       = menuTool->addAction(tr("Clear all"));
    checkForUpdate = menuTool->addAction(tr("Check for update"));
    changeLanguage = menuTool->addAction(tr("Change language"));
    prepareProgram = menuTool->addAction(tr("Prepare program"));
    
    toolBtn->setMenu(menuTool);

    // Menu media
    mediaToolBtn = new QToolButton(this);
    mediaToolBtn->setPopupMode(QToolButton::InstantPopup);
    mediaToolBtn->setFixedSize(90, 30);

    menuMedia = new QMenu(this);

    selectAllMedia   = menuMedia->addAction(tr("Select all media"));
    deselectAllMedia = menuMedia->addAction(tr("Deselect all media"));
    clearMedia       = menuMedia->addAction(tr("Clear media"));

    mediaToolBtn->setMenu(menuMedia);

    // Stop buttons
    stopBtn        = new QPushButton(this);
    stopForNextBtn = new QPushButton(this);
    
    stopForNextBtn->hide();
    stopBtn->hide();

    layoutBtns->setSpacing(3);

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    layoutMain->addWidget(logs);
    layoutMain->addWidget(mediaToolBtn);
    layoutBtnsHOne->addWidget(videoBtn,  4);
    layoutBtnsHOne->addWidget(musicBtn,  4);
    layoutBtnsHOne->addWidget(lyricsBtn, 4);
    
    layoutBtnsHOne->addStretch();
    layoutBtnsHOne->addWidget(mediaToolBtn);
    
    layoutBtnsHTwo->addWidget(settingBtn, 4);
    layoutBtnsHTwo->addWidget(titleBtn,   4);
 
    layoutBtnsHTwo->addStretch();
    layoutBtnsHTwo->addWidget(stopBtn);
    layoutBtnsHTwo->addWidget(stopForNextBtn);
    layoutBtnsHTwo->addWidget(toolBtn);

    layoutBtns->addLayout(layoutBtnsHOne);
    layoutBtns->addLayout(layoutBtnsHTwo);

    layoutMain->addLayout(layoutBtns);

    /*
    EXAMPLE:
    
    QPushButton *newbtn = new QPushButton("delete");
    logs->addItem("nirvana", "40vm", "deleted", newbtn);

    connect(newbtn, &QPushButton::clicked, [this] () {
        logs->removeAlso(0);
    });
    */

    retranslateUI();

    connect(titleBtn, &QPushButton::clicked, [this] () {
        logs->appendText("Wait...");
        
        manager->getMedia(inputURL->text());

        setupBeforeDownload(false);
        stopBtn->show();
    }); 
    
    connect(settingBtn, &QPushButton::clicked, [this] () {
        settingDialog diag(this);
        if(diag.exec() == QDialog::Accepted) {
            manager->setFormats(diag.getAudioFormat(),  diag.getVideoFormat(), diag.getLyricsFormat(),
                                diag.getVideoQuality(), diag.getAudioQuality());
            manager->setCookies(diag.getCookiesBrowser());
            manager->setJavaScript(diag.getJSRuntime());
        }
    });

    connect(musicBtn,  &QPushButton::clicked, [this] () {  handleDownload(true);  });
    connect(videoBtn,  &QPushButton::clicked, [this] () {  handleDownload();  });
    connect(lyricsBtn, &QPushButton::clicked, [this] () {  handleDownload(false, true);  });

    connect(selectAllMedia,   &QAction::triggered,   logs, &logView::setSelectAllItem        );
    connect(changeLanguage,   &QAction::triggered,   this, &MainWindow::switchLanguageClicked);
    connect(logsAction,       &QAction::toggled,     this, &MainWindow::onLogsToggled        );
    connect(deselectAllMedia, &QAction::triggered,   logs, &logView::setDeselectAllItem      );
    
    connect(clearMedia,  &QAction::triggered, [this] () {  
        logs->clearTitle();  
        manager->clearMedia();
    });
    connect(clearAll,   &QAction::triggered, [this] () {
        logs->clearAll();
        manager->clearMedia();
    });  
    connect(checkForUpdate, &QAction::triggered, [this] () {  
        manager->checkAndPrepareFiles();
        setupBeforeDownload(false);
        stopBtn->show();
    });

    connect(stopBtn, &QPushButton::clicked, [this] () {
        manager->stopDownload();
        setupBeforeDownload(false);
    });
    connect(stopForNextBtn, &QPushButton::clicked, [this] () {
        manager->setIsStopped(true);
        setupBeforeDownload(false);
    });

    connect(manager, &downloadManager::mediaAdded, this, [this] (const mediaInfo *media) {  logs->addItem(media);  });
    connect(manager, &downloadManager::pBarRequested, logs, &logView::updatePBar);
    connect(manager, &downloadManager::logMessageRequested, logs, &logView::log);
    connect(manager, &downloadManager::messageRequested, logs, &logView::appendText);
    connect(manager, &downloadManager::colorLogMessageRequested, logs, &logView::colorLog);
    connect(manager, &downloadManager::activeTasksCountChanged, this, 
            [this] (const int count) {
        if (count > 0) {
            setupBeforeDownload(true);
        } else if (count == 0) {
            setupBeforeDownload(false);
            logs->appendText(tr("The task has been completed"));   
        }
    });

    connect(manager, &downloadManager::updateStatusRequested, this, 
            [this] (const QString &id, const QString &status) {
        int row = logs->findRowById(id);
        logs->updateStatus(row, status);
    });
}

void MainWindow::retranslateUI()
{
    logs->retranslateUI();

    inputFolder->setPlaceholderText(tr("Enter folder... (default: system): "));
    inputURL->setPlaceholderText(tr("Enter url... (only youtube)"));
    
    settingBtn->setText(tr("Setting"));
    titleBtn->setText(tr("Playlist"));
    musicBtn->setText(tr("Music download(s)"));
    toolBtn->setText(tr("Menu"));
    mediaToolBtn->setText(tr("Media menu"));

    clearAll->setText(tr("Clear all"));
    selectAllMedia->setText(tr("Select all media"));
    deselectAllMedia->setText(tr("Deselect all media"));
    logsAction->setText(tr("Show logs"));
    checkForUpdate->setText(tr("Check for update"));
    changeLanguage->setText(tr("Change language"));
    prepareProgram->setText(tr("Prepare program"));
    clearMedia->setText(tr("Clear media"));

    stopBtn->setText(tr("Stop"));
    stopForNextBtn->setText(tr("Stop for next"));
    lyricsBtn->setText(tr("Lyric download(s)"));
    videoBtn->setText(tr("Video download(s)"));
}

void MainWindow::switchLanguageClicked()
{
    if (!m_russian) {
        if (m_translator->load(":/i18n/translate_ru.qm")) {
            qApp->installTranslator(m_translator);
            m_russian = true;
        }
    } else {
        qApp->removeTranslator(m_translator);
        m_russian = false;
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUI();

    QMainWindow::changeEvent(event);
}

void MainWindow::handleDownload(bool isSongs, bool isLyrics)
{
    logs->appendText("Wait...");

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
    if (!url.isEmpty()) {
        manager->getMedia(url, folder, true, isSongs, isLyrics);
    } else if (!logs->getTableWidgetCount()) {
        manager->startDownload(folder, isSongs, isLyrics);
    } else if (url.isEmpty()) {
        logs->appendText(tr("Not URL"));
    }
    
}

void MainWindow::setupBeforeDownload(bool set)
{
    if (set) {
        toolBtn->hide();

        stopBtn->show();
        stopForNextBtn->show();
    } else {
        stopBtn->hide();
        stopForNextBtn->hide();

        toolBtn->show();
    }
}

void MainWindow::onLogsToggled(bool checked)
{
    if (checked) {
        logsAction->setText(tr("Hide logs"));
        logs->showLogText();
    
    } else {
        logsAction->setText(tr("Show logs"));
        logs->hideLogText();

    }
}
MainWindow::~MainWindow()
{
    qDebug() << "ok";
}

