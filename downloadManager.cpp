#include "downloadManager.h"
#include "logView.h"
#include "settingDialog.h"

#include <QProcess>
#include <QListWidgetItem>
#include <QApplication>
#include <QProcessEnvironment>
#include <QDir>
#include <QString>
#include <QProgressBar>
#include <QMap>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#define MAX_SONGS   50

downloadManager::downloadManager(QObject *parent)
    : QObject(parent)
{
    _isStopped = false;
    
    // Default formats
    setFormats(".mp3", ".mp4", ".txt", "2160p60", "0");
    setCookies("firefox");
    setJavaScript("deno");

    // Default path
    appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);

}

downloadManager::~downloadManager()
{
    for(QProcess *process : _activeProcesses.values())
    {
        if(process)  process->deleteLater();
    }
    _activeProcesses.clear();

    for(mediaInfo *song : _Media)
    {
        if (song != nullptr)  delete song;
    }
    _Media.clear();
}

void downloadManager::getMedia(const QString &url, const QString &folder, bool startAfter, bool isSongs, bool lyrics)
{
    
    QProcess *process = new QProcess(this);
    _activeProcesses.insert(url, process);

    connect(process, &QProcess::readyReadStandardOutput, [this, url, process] () {
        QString output = process->readAllStandardOutput();

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i + 3 < lines.size() && (i < MAX_SONGS * 4); i += 4) 
        {
            mediaInfo *media = new mediaInfo();

            if (lines[i + 3] == "NA - NA")
                media->name = lines[i + 1];
            else
                media->name = lines[i + 3];

            media->id     = lines[i + 2];
            media->size   = lines[i].toLongLong();
            media->status = "";
            media->widget = new QProgressBar();

            bool exist = false;
            for(mediaInfo *songItem : _Media) 
                if (songItem->id == media->id) {
                    exist = true;
                    break;
                }

            if (!exist) {
                _Media.append(media);
                emit mediaAdded(media);
            } else
                delete media;
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [=, this] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        emit messageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));
        
        if (QProcess *process = _activeProcesses.value(url)) {
            process->deleteLater();
            _activeProcesses.remove(url);
        }
        if (startAfter && !_Media.isEmpty()) {
            _Media[0]->isChecked = true;
            startDownload(folder, isSongs, lyrics);
        }
        
    });

#ifdef Q_OS_WIN
    QString program = appDataDir + "/yt-dlp.exe";
#else
    QString program = appDataDir + "/yt-dlp_linux";
#endif

    QStringList args;
    args << "--js-runtimes" << _jsRuntime
         << "--cookies-from-browser" << _CookiesBrowser
         << "-O" << "%(filesize,filesize_approx)s\n%(title)s\n%(id)s\n%(artist)s - %(track)s"
         << url;

    // yt-dlp --flat-playlist --get-id --get-title --get-filename -o "%(artist)s - %(track)s" https://youtube.com

    process->start(program, args);
}

void downloadManager::startDownload(const QString &folder, bool isSongs, bool isLyrics)
{
    emit messageRequested(QString("<span style='color:silver;'>FOLDER: %1</span>").arg(folder));
    emit activeTasksCountChanged(1);

    for(int i = 0; i < _Media.size() && !_isStopped; ++i)
    {
        mediaInfo *media = _Media[i];

        if (media->isChecked == false)  continue;

        media->status = "Updating";
        emit updateStatusRequested(media->id, media->status);

        if (isLyrics)
            lyricsDownload(media, folder);
        else
            mediaDownload(media, folder, isSongs);
    }
}

void downloadManager::lyricsDownload(mediaInfo *media, const QString &folder)
{
    QProcess *process = new QProcess(this);

    _activeProcesses.insert(media->id, process);
    setWorking(process);
    
    QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget);
    setupProcessLogging(media->id, pBar, false); 

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, media, process] (int exitCode) {
        cleanupProcess(media->id, exitCode);

        if (exitCode)
            media->status = "Error";
        else if (process->property("notLyrics").toBool())
            media->status = "Not Lyrics";
        else
            media->status = "Done";
        
        emit updateStatusRequested(media->id, media->status);
        emit activeTasksCountChanged(_activeProcesses.size());
    });

#ifdef Q_OS_WIN
    QString program = appDataDir + "/syncedlyrics_bin.exe";
#else
    QString program = appDataDir + "/syncedlyrics_bin";
#endif

    QString songName = media->name;

    QStringList args;
    args << songName
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + "." + _formatLyrics
         << "--verbose";

    process->start(program, args);

    // syncedlyrics [args] songName
}

void downloadManager::mediaDownload(mediaInfo *media, const QString &folder, bool isSong)
{
    QProcess *process = new QProcess(this);

    _activeProcesses.insert(media->id, process);
    setWorking(process);
    
    QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget);
    setupProcessLogging(media->id, pBar, false); 
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, media] (int exitCode) {
        cleanupProcess(media->id, exitCode);

        media->status = exitCode ? "Error" : "Done";
        
        emit updateStatusRequested(media->id, media->status);
        emit activeTasksCountChanged(_activeProcesses.size());
    });

#ifdef Q_OS_WIN
    QString program = appDataDir + "/yt-dlp.exe";
#else
    QString program = appDataDir + "/yt-dlp";
#endif

    QString mediaName = media->name;

    QStringList args;
    args // << "--ffmpeg-location" << appDataDir
         << "--buffer-size" << "64K"
         << "--concurrent-fragments" << "5"
         << "--no-mtime" << "--no-playlist" 
         << "--js-runtimes" << _jsRuntime
         << "--cookies-from-browser" << _CookiesBrowser
         << "--newline";

    if (isSong)
        args << "-x" 
             << "--audio-format" << _formatAudio
             << "--audio-quality" << _qualityAudio
             << "-o" << folder + "/" + mediaName + "." + _formatAudio;
    else
        args << "-f" 
             << QString("bestvideo[height<=%1]+bestaudio/bestvideo+bestaudio").arg(_qualityVideo.split("p").first())
             << "--merge-output-format" << _formatVideo
             << "-o" << folder + "/" + mediaName + "." + _formatVideo;

    args << "--" << media->id;

    process->start(program, args);

    // yt-dlp [args] (id)
}

void downloadManager::downloadFile(QUrl &url, QString path)
{
    appDataDir = path;
    QFile *file = new QFile(this);
    
    mediaInfo *media = new mediaInfo();
    media->isChecked = true;
    media->status = "Download";
    media->widget = new QProgressBar();
    QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget);
    
    QNetworkAccessManager *netManager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy); // For github

    QNetworkReply *reply = netManager->get(request);
    connect(reply, &QNetworkReply::metaDataChanged, this, [this, reply, file, media] () {
        qint64 size = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        if (size <= 0)  return;
        if (reply->error() != QNetworkReply::NoError) {
            emit logMessageRequested("Reply return error: " + reply->errorString());
            media->status = "Error";
            return;
        }

        media->size = size;
        media->name = reply->header(QNetworkRequest::ContentDispositionHeader).toString().split("=").last();
        media->id   = reply->header(QNetworkRequest::ETagHeader).toString();

        QString pathApp = appDataDir + "/" + media->name;

        file->setFileName(pathApp);
        if (!file->open(QIODevice::WriteOnly)) {
            emit logMessageRequested("Couldn't create file for download");
            return;
        }
        
        _Media.append(media);
        emit mediaAdded(media);
    });
    
    connect(reply, &QNetworkReply::readyRead, this, [this, file, reply] () {
        if (file->isOpen())  file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [this, pBar] (qint64 bytes, qint64 total) {
        if (total > 0)  emit progressBarRequested(pBar, static_cast<qint64>(bytes * 100) / total);
    });
    connect(reply, &QNetworkReply::finished, this, [this, file, reply, media] () {
        file->close();
        reply->deleteLater();
        media->status = "Done";
        emit updateStatusRequested(media->id, media->status);
    });
}

void downloadManager::cleanupProcess(const QString &id, int exitCode)
{
    QString output;
    for(int i = _Media.size() - 1; i >= 0; --i)
    {
        if (_Media[i]->id == id)  output = _Media[i]->name;
    }
    output += (exitCode ? ": Error" : ": Done!");
    emit messageRequested(QString("<span style='color:silver;'>%1</span>").arg(output));

    if (_activeProcesses.contains(id)) {
        if (QProcess *process = _activeProcesses.value(id))  process->deleteLater();

        _activeProcesses.remove(id);
    }
}

void downloadManager::setupProcessLogging(const QString &id, QProgressBar *pBar, bool isLyrics) 
{
    if (!_activeProcesses.contains(id))  return;
    int *stepCount = new int(0);
    
    QProcess *process = _activeProcesses.value(id);
    
    connect(process, &QProcess::readyReadStandardOutput, [this, process, pBar, isLyrics] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        QRegularExpression percentReg(R"(\[download\]\s+(\d+(?:\.\d+)?)\s*%)");
        QRegularExpressionMatch match = percentReg.match(output);

        if (output.isEmpty())  return;  

        if (match.hasMatch() && pBar) {
            int percent = static_cast<int >(match.captured(1).toFloat());
            if (percent > 100)  percent = 100;

            emit progressBarRequested(pBar, percent);
        }

        if (isLyrics)  
            emit logMessageRequested(QString("<span style='color:DarkSeaGreen;'>INFO: %1</span>").arg(output));
        else           
            emit logMessageRequested(QString("<span style='color:silver;'>INFO: </span>") + output);
        
    });

    connect(process, &QProcess::readyReadStandardError, [this, process, stepCount, pBar] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        QRegularExpression percentReg(R"((continuing search|Lyrics found|No suitable lyrics found for))");
        QRegularExpressionMatch match = percentReg.match(output);

        if (output.isEmpty())  return;

        if (match.hasMatch()) {
            QString search = match.captured(1);
            int percent = ++(*stepCount) * 25;
            
            if (search == "No suitable lyrics found for") {
                percent = 100;
                process->setProperty("notLyrics", true);
            } else if(search == "Lyrics found" || percent > 100) {
                percent = 100;
            }
            emit progressBarRequested(pBar, percent);
        }

        output.replace("DEBUG:", QString("<span style='color:silver;'>DEBUG: </span>"));
        output.replace("INFO:", QString("<span style='color:silver;'>INFO: </span>"));
        output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING:</span>"));
        output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR:</span>"));

        emit logMessageRequested(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [stepCount] (int) {
        delete stepCount;
    });
}

void downloadManager::setWorking(QProcess *process)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_WIN
    env.insert("PATH", appDataDir + ";" + env.value("PATH"));
#else
    env.insert("PATH", appDataDir + ":" + env.value("PATH"));
    env.remove("LD_LIBRARY_PATH");
    env.insert("LD_LIBRARY_PATH", ""); 
#endif
    
    process->setProcessEnvironment(env);
    process->setWorkingDirectory(QDir::tempPath());
}

void downloadManager::stopDownload()
{
    _isStopped = true;

    for(QProcess *process : _activeProcesses.values())
        if(process) {
            process->disconnect();
            if(process->state() == QProcess::Running)  process->kill();

            process->deleteLater();

            emit messageRequested(QString("<span style='color:silver;'>User killed process</span>"));
        } else
            emit messageRequested(QString("<span style='color:silver;'>No active processes</span>"));
    _activeProcesses.clear();
}

void downloadManager::updateSongCheckState(const QString &id, bool isChecked)
{
    for(int i = 0; i < _Media.size(); ++i)
    {
        if (_Media[i]->id == id) {
            _Media[i]->isChecked = isChecked;
            break;
        }
    }
}

void downloadManager::setIsStopped(bool set)
{
    this->_isStopped = set;
}

void downloadManager::clearMedia()
{
    for(mediaInfo *song : _Media)
    {
        if (song != nullptr)  delete song;
    }

    _Media.clear();
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

void downloadManager::setFormats(const QString &formatAudio,  const QString &formatVideo, const QString &formatLyrics,
                                 const QString &qualityVideo, const QString &qualityAudio)
{
    _formatAudio = formatAudio;
    _formatVideo = formatVideo;
    _formatLyrics = formatLyrics;

    _qualityVideo = qualityVideo;
    _qualityAudio = qualityAudio;

}

void downloadManager::setCookies(const QString &Cookies)
{
    _CookiesBrowser = Cookies;
}

void downloadManager::setJavaScript(const QString &jsRuntime)
{
    _jsRuntime = jsRuntime;
}

void downloadManager::checkForUpdate()
{
    QProcess *process = new QProcess();

#ifdef Q_OS_WIN
    QString program = appDataDir + "/yt-dlp.exe";
#else
    QString program = appDataDir + "/yt-dlp_linux";
#endif

    connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        emit logMessageRequested(output);
    });
    connect(process, &QProcess::readyReadStandardError, [this, process] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        emit logMessageRequested(output);
    });

    QStringList args;
    args << "-U";

    process->start(program, args);
}
