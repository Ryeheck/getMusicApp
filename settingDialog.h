#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>

class settingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit settingDialog(QWidget *parent = nullptr);
    // ~settingWindow() override;

    void saveSetting() const;
    QString getAudioFormat() const;
    QString getVideoFormat() const;
private:
    QVBoxLayout *mainLayout;
    QComboBox *formatAudio;
    QComboBox *formatVideo;
};

#endif // SETTINGWINDOW_H