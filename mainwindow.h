#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logView.h"
#include "downloadManager.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMenu>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupBeforeDownload(bool set);
    void handleDownload(bool isSongs = false, bool isLyrics = false);
    ~MainWindow() override;

    void allTitle();
    
private slots:
    void onLogsToggled(bool checked);

private:
    QWidget *centralWidget;
    QBoxLayout *layoutMain;
    QBoxLayout *layoutBtns;
    QHBoxLayout *layoutBtnsHOne;
    QHBoxLayout *layoutBtnsHTwo;
    
    logView *logs = nullptr;
    downloadManager *manager = nullptr;

    QLineEdit *inputURL;
    QLineEdit *inputFolder;
    QPushButton *settingBtn;
    QPushButton *titleBtn;
    QPushButton *musicBtn;

    QToolButton *logsToolBtn;
    QMenu *menuLogsBtns;
    QAction *clearListAction;
    QAction *deselectAllAction;
    QAction *selectAllAction;
    QAction *logsAction;
    QAction *clearTitleAction;
    
    QPushButton *stopBtn;
    QPushButton *stopForNextBtn;
    QPushButton *lyricsBtn;
    QPushButton *videoBtn;
    

};
#endif // MAINWINDOW_H