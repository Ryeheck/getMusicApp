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
    void startDownload(QString folder = "", bool lyrics = false);
    void nextDownload(QString folder);
    void lyricsDownloadForName(QString folder);

    void updateSongCheckState(QString &id, bool isChecked);
    void clearSongs();
    void setWorking(QProcess *process);
    void stopDownload();
    void setIsStoppedForNext(bool set);

    QList<songInfo> getSongs();
    int getSongsCount();

private:
    QList<songInfo> Songs;

    bool isStoppedForNext = false;
    int currentDownloadIndex = -1;
    QProcess *currentProcess;
};


#endif // DOWNLOADMANAGER_H