#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "downloadManager.h"

#include <QPlainTextEdit>
#include <QList>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTableWidget>
#include <QString>

class logView : public QWidget
{
    Q_OBJECT

signals:
    void setupDownloadRequested(bool set);

public:
    explicit logView(QWidget *parent=nullptr);
    // ~logView() override;

    void setSelectAllItem();
    void setDeselectAllItem();
    void clearAll();
    void clearSelect();
    void clearDeselect();
    void clearTitle();

    void colorLog(const QString &firstColor, const QString &firstMessage, 
                  const QString &lastColor="", const QString &lastMessage="");
    void log(const QString &message="");
    void addItem(const mediaInfo *media);
    void updateStatus(int row, const QString &newStatus);
    void setWidget(int row, QWidget *widget);
    int findRowById(const QString &id);
    QList<QTableWidgetItem *> getItemsFromColumn(int column);

    void showLogText()                                   {  logText->show();  };
    void hideLogText()                                   {  logText->hide();  };
    void appendText(const QString &message)              {  text->appendHtml(QString("<span style='color:silver;'>%1</span>").arg(message));  };
    void updatePBar(QProgressBar *pBar, qint64 percent)  {  pBar->setValue(percent);         };
    void removeAlso(int row)                             {  titleWidget->removeRow(row);     };
    int getPBarPercent(QProgressBar *pBar)               {  return pBar->value();            };
    int getTableWidgetCount()                            {  return titleWidget->rowCount();  };

private:
    QHBoxLayout *HLayout;
    QVBoxLayout *VLayout;
    QPlainTextEdit *logText;
    QPlainTextEdit *text;
    QTableWidget *titleWidget;
};

#endif // LOGVIEW_H
