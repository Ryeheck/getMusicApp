#include "logView.h"

#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QPushButton>
#include <QHBoxLayout>

#define MAX_SONGS   50

void logView::getLog(QPlainTextEdit *plainText, QListWidget *listWidget)
{ 
    plainText = this->logText;
    listWidget = this->listWidget;
}

QList<QListWidgetItem *> logView::getItems()
{
    return this->Items;
}

logView::logView(QWidget *parent)
{
    logText = new QPlainTextEdit(this);
    logText->setReadOnly(true);

    listWidget = new QListWidget(this);
    HLayout = new QHBoxLayout(this);

    HLayout->addWidget(logText);
    HLayout->addWidget(listWidget);
}

void logView::getTitle(QString url)
{
    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, [this, process, url] () {
        QString output = process->readAllStandardOutput();

        QStringList Names = output.split('\n', Qt::SkipEmptyParts);
        
        for(int i = 0; i < Names.size() - 1 && i < MAX_SONGS; i += 2) {
            QListWidgetItem *item = new QListWidgetItem(Names[i], listWidget);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setForeground(Qt::white);
            item->setData(Qt::UserRole, Names[i + 1]);

            Items.append(item);
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", output);
        this->log(message);
    });


    QStringList args;
    args << "--flat-playlist";
    args << "--get-title" << "--get-url"; 
    args << url;

    process->start("yt-dlp", args);
    
}

// void MainWindow::allTitle();

void logView::startDowload(QString folder)
{
    currentDowloadIndex = 0;
    nextDowload(folder);
}

void logView::nextDowload(QString folder)
{
    if (currentDowloadIndex >= Items.size()) {
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", "All Done!");
        this->log(message);
        currentDowloadIndex = -1;
        return;
    }

    QListWidgetItem *item = Items[currentDowloadIndex];
    if (item->checkState() != Qt::Checked) {
        currentDowloadIndex++;
        nextDowload(folder);
        return;
    }

    QProcess *process = new QProcess(this);
    
    connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        QString output = process->readAllStandardOutput();
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", "INFO: ") + output;
        this->log(message);
    });

    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QString output = process->readAllStandardError();
        QString message = QString("<span style='color:%1;'>%2</span>").arg("red", "ERROR: ") + output;
        this->log(message);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, folder, process] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", output);
        this->log(message);

        process->deleteLater();
        currentDowloadIndex++;
        nextDowload(folder);
    });

    QStringList args;
    args << "--no-playlist";
    args << "-x" << "--audio-format" << "mp3" << "-o";
    args << folder + "/%(title)s.mp3 ";
    args << item->data(Qt::UserRole).toString();

    QString message = QString("<span style='color:%1;'>%2</span>").arg("white", ": " + item->text());
    log(message);

    process->start("yt-dlp", args);
}

void logView::setSelectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Checked);
    }
    update();
}

void logView::setDeselectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Unchecked);
    }
    update();
}

void logView::clearAll()
{
    listWidget->clear();
    Items.clear();
}

/*
void logView::clearSelect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Checked)  
    }
}

void logView::clearDeselect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Unchecked)  
    }
}
*/

void logView::log(const QString &message)
{
    logText->setReadOnly(true);

    if (logText) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        logText->appendHtml("[" + time + "] " + message);
    }
}
