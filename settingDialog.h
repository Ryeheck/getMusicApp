#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>

class settingDialog : public QDialog
{
    Q_OBJECT

signals:
    void logMessageRequested(const QString &message);

public:
    explicit settingDialog(QWidget *parent = nullptr);
    // ~settingWindow() override;

    void saveSetting();
    void loadSetting();

    QString getAudioFormat()    const {  return formatAudio->currentText().split(" ").first();    };
    QString getVideoFormat()    const {  return formatVideo->currentText().split(" ").first();    };
    QString getLyricsFormat()   const {  return formatLyrics->currentText().split(" ").first();   };
    QString getVideoQuality()   const {  return qualityVideo->currentText().split(" ").first();   };
    QString getAudioQuality()   const {  return qualityAudio->currentText().split(" ").first();   };
    QString getCookiesBrowser() const {  return CookiesBrowser->currentText().split(" ").first(); };

private:
    QVBoxLayout *mainLayout;
    QComboBox *formatAudio;
    QComboBox *formatVideo;
    QComboBox *formatLyrics;
    QComboBox *qualityVideo;
    QComboBox *qualityAudio;
    QComboBox *CookiesBrowser;
};

#endif // SETTINGWINDOW_H