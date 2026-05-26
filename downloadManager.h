#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include <QProgressBar>
#include <QMap>

struct songInfo {
    QString id;
    QString name;
    QString status;
    QWidget *widget;
    long long size;
    bool isChecked = false;
};

class downloadManager : public QObject
{
    Q_OBJECT

signals:
    void logMessageRequested(const QString &message);
    void activeTasksCountChanged(const int count);
    void songAdded(const songInfo *song);
    void progressBarRequested(QProgressBar *bar, const int percent);
    void updateStatusRequested(const songInfo *song);

public:
    explicit downloadManager(QObject *parent = nullptr);
    ~downloadManager() override;

    void getSongs(const QString &url, const QString &folder = "", bool startAfter = false, bool lyrics = false);
    void startDownload(const QString &folder = "", bool isLyrics = false);
    void songDownload(songInfo *song, const QString &folder);
    void lyricsDownload(songInfo *song, const QString &folder);

    void updateSongCheckState(const QString &id, bool isChecked);
    void clearSongs();
    void stopDownload();
    void setIsStopped(bool set);

    static QString formatBytes(long long bytes);
    int getSongsCount();

private:
    void setupProgressBar(const QString &id, QProgressBar *pBar, QString *status = nullptr);
    void setWorking(QProcess *process);
    void setupProcessLogging(const QString &id, bool isLyrics = false);
    void cleanupProcess(const QString &id, int exitCode);

    QMap<QString, QProcess *> _activeProcesses; 
    bool isStopped;
    QList<songInfo *> Songs;
};


#endif // DOWNLOADMANAGER_H