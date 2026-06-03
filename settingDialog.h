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
    QString getAudioFormat() const;
    QString getVideoFormat() const;
    QString getLyricsFormat() const;
    
private:
    QVBoxLayout *mainLayout;
    QComboBox *formatAudio;
    QComboBox *formatVideo;
    QComboBox *formatLyrics;
};

#endif // SETTINGWINDOW_H