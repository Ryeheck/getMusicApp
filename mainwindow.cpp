#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QProcess>
#include <QString>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->inputFolder->setPlaceholderText("Enter folder... (default: Music/songs): ");
    ui->inputURL->setPlaceholderText("Enter URL (youtube)");
    
    connect(ui->startButton, &QPushButton::clicked, [this] () {
        this->startDowload();
    });
}

void MainWindow::startDowload(QString url, QString folder)
{
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

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this] (int exitCode) {
        QString output = (exitCode == 0 ? "Done!" : "Error");
        QString message = QString("<span style='color:%1;'>%2</span>").arg("white", output);
        this->log(message);
    });

    
    if (!ui->inputFolder->text().isEmpty())  folder = ui->inputFolder->text();
    if (!ui->inputURL->text().isEmpty())     url = ui->inputURL->text();

    QStringList args;
    args << "--no-playlist";
    args << "-x" << "--audio-format" << "mp3" << "-o";
    args << folder + "/%(title)s.mp3 ";
    args << url;

    process->start("yt-dlp", args);
    
}


void MainWindow::log(const QString &message)
{
    ui->logView->setReadOnly(true);

    if (ui && ui->logView) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        ui->logView->appendHtml("[" + time + "] " + message);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
