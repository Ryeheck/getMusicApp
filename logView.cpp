#include "logView.h"
#include "mainwindow.h"

#include <QPlainTextEdit>
#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QHBoxLayout>
#include <QDir>
#include <QObject>

void logView::getLog(QPlainTextEdit *plainText, QListWidget *listWidget)
{ 
    plainText = this->logText;
    listWidget = this->listWidget;
}

QList<QListWidgetItem *> logView::getItems()
{
    return Items;
}

logView::logView(QWidget *parent)
    : QWidget(parent)
{
    logText = new QPlainTextEdit(this);
    logText->setReadOnly(true);

    listWidget = new QListWidget(this);
    HLayout = new QHBoxLayout(this);

    HLayout->addWidget(logText);
    HLayout->addWidget(listWidget);
}

void logView::addPlaylistItem(QListWidgetItem *item)
{
    listWidget->addItem(item);
    Items.append(item);
}

void logView::setSelectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Checked);
    }
    update();
}

void logView::setDeselectAllItem()
{
    for(QListWidgetItem *item : Items)
    {
        item->setCheckState(Qt::Unchecked);
    }
    update();
}

int logView::getItemsCount()
{
    return Items.size();
}

void logView::clearAll()
{
    listWidget->clear();
    Items.clear();
}

/*
void logView::clearSelect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Checked)  
    }
}

void logView::clearDeselect()
{
    for(QListWidgetItem *item : Items)
    {
        if (item->checkState() == Qt::Unchecked)  
    }
}
*/

void logView::log(const QString &message)
{
    logText->setReadOnly(true);

    if (logText) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        logText->appendHtml("[" + time + "] " + message);
    }
}
