#include "videocontrolbar.h"

VideoControlBar::VideoControlBar(QMediaPlayer* player, QAudioOutput* output, QWidget *parent)
    : player(player), output(output), QWidget{parent}
{
    this->setAttribute(Qt::WA_StyledBackground);
    this->setMouseTracking(true);
    this->setObjectName("this");

    this->setFixedHeight(120);

    this->widget_control = new QWidget(this);
    this->widget_control->setObjectName("widget_control");

    this->layout = new QHBoxLayout(this->widget_control);
    this->layout->setContentsMargins(10, 4, 10, 4);
    this->layout->setSpacing(6);

    this->btn_play = new QToolButton(this->widget_control);
    this->btn_play->setObjectName("btn_play");
    this->btn_play->setIcon(colorToIcon(style()->standardIcon(QStyle::SP_MediaPlay), Qt::white));
    this->btn_play->setIconSize(QSize(18, 18));
    this->btn_play->setAutoRaise(true);

    this->label_currentTime = new QLabel("00:00:00", this->widget_control);
    this->label_currentTime->setObjectName("label_currentTime");
    this->label_currentTime->setAlignment(Qt::AlignCenter);

    this->label_totalTime = new QLabel("00:00:00", this->widget_control);
    this->label_totalTime->setObjectName("label_totalTime");
    this->label_totalTime->setAlignment(Qt::AlignCenter);

    this->slider_video = new QSlider(Qt::Horizontal, this->widget_control);
    this->slider_video->setObjectName("slider_video");
    this->slider_video->setRange(0, 0);

    this->btn_audio = new QToolButton(this->widget_control);
    this->btn_audio->setObjectName("btn_audio");
    this->btn_audio->setIcon(colorToIcon(style()->standardIcon(QStyle::SP_MediaVolume), Qt::white));
    this->btn_audio->setIconSize(QSize(18, 18));
    this->btn_audio->setAutoRaise(true);

    this->slider_audio = new QSlider(Qt::Vertical, this);
    this->slider_audio->setObjectName("slider_audio");
    this->slider_audio->setRange(0, 100);
    this->slider_audio->setFixedSize(20, 80);
    this->slider_audio->hide();

    int value = static_cast<int>(this->output->volume() * 100);
    this->slider_audio->setValue(value);
    this->lastVolume = value > 0 ? value : 100;

    this->timer_audioSlider = new QTimer(this);
    this->timer_audioSlider->setInterval(800);
    connect(this->timer_audioSlider, &QTimer::timeout, this, [this](){
        if(!this->slider_audio || !this->slider_audio->isVisible())
        {
            this->timer_audioSlider->stop();
            return;
        }

        QPoint pos = QCursor::pos();
        QRect btnRect(this->btn_audio->mapToGlobal(QPoint(0, 0)), this->btn_audio->size());
        QRect sliderRect(this->slider_audio->mapToGlobal(QPoint(0, 0)), this->slider_audio->size());

        if(btnRect.contains(pos) || sliderRect.contains(pos))
            return;

        this->slider_audio->hide();
        this->timer_audioSlider->stop();
    });

    this->btn_rate = new QToolButton(this->widget_control);
    this->btn_rate->setObjectName("btn_rate");
    this->btn_rate->setText("倍速");
    this->btn_rate->setCheckable(true);
    this->btn_rate->setChecked(false);
    this->btn_rate->setAutoRaise(true);
    this->btn_rate->setFixedWidth(54);

    this->menu_rate = new RateWidget(this->btn_rate, this);
    connect(this->menu_rate, &RateWidget::rateSelected, this, [this](qreal rate){
        if(this->player && rate > 0.0)
            this->player->setPlaybackRate(rate);
        if(this->btn_rate)
        {
            if(qFuzzyCompare(rate, 1.0))
                this->btn_rate->setText("倍速");
            else
                this->btn_rate->setText(QString::number(rate) + "x");
        }
    });

    connect(this->menu_rate, &RateWidget::closed, this, [this](){
        if(this->btn_rate->underMouse())
            return;
        this->btn_rate->setChecked(false);
    });

    this->layout->addWidget(this->btn_play);
    this->layout->addWidget(this->label_currentTime);
    this->layout->addWidget(this->slider_video, 1);
    this->layout->addWidget(this->label_totalTime);
    this->layout->addWidget(this->btn_rate);
    this->layout->addWidget(this->btn_audio);

    initStyle();

    this->installEventFilter(this);
    this->btn_audio->installEventFilter(this);
    this->slider_video->installEventFilter(this);

    connect(this->btn_play, &QToolButton::clicked, this, [this](){
        if(this->player->playbackState() == QMediaPlayer::PlayingState)
            this->player->pause();
        else
            this->player->play();
    });

    connect(this->player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state){
        this->btn_play->setIcon(colorToIcon(style()->standardIcon(state == QMediaPlayer::PlayingState ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay), Qt::white));
    });

    connect(this->player, &QMediaPlayer::durationChanged, this, [this](qint64 dur){
        this->slider_video->setRange(0, static_cast<int>(dur));
        this->label_currentTime->setText(formatTime(0));
        this->label_totalTime->setText(formatTime(dur));
    });

    connect(this->player, &QMediaPlayer::positionChanged, this, [this](qint64 pos){
        if(!this->slider_video->isSliderDown())
            this->slider_video->setValue(static_cast<int>(pos));
        this->label_currentTime->setText(formatTime(pos));
    });

    connect(this->slider_video, &QSlider::sliderMoved, this, [this](int value){
        this->player->setPosition(value);
    });

    connect(this->slider_audio, &QSlider::valueChanged, this, [this](int value){
        if(value > 0)
            this->lastVolume = value;
        this->output->setVolume(value / 100.0);
        this->output->setMuted(value == 0);
        this->btn_audio->setIcon(colorToIcon(style()->standardIcon(value == 0 ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume), Qt::white));
    });

    connect(this->btn_audio, &QToolButton::clicked, this, [this](){
        if(this->output->isMuted() || this->slider_audio->value() == 0)
            this->slider_audio->setValue(this->lastVolume > 0 ? this->lastVolume : 100);
        else
        {
            this->lastVolume = this->slider_audio->value();
            this->slider_audio->setValue(0);
        }
    });

    connect(this->btn_rate, &QToolButton::toggled, this, [this](bool checked){
        if(!this->menu_rate)
            return;

        if(checked)
            this->menu_rate->showAtBtn();
        else
            this->menu_rate->hide();
    });
}

void VideoControlBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        this->pressPos = event->pos();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void VideoControlBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && this->pressPos.x() > 0)
    {
        QPoint offset = event->pos() - this->pressPos;
        if(offset.manhattanLength() < 5)
            emit transAreaClicked();

        this->pressPos = QPoint(-1, -1);
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

bool VideoControlBar::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this->btn_audio && event->type() == QEvent::Enter)
    {
        updateAudioSlierPosistion();
        this->slider_audio->show();
        this->slider_audio->raise();

        if(!this->timer_audioSlider->isActive())
            this->timer_audioSlider->start();
    }

    if(obj == this->slider_video && event->type() == QEvent::MouseButtonPress)
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if(mouse->button() == Qt::LeftButton)
        {
            double ratio = qBound(0.0, static_cast<double>(mouse->pos().x()) / this->slider_video->width(), 1.0);
            int value = this->slider_video->minimum() + static_cast<int>(ratio * (this->slider_video->maximum() - this->slider_video->minimum()));

            this->slider_video->setValue(value);
            this->player->setPosition(value);
        }
    }

    if(obj == this->btn_audio && (event->type() == QEvent::Move || event->type() == QEvent::Resize))
    {
        updateAudioSlierPosistion();
    }

    return QWidget::eventFilter(obj, event);
}

void VideoControlBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if(this->widget_control)
        this->widget_control->setGeometry(0, this->height() - 36, this->width(), 36);
}

void VideoControlBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    emit userActivity();
}

void VideoControlBar::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    emit userActivity();
}

QString VideoControlBar::formatTime(qint64 ms)
{
    if(ms < 0)
        ms = 0;
    qint64 totalSec = ms / 1000;
    qint64 h = totalSec / 3600;
    qint64 m = (totalSec % 3600) / 60;
    qint64 s = totalSec % 60;

    if(h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void VideoControlBar::initStyle()
{
    this->setStyleSheet(R"(
        #widget_control
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
        #label_currentTime,#label_totalTime
        {
            color: white;
            font-size: 12px;
        }
        #slider_video::groove:horizontal
        {
            height: 4px;
            background: rgba(255, 255, 255, 60);
            border-radius: 2px;
        }
        #slider_video::sub-page:horizontal
        {
            background: #5B9BD5;
            border-radius: 2px;
        }
        #slider_video::handle:horizontal
        {
            background: white;
            width: 12px;
            height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        #slider_video::handle:horizontal:hover
        {
            background: #E8E8E8;
        }
        #slider_audio
        {
            background: rgba(30, 30, 30, 200);
            border-radius: 6px;
        }
        #slider_audio::groove:vertical
        {
            width: 4px;
            margin: 6px 0;
            background: rgba(255, 255, 255, 60);
            border-radius: 2px;
        }
        #slider_audio::sub-page:vertical
        {
            background: rgba(255, 255, 255, 60);
            margin:6px 0;
            border-radius: 2px;
        }
        #slider_audio::add-page:vertical
        {
            background: #5B9BD5;
            margin:6px 0;
            border-radius: 2px;
        }
        #slider_audio::handle:vertical
        {
            background: white;
            width: 14px;
            height: 14px;
            margin: 0 -5px;
            border-radius: 7px;
        }
        #slider_audio::handle:vertical:hover
        {
            background: #E8E8E8;
        }
        #btn_rate
        {
            background: transparent;
            color: white;
            border: none;
            padding: 3px 10px;
            border-radius: 4px;
            font-size: 10pt;
        }
        #btn_rate:hover
        {
            background: rgba(255, 255, 255, 40);
        }
        #btn_rate:pressed
        {
            background: rgba(255, 255, 255, 70);
        }
    )");
}

void VideoControlBar::updateAudioSlierPosistion()
{
    if(this->slider_audio && this->btn_audio)
    {
        QPoint btnCenter = this->btn_audio->mapTo(this, QPoint(this->btn_audio->width() / 2, this->btn_audio->height() / 2));

        int x = btnCenter.x() - this->slider_audio->width() / 2;
        int y = btnCenter.y() - this->btn_audio->height() / 2 - this->slider_audio->height() - 6;

        this->slider_audio->move(qMax(0, x), qMax(0, y));
        this->slider_audio->raise();
    }
}

QIcon VideoControlBar::colorToIcon(const QIcon &icon, const QColor &color, const QSize &size)
{
    QPixmap pix = icon.pixmap(size, QIcon::Normal, QIcon::On);
    if(pix.isNull())
        return icon;

    QImage img = pix.toImage();

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(img.rect(), color);
    painter.end();

    QPixmap result = QPixmap::fromImage(img);
    result.setDevicePixelRatio(pix.devicePixelRatio());

    return QIcon(result);
}