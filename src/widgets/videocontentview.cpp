#include "videocontentview.h"

VideoContentView::VideoContentView(QMediaPlayer* player, QAudioOutput* output, QWidget *parent)
    : QOpenGLWidget{parent}
{
    this->setMouseTracking(true);
    rebuildPath();

    this->bar = new VideoControlBar(player, output, this);
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

void VideoContentView::setFrame(const QImage &frame)
{
    this->frame = frame;
    update();
}

void VideoContentView::setErrorText(const QString &text)
{
    this->errorText = text;
    update();
}

void VideoContentView::clearFrame()
{
    this->frame = QImage();
    this->currentState = State::None;
    this->errorText.clear();
    update();
}

void VideoContentView::paintGL()
{
    if(this->width() <= 0 || this->height() <= 0 || this->path.isEmpty())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillPath(this->path, QColor(240, 240, 240));

    if(this->currentState == State::Error)
    {
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(this->rect(), Qt::AlignCenter, this->errorText.isEmpty() ? QStringLiteral("播放失败") : this->errorText);
        return;
    }

    if(this->frame.isNull())
    {
        QString info = (this->currentState == State::Loading) ? QStringLiteral("加载中...") : QStringLiteral("暂无视频");
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(this->rect(), Qt::AlignCenter, info);
        return;
    }

    QSize videoSize = this->frame.size();
    if(videoSize.width() <= 0 || videoSize.height() <= 0)
        return;

    int x = (this->width() - videoSize.width()) / 2;
    int y = (this->height() - videoSize.height()) / 2;
    QRect videoRect(x, y, videoSize.width(), videoSize.height());

    painter.setClipPath(this->path);
    painter.drawImage(videoRect, this->frame);
}

void VideoContentView::resizeGL(int w, int h)
{
    rebuildPath();
    if(!this->bar)
        return;

    this->bar->adjustSize();
    int barWidth = this->bar->width();
    int barHeight = this->bar->height();

    if(barWidth <= 0 || barHeight <= 0 || this->height() <= 0)
        return;

    int margin = 12;
    this->bar->move((this->width() - barWidth) / 2, this->height() - barHeight - margin);
    this->bar->raise();
}

void VideoContentView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //若修改记得修改preview窗口
        const int limit = 8;
        const QPoint pos = event->pos();
        if(pos.x() < limit || pos.x() > width() - limit || pos.y() < limit || pos.y() > height() - limit)
        {
            event->ignore();
            return;
        }
        this->pressPos = pos;
    }
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

void VideoContentView::rebuildPath()
{
    this->path = QPainterPath();

    if(this->width() <= 0 || this->height() <= 0)
        return;

    QRectF rect = this->rect();
    qreal radius = 6;

    this->path = QPainterPath();

    path.moveTo(rect.topLeft());
    path.lineTo(rect.topRight());
    path.lineTo(rect.right(), rect.bottom() - radius);
    path.arcTo(rect.right() - 2 * radius, rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 0, -90);

    path.lineTo(rect.left() + radius, rect.bottom());
    path.arcTo(rect.left(), rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 270, -90);
    path.closeSubpath();
}