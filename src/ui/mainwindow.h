#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logView.h"
#include "../core/downloadManager.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QTranslator>
#include <qcoreapplication.h>
#include <qmenu.h>
#include <qtoolbutton.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    void setupBeforeDownload(bool set);
    void handleDownload(bool isSongs=false, bool isLyrics=false);
    ~MainWindow() override;

    void allTitle();
    
private slots:
    void onLogsToggled(bool checked);

private:
    void retranslateUI();
    void switchLanguageClicked();
    void changeEvent(QEvent *event) override;
    bool m_russian;
    QTranslator *m_translator;
    
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

    QToolButton *toolBtn;
    QToolButton *mediaToolBtn;
    QMenu *menuMedia;
    QMenu *menuTool;
    QAction *clearAll;
    QAction *deselectAllMedia;
    QAction *logsAction;
    QAction *clearMedia;
    QAction *checkForUpdate;
    QAction *changeLanguage;
    QAction *prepareProgram;
    QAction *selectAllMedia;

    QPushButton *stopBtn;
    QPushButton *stopForNextBtn;
    QPushButton *lyricsBtn;
    QPushButton *videoBtn;
    

};
#endif // MAINWINDOW_H