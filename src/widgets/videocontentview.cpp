#include "videocontentview.h"

VideoContentView::VideoContentView(QMediaPlayer* player, QAudioOutput* output, QWidget *parent)
    : QGraphicsView{parent}, player(player), output(output)
{
    this->setMouseTracking(true);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);

    this->setStyleSheet("QGraphicsView { border: 0px; background: transparent; }");
    this->setViewportMargins(0, 0, 0, 0);

    if(GlobalVariable::hasGPU())
    {
        auto* opengl = new QOpenGLWidget(this);
        opengl->setMouseTracking(true);
        opengl->setAttribute(Qt::WA_TranslucentBackground, false);
        this->setViewport(opengl);
        this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }

    this->viewport()->setAutoFillBackground(false);
    this->viewport()->setStyleSheet("background: transparent;");

    this->setRenderHint(QPainter::Antialiasing);
    this->setRenderHint(QPainter::SmoothPixmapTransform);

    this->setTransformationAnchor(QGraphicsView::NoAnchor);
    this->setResizeAnchor(QGraphicsView::NoAnchor);
    this->setAlignment(Qt::AlignCenter);

    this->scene = new QGraphicsScene(this);
    this->scene->setBackgroundBrush(QColor(240, 240, 240));
    this->setScene(this->scene);

    this->item_video = new QGraphicsVideoItem;
    this->item_video->setAspectRatioMode(Qt::IgnoreAspectRatio);
    this->scene->addItem(this->item_video);

    this->player->setVideoOutput(this->item_video);
}

void VideoContentView::setState(State state)
{
    if(state == this->currentState)
        return;
    this->currentState = state;
    update();
}

VideoContentView::State VideoContentView::getState() const
{
    return this->currentState;
}

void VideoContentView::setErrorText(const QString &text)
{
    this->errorText = text;
    update();
}

void VideoContentView::setVideoNativeSize(const QSize &size)
{
    if(!size.isValid() || size.isEmpty())
    {
        this->videoNativeSize = QSize();
        this->scene->setSceneRect(0, 0, 1, 1);
        this->item_video->setSize(QSizeF(1, 1));
        this->resetTransform();
        return;
    }

    this->videoNativeSize = size;

    this->scene->setSceneRect(0, 0, size.width(), size.height());

    this->item_video->setSize(QSizeF(size));
    this->item_video->setPos(0, 0);

    updateViewTransform();
}

void VideoContentView::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);

    if(this->currentState == State::Error)
    {
        painter->setPen(QColor(150, 150, 150));
        painter->drawText(this->rect(), Qt::AlignCenter, this->errorText.isEmpty() ? QStringLiteral("播放失败") : this->errorText);
    }
    else if(this->currentState == State::None || this->currentState == State::Loading)
    {
        QString info = (this->currentState == State::Loading) ? QStringLiteral("加载中...") : QStringLiteral("暂无视频");
        painter->setPen(QColor(150, 150, 150));
        painter->drawText(this->rect(), Qt::AlignCenter, info);
    }
}

void VideoContentView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateViewTransform();
    updateMask();
}

void VideoContentView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        this->pressPos = event->pos();

    QWidget::mousePressEvent(event);
}

void VideoContentView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && this->pressPos.x() >= 0)
    {
        const QPoint offset = event->pos() - this->pressPos;
        if(offset.manhattanLength() < 5)
            emit clicked();

        this->pressPos = QPoint(-1, -1);
    }
    QWidget::mouseReleaseEvent(event);
}

void VideoContentView::updateViewTransform()
{
    if(!this->videoNativeSize.isValid() || this->videoNativeSize.isEmpty())
        return;

    QSize size = this->viewport()->size();
    if(size.width() <= 0 || size.height() <= 0)
        return;

    double scaleX = static_cast<double>(size.width()) / this->videoNativeSize.width();
    double scaleY = static_cast<double>(size.height()) / this->videoNativeSize.height();
    double scale = qMin(scaleX, scaleY);

    QTransform t;
    t.scale(scale, scale);
    this->setTransform(t);
}

void VideoContentView::updateMask()
{
    if(this->width() <= 0 || this->height() <= 0)
        return;


    QRectF rect(0, 0, this->width(), this->height());
    qreal radius = 8;

    QPainterPath path;
    path.moveTo(rect.topLeft());
    path.lineTo(rect.topRight());
    path.lineTo(rect.right(), rect.bottom() - radius);
    path.arcTo(rect.right() - 2 * radius, rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 0, -90);

    path.lineTo(rect.left() + radius, rect.bottom());
    path.arcTo(rect.left(), rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 270, -90);
    path.closeSubpath();

    QRegion region(path.toFillPolygon().toPolygon());
    this->setMask(region);
    if(this->viewport())
        this->viewport()->setMask(region);
}