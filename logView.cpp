#include "logView.h"
#include "mainwindow.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QHBoxLayout>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>

#define MAX_SONGS   50

void logView::getLog(QPlainTextEdit *plainText, QListWidget *listWidget)
{ 
    plainText = this->logText;
    listWidget = this->listWidget;
}

QList<QListWidgetItem *> logView::getItems()
{
    return this->Items;
}

logView::logView(QWidget *parent)
{
    logText = new QPlainTextEdit(this);
    logText->setReadOnly(true);

    listWidget = new QListWidget(this);
    HLayout = new QHBoxLayout(this);

    HLayout->addWidget(logText);
    HLayout->addWidget(listWidget);
}

void logView::getTitle(QString url, QString folder, bool startAfter, bool lyrics)
{
    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, [this, url] () {
        QString output = process->readAllStandardOutput();

        QStringList Names = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i < Names.size() - 1 && (i < MAX_SONGS * 3); i += 3) {
            QListWidgetItem *item = new QListWidgetItem;
            if (Names[i + 2] == QString("NA - NA"))
                item->setText(Names[i]);
            else
                item->setText(Names[i + 2]);
            
            if (!listWidget->findItems(item->text(), Qt::MatchExactly).isEmpty()) {
                delete item;
                continue;
            }

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setForeground(Qt::white);
            item->setData(Qt::UserRole, Names[i + 1]);

            listWidget->addItem(item);
            Items.append(item);
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, startAfter, folder, lyrics] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        this->log(QString("<span style='color:silver;'>%1</span>").arg(output));
        process->deleteLater();

        if (startAfter) {
            Items[0]->setCheckState(Qt::Checked);
            startDownload(folder, lyrics);
        }
        
    });

    QString appDir = qApp->applicationDirPath();
    QStringList args;
    args << "--flat-playlist"
         << "--get-id" << "--get-filename" << "--get-title"
         << "-o" << "%(artist)s - %(track)s"
         << url;

    // yt-dlp --get-title --get-filename -o "%(artist)s - %(track)s" https://youtube.com

    process->start(appDir + "/yt-dlp", args);
}

void logView::startDownload(QString folder, bool lyrics)
{
    currentDownloadIndex = 0;
    emit setupDownloadRequested(true);

    if (folder.isEmpty())  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    
    log(QString("<span style='color:silver;'>FOLDER: %1</span>").arg(folder));

    if (lyrics) 
        lyricsDownloadForName(folder);
    else 
        nextDownload(folder);

}

void logView::nextDownload(QString folder)
{
    if (isStoppedForNext)  return;

    if (currentDownloadIndex >= Items.size()) {
        this->log(QString("<span style='color:silver;'>All Done!</span>"));
        currentDownloadIndex = -1;
        emit setupDownloadRequested(false);
        return;
    }

    QListWidgetItem *item = Items[currentDownloadIndex];
    if (item->checkState() != Qt::Checked) {
        currentDownloadIndex++;
        nextDownload(folder);
        return;
    }

    process = new QProcess(this);
    
    connect(process, &QProcess::readyReadStandardOutput, [this] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            this->log(QString("<span style='color:silver;'>INFO: </span>") + output);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING:</span>"));
        output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR:</span>"));

        this->log(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        this->log(QString("<span style='color:silver;'>%1</span>").arg(output));

        process->deleteLater();
        currentDownloadIndex++;
        nextDownload(folder);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking();

    QString songName = item->text();

    QStringList args;
    args << "--ffmpeg-location" << appDir
            << "--buffer-size" << "64K"
            << "--concurrent-fragments" << "5"
            << "--no-mtime"
            << "--no-playlist" 
            << "--newline"
            << "-x" 
            << "--audio-format" << "mp3" 
            << "--audio-quality" << "0"
            << "-o" << folder + "/" + songName + ".%(ext)s"
            << "--"
            << item->data(Qt::UserRole).toString();

    log(QString("<span style='color:silver;'>Song: %1</span>").arg(songName));
    
    process->start(appDir + "/" + "yt-dlp", args);
}

/*
void logView::nextDowload(QString folder, QString processName)
{
    if (isStoppedForNext)  return;

    if (currentDowloadIndex >= Items.size()) {
        this->log(QString("<span style='color:%1;'>%2</span>").arg("white", "All Done!"));
        currentDowloadIndex = -1;
        return;
    }

    QListWidgetItem *item = Items[currentDowloadIndex];
    if (item->checkState() != Qt::Checked) {
        currentDowloadIndex++;
        nextDowload(folder, processName);
        return;
    }

    process = new QProcess(this);
    
    connect(process, &QProcess::readyReadStandardOutput, [this] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            this->log(QString("<span style='color:%1;'>%2</span>").arg("white", "INFO: ") + output);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        this->log(QString("<span style='color:%1;'>%2</span>").arg("red", "ERROR: ") + output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder, processName] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", output);
        this->log(message + output);

        process->deleteLater();
        currentDowloadIndex++;
        nextDowload(folder, processName);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking();

    QString songName = item->text();

    QStringList args;

    if (processName == "yt-dlp") {
        args << "--ffmpeg-location" << appDir
            << "--buffer-size" << "64K"
            << "--concurrent-fragments" << "5"
            << "--no-mtime"
            << "--no-playlist" 
            << "--newline"
            << "-x" 
            << "--audio-format" << "mp3" 
            << "--audio-quality" << "0"
            << "-o" << folder + "/" + songName + ".%(ext)s"
            << "--"
            << item->data(Qt::UserRole).toString();
    } 
    if (processName == "syncedlyrics_bin") {
        args << songName
             << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
             << "-o" << folder + "/" + songName + ".lrc"
             << "--verbose";
    }

    log(QString("<span style='color:%1;'>%2</span>").arg("white", processName == "yt-dlp" ? "Song: " : "Lyrics: " + songName));
    
    process->start(appDir + "/" + processName, args);
}
*/

void logView::setWorking()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString appDir = qApp->applicationDirPath();

    env.insert("PATH", appDir + ":" + env.value("PATH"));
    env.remove("LD_LIBRARY_PATH");
    env.insert("LD_LIBRARY_PATH", ""); 
    process->setProcessEnvironment(env);
    process->setWorkingDirectory(QDir::tempPath());
}

void logView::setSelectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Checked);
    }
    update();
}

void logView::stopDownload()
{
    isStoppedForNext = true;
    if(process && process->state() == QProcess::Running) {
        process->kill();
        process->waitForFinished(2000);
    }
    process->deleteLater();
    process = nullptr;

    log(QString("<span style='color:silver;'>User killed process</span>"));
}

void logView::setIsStoppedForNext(bool set)
{
    this->isStoppedForNext = set;
}

void logView::setDeselectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Unchecked);
    }
    update();
}

int logView::getItemsCount()
{
    return Items.size();
}

void logView::clearAll()
{
    listWidget->clear();
    Items.clear();
}

/*
void logView::clearSelect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Checked)  
    }
}

void logView::clearDeselect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Unchecked)  
    }
}
*/

void logView::log(const QString message)
{
    logText->setReadOnly(true);

    if (logText) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        logText->appendHtml("[" + time + "] " + message);
    }
}
