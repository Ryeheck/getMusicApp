#include "logView.h"

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

void logView::getTitle(QString url, bool startAfter, QString folder)
{
    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, [this, process, url] () {
        QString output = process->readAllStandardOutput();

        QStringList Names = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i < Names.size() - 1 && (i < MAX_SONGS * 3); i += 3) {
            QListWidgetItem *item = new QListWidgetItem("", listWidget);
            if (Names[i + 2] == QString("NA - NA"))
                item->setText(Names[i]);
            else
                item->setText(Names[i + 2]);
            
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setForeground(Qt::white);
            item->setData(Qt::UserRole, Names[i + 1]);

            Items.append(item);
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, startAfter, folder] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", output);
        this->log(message);

        if (startAfter) {
            Items[0]->setCheckState(Qt::Checked);
            startDowload(folder, lyrics);
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

void logView::startDowload(QString folder, bool lyrics)
{
    currentDowloadIndex = 0;

    if (folder.isEmpty())  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    
    QString message = QString("<span style='color:%1;'>%2</span>").arg("white", "FOLDER: " + folder);
    log(message);

    if (lyrics)  lyricsDowloadForName(folder);
    else         nextDowload(folder);

}

void logView::nextDowload(QString folder)
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
        nextDowload(folder);
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

    connect(process, &QProcess::readyReadStandardError, [this]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        this->log(QString("<span style='color:%1;'>%2</span>").arg("red", "ERROR: ") + output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        this->log(QString("<span style='color:%1;'>%2</span>").arg("white", output));

        process->deleteLater();
        currentDowloadIndex++;
        nextDowload(folder);
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

    log(QString("<span style='color:%1;'>%2</span>").arg("white", ": " + songName));
    
    process->start(appDir + "/yt-dlp", args);
}

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

void logView::stopDowload()
{
    isStoppedForNext = true;
    if(process && process->state() == QProcess::Running)
        process->kill();

    QString message = QString("<span style='color:%1;'>%2</span>").arg("white", ": User killed process");
    log(message);
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

void logView::setLyrics(bool set)
{
    lyrics = set;
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

void logView::log(const QString &message)
{
    logText->setReadOnly(true);

    if (logText) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        logText->appendHtml("[" + time + "] " + message);
    }
}
