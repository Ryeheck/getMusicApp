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
    formatAudio->addItems({"mp3", "aac", "flac"});

    layoutAudio->addWidget(formatAudio);
    mainLayout->addLayout(layoutAudio);

    // Lyrics format
    QHBoxLayout *layoutLyrics = new QHBoxLayout();
    QLabel *titleLyrics = new QLabel("Lyrics:", this);
    titleLyrics->setFixedWidth(50);
    layoutLyrics->addWidget(titleLyrics);

    formatLyrics = new QComboBox(this);
    formatLyrics->addItems({"lrc", "srt", "vtt", "txt"});

    layoutLyrics->addWidget(formatLyrics);
    mainLayout->addLayout(layoutLyrics);

    // Video format
    QHBoxLayout *layoutVideo = new QHBoxLayout();
    QLabel *titleVideo = new QLabel("Video:", this);
    titleVideo->setFixedWidth(50);
    layoutVideo->addWidget(titleVideo);

    formatVideo = new QComboBox(this);
    formatVideo->addItems({"mp4", "mkv", "webm"});

    layoutVideo->addWidget(formatVideo);
    mainLayout->addLayout(layoutVideo);

    // Quality
    QHBoxLayout *layoutVideoQuality = new QHBoxLayout();
    QLabel *titleVideoQuality = new QLabel("Quality:", this);
    titleVideoQuality->setFixedWidth(50);
    layoutVideoQuality->addWidget(titleVideoQuality);

    qualityVideo = new QComboBox(this);
    qualityVideo->addItems({"2160p60", "1440p60", "1080p60", "720p60", "480p60"});

    layoutVideoQuality->addWidget(qualityVideo);
    mainLayout->addLayout(layoutVideoQuality);

    QHBoxLayout *layoutAudioQuality = new QHBoxLayout();
    QLabel *titleAudioQuality = new QLabel("Quality:", this);
    titleAudioQuality->setFixedWidth(50);
    layoutAudioQuality->addWidget(titleAudioQuality);

    qualityAudio = new QComboBox(this);
    qualityAudio->addItems({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});

    layoutAudioQuality->addWidget(qualityAudio);
    mainLayout->addLayout(layoutAudioQuality);

    // Cookies
    QHBoxLayout *layoutCookiesBrowser = new QHBoxLayout();
    QLabel *titleCookiesBrowser = new QLabel("Cookies from browser:", this);
    titleCookiesBrowser->setFixedWidth(50);
    layoutCookiesBrowser->addWidget(titleCookiesBrowser);

    CookiesBrowser = new QComboBox(this);
    CookiesBrowser->addItems({"google", "chrome", "yandex", "firefox"});

    layoutCookiesBrowser->addWidget(CookiesBrowser);
    mainLayout->addLayout(layoutCookiesBrowser);

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

void settingDialog::saveSetting()
{
    QSettings setting("config.conf", QSettings::IniFormat);

    // Formats
    setting.setValue("Formats/AudioFormat", getAudioFormat());
    setting.setValue("Formats/VideoFormat", getVideoFormat());
    setting.setValue("Formats/LyricsFormat", getLyricsFormat());
    
    // Quality
    setting.setValue("Quality/VideoQuality", getVideoQuality());
    setting.setValue("Quality/AudioQuality", getAudioQuality());

    // Cookies
    setting.setValue("Cookies/CookiesBrowser", getCookiesBrowser());

}

void settingDialog::loadSetting() 
{
    QSettings setting("config.conf", QSettings::IniFormat);

    // Formats
    formatAudio->setCurrentText(setting.value("Formats/AudioFormat", ".mp3").toString());
    formatVideo->setCurrentText(setting.value("Formats/VideoFormat", ".mp4").toString());
    formatLyrics->setCurrentText(setting.value("Formats/LyricsFormat", ".lrc").toString());

    // Quality
    qualityVideo->setCurrentText(setting.value("Quality/VideoQuality", "2160p60").toString());
    qualityAudio->setCurrentText(setting.value("Quality/AudioQuality", "0").toString());

    // Cookies
    CookiesBrowser->setCurrentText(setting.value("Cookies/CookiesBrowser", "firefox").toString());

}