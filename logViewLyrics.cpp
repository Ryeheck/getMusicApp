#include "logView.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QHBoxLayout>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>

void logView::lyricsDownloadForName(QString folder)
{
    if (isStoppedForNext)  return;

    if (currentDownloadIndex >= Items.size()) {
        this->log(QString("<span style='color:silver;'>All Done!</span>"));
        currentDownloadIndex = -1;
        emit setupDownloadRequested(false);
        return;
    }

    QListWidgetItem *item = Items[currentDownloadIndex];
    if (item->checkState() != Qt::Checked) {
        currentDownloadIndex++;
        lyricsDownloadForName(folder);
        return;
    }
    
    process = new QProcess();

    connect(process, &QProcess::readyReadStandardOutput, [this] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            this->log(QString("<span style='color:DarkSeaGreen;'>INFO: %1</span>").arg(output));
        }
    });

   connect(process, &QProcess::readyReadStandardError, [this] () {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);
        
        output.replace("DEBUG:", QString("<span style='color:silver;'>DEBUG: </span>"));
        output.replace("INFO:", QString("<span style='color:silver;'>INFO: </span>"));
        output.replace("WARNING:", QString("<span style='color:DarkOrange;'>WARNING: </span>"));
        output.replace("ERROR:", QString("<span style='color:IndianRed;'>ERROR: </span>"));

        log(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        this->log(QString("<span style='color:silver;'>%1</span>").arg(output));

        process->deleteLater();
        currentDownloadIndex++;
        lyricsDownloadForName(folder);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking();

    QString songName = item->text();

    QStringList args;
    args << songName
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + ".lrc"
         << "--verbose";

    log(QString("<span style='color:silver;'>Lyrics: %1</span>").arg(songName));
    
    process->start(appDir + "/syncedlyrics_bin", args);

    // syncedlyrics "Nirvana - Smells Like Teen Spirit" -o "Nirvana.lrc"
}