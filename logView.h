#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QListWidget>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>

class logView : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit logView(QWidget *parent = nullptr);
    // ~logView() override;

    void getTitle(QString url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ");

    void log(const QString &message);
    void startDowload(QString folder="Music/songs");
    void nextDowload(QString folder);
    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();

    void getLog(QPlainTextEdit *plainText, QListWidget *listWidget);
    QList<QListWidgetItem *> getItems();

private:
    QHBoxLayout *HLayout;
    QList<QListWidgetItem *> Items;
    QListWidget *listWidget;
    QPlainTextEdit *logText;

    int currentDowloadIndex = -1;
};

#endif // LOGVIEW_H
