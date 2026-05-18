#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>

struct songInfo {
    QString id;
    QString title;
    QString name;
    bool isChecked = false;
};

class downloadManager : public QObject
{
    Q_OBJECT

signals:
    void logMessageRequested(const QString &message);
    void setupDownloadRequested(bool set);
    void songAdded(QString name, QString id);

public:
    explicit downloadManager(QObject *parent = nullptr);

    void getTitle(QString url, QString folder = "", bool startAfter = false, bool lyrics = false);
    void startDownload(QString folder = "", bool isLyrics = false);
    void songDownload(songInfo &song, QString folder);
    void lyricsDownload(songInfo &song, QString folder);

    void updateSongCheckState(QString &id, bool isChecked);
    void clearSongs();
    void stopDownload();
    void setIsStopped(bool set);

    QList<songInfo> getSongs();
    int getSongsCount();

private:
    void setWorking(QProcess *process);
    void setupProcessLogging(QProcess *process, bool isLyrics = false);
    void cleanupProcess(int exitCode);
    
    bool isStopped;
    QList<songInfo> Songs;
    QProcess *currentProcess;
};


#endif // DOWNLOADMANAGER_H