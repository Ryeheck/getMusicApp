#include "logView.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QHBoxLayout>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>

void logView::lyricsDowloadForName(QString folder)
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
        lyricsDowloadForName(folder);
        return;
    }
    
    process = new QProcess();

    connect(process, &QProcess::readyReadStandardOutput, [this] () {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();

        if (!output.isEmpty()) {
            this->log(QString("<span style='color:%1;'>%2</span>").arg("white", "INFO: ") + output);
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data);

        this->log(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        this->log(QString("<span style='color:%1;'>%2</span>").arg("white", output));

        process->deleteLater();
        currentDowloadIndex++;
        lyricsDowloadForName(folder);
    });

    QString appDir = qApp->applicationDirPath();
    setWorking();

    QString songName = item->text();

    QStringList args;
    args << item->text()
         << "-p" << "Lrclib" << "NetEase" << "Megalobiz" << "Musixmatch"
         << "-o" << folder + "/" + songName + ".lrc"
         << "--verbose";

    log(QString("<span style='color:%1;'>%2</span>").arg("white", ": Lyrics: " + songName));
    
    process->start(appDir + "/syncedlyrics_bin", args);

    // syncedlyrics "Nirvana - Smells Like Teen Spirit" -o "Nirvana.lrc"
}