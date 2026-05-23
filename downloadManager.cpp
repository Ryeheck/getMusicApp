#include "downloadManager.h"
#include "logView.h"

#include <QProcess>
#include <QListWidgetItem>
#include <QApplication>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>
#include <QString>
#include <QProgressBar>

#define MAX_SONGS   50

downloadManager::downloadManager(QObject *parent)
    : QObject(parent)
{
    isStopped = false;
}

downloadManager::~downloadManager()
{
    if (currentProcess != nullptr) {
        currentProcess->kill();
        currentProcess->waitForFinished(1000);
    }
    for(songInfo *song : Songs)
    {
        if (song != nullptr)  delete song;
    }

    Songs.clear();
}

void downloadManager::getSongs(QString url, QString folder, bool startAfter, bool lyrics)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;

    connect(process, &QProcess::readyReadStandardOutput, [this, url, process] () {
        QString output = process->readAllStandardOutput();

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i + 3 < lines.size() && (i < MAX_SONGS * 4); i += 4) 
        {
            songInfo *song = new songInfo();

            if (lines[i + 3] == "NA - NA")
                song->name = lines[i + 1];
            else
                song->name = lines[i + 3];

            song->id     = lines[i + 2];
            song->size   = lines[i].toLongLong();
            song->status = "";
            song->widget = new QProgressBar();

            bool exist = false;
            for(songInfo *songItem : Songs) 
                if (songItem->id == song->id) {
                    exist = true;
                    break;
                }

            if (!exist) {
                Songs.append(song);
                emit songAdded(song);
            } else
                delete song;
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, startAfter, folder, lyrics, process] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        emit logMessageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

        if (currentProcess == process)  currentProcess = nullptr;
        
        process->deleteLater();

        if (startAfter && !Songs.isEmpty()) {
            Songs[0]->isChecked = true;
            startDownload(folder, lyrics);
        }
        
    });

    QString appDir = qApp->applicationDirPath();
    QStringList args;
    args // << "--flat-playlist"
         << "-O" << "%(filesize_approx)s\n%(title)s\n%(id)s\n%(artist)s - %(track)s"
         << url;

    // yt-dlp --flat-playlist --get-id --get-title --get-filename -o "%(artist)s - %(track)s" https://youtube.com

    process->start(appDir + "/yt-dlp", args);
}

void downloadManager::startDownload(QString folder, bool isLyrics)
{
    if (folder.isEmpty())  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    
    emit logMessageRequested(QString("<span style='color:silver;'>FOLDER: %1</span>").arg(folder));
    emit setupDownloadRequested(true);

    for(int i = 0; i < Songs.size() && !isStopped; ++i)
    {
        songInfo *song = Songs[i];

        if (song->isChecked == false)  continue;

        song->status = "Updating";
        emit updateStatusRequested(song);

        if (isLyrics)
            lyricsDownload(song, folder);
        else
            songDownload(song, folder);

        QEventLoop loop;
        if (currentProcess)
            connect(currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                    &loop, &QEventLoop::quit);

        loop.exec();

        emit updateStatusRequested(song);
    }

    emit logMessageRequested(QString("<span style='color:silver;'>All Done!</span>"));
    emit setupDownloadRequested(false);
}

void downloadManager::lyricsDownload(songInfo *song, QString folder)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;
    setWorking(process);
    
    if (QProgressBar *pBar = qobject_cast<QProgressBar *>(song->widget))
        setupProgressBar(process, pBar, &song->status);
    else 
        setupProcessLogging(process, true);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process, song] (int exitCode) {
        cleanupProcess(exitCode);

        if (!exitCode && song->status == "Updating")
            song->status = "Done";
        else if (exitCode)
            song->status = "Error";
    });

    QString appDir = qApp->applicationDirPath();
    QString songName = song->name;

    QStringList args;
    args << songName
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + ".lrc"
         << "--verbose";

    emit logMessageRequested(QString("<span style='color:silver;'>Lyrics: %1</span>").arg(songName));

    process->start(appDir + "/syncedlyrics_bin", args);

    // syncedlyrics [args] songName
}

void downloadManager::songDownload(songInfo *song, QString folder)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;
    setWorking(process);
    
    if (QProgressBar *pBar = qobject_cast<QProgressBar *>(song->widget))
        setupProgressBar(process, pBar);
    else
        setupProcessLogging(process);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process, song] (int exitCode) {
        cleanupProcess(exitCode);

        if (!exitCode && song->status == "Updating")
            song->status = "Done";
        else if (exitCode)
            song->status = "Error";
    });

    QString appDir = qApp->applicationDirPath();
    QString songName = song->name;

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
         << song->id;

    emit logMessageRequested(QString("<span style='color:silver;'>Song: %1</span>").arg(songName));

    process->start(appDir + "/yt-dlp", args);

    // yt-dlp [args] (url/id)
}

void downloadManager::cleanupProcess(int exitCode)
{
    QString output = (exitCode == 0 ? "Done!" : "Error");
    emit logMessageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

    if (currentProcess) {
        currentProcess->deleteLater();
        currentProcess = nullptr;
    }
}

void downloadManager::setupProgressBar(QProcess *process, QProgressBar *pBar, QString *status) 
{
    if (!pBar)  return;
    int *stepCount = new int(0);

    // status not working

    connect(process, &QProcess::readyReadStandardOutput, [this, process, pBar] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        QRegularExpression percentReg(R"(\[download\]\s+(\d+(?:\.\d+)?)\s*%)");
        QRegularExpressionMatch match = percentReg.match(output);

        if (match.hasMatch()) {
            int percent = static_cast<int >(match.captured(1).toFloat());
            emit progressBarRequested(pBar, percent);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this, process, pBar, stepCount, status] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        QRegularExpression percentReg(R"((continuing search|Lyrics found|No suitable lyrics found for))");
        QRegularExpressionMatch match = percentReg.match(output);

        if (match.hasMatch()) {
            QString search = match.captured(1);
            int percent = ++(*stepCount) * 25;
            
            if (search == "No suitable lyrics found for" && status) {
                percent = 100;
                *status = "Not lyric";
            } else if(search == "Lyrics found" || percent > 100)  
                percent = 100;  
            
            emit progressBarRequested(pBar, percent);
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [stepCount] (int) {
        delete stepCount;
    });
}

void downloadManager::setupProcessLogging(QProcess *process, bool isLyrics)
{
    connect(process, &QProcess::readyReadStandardOutput, [this, process, isLyrics] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            if (isLyrics)
                emit logMessageRequested(QString("<span style='color:DarkSeaGreen;'>INFO: %1</span>").arg(output));
            else
                emit logMessageRequested(QString("<span style='color:silver;'>INFO: </span>") + output);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this, process] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        if (!output.isEmpty()) {
            output.replace("DEBUG:", QString("<span style='color:silver;'>DEBUG: </span>"));
            output.replace("INFO:", QString("<span style='color:silver;'>INFO: </span>"));
            output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING:</span>"));
            output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR:</span>"));

            emit logMessageRequested(output);
        }
    });
}

void downloadManager::setWorking(QProcess *process)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString appDir = qApp->applicationDirPath();

    env.insert("PATH", appDir + ":" + env.value("PATH"));
    env.remove("LD_LIBRARY_PATH");
    env.insert("LD_LIBRARY_PATH", ""); 
    process->setProcessEnvironment(env);
    process->setWorkingDirectory(QDir::tempPath());
}

void downloadManager::stopDownload()
{
    isStopped = true;

    if(currentProcess && currentProcess->state() == QProcess::Running) {
        currentProcess->kill();
        currentProcess->waitForFinished(2000);

        emit logMessageRequested(QString("<span style='color:silver;'>User killed process</span>"));
    } else
        emit logMessageRequested(QString("<span style='color:silver;'>No active processes</span>"));

    if (currentProcess) {
        currentProcess->deleteLater();
        currentProcess = nullptr;
    }
}

void downloadManager::updateSongCheckState(QString &id, bool isChecked)
{
    for(int i = 0; i < Songs.size(); ++i)
    {
        if (Songs[i]->id == id) {
            Songs[i]->isChecked = isChecked;
            break;
        }
    }
}

void downloadManager::setIsStopped(bool set)
{
    this->isStopped = set;
}

void downloadManager::clearSongs()
{
    for(songInfo *song : Songs)
    {
        if (song != nullptr)  delete song;
    }

    Songs.clear();
}

QString downloadManager::formatBytes(long long bytes)
{
    double num = bytes;
    QStringList format = {"B", "KB", "MB", "GB"};
    int i = 0;

    while(num >= 1024 && i < format.size())
    {
        num /= 1024;
        i++;
    }

    return QString::number(num, 'f', 1) + " " + format[i];
}