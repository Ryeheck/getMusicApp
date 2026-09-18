#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "../core/downloadManager.h"

#include <QPlainTextEdit>
#include <QList>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTableWidget>
#include <QString>
#include <qnamespace.h>
#include <qobject.h>
#include <QGroupBox>

class logView : public QWidget
{
    Q_OBJECT

signals:
    void setupDownloadRequested(bool set);

public:
    explicit logView(QWidget *parent=nullptr);
    // ~logView() override;

    
    void clearAll();
    void clearSelect();
    void clearDeselect();
    void clearTitle();
    void retranslateUI();
    
    void colorLog(const QString &firstColor, const QString &firstMessage, 
                  const QString &lastColor="", const QString &lastMessage="");
    void log(const QString &message="");
    
    
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
    
public slots:
    void setMediaCheckedById(const QString &id);
    void updateStatusById(const QString &id, const QString &newStatus);
    void setSelectAllItem();
    void setDeselectAllItem();
    void addItem(const mediaInfo *media);

private:
    QHBoxLayout *HLayout;
    QVBoxLayout *VLayout;
    QPlainTextEdit *logText;
    QPlainTextEdit *text;
    QTableWidget *titleWidget;
    QGroupBox *menuGroupBox;
    QVBoxLayout *groupLayout;
};

#endif // LOGVIEW_H
