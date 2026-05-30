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

    tableWidget = new QTableWidget(this);
    
    VLayout = new QVBoxLayout(this);
    HLayout = new QHBoxLayout();

    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(4);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->verticalHeader()->setVisible(false);

    QStringList headers = {"Name", "Size", "Status", "Action"};
    tableWidget->setHorizontalHeaderLabels(headers);
    
    QHeaderView *header = tableWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    HLayout->addWidget(logText);
    HLayout->addWidget(tableWidget);
    
    VLayout->addLayout(HLayout);
}

void logView::setSelectAllItem()
{
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        tableWidget->item(row, 0)->setCheckState(Qt::Checked);
    }
    update();
}

void logView::setDeselectAllItem()
{
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        tableWidget->item(row, 0)->setCheckState(Qt::Unchecked);
    }
    update();
}

QList<QTableWidgetItem *> logView::getItemsFromColumn(int column)
{
    QList<QTableWidgetItem *> Items;
    
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = tableWidget->item(row, column);

        if(item)  Items.append(item);
    }
    return Items;
}

int logView::getTableWidgetCount()
{
    return tableWidget->rowCount();
}

void logView::clearAll()
{
    tableWidget->clearContents();
    tableWidget->setRowCount(0);
}

void logView::log(const QString &message)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->appendHtml("[" + time + "] " + message);
}

void logView::updateProgressBar(QProgressBar *progressBar, const int percent)
{
    progressBar->setValue(percent);
}

int logView::getProgressBarPercent(QProgressBar *progressBar)
{
    return progressBar->value();
}

void logView::addItem(const songInfo *song)
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);

    QTableWidgetItem *itemName = new QTableWidgetItem(song->name);
    itemName->setFlags(itemName->flags() | Qt::ItemIsUserCheckable);
    itemName->setCheckState(Qt::Unchecked);
    itemName->setForeground(Qt::white);
    itemName->setData(Qt::UserRole, song->id);

    tableWidget->setItem(row, 0, itemName);
    tableWidget->setItem(row, 1, new QTableWidgetItem(downloadManager::formatBytes(song->size)));
    tableWidget->setItem(row, 2, new QTableWidgetItem(song->status));

    if (song->widget) {
        song->widget->setParent(tableWidget);   
        tableWidget->setCellWidget(row, 3, song->widget);
    }
}

void logView::updateStatus(int row, const QString &newStatus)
{
    QTableWidgetItem *item = tableWidget->item(row, 2);

    if (item != nullptr)  item->setText(newStatus);
}

void logView::setWidget(int row, QWidget *widget)
{
    if (widget)  tableWidget->setCellWidget(row, 3, widget);
}

void logView::removeAlso(int row)
{
    tableWidget->removeRow(row);
}

int logView::findRowById(const QString &id)
{
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        QString itemId = tableWidget->item(row, 0)->data(Qt::UserRole).toString();

        if (itemId == id)  return row;
    }

    return -1;
}