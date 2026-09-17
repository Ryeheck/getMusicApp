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
#include <qobject.h>

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

    logsToolBtn = new QToolButton(this);
    logsToolBtn->setPopupMode(QToolButton::MenuButtonPopup);

    menuLogsBtns = new QMenu(this);

    deselectAllAction = menuLogsBtns->addAction("");
    logsAction        = menuLogsBtns->addAction("");
    clearTitleAction  = menuLogsBtns->addAction("");
    clearListAction   = menuLogsBtns->addAction("");
    checkForUpdate    = menuLogsBtns->addAction("");
    switchLanguage    = menuLogsBtns->addAction("");
    prepareProgram    = menuLogsBtns->addAction("");

    logsAction->setCheckable(true);
    logsToolBtn->setMenu(menuLogsBtns);
    
    stopBtn        = new QPushButton(this);
    stopForNextBtn = new QPushButton(this);
    
    stopForNextBtn->hide();
    stopBtn->hide();

    layoutBtns->setSpacing(3);

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    layoutMain->addWidget(logs);

    layoutBtnsHOne->addWidget(videoBtn,  4);
    layoutBtnsHOne->addWidget(musicBtn,  4);
    layoutBtnsHOne->addWidget(lyricsBtn, 4);

    layoutBtnsHTwo->addWidget(settingBtn, 4);
    layoutBtnsHTwo->addWidget(titleBtn,   4);
    
    layoutBtnsHOne->addStretch();
    layoutBtnsHOne->addWidget(logsToolBtn);

    layoutBtnsHTwo->addStretch();
    layoutBtnsHTwo->addWidget(stopBtn);
    layoutBtnsHTwo->addWidget(stopForNextBtn);

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

    

    connect(titleBtn, &QPushButton::clicked,   [this] () {
        logs->appendText(tr("Wait..."));
        manager->getMedia(inputURL->text());
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

    connect(musicBtn,  &QPushButton::clicked, [this] () {  handleDownload(true);                   });
    connect(videoBtn,  &QPushButton::clicked, [this] () {  handleDownload();                               });
    connect(lyricsBtn, &QPushButton::clicked, [this] () {  handleDownload(false, true);  });

    connect(logsToolBtn,       &QToolButton::clicked, logs, &logView::setSelectAllItem        );
    connect(switchLanguage,    &QAction::triggered,   this, &MainWindow::switchLanguageClicked);
    connect(logsAction,        &QAction::toggled,     this, &MainWindow::onLogsToggled        );
    connect(deselectAllAction, &QAction::triggered,   logs, &logView::setDeselectAllItem      );
    
    connect(clearTitleAction,  &QAction::triggered, [this] () {  
        logs->clearTitle();  
        manager->clearMedia();
    });
    connect(clearListAction,   &QAction::triggered, [this] () {
        logs->clearAll();
        manager->clearMedia();
        inputURL->clear();
    });  

    connect(prepareProgram, &QAction::triggered, manager, &downloadManager::checkAndPrepareFiles);
    connect(checkForUpdate, &QAction::triggered, manager, &downloadManager::updateYtDlp         );

    connect(stopBtn, &QPushButton::clicked,        manager, &downloadManager::stopDownload);
    connect(stopForNextBtn, &QPushButton::clicked, manager, &downloadManager::setIsStopped);
    
    connect(manager, &downloadManager::setMediaCheckedRequested, logs, &logView::setMediaCheckedById);
    connect(manager, &downloadManager::updateStatusRequested,    logs, &logView::updateStatusById   );
    connect(manager, &downloadManager::mediaAdded,               logs, &logView::addItem            );
    connect(manager, &downloadManager::pBarRequested,            logs, &logView::updatePBar         );
    connect(manager, &downloadManager::logMessageRequested,      logs, &logView::log                );
    connect(manager, &downloadManager::messageRequested,         logs, &logView::appendText         );
    connect(manager, &downloadManager::colorLogMessageRequested, logs, &logView::colorLog           );
    connect(manager, &downloadManager::activeTasksCountChanged,  this, 
            [this] (const int count) {
        if (count > 0) {
            setupBeforeDownload(true);
        } else if (count == 0) {
            setupBeforeDownload(false);
            logs->appendText(tr("All Done!"));   
        }
    });

    
    
    prepareProgram->trigger();
    retranslateUI();
}

void MainWindow::retranslateUI()
{
    logs->retranslateUI();

    inputFolder->setPlaceholderText(tr("Enter folder... (default: Movies/Video/Music): "));
    inputURL->setPlaceholderText(tr("Enter url... (only youtube)"));
    
    settingBtn->setText(tr("Setting"));
    titleBtn->setText(tr("Playlist"));
    musicBtn->setText(tr("Music download(s)"));
    logsToolBtn->setText(tr("Select all"));
    
    clearListAction->setText(tr("Clear all"));
    deselectAllAction->setText(tr("Deselect all with media"));
    logsAction->setText(tr("Show logs"));
    clearTitleAction->setText(tr("Clear media"));
    checkForUpdate->setText(tr("Check for update program"));
    switchLanguage->setText(tr("Switch language"));
    prepareProgram->setText(tr("Prepare program"));

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
    logs->appendText(tr("Wait..."));

    QString url    = inputURL->text();
    QString folder = inputFolder->text();
    
    inputURL->clear();
    if (folder.isEmpty() && (isSongs || isLyrics))  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation );
    else if (folder.isEmpty())                      folder = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    manager->setIsStopped(false);
    
    for(QTableWidgetItem *item : logs->getItemsFromColumn(0)) 
    {
        QString id = item->data(Qt::UserRole).toString();
        bool isChecked = item->checkState() == Qt::Checked;

        manager->updateSongCheckState(id, isChecked);
    }
    if (!logs->getTableWidgetCount()) {
        if (!url.isEmpty()) {
            manager->getMedia(url, folder, true, isSongs, isLyrics);
        }
    } else {
        manager->startDownload(folder, isSongs, isLyrics);
    }
    
}

void MainWindow::setupBeforeDownload(bool set)
{
    if (set) {
        logsToolBtn->hide();

        stopBtn->show();
        stopForNextBtn->show();
    } else {
        stopBtn->hide();
        stopForNextBtn->hide();

        logsToolBtn->show();
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

