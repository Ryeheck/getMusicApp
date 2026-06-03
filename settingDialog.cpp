#include "settingDialog.h"

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QString>
#include <QSettings>

settingDialog::settingDialog(QWidget *parent)
    : QDialog(parent)
{
    mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Formats:", this);
    mainLayout->addWidget(titleLabel);

    // Audio format
    QHBoxLayout *layoutAudio = new QHBoxLayout();
    QLabel *titleAudio = new QLabel("Audio:", this);
    titleAudio->setFixedWidth(50);
    layoutAudio->addWidget(titleAudio);

    formatAudio = new QComboBox(this);
    formatAudio->addItems({".mp3", ".aac", ".flac"});
    layoutAudio->addWidget(formatAudio);

    mainLayout->addLayout(layoutAudio);

    // Lyrics format
    QHBoxLayout *layoutLyrics = new QHBoxLayout();
    QLabel *titleLyrics = new QLabel("Lyrics:", this);
    titleLyrics->setFixedWidth(50);
    layoutLyrics->addWidget(titleLyrics);

    formatLyrics = new QComboBox(this);
    formatLyrics->addItems({".lrc", ".srt", ".vtt", ".txt"});
    layoutLyrics->addWidget(formatLyrics);

    mainLayout->addLayout(layoutLyrics);

    // Video format
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

    loadSetting();
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

QString settingDialog::getLyricsFormat() const
{
    return formatLyrics->currentText().split(" ").first();
}

void settingDialog::saveSetting()
{
    QSettings setting("config.conf", QSettings::IniFormat);

    // Formats
    setting.setValue("Formats/AudioFormat", getAudioFormat());
    setting.setValue("Formats/VideoFormat", getVideoFormat());
    setting.setValue("Formats/LyricsFormat", getLyricsFormat());

}

void settingDialog::loadSetting() 
{
    QSettings setting("config.conf", QSettings::IniFormat);

    // Formats
    formatAudio->setCurrentText(setting.value("Formats/AudioFormat", ".mp3").toString());
    formatVideo->setCurrentText(setting.value("Formats/VideoFormat", ".mp4").toString());
    formatLyrics->setCurrentText(setting.value("Formats/LyricsFormat", ".lrc").toString());
    
}