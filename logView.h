#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "downloadManager.h"

#include <QPlainTextEdit>
#include <QList>
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

    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();
    void clearTitle();

    void log(const QString &message = "");
    void addItem(const mediaInfo *media);
    void updateStatus(int row, const QString &newStatus);
    void setWidget(int row, QWidget *widget);
    int findRowById(const QString &id);
    QList<QTableWidgetItem *> getItemsFromColumn(int column);

    void showLogText() {  logText->show();  };
    void hideLogText() {  logText->hide();  };
    void appendText(const QString &message) {  text->appendHtml(message);  };
    void updateProgressBar(QProgressBar *progressBar, const qint64 percent) {  progressBar->setValue(percent);  };
    void removeAlso(int row) {  titleWidget->removeRow(row);  };
    int getProgressBarPercent(QProgressBar *progressBar) {  return progressBar->value();  };
    int getTableWidgetCount() {  return titleWidget->rowCount();  };

private:
    QHBoxLayout *HLayout;
    QVBoxLayout *VLayout;
    QPlainTextEdit *logText;
    QPlainTextEdit *text;
    QTableWidget *titleWidget;
};

#endif // LOGVIEW_H
