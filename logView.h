#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QListWidget>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProgressBar>

class logView : public QWidget
{
    Q_OBJECT

signals:
    void setupDownloadRequested(bool set);

public:
    explicit logView(QWidget *parent = nullptr);
    // ~logView() override;

    void log(const QString &message = "");
    void updateProgressBar(const int percent);
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
    QVBoxLayout *VLayout;
    QList<QListWidgetItem *> Items;
    QListWidget *listWidget;
    QPlainTextEdit *logText;
    QProgressBar *progressBar;
};

#endif // LOGVIEW_H
