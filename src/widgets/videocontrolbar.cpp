#include "videocontrolbar.h"

VideoControlBar::VideoControlBar(QMediaPlayer* player, QAudioOutput* output, QWidget *parent)
    : player(player), output(output), QWidget{parent}
{
    this->setAttribute(Qt::WA_StyledBackground);
    this->setMouseTracking(true);
    this->setObjectName("this");

    this->layout = new QHBoxLayout(this);
    this->layout->setContentsMargins(10, 4, 10, 4);
    this->layout->setSpacing(8);

    this->btn_play = new QToolButton(this);
    this->btn_play->setObjectName("btn_play");
    this->btn_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    this->btn_play->setIconSize(QSize(18, 18));
    this->btn_play->setAutoRaise(true);

    this->label_time = new QLabel("00:00:00 / 00:00:00", this);
    this->label_time->setObjectName("label_time");
    this->label_time->setMinimumWidth(110);
    this->label_time->setAlignment(Qt::AlignCenter);

    this->slider_video = new QSlider(Qt::Horizontal, this);
    this->slider_video->setObjectName("slider_video");
    this->slider_video->setRange(0, 0);

    this->btn_audio = new QToolButton(this);
    this->btn_audio->setObjectName("btn_audio");
    this->btn_audio->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    this->btn_audio->setIconSize(QSize(18, 18));
    this->btn_audio->setAutoRaise(true);

    this->slider_audio = new QSlider(Qt::Horizontal, this);
    this->slider_audio->setObjectName("slider_audio");
    this->slider_audio->setRange(0, 100);
    this->slider_audio->setFixedWidth(70);

    this->combobox_rate = new QComboBox(this);
    this->combobox_rate->setObjectName("combobox_rate");
    this->combobox_rate->addItems({"0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"});
    this->combobox_rate->setCurrentIndex(2);

    this->layout->addWidget(this->btn_play);
    this->layout->addWidget(this->label_time);
    this->layout->addWidget(this->slider_video, 1);
    this->layout->addWidget(this->btn_audio);
    this->layout->addWidget(this->slider_audio);
    this->layout->addWidget(this->combobox_rate);

    initStyle();
}

void VideoControlBar::initStyle()
{
    this->setStyleSheet(R"(
        #this
        {
            background: rgba(30, 30, 30, 180);
            border-radius: 8px;
        }
        #btn_play,#btn_audio
        {
            background: transparent;
            border: none;
            padding: 4px;
            border-radius: 4px;
        }
        #btn_play:hover,
        #btn_audio:hover
        {
            background: rgba(255, 255, 255, 40);
        }
        #btn_play:pressed,
        #btn_audio:pressed
        {
            background: rgba(255, 255, 255, 70);
        }
        #label_time
        {
            color: white;
            font-size: 12px;
        }
        QSlider::groove:horizontal
        {
            height: 4px;
            background: rgba(255, 255, 255, 60);
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal
        {
            background: #5B9BD5;
            border-radius: 2px;
        }
        QSlider::handle:horizontal
        {
            background: white;
            width: 12px;
            height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        QSlider::handle:horizontal:hover
        {
            background: #E8E8E8;
        }
        #combobox_rate
        {
            background: rgba(255, 255, 255, 30);
            color: white;
            border: none;
            padding: 2px 8px;
            border-radius: 4px;
            font-size: 12px;
        }
        #combobox_rate:hover
        {
            background: rgba(255, 255, 255, 50);
        }
        #combobox_rate:drop-down
        {
            border: none;
            width: 16px;
        }
        #combobox_rate QAbstractItemView
        {
            background: rgba(30, 30, 30, 230);
            color: white;
            selection-background-color: #5B9BD5;
            border: none;
            outline: none;
        }
    )");
}