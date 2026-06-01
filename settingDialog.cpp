#include "settingDialog.h"

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDialogButtonBox>

settingDialog::settingDialog(QWidget *parent)
    : QDialog(parent)
{
    mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Formats:", this);
    mainLayout->addWidget(titleLabel);

    // Audio formats
    QHBoxLayout *layoutAudio = new QHBoxLayout();
    QLabel *titleAudio = new QLabel("Audio:", this);
    titleAudio->setFixedWidth(50);
    layoutAudio->addWidget(titleAudio);

    formatAudio = new QComboBox(this);
    formatAudio->addItems({".mp3", ".aac", ".flac"});
    layoutAudio->addWidget(formatAudio);

    mainLayout->addLayout(layoutAudio);

    // Video formats
    QHBoxLayout *layoutVideo = new QHBoxLayout();
    QLabel *titleVideo = new QLabel("Video:", this);
    titleVideo->setFixedWidth(50);
    layoutVideo->addWidget(titleVideo);

    formatVideo = new QComboBox(this);
    formatVideo->addItems({".mp4", ".mkv", ".webm"});
    layoutVideo->addWidget(formatVideo);

    mainLayout->addLayout(layoutVideo);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, 
        this);

    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this] () {
        saveSetting();
        accept();
    });

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

QString settingDialog::getAudioFormat() const
{
    return formatAudio->currentText().split(" ").first();
}

QString settingDialog::getVideoFormat() const
{
    return formatVideo->currentText().split(" ").first();
}

void settingDialog::saveSetting() const
{
    QFile file("setting.txt");

    if (file.open(QIODeviceBase::WriteOnly | QIODevice::Text)) {
        QTextStream output(&file);

        output << "AudioFormat=" << getAudioFormat() << "\n";
        output << "VideoFormat=" << getVideoFormat() << "\n";

        file.close();
    }

    
}