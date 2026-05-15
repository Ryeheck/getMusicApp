#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QListWidget>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProcess>

class logView : public QWidget
{
    Q_OBJECT

signals:
    void setupDownloadRequested(bool set);

public:
    explicit logView(QWidget *parent = nullptr);
    // ~logView() override;

    void log(const QString &message = "");
    void getLog(QPlainTextEdit *plainText, QListWidget *listWidget);
    void addPlaylistItem(QListWidgetItem *item);
    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();

    QList<QListWidgetItem *> getItems();
    int getItemsCount();

private:
    QHBoxLayout *HLayout;
    QList<QListWidgetItem *> Items;
    QListWidget *listWidget;
    QPlainTextEdit *logText;
};

#endif // LOGVIEW_H
