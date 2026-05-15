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


    isStoppedForNext = false;
    currentDownloadIndex = -1;
}

void downloadManager::getTitle(QString url, QString folder, bool startAfter, bool lyrics)
{
    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, [this, url, process] () {
        QString output = process->readAllStandardOutput();

        QStringList Names = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i + 2 < Names.size() && (i < MAX_SONGS * 3); i += 3) {
            songInfo song;
            song.title = Names[i];
            song.id    = Names[i + 1];
            song.name  = Names[i + 2];
            
            bool exist = false;
            for(songInfo songItem : Songs) 
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


void downloadManager::startDownload(QString folder, bool lyrics)
{
    currentDownloadIndex = 0;
    emit setupDownloadRequested(true);

    if (folder.isEmpty())  folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    
    emit logMessageRequested(QString("<span style='color:silver;'>FOLDER: %1</span>").arg(folder));

    if (lyrics) 
        lyricsDownloadForName(folder);
    else 
        nextDownload(folder);

}

void downloadManager::nextDownload(QString folder)
{
    if (isStoppedForNext)  return;

    if (currentDownloadIndex >= Songs.size()) {
        emit logMessageRequested(QString("<span style='color:silver;'>All Done!</span>"));
        currentDownloadIndex = -1;
        emit setupDownloadRequested(false);
        return;
    }

    songInfo &song = Songs[currentDownloadIndex];
    if (song.isChecked == false) {
        currentDownloadIndex++;
        nextDownload(folder);
        return;
    }

    QProcess *process = new QProcess(this);
    currentProcess = process;
    
    connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            emit logMessageRequested(QString("<span style='color:silver;'>INFO: </span>") + output);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this, process] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING:</span>"));
        output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR:</span>"));

        emit logMessageRequested(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        emit logMessageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

        if (currentProcess == process)  currentProcess = nullptr;

        process->deleteLater();
        currentDownloadIndex++;
        nextDownload(folder);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking(process);

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
}


void downloadManager::lyricsDownloadForName(QString folder)
{
    if (isStoppedForNext)  return;

    if (currentDownloadIndex >= Songs.size()) {
        emit logMessageRequested(QString("<span style='color:silver;'>All Done!</span>"));
        currentDownloadIndex = -1;
        emit setupDownloadRequested(false);
        return;
    }

    songInfo &song = Songs[currentDownloadIndex];
    if (song.isChecked == false) {
        currentDownloadIndex++;
        lyricsDownloadForName(folder);
        return;
    }
    
    QProcess *process = new QProcess(this);
    currentProcess = process;

    connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            emit logMessageRequested(QString("<span style='color:DarkSeaGreen;'>INFO: %1</span>").arg(output));
        }
    });

   connect(process, &QProcess::readyReadStandardError, [this, process] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);
        
        output.replace("DEBUG:", QString("<span style='color:silver;'>DEBUG: </span>"));
        output.replace("INFO:", QString("<span style='color:silver;'>INFO: </span>"));
        output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING: </span>"));
        output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR: </span>"));

        emit logMessageRequested(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, process] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        emit logMessageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

        if (currentProcess == process)  currentProcess = nullptr;

        process->deleteLater();
        currentDownloadIndex++;
        lyricsDownloadForName(folder);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking(process);

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

    // syncedlyrics "Nirvana - Smells Like Teen Spirit" -o "Nirvana.lrc"
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
    isStoppedForNext = true;
    if(currentProcess && currentProcess->state() == QProcess::Running) {
        currentProcess->kill();
        currentProcess->waitForFinished(2000);

        emit logMessageRequested(QString("<span style='color:silver;'>User killed process</span>"));
    } else
        emit logMessageRequested(QString("<span style='color:silver;'>No active processes</span>"));

    currentProcess->deleteLater();
    currentProcess = nullptr;
}

void downloadManager::updateSongCheckState(QString &id, bool isChecked)
{
    for(int i = 0; i < Songs.size(); ++i)
        if (Songs[i].id == id) {
            Songs[i].isChecked = isChecked;
            break;
        }
}

void downloadManager::setIsStoppedForNext(bool set)
{
    this->isStoppedForNext = set;
}

void downloadManager::clearSongs()
{
    currentDownloadIndex = -1;
    Songs.clear();
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