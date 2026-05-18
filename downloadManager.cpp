#include "downloadManager.h"
#include "logView.h"

#include <QProcess>
#include <QListWidgetItem>
#include <QApplication>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>

#define MAX_SONGS   50

downloadManager::downloadManager(QObject *parent)
    : QObject(parent)
{
    isStopped = false;
}

void downloadManager::getTitle(QString url, QString folder, bool startAfter, bool lyrics)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;

    connect(process, &QProcess::readyReadStandardOutput, [this, url, process] () {
        QString output = process->readAllStandardOutput();

        QStringList Names = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i + 2 < Names.size() && (i < MAX_SONGS * 3); i += 3) {
            songInfo song;
            song.title = Names[i];
            song.id    = Names[i + 1];
            song.name  = Names[i + 2];
            
            bool exist = false;
            for(songInfo &songItem : Songs) 
                if (songItem.id == song.id) {
                    exist = true;
                    break;
                }

            if (!exist) {
                Songs.append(song);

                if (song.name == "NA - NA")
                    emit songAdded(song.title, song.id);
                else
                    emit songAdded(song.name, song.id);
            }
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, startAfter, folder, lyrics, process] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        emit logMessageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

        if (currentProcess == process)  currentProcess = nullptr;
        
        process->deleteLater();

        if (startAfter && !Songs.isEmpty()) {
            Songs[0].isChecked = true;
            startDownload(folder, lyrics);
        }
        
    });

    QString appDir = qApp->applicationDirPath();
    QStringList args;
    args // << "--flat-playlist"
         << "--get-id" << "--get-filename" << "--get-title"
         << "-o" << "%(artist)s - %(track)s"
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
        songInfo &song = Songs[i];
        if (song.isChecked == false)  continue;
        
        if (isLyrics)
            lyricsDownload(song, folder);
        else
            songDownload(song, folder);

        QEventLoop loop;

        if (currentProcess)
            connect(currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                    &loop, &QEventLoop::quit);

        loop.exec();
    }

    emit logMessageRequested(QString("<span style='color:silver;'>All Done!</span>"));
    emit setupDownloadRequested(false);
}

void downloadManager::lyricsDownload(songInfo &song, QString folder)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;
    setWorking(process);

    // setupProcessLogging(process, true);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process] (int exitCode) {
        cleanupProcess(exitCode);
    });

    QString appDir = qApp->applicationDirPath();
    QString songName;

    if (song.name == "NA - NA")
        songName = song.title;
    else
        songName = song.name;

    QStringList args;
    args << songName
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + ".lrc"
         << "--verbose";

    emit logMessageRequested(QString("<span style='color:silver;'>Lyrics: %1</span>").arg(songName));

    process->start(appDir + "/syncedlyrics_bin", args);

    // syncedlyrics [args] songName
}

void downloadManager::songDownload(songInfo &song, QString folder)
{
    QProcess *process = new QProcess(this);
    currentProcess = process;
    setWorking(process);

    // setupProcessLogging(process);
    
    setupProcessBar(process, false);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process] (int exitCode) {
        cleanupProcess(exitCode);
    });

    QString appDir = qApp->applicationDirPath();
    QString songName;

    if (song.name == "NA - NA")
        songName = song.title;
    else
        songName = song.name;

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
         << song.id;

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

void downloadManager::setupProcessBar(QProcess *process, bool isLyrics)
{
    connect(process, &QProcess::readyReadStandardOutput, [this, process, isLyrics] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        QRegularExpression percentReg(R"(\[download\]\s+(\d+(?:\.\d+)?)\s*%)");
        QRegularExpressionMatch match = percentReg.match(output);

        if (match.hasMatch()) {
            int percent = static_cast<int>(match.captured(1).toFloat());
            emit progressBarRequested(percent);
        }
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
        if (Songs[i].id == id) {
            Songs[i].isChecked = isChecked;
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
    Songs.clear();
}