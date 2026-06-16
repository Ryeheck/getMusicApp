#include "logView.h"
#include "downloadManager.h"

#include <QPlainTextEdit>
#include <QString>
#include <QDateTime>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>


logView::logView(QWidget *parent)
    : QWidget(parent)
{
    logText = new QPlainTextEdit(this);
    logText->setReadOnly(true);
    logText->document()->setMaximumBlockCount(500);
    logText->hide();

    text = new QPlainTextEdit(this);
    text->document()->setMaximumBlockCount(500);
    text->setReadOnly(true);

    titleWidget = new QTableWidget(this);

    VLayout = new QVBoxLayout(this);
    HLayout = new QHBoxLayout();

    titleWidget->setRowCount(0);
    titleWidget->setColumnCount(4);
    titleWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    titleWidget->verticalHeader()->setVisible(false);

    QStringList headers = {"Name", "Size", "Status", "Action"};
    titleWidget->setHorizontalHeaderLabels(headers);
    
    QHeaderView *header = titleWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    HLayout->addWidget(text);
    HLayout->addWidget(titleWidget);
    
    VLayout->addLayout(HLayout);
    VLayout->addWidget(logText);
}

void logView::setSelectAllItem()
{
    for(int row = 0; row < titleWidget->rowCount(); ++row)
    {
        titleWidget->item(row, 0)->setCheckState(Qt::Checked);
    }
    update();
}

void logView::setDeselectAllItem()
{
    for(int row = 0; row < titleWidget->rowCount(); ++row)
    {
        titleWidget->item(row, 0)->setCheckState(Qt::Unchecked);
    }
    update();
}

QList<QTableWidgetItem *> logView::getItemsFromColumn(int column)
{
    QList<QTableWidgetItem *> Items;
    
    for(int row = 0; row < titleWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = titleWidget->item(row, column);

        if(item)  Items.append(item);
    }
    return Items;
}

void logView::clearAll()
{
    titleWidget->clearContents();
    titleWidget->setRowCount(0);
    text->clear();
    logText->clear();
}

void logView::clearTitle() 
{  
    titleWidget->clearContents();  
    titleWidget->setRowCount(0);
}

void logView::log(const QString &message)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->appendHtml("[" + time + "] " + message);
}

void logView::addItem(const mediaInfo *song)
{
    int row = titleWidget->rowCount();
    titleWidget->insertRow(row);

    QTableWidgetItem *itemName = new QTableWidgetItem(song->name);
    itemName->setFlags(itemName->flags() | Qt::ItemIsUserCheckable);
    itemName->setCheckState(Qt::Unchecked);
    itemName->setForeground(Qt::white);
    itemName->setData(Qt::UserRole, song->id);

    titleWidget->setItem(row, 0, itemName);
    titleWidget->setItem(row, 1, new QTableWidgetItem(downloadManager::formatBytes(song->size)));
    titleWidget->setItem(row, 2, new QTableWidgetItem(song->status));

    if (song->widget) {
        song->widget->setParent(titleWidget);   
        titleWidget->setCellWidget(row, 3, song->widget);
    }
}

void logView::updateStatus(int row, const QString &newStatus)
{
    QTableWidgetItem *item = titleWidget->item(row, 2);

    if (item != nullptr)  item->setText(newStatus);
}

void logView::setWidget(int row, QWidget *widget) 
{  
    if (widget)  titleWidget->setCellWidget(row, 3, widget);
}

int logView::findRowById(const QString &id)
{
    for(int row = 0; row < titleWidget->rowCount(); ++row)
    {
        QString itemId = titleWidget->item(row, 0)->data(Qt::UserRole).toString();

        if (itemId == id)  return row;
    }

    return -1;
}