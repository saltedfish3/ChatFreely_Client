#ifndef VIDEOCONTROLBAR_H
#define VIDEOCONTROLBAR_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLayout>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>


class VideoControlBar : public QWidget
{
    Q_OBJECT
public:
    explicit VideoControlBar(QMediaPlayer* player, QAudioOutput* output, QWidget *parent = nullptr);

signals:

private:
    void initStyle();

    QMediaPlayer* player;
    QAudioOutput* output;

    QHBoxLayout* layout;
    QToolButton* btn_play;
    QToolButton* btn_audio;

    QLabel* label_time;

    QSlider* slider_video;
    QSlider* slider_audio;

    QComboBox* combobox_rate;
};

#endif // VIDEOCONTROLBAR_H
