#include "downloadManager.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QString>
#include <QProgressBar>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QFile>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quazip.h>
#include <quazipfile.h>
#include <functional>
#include <QRegularExpression>

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

    // Network
    netManager = new QNetworkAccessManager(this);

}

downloadManager::~downloadManager()
{
    for(QProcess *process : _activeProcesses.values())
    {
        if(process)  process->deleteLater();
    }
    _activeProcesses.clear();

    _Media.clear();
}

void downloadManager::getMedia(const QString &url, const QString &folder, bool startAfter, bool isSongs, bool lyrics)
{
    
    QProcess *process = new QProcess(this);
    _activeProcesses.insert(url, process);

    connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        QString output = process->readAllStandardOutput();

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i + 3 < lines.size() && (i < MAX_SONGS * 4); i += 4) 
        {
            auto media = mediaPtr(new mediaInfo());

            if (lines[i + 3] == "NA - NA")
                media->name = lines[i + 1];
            else
                media->name = lines[i + 3];

            media->id     = lines[i + 2];
            media->size   = lines[i].toLongLong();
            media->status = "";
            media->widget = new QProgressBar();

            bool exist = false;
            for(mediaPtr songItem : _Media) 
                if (songItem->id == media->id) {
                    exist = true;
                    break;
                }

            if (!exist) {
                _Media.append(media);
                emit mediaAdded(media.get());
            } 
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this, process] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);
        if (output.contains("Failed to resolve") || output.contains("Failed to establish"))
            emit messageRequested("Maybe fix: use another VPN");
        emit logMessageRequested(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, url, startAfter, folder, isSongs, lyrics] (int exitCode) {
        cleanupProcess(url, exitCode);
        
        if (exitCode == 0 && startAfter && !_Media.isEmpty()) {
            _Media.last()->isChecked = true;
            startDownload(folder, isSongs, lyrics);
        }
    });

#ifdef Q_OS_WIN
    QString exec ="yt-dlp.exe";
#else
    QString exec = "yt-dlp_linux";
#endif

    QString program;
    QString path = QDir(appDataDir).filePath(exec);
    if (QFile::exists(path)) {
        // If exec in appData
        program = path;
    } else if (!QStandardPaths::findExecutable(exec).isEmpty()) {
        // If exec in system
        program = exec;
    } else {
        // If not exec
        emit messageRequested(QString("%1 not exists").arg(exec));
        emit messageRequested("Please prepare program");
        return;
    }

    QStringList args;
    args << "--js-runtimes" << _jsRuntime
         << "--cookies-from-browser" << _CookiesBrowser
         << "-O" << "%(filesize,filesize_approx)s\n%(title)s\n%(id)s\n%(artist)s - %(track)s"
         << url;

    emit activeTasksCountChanged(_activeProcesses.size());
    process->start(program, args);
}

void downloadManager::startDownload(const QString &folder, bool isSongs, bool isLyrics)
{
    emit messageRequested("Folder: " + folder);

    for(int i = 0; i < _Media.size() && !_isStopped; ++i)
    {
        auto media = _Media[i];

        if (media->isChecked == false)  continue;

        media->status = "Updating";
        emit updateStatusRequested(media->id, media->status);
        if (QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget))
            emit pBarRequested(pBar, 0);

        if (isLyrics)
            lyricsDownload(media, folder);
        else
            mediaDownload(media, folder, isSongs);
    }
}

void downloadManager::lyricsDownload(mediaPtr media, const QString &folder)
{
    QProcess *process = new QProcess(this);
    _activeProcesses.insert(media->id, process);
    
    if (QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget))
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
    });

#ifdef Q_OS_WIN
    QString exec = "syncedlyrics.exe";
#else
    QString exec = "syncedlyrics";
#endif

    QString program;
    QString path = QDir(appDataDir).filePath(exec);
    if (QFile::exists(path)) {
        // If exec in appData
        program = path;
    } else if (!QStandardPaths::findExecutable(exec).isEmpty()) {
        // If exec in system
        program = exec;
    } else {
        // If not exec
        emit messageRequested(QString("%1 not exists").arg(exec));
        emit messageRequested("Please prepare program");
        return;
    }

    QString songName = media->name;

    QStringList args;
    args << songName
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + "." + _formatLyrics
         << "--verbose";

    emit activeTasksCountChanged(_activeProcesses.size());
    process->start(program, args);

    // syncedlyrics [args] songName
}

void downloadManager::mediaDownload(mediaPtr media, const QString &folder, bool isSong)
{
    QProcess *process = new QProcess(this);
    _activeProcesses.insert(media->id, process);
    
    if (QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget))
        setupProcessLogging(media->id, pBar, false); 
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, folder, media] (int exitCode) {
        cleanupProcess(media->id, exitCode);

        media->status = exitCode ? "Error" : "Done";
        
        emit updateStatusRequested(media->id, media->status);
    });

#ifdef Q_OS_WIN
    QString exec = "yt-dlp.exe";
#else
    QString exec = "yt-dlp_linux";
#endif

    QString program;
    QString path = QDir(appDataDir).filePath(exec);
    if (QFile::exists(path)) {
        // If exec in appData
        program = path;
    } else if (!QStandardPaths::findExecutable(exec).isEmpty()) {
        // If exec in system
        program = exec;
    } else {
        // If not exec
        emit messageRequested(QString("%1 not exists").arg(exec));
        emit messageRequested("Please prepare program");
        return;
    }
    QString mediaName = media->name;

    QStringList args;
    args << "--buffer-size" << "64K"
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

    emit activeTasksCountChanged(_activeProcesses.size());
    process->start(program, args);
}

void downloadManager::downloadFile(QUrl &url, QString savePath, std::function<void()> onSuccess)
{
    appDataDir = savePath;
    QFile *file = new QFile();
    
    auto media = mediaPtr(new mediaInfo());
    media->isChecked = true;
    media->status = "Download";
    media->widget = new QProgressBar();
    QProgressBar *pBar = qobject_cast<QProgressBar *>(media->widget);
    emit updateStatusRequested(media->id, media->status);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy); // For github

    QNetworkReply *reply = netManager->get(request);
    
    auto cleanup = [file, reply] () {
        file->deleteLater();
        reply->deleteLater();
    };

    connect(reply, &QNetworkReply::metaDataChanged, this, [this, reply, file, media, cleanup, onSuccess] () {
        if (reply->error() != QNetworkReply::NoError) {
            reply->abort();
            return;
        }
        if (_isStopped) {
            reply->abort();
            return;
        }
        
        // If doesnt start download
        qint64 size = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        if (size <= 0) return;

        QString filename = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
        if (filename.contains("filename=")) 
            filename = filename.section("filename=", 1).section(";", 0, 0);
        else 
            filename = reply->url().fileName();
        
        media->id   = QUuid::createUuid().toString();
        media->size = size;
        media->name = filename;

        QString pathApp = QDir(appDataDir).filePath(media->name);
        file->setFileName(pathApp);

        if (!QStandardPaths::findExecutable(media->name).isEmpty()) {
            emit colorLogMessageRequested("silver", "Download: ", 
                                          "DarkSeaGreen", QString("%1 in path").arg(media->name));
            reply->abort();
            return;
        } 
        if (file->exists()) {
            emit colorLogMessageRequested("silver", "Download: ", 
                                          "DarkSeaGreen", QString("%1 already exists").arg(media->name));
            reply->abort();
            return;
        } 
        if (!file->open(QIODevice::WriteOnly)) {
            emit colorLogMessageRequested("silver", "Download: ", 
                                          "IndianRed", QString("Couldn't create file (%1) for download").arg(media->name));
            reply->abort();
            return;
        }
        
        _Media.append(media);
        emit mediaAdded(media.get());
    });
    
    connect(reply, &QNetworkReply::readyRead, this, [this, file, reply] () {
        if (_isStopped) {
            reply->abort();
            return;
        }
        if (file->isOpen())  
            file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, this, [this, pBar, file] (qint64 bytes, qint64 total) {
        if (file->isOpen() && total > 0)  
            emit pBarRequested(pBar, static_cast<qint64>(bytes * 100) / total);
    });

    connect(reply, &QNetworkReply::finished, this, [this, file, reply, media, onSuccess, cleanup] () {
        if (_isStopped || reply->error() != QNetworkReply::NoError) {
            if (file->isOpen()) {
                file->close();
                file->remove();
            }
            media->status = -_isStopped ? "Canceled" : "Error";
            cleanup();
            emit updateStatusRequested(media->id, media->status);

            if (_isStopped)
                emit colorLogMessageRequested("silver", "Download:", 
                                             "IndianRed", "Download canceled");
            else 
                emit colorLogMessageRequested("silver", "Reply return error: ", 
                                              "IndianRed", reply->errorString());
            return;
        }
        if (file->isOpen()) {
#ifdef Q_OS_WIN
            // Permissions on windows
#else
            QFile::setPermissions(file->fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                    QFile::ReadGroup | QFile::ExeGroup |
                                                    QFile::ReadOther | QFile::ExeOther);
#endif
            file->close();
            media->status = "Done";
            emit updateStatusRequested(media->id, media->status);
        }  
        if (onSuccess)  onSuccess();
        cleanup();
        emit colorLogMessageRequested("silver", "Download: ", 
                                      "DarkSeaGreen", QString("%1 installed").arg(media->name));
    });
}

void downloadManager::cleanupProcess(const QString &id, int exitCode)
{
    QString output = id;
    for(int i = _Media.size() - 1; i >= 0; --i)
    {
        if (_Media[i]->id == id) {
            output = _Media[i]->name;
            break;
        }
    }

    output += (exitCode ? ": Error!" : ": Done!");
    emit messageRequested(output);

    if (_activeProcesses.contains(id)) {
        if (QProcess *process = _activeProcesses.value(id))  process->deleteLater();

        _activeProcesses.remove(id);
    }

    emit activeTasksCountChanged(_activeProcesses.size());
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

            emit pBarRequested(pBar, percent);
        }

        if (isLyrics)  
            emit colorLogMessageRequested("silver", "Download: INFO: ", "DarkSeaGreen", output);
        else           
            emit colorLogMessageRequested("silver","Download: INFO: " + output);
        
    });

    connect(process, &QProcess::readyReadStandardError, [this, process, stepCount, pBar] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        QRegularExpression percentReg(R"((continuing search|Lyrics found|No suitable lyrics found for))");
        QRegularExpressionMatch match = percentReg.match(output);

        if (output.isEmpty())  return;
        
        if (output.contains("error: unsupported browser")) {
            emit messageRequested(QString("Cookie not found: %1").arg(_CookiesBrowser));
            emit messageRequested("Please use another cookie in the setting (left bottom button)");
        }

        if (match.hasMatch()) {
            QString search = match.captured(1);
            int percent = ++(*stepCount) * 25;
            
            if (search == "No suitable lyrics found for") {
                percent = 100;
                process->setProperty("notLyrics", true);
            } else if(search == "Lyrics found" || percent > 100) {
                percent = 100;
            }
            emit pBarRequested(pBar, percent);
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

void downloadManager::extractProgram(const QString targetPath, const QString savePath)
{
    // Open zip archive and extract all files
    QuaZip zip(targetPath);
    if (!zip.open(QuaZip::mdUnzip)) {
        emit colorLogMessageRequested("silver", "Extract: ", "IndianRed", "not open zip archive");
        return;
    }

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        QuaZipFile inFile(&zip);
        if (!inFile.open(QIODevice::ReadOnly)) {
            emit colorLogMessageRequested("silver", "Extract: ", 
                                          "IndianRed", QString("not open file to read: %1").arg(targetPath));
            return;
        }

        // Get file info
        QuaZipFileInfo64 fileInfo;
        if (!zip.getCurrentFileInfo(&fileInfo)) {
            emit colorLogMessageRequested("silver", "Extract: ", "IndianRed", "not current file info");
            return;
        }
        // If it isnt a program
        int lastPoint = fileInfo.name.lastIndexOf(".");
        QString ext = (lastPoint > 0 && lastPoint < fileInfo.name.length() - 1) 
                      ? fileInfo.name.mid(lastPoint + 1) 
                      : "";  // this is linux
        if (!(ext.isEmpty() || ext == "exe") || fileInfo.name.endsWith("/")) {  // If its not executable file 
            inFile.close();
            continue;
        }
        
        // Create output file
        QString outPath = QDir(savePath).filePath(fileInfo.name.section("/", -1));

        QFile outFile(outPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            emit colorLogMessageRequested("silver", "Extract: ", 
                                          "IndianRed", QString("not open file to write: %1").arg(outPath));
            inFile.close();
            return;
        }

        // Copy data
        outFile.write(inFile.readAll());

        outFile.close();
        inFile.close();
        emit colorLogMessageRequested("silver", "Extract: ", 
                                      "DarkSeaGreen", QString("%1 successful").arg(outPath));
    }
    zip.close();
}

void downloadManager::stopDownload()
{
    _isStopped = true;

    for(QProcess *process : _activeProcesses.values())
        if(process) {
            process->disconnect();
            if(process->state() == QProcess::Running)  process->kill();

            process->deleteLater();

            emit messageRequested("User killed process");
        } else
            emit messageRequested("No active processes");
    _activeProcesses.clear();

    emit activeTasksCountChanged(_activeProcesses.size());
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

void downloadManager::checkAndPrepareFiles()
{
    QProcess *process = new QProcess();
    QString id = QUuid::createUuid().toString();
    _activeProcesses.insert(id, process);

#ifdef Q_OS_WIN
    QString yt_dlp       = "yt-dlp.exe";
    QString syncedlyrics = "syncedlyrics.exe";
    QString ffmpeg       = "ffmpeg.exe";
    QString ffprobe      = "ffprobe.exe";
    QString jsRuntime    = _jsRuntime + ".exe";
    QString zipDeno      = "deno-x86_64-pc-windows-msvc.zip";
    QString zipFfmpeg    = "ffmpeg-master-latest-win64-gpl.zip";
#else
    QString yt_dlp       = "yt-dlp_linux";
    QString syncedlyrics = "syncedlyrics";
    QString ffmpeg       = "ffmpeg";
    QString ffprobe      = "ffprobe";
    QString jsRuntime    = _jsRuntime;
    QString zipDeno      = "deno-x86_64-unknown-linux-gnu.zip";
    QString zipFfmpeg    = "ffmpeg-master-latest-linux64-gpl.tar.xz";
#endif
    
    // Check the yt-dlp and download it if necessary 
    QString path         = QDir(appDataDir).filePath(yt_dlp);
    bool existsInAppData = QFile::exists(path);
    bool existsInSystem  = !QStandardPaths::findExecutable(yt_dlp).isEmpty();
    
    if (!(existsInAppData || existsInSystem)) {
        QUrl url(QString("https://github.com/yt-dlp/yt-dlp/releases/latest/download/%1").arg(yt_dlp));
        downloadFile(url);
    } else {
        emit colorLogMessageRequested("silver", "Download: ", 
                                       "DarkSeaGreen", QString("%1 already exists").arg(yt_dlp));
        emit colorLogMessageRequested("silver", QString("Path: %1 ").arg(path),
                                      "silver", QString("or in system: %1").arg(existsInSystem));
    }
    // Check the syncedlirycs and move it if to appDataDir
    path            = QDir(appDataDir).filePath(syncedlyrics);
    existsInAppData = QFile::exists(path);
    existsInSystem  = !QStandardPaths::findExecutable(syncedlyrics).isEmpty(); 

    if (!(existsInAppData || existsInSystem)) {
        if (QFile::exists(syncedlyrics)) {
            QFile::rename(syncedlyrics, path);
        } else {
            emit colorLogMessageRequested("silver", "Download: ", 
                                          "DarkOrange", QString("please install %1: pip install syncedlyrics").arg(syncedlyrics));
            emit messageRequested(QString("Please install %1").arg(syncedlyrics));
        }
    } else {
        emit colorLogMessageRequested("silver", "Download: ", 
                                       "DarkSeaGreen", QString("%1 already exists").arg(syncedlyrics));
        emit colorLogMessageRequested("silver", QString("Path: %1 ").arg(path),
                                      "silver", QString("or in system: %1").arg(existsInSystem));
    }
    // Check javascript and extract -> remove ZIP file 
    path            = QDir(appDataDir).filePath(jsRuntime);
    existsInAppData = QFile::exists(path);
    existsInSystem  = !QStandardPaths::findExecutable(jsRuntime).isEmpty();

    if (!(existsInAppData || existsInSystem)) {
        QString filenameZIP = QDir(appDataDir).filePath(zipDeno);

        if (_jsRuntime == "deno") {
            QUrl url(QString("https://github.com/denoland/deno/releases/latest/download/%1").arg(zipDeno));
            downloadFile(url, appDataDir, 
                        [this, filenameZIP] () {  // Extract in appDataDir and remove .zip file
                            extractProgram(filenameZIP, appDataDir);
                            QFile::remove(filenameZIP);
                            emit logMessageRequested(QString("Remove: %1").arg(filenameZIP));
                        });

        } else if (_jsRuntime == "node") {
            emit messageRequested("Please install node");
            emit colorLogMessageRequested("silver", "Download: ", 
                                          "DarkOrange", 
                                          QString("please install %1: https://nodejs.org/en/download/current").arg(_jsRuntime));
            /* comming soon... */
        }
    } else {
        emit colorLogMessageRequested("silver", "Download: ", 
                                       "DarkSeaGreen", QString("%1 already exists").arg(jsRuntime));
        emit colorLogMessageRequested("silver", QString("Path: %1 ").arg(path),
                                      "silver", QString("or in system: %1").arg(existsInSystem));
    }
    path = QDir(appDataDir).filePath(ffmpeg);
    existsInAppData = QFile::exists(path);
    existsInSystem  = !QStandardPaths::findExecutable(ffmpeg).isEmpty();

    if (!(existsInAppData || existsInSystem)) {
        QString filenameZIP = QDir(appDataDir).filePath(zipFfmpeg);
        QUrl url(QString("https://github.com/BtbN/FFmpeg-Builds/releases/latest/download/%1").arg(zipFfmpeg));
        downloadFile(url, appDataDir, 
                    [this, filenameZIP] () {  // Extract in appDataDir and remove .zip file
                        extractProgram(filenameZIP, appDataDir);
                        QFile::remove(filenameZIP);
                        emit logMessageRequested(QString("Remove: %1").arg(filenameZIP));
                    });
    } else {
        emit colorLogMessageRequested("silver", "Download: ", 
                                       "DarkSeaGreen", QString("%1 already exists").arg(ffmpeg));
        emit colorLogMessageRequested("silver", QString("Path: %1 ").arg(path),
                                      "silver", QString("or in system: %1").arg(existsInSystem));
    }
    path = QDir(appDataDir).filePath(ffprobe);
    existsInAppData = QFile::exists(path);
    existsInSystem  = !QStandardPaths::findExecutable(ffprobe).isEmpty();

    if (!(existsInAppData || existsInSystem) || true) {
        QString filenameZIP = QDir(appDataDir).filePath(zipFfmpeg);
        QUrl url(QString("https://github.com/BtbN/FFmpeg-Builds/releases/latest/download/%1").arg(zipFfmpeg));
        downloadFile(url, appDataDir, 
                    [this, filenameZIP] () {  // Extract in appDataDir and remove .zip file
                        extractProgram(filenameZIP, appDataDir);
                        QFile::remove(filenameZIP);
                        emit logMessageRequested(QString("Remove: %1").arg(filenameZIP));
                    });
    } else {
        emit colorLogMessageRequested("silver", "Download: ", 
                                       "DarkSeaGreen", QString("%1 already exists").arg(ffprobe));
        emit colorLogMessageRequested("silver", QString("Path: %1 ").arg(path),
                                      "silver", QString("or in system: %1").arg(existsInSystem));
    }

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
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [this, id] (int exitCode) {
        cleanupProcess(id, exitCode);
    });

    QString program;
    path = QDir(appDataDir).filePath(yt_dlp);
    if (QFile::exists(path)) 
        program = path;
    else if (!QStandardPaths::findExecutable(yt_dlp).isEmpty())
        program = yt_dlp;
    else {
        emit messageRequested(QString("%1 not exists").arg(yt_dlp));
        emit messageRequested("Please prepare program");
        return;
    }

    QStringList args;
    args << "-U";

    emit activeTasksCountChanged(_activeProcesses.size());
    process->start(program, args);
}