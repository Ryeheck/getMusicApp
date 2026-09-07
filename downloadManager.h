#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include <QProgressBar>
#include <QMap>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QPointer>
#include <memory>

struct mediaInfo {
    QString id;
    QString name;
    QString status;
    QPointer<QWidget> widget;
    long long size;
    bool isChecked = false;

    ~mediaInfo() {
        if (widget)  
            widget->deleteLater();
    }
};

using mediaPtr = std::shared_ptr<mediaInfo>;

class downloadManager : public QObject
{
    Q_OBJECT

signals:
    void colorLogMessageRequested(const QString &firstColor, const QString &firstMessage, 
                                  const QString &lastColor="", const QString &lastMessage="");
    void messageRequested(const QString &message);
    void logMessageRequested(const QString &message);
    void activeTasksCountChanged(const int count);
    void mediaAdded(const mediaInfo *media);
    void pBarRequested(QProgressBar *pBar, const int percent);
    void updateStatusRequested(const QString &id, const QString &status);

public:
    explicit downloadManager(QObject *parent = nullptr);
    ~downloadManager() override;

    void getMedia(const QString &url, const QString &folder="", 
                  bool startAfter=false, bool isSongs=false, bool lyrics=false);
    void startDownload(const QString &folder="", bool isSongs=false, bool isLyrics=false);
    void mediaDownload(mediaPtr media, const QString &folder, bool isSong);
    void lyricsDownload(mediaPtr media, const QString &folder);
    void downloadFile(QUrl &url, QString path=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    void updateSongCheckState(const QString &id, bool isChecked);
    void clearMedia();
    void stopDownload();
    void setIsStopped(bool set);
    void setFormats(const QString &formatAudio, const QString &formatVideo, const QString &formatLyrics,
                    const QString &qualityVideo, const QString &qualityAudio);
    void setCookies(const QString &Cookies);
    void setJavaScript(const QString &jsRuntime);
    void checkAndPrepareFiles();
    
    static QString formatBytes(long long bytes);
    int getMediaCount();
    

private:
    void setupProcessLogging(const QString &id, QProgressBar *pBar, bool isLyrics);
    void setWorking(QProcess *process);
    void cleanupProcess(const QString &id, int exitCode);

    QMap<QString, QProcess *> _activeProcesses;
    QNetworkAccessManager *netManager;
    QString _formatAudio;
    QString _formatVideo;
    QString _formatLyrics;
    QString _qualityVideo;
    QString _qualityAudio;
    QString _CookiesBrowser;
    QString _jsRuntime;
    QString appDataDir;

    bool _isStopped;
    QList<std::shared_ptr<mediaInfo>> _Media;
};


#endif // DOWNLOADMANAGER_H