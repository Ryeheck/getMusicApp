#include "mainwindow.h"
#include "logView.h"
#include "downloadManager.h"
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
#include <qcoreevent.h>
#include <qguiapplication.h>
#include <qmainwindow.h>
#include <qtranslator.h>

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
    inputFolder->setPlaceholderText("Enter folder... (default: system): ");
    inputURL->setPlaceholderText("Enter url... (only youtube)");

    logs    = new logView(this);
    manager = new downloadManager(this);
    
    settingDialog diag(this);
    manager->setFormats(diag.getAudioFormat(), diag.getVideoFormat(), diag.getLyricsFormat(),
                        diag.getVideoQuality(), diag.getAudioQuality());
    manager->setCookies(diag.getCookiesBrowser());
    manager->setJavaScript(diag.getJSRuntime());
    
    // Translator
    m_translator = new QTranslator(this);


    lyricsBtn  = new QPushButton("Lyric download(s)", this);
    musicBtn   = new QPushButton("Music download(s)", this);
    titleBtn   = new QPushButton("Playlist", this);
    settingBtn = new QPushButton("Setting", this);
    videoBtn   = new QPushButton("Video download(s)", this);

    logsToolBtn = new QToolButton(this);
    logsToolBtn->setText("Select all");
    logsToolBtn->setPopupMode(QToolButton::MenuButtonPopup);

    menuLogsBtns = new QMenu(this);
    deselectAllAction = menuLogsBtns->addAction("Deselect all");
    logsAction        = menuLogsBtns->addAction("Show logs");
    clearTitleAction  = menuLogsBtns->addAction("Clear title");
    clearListAction   = menuLogsBtns->addAction("Clear all");
    checkForUpdate    = menuLogsBtns->addAction("Check and prepare program");
    switchLanguage    = menuLogsBtns->addAction("Switch language");

    logsAction->setCheckable(true);
    logsToolBtn->setMenu(menuLogsBtns);
    
    stopBtn        = new QPushButton("Stop", this);
    stopForNextBtn = new QPushButton("Stop for next", this);
    
    stopForNextBtn->hide();
    stopBtn->hide();

    layoutBtns->setSpacing(3);

    layoutMain->addWidget(inputFolder);
    layoutMain->addWidget(inputURL);

    layoutMain->addWidget(logs);

    layoutBtnsHOne->addWidget(videoBtn, 4);
    layoutBtnsHOne->addWidget(musicBtn, 4);
    layoutBtnsHOne->addWidget(lyricsBtn, 4);

    layoutBtnsHTwo->addWidget(settingBtn, 4);
    layoutBtnsHTwo->addWidget(titleBtn, 4);
    
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

    connect(logsToolBtn,   &QToolButton::clicked, [this] () {  logs->setSelectAllItem();  });

    connect(switchLanguage, &QAction::toggled, this, &MainWindow::switchLanguageClicked);
    connect(logsAction, &QAction::toggled, this, &MainWindow::onLogsToggled);
    connect(deselectAllAction, &QAction::triggered, [this] () {  logs->setDeselectAllItem();  });
    connect(clearTitleAction,  &QAction::triggered, [this] () {  
        logs->clearTitle();  
        manager->clearMedia();
    });
    connect(clearListAction,   &QAction::triggered, [this] () {
        logs->clearAll();
        manager->clearMedia();
    });  
    connect(checkForUpdate, &QAction::triggered, [this] () {  
        manager->checkAndPrepareFiles();
        setupBeforeDownload(true);
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
            logs->appendText("All Done!");   
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
    inputFolder->setText(tr("Введите папку загрузки (по умолчанию: система):"));
    inputURL->setText(tr("Введите ссылку:"));
    settingBtn->setText(tr("Настройки"));
    logsToolBtn->setText(tr("Логи"));
    clearListAction->setText(tr("Очистить список"));
    deselectAllAction->setText(tr("Убрать галочки"));
    selectAllAction->setText(tr("Поставить галочки"));
    logsAction->setText(tr("Логи еще"));
    clearTitleAction->setText(tr("Очистить все"));
    checkForUpdate->setText(tr("Проверить обновления"));
    stopBtn->setText(tr("Стоп"));
    stopForNextBtn->setText(tr("Стоп на следующем"));
    lyricsBtn->setText(tr("Субтитры"));
    videoBtn->setText(tr("Видео"));
    
    if (!m_russian)
        switchLanguage->setText(tr("Change language"));
    else
        switchLanguage->setText(tr("Сменить язык"));
}

void MainWindow::switchLanguageClicked()
{
    if (m_russian) {
        if (m_translator->load("filename")) {
            qApp->installTranslator(m_translator);
            m_russian = false;
        }
    } else {
        qApp->removeTranslator(m_translator);
        m_russian = true;
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

    if (!logs->getTableWidgetCount())
        manager->getMedia(url, folder, true, isSongs, isLyrics);
    else                         
        manager->startDownload(folder, isSongs, isLyrics);
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
        logsAction->setText("Hide logs");
        // manager->setupProcessLogging(id, isLyrics);
        logs->showLogText();
    
    } else {
        logsAction->setText("Show logs");
        logs->hideLogText();

    }
}
MainWindow::~MainWindow()
{
    qDebug() << "ok";
}

