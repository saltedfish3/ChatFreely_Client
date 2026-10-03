#include "videopreviewwidget.h"

VideoPreviewWidget &VideoPreviewWidget::getPreviewWidget()
{
    static VideoPreviewWidget widget;
    return widget;
}

void VideoPreviewWidget::setVideoUrl(const QUrl &url, qint64 totalSize)
{
    if(!url.isValid() || url.isEmpty())
    {
        this->view_video->setState(VideoContentView::State::Error);
        this->view_video->setErrorText("视频已过期");
        return;
    }

    this->currentVideoUrl = url.toString();
    this->currentVideoSize = totalSize;
    this->updateSaveButton();

    this->view_video->setVideoNativeSize(QSize());
    this->view_video->setState(VideoContentView::State::Loading);

    QUrl playUrl = url;
    QString savePath = getSavePath();
    if(!savePath.isEmpty() && QFileInfo::exists(savePath))
        playUrl = QUrl::fromLocalFile(savePath);

    this->player->setSource(playUrl);
    this->player->play();
    this->view_video->setFocus(Qt::OtherFocusReason);
}

void VideoPreviewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect = QRect(QRect(0, 0, this->width(), this->height()));
    rect.adjust(1, 1, -1, -1);

    painter.setBrush(QColor(240, 240, 240));
    painter.setPen(QPen(QColor(0, 0, 0, 26), 1));
    painter.drawRoundedRect(rect, 8, 8);
}

bool VideoPreviewWidget::eventFilter(QObject *obj, QEvent *ev)
{
    //过滤窗口拖动事件
    if (obj != this && obj != this->view_video && obj != this->view_video->viewport())
        return QObject::eventFilter(obj, ev);

    if(ev->type() != QEvent::MouseButtonPress && ev->type() != QEvent::MouseButtonRelease && ev->type() != QEvent::MouseMove)
        return QObject::eventFilter(obj, ev);

    QMouseEvent* qme = static_cast<QMouseEvent*>(ev);
    if(!qme)
        return QObject::eventFilter(obj, ev);

    QPoint posThis;
    if(obj == this)
        posThis = qme->pos();
    else if(obj == this->view_video)
        posThis = this->view_video->mapTo(this, qme->pos());
    else
        posThis = this->view_video->viewport()->mapTo(this, qme->pos());

    switch(ev->type())
    {
    case QEvent::MouseButtonPress:
    {
        if(qme->button() != Qt::LeftButton)
            break;

        this->edge = edgeAt(posThis);
        if(this->edge != Edge::None)
        {
            this->pos_startScaleGlobal = qme->globalPosition().toPoint();
            this->rect_startGeometry = this->geometry();
            return true;
        }

        if(posThis.y() <= this->widget_titleBar->height())
        {
            this->pos_widget = posThis;
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        this->pos_widget = QPoint();
        this->updateCursor(this->edgeAt(posThis));
        break;
    }
    case QEvent::MouseMove:
    {
        this->updateCursor(this->edgeAt(posThis));

        if(this->edge != Edge::None && (qme->buttons() & Qt::LeftButton))
        {
            QPoint offset = qme->globalPosition().toPoint() - this->pos_startScaleGlobal;
            QRect geometry = this->rect_startGeometry;

            switch(this->edge)
            {
            case Edge::Left:
                geometry.setLeft(this->rect_startGeometry.left() + offset.x());
                break;
            case Edge::Right:
                geometry.setRight(this->rect_startGeometry.right() + offset.x());
                break;
            case Edge::Top:
                geometry.setTop(this->rect_startGeometry.top() + offset.y());
                break;
            case Edge::Bottom:
                geometry.setBottom(this->rect_startGeometry.bottom() + offset.y());
                break;
            case Edge::TopLeft:
                geometry.setTopLeft(this->rect_startGeometry.topLeft() + offset);
                break;
            case Edge::TopRight:
                geometry.setTopRight(this->rect_startGeometry.topRight() + offset);
                break;
            case Edge::BottomLeft:
                geometry.setBottomLeft(this->rect_startGeometry.bottomLeft() + offset);
                break;
            case Edge::BottomRight:
                geometry.setBottomRight(this->rect_startGeometry.bottomRight() + offset);
                break;
            default:
                break;
            }
            if(geometry.width() < this->minimumWidth())
            {
                if(this->edge == Edge::Left || this->edge == Edge::TopLeft || this->edge == Edge::BottomLeft)
                    geometry.setLeft(geometry.right() - this->minimumWidth());
                else
                    geometry.setRight(geometry.left() + this->minimumWidth());
            }
            if(geometry.height() < this->minimumHeight())
            {
                if(this->edge == Edge::Top || this->edge == Edge::TopLeft || this->edge == Edge::TopRight)
                    geometry.setTop(geometry.bottom() - this->minimumHeight());
                else
                    geometry.setBottom(geometry.top() + this->minimumHeight());
            }

            this->setGeometry(geometry);
            return true;
        }

        //移动
        if(this->pos_widget.isNull() == false && (qme->buttons() & Qt::LeftButton))
        {
            this->move(qme->globalPosition().toPoint() - this->pos_widget);
            return true;
        }
        else
            this->pos_widget = QPoint();
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(obj, ev);
}

void VideoPreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int titleHeight = this->widget_titleBar ? this->widget_titleBar->height() : 40;

    if(this->widget_titleBar)
    {
        this->widget_titleBar->resize(this->width() - 2, 40);
        this->widget_titleBar->move(1, 1);   
    }
    if(this->view_video)
    {
        this->view_video->resize(this->width() - 2 * 3, this->height() - titleHeight - 2 * 3);
        this->view_video->move(3, titleHeight + 1);
    }
    update();
}

void VideoPreviewWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(this->view_video)
        this->view_video->setFocus(Qt::OtherFocusReason);
}

VideoPreviewWidget::VideoPreviewWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->setMinimumSize(400, 300);
    this->resize(width, height);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    this->widget_titleBar = new TitleBarWidget(this->width() - 2, 40, 8, this);
    this->widget_titleBar->move(1,1);

    this->btn_save = new SaveButton(this->widget_titleBar);
    this->btn_save->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_save->move(this->widget_titleBar->leftLimit(), 0);

    this->player = new QMediaPlayer(this);
    this->output = new QAudioOutput(this);

    this->player->setAudioOutput(this->output);

    this->view_video = new VideoContentView(player, output, this);
    this->view_video->resize(this->width() - 2 * 3, this->height() - this->widget_titleBar->height() - 2 * 3);
    this->view_video->move(3, this->widget_titleBar->height() + 1);

    //分辨率
    connect(this->player, &QMediaPlayer::metaDataChanged, this, [this](){
        trySetVideoSize();
    });

    //播放状态
    connect(this->player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state){
        switch(state)
        {
        case QMediaPlayer::PlayingState:
            this->view_video->setState(VideoContentView::State::Playing);
            break;
        case QMediaPlayer::PausedState:
            this->view_video->setState(VideoContentView::State::Paused);
            break;
        case QMediaPlayer::StoppedState:
            if(this->view_video->getState() != VideoContentView::State::Error)
                this->view_video->setState(VideoContentView::State::Paused);
            break;
        }
    });

    //媒体状态
    connect(this->player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status){
        switch(status)
        {
        case QMediaPlayer::LoadingMedia:
        case QMediaPlayer::StalledMedia:
            this->view_video->setState(VideoContentView::State::Loading);
            break;
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferedMedia:
            trySetVideoSize();
            break;
        case QMediaPlayer::InvalidMedia:
            this->view_video->setState(VideoContentView::State::Error);
            this->view_video->setErrorText("无法识别视频格式");
            break;
        case QMediaPlayer::EndOfMedia:
        {
            qint64 dur = this->player->duration();
            if(dur > 0)
            {
                this->player->pause();
                this->player->setPosition(0);
            }
            break;
        }
        default:
            break;
        }
    });

    //播放器错误
    connect(this->player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString& msg){
        this->view_video->setState(VideoContentView::State::Error);
        this->view_video->setErrorText(msg.isEmpty() ? "视频加载失败" : msg);
    });

    connect(this->view_video, &VideoContentView::clicked, this, [this](){
        if(this->player->playbackState() == QMediaPlayer::PlayingState)
            this->player->pause();
        else
            this->player->play();
    });

    connect(this->widget_titleBar, &TitleBarWidget::closeApp, this, [this](){
        if(this->player)
            this->player->stop();
        this->hide();
    });

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this](){
        if(this->player)
            this->player->stop();
    });

    connect(this->widget_titleBar, &TitleBarWidget::minimizeApp, this, &VideoPreviewWidget::showMinimized);

    connect(this->btn_save, &QToolButton::clicked, this, [this](){
        if(this->currentVideoUrl.isEmpty())
        {
            ToastManager::getToastManager(this).error("下载失败，请稍后重试");
            return;
        }

        auto Status = HttpShortConnection::getHttpClient().getDownloadStatus(this->currentVideoUrl);
        if(Status.isDownloading)
        {
            HttpShortConnection::getHttpClient().cancelDownloadFile(this->currentVideoUrl);
            return;
        }

        QString savePath = getSavePath();
        if(!savePath.isEmpty() && QFileInfo::exists(savePath))
            return;

        QString targetPath = HttpShortConnection::getHttpClient().resolveTargetPath(savePath);
        if(targetPath.isEmpty())
        {
            ToastManager::getToastManager(this).error("下载失败，请稍后重试");
            return;
        }

        HttpShortConnection::getHttpClient().downloadFile(this->currentVideoUrl, targetPath);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadStarted, this, [this](const QString& url){
        if(url != this->currentVideoUrl)
            return;

        this->btn_save->setState(SaveButton::State::Downloading);
        this->btn_save->setProgress(0);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadProgressChanged, this, [this](const QString& url, qint64 received, qint64 total){
        if(url != this->currentVideoUrl)
            return;

        int percent = total > 0 ? static_cast<int>(received * 100 / total) : 0;
        this->btn_save->setState(SaveButton::State::Downloading);
        this->btn_save->setProgress(percent);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadFinished, this, [this](const QString& url, bool isSuccess, const QString& info){
        if(url != this->currentVideoUrl)
            return;

        this->updateSaveButton();

        if(isSuccess)
            ToastManager::getToastManager(this).success(info);
        else
            ToastManager::getToastManager(this).error(info);
    });

    this->installEventFilter(this);
    this->view_video->installEventFilter(this);
    this->view_video->viewport()->installEventFilter(this);

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, [this](){
        if(this->player)
            this->player->stop();
        setVideoUrl({});
        this->hide();
    });
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, [this](){
        if(this->player)
            this->player->stop();
        setVideoUrl({});
        this->hide();
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, [this](){
        if(this->player)
            this->player->stop();
        setVideoUrl({});
        this->hide();
    });
}

VideoPreviewWidget::Edge VideoPreviewWidget::edgeAt(const QPoint &pos)
{
    int limitWidth = 8;
    int x = pos.x();
    int y = pos.y();
    int width = this->width();
    int height = this->height();

    bool left = x < limitWidth;
    bool right = x > width - limitWidth;
    bool top = y < limitWidth;
    bool bottom = y > height - limitWidth;

    if(top && left)
        return Edge::TopLeft;
    if(top && right)
        return Edge::TopRight;
    if(bottom && left)
        return Edge::BottomLeft;
    if(bottom && right)
        return Edge::BottomRight;
    if(left)
        return Edge::Left;
    if(right)
        return Edge::Right;
    if(top)
        return Edge::Top;
    if(bottom)
        return Edge::Bottom;
    return Edge::None;
}

void VideoPreviewWidget::updateCursor(Edge edge)
{
    QCursor cursor;

    switch(edge)
    {
    case Edge::Left:
    case Edge::Right:
        cursor = Qt::SizeHorCursor;
        break;
    case Edge::Top:
    case Edge::Bottom:
        cursor = Qt::SizeVerCursor;
        break;
    case Edge::TopLeft:
    case Edge::BottomRight:
        cursor = Qt::SizeFDiagCursor;
        break;
    case Edge::TopRight:
    case Edge::BottomLeft:
        cursor = Qt::SizeBDiagCursor;
        break;
    default:
        cursor = QCursor(Qt::ArrowCursor);
        break;
    }

    this->setCursor(cursor);
    if(this->view_video)
    {
        this->view_video->setCursor(cursor);
        this->view_video->viewport()->setCursor(cursor);
    }
}

void VideoPreviewWidget::updateSaveButton()
{
    if(!this->btn_save)
        return;

    if(this->currentVideoUrl.isEmpty())
    {
        this->btn_save->setState(SaveButton::State::None);
        return;
    }

    QString savePath = getSavePath();
    if(!savePath.isEmpty() && QFileInfo::exists(savePath))
    {
        this->btn_save->setState(SaveButton::State::Finished);
        return;
    }

    auto status = HttpShortConnection::getHttpClient().getDownloadStatus(this->currentVideoUrl);

    if(status.isDownloading)
    {
        this->btn_save->setState(SaveButton::State::Downloading);
        this->btn_save->setProgress(status.getDownloadPercent());
        return;
    }

    if(status.isFinished)
    {
        this->btn_save->setState(SaveButton::State::Finished);
        return;
    }

    if(status.receivedSize > 0 && status.totalSize > 0)
    {
        this->btn_save->setState(SaveButton::State::Paused);
        this->btn_save->setProgress(status.getDownloadPercent());
        return;
    }

    if(!savePath.isEmpty() && QFileInfo::exists(savePath + ".part"))
    {
        this->btn_save->setState(SaveButton::State::Paused);
        qint64 partSize = QFileInfo(savePath + ".part").size();
        int percent = this->currentVideoSize > 0 ? static_cast<int>(partSize * 100 / this->currentVideoSize) : 0;
        this->btn_save->setProgress(percent);
        return;
    }

    this->btn_save->setState(SaveButton::State::None);
}

void VideoPreviewWidget::trySetVideoSize()
{
    if(!this->player || !this->view_video)
        return;

    QVariant var = this->player->metaData().value(QMediaMetaData::Resolution);
    if(!var.isValid())
        return;

    QSize size = var.toSize();
    if(!size.isValid() || size.isEmpty())
        return;

    this->view_video->setVideoNativeSize(size);
}

QString VideoPreviewWidget::getSavePath()
{
    if(this->currentVideoUrl.isEmpty())
        return {};

    return VideoUtils::getLocalUrlPath(this->currentVideoUrl);
}