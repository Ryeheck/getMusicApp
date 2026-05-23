#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include <QProgressBar>

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
    void setupDownloadRequested(const bool set);
    void songAdded(const songInfo *song);
    void progressBarRequested(QProgressBar *bar, const int percent);
    void updateStatusRequested(const songInfo *song);

public:
    explicit downloadManager(QObject *parent = nullptr);
    ~downloadManager() override;

    void getSongs(QString url, QString folder = "", bool startAfter = false, bool lyrics = false);
    void startDownload(QString folder = "", bool isLyrics = false);
    void songDownload(songInfo *song, QString folder);
    void lyricsDownload(songInfo *song, QString folder);

    void updateSongCheckState(QString &id, bool isChecked);
    void clearSongs();
    void stopDownload();
    void setIsStopped(bool set);

    static QString formatBytes(long long bytes);
    int getSongsCount();

private:
    void setupProgressBar(QProcess *process, QProgressBar *pBar, QString *status = nullptr);
    void setWorking(QProcess *process);
    void setupProcessLogging(QProcess *process, bool isLyrics = false);
    void cleanupProcess(int exitCode);

    bool isStopped;
    QList<songInfo *> Songs;
    QProcess *currentProcess;
};


#endif // DOWNLOADMANAGER_H