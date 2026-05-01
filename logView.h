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

public:
    explicit logView(QWidget *parent = nullptr);
    // ~logView() override;

    void getTitle(QString url, bool startAfter=false, QString folder="Music");

    void log(const QString &message);
    void startDowload(QString folder="Music", QString url="");
    void nextDowload(QString folder);
    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();
    void getLog(QPlainTextEdit *plainText, QListWidget *listWidget);
    QList<QListWidgetItem *> getItems();
    int getItemsCount();
    void stopDowload();
    void setIsStoppedForNext(bool set);

private:
    QHBoxLayout *HLayout;
    QList<QListWidgetItem *> Items;
    QListWidget *listWidget;
    QPlainTextEdit *logText;
    QProcess *process = nullptr;

    bool isStoppedForNext = false;
    int currentDowloadIndex = -1;
};

#endif // LOGVIEW_H
