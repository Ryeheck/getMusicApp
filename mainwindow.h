#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logView.h"
#include "downloadManager.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupBeforeDownload(bool set);
    void handleDownload(QPushButton *button, bool isLyrics);
    ~MainWindow() override;

    void allTitle();
    
private:
    void setupConnections();
    QWidget *centralWidget;
    QBoxLayout *layoutMain;
    QBoxLayout *layoutButtons;
    QHBoxLayout *layoutButtonsHOne;
    QHBoxLayout *layoutButtonsHTwo;
    
    logView *logs = nullptr;
    downloadManager *manager = nullptr;

    QLineEdit *inputURL;
    QLineEdit *inputFolder;
    QPushButton *titleButton;
    QPushButton *musicButton;
    QPushButton *clearListButton;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    QPushButton *stopButton;
    QPushButton *stopForNextButton;
    QPushButton *lyricsButton;
};
#endif // MAINWINDOW_H