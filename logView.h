#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QListWidget>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProcess>

class logView : public QPlainTextEdit
{
    Q_OBJECT

signals:
    void setupDownloadRequested(bool set);

public:
    explicit logView(QWidget *parent = nullptr);
    // ~logView() override;

    void getTitle(QString url, QString folder = "", bool startAfter = false, bool lyrics = false);
    void log(const QString message = "");
    void startDownload(QString folder = "", bool lyrics = false);
    void nextDownload(QString folder);
    void lyricsDownloadForName(QString folder);

    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();

    void setWorking();
    void stopDownload();
    void setIsStoppedForNext(bool set);
    QList<QListWidgetItem *> getItems();
    int getItemsCount();
    void getLog(QPlainTextEdit *plainText, QListWidget *listWidget);

private:
    QHBoxLayout *HLayout;
    QList<QListWidgetItem *> Items;
    QListWidget *listWidget;
    QPlainTextEdit *logText;
    QProcess *process = nullptr;

    bool isStoppedForNext = false;
    int currentDownloadIndex = -1;
};

#endif // LOGVIEW_H
