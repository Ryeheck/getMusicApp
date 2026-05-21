#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "downloadManager.h"

#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QListWidget>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTableWidget>

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
    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();

    QList<QTableWidgetItem *> getItemsFromColumn(int column);
    int getTableWidgetCount();

    int findRowById(const QString &id);
    void removeAlso(int row);
    void addItem(const songInfo *song);
    void updateStatus(int row, const QString &newStatus);
    void setWidget(int row, QWidget *widget);

private:
    QHBoxLayout *HLayout;
    QVBoxLayout *VLayout;
    QPlainTextEdit *logText;
    QProgressBar *progressBar;
    QTableWidget *tableWidget;
};

#endif // LOGVIEW_H
