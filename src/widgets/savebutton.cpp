#include "savebutton.h"

SaveButton::SaveButton(QWidget *parent)
    : QToolButton{parent}
{
    this->icon_normal = QIcon(":/default/images/download.png");
    this->icon_finished = QIcon(":/default/images/downloadSuccess.png");

    this->setIcon(this->icon_normal);
    this->setIconSize(QSize(20, 20));
    this->setMouseTracking(true);
}

void SaveButton::setState(State state)
{
    if(this->currentState == state)
        return;

    this->currentState = state;
    update();
}

void SaveButton::setProgress(int percent)
{
    percent = qBound(0, percent, 100);
    if(this->currentProgress == percent)
        return;
    this->currentProgress = percent;

    if(this->currentState == State::Downloading || this->currentState == State::Paused)
        update();
}

int SaveButton::getProgress() const
{
    return this->currentProgress;
}

void SaveButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(this->isHovered || this->isDown())
    {
        QColor background = this->isDown() ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 13);
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawRect(this->rect());
    }

    bool isPaused = this->currentState == State::Paused;

    if(this->currentState == State::Downloading || this->currentState == State::Paused)
    {
        qreal minSide = qMin(this->width(), this->height());
        qreal diameter = minSide - (minSide / 8.0) * 2;
        QRectF circleRect((this->width() - diameter) / 2.0, (this->height() - diameter) / 2.0, diameter, diameter);

        painter.setPen(QPen(QColor(0, 0, 0, 40), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(circleRect, 0, 360 * 16);

        if(this->currentProgress > 0)
        {
            painter.setPen(QPen(QColor(91, 155, 213), 2, Qt::SolidLine, Qt::RoundCap));
            painter.drawArc(circleRect, 90 * 16, -this->currentProgress * 360 * 16 / 100);
        }

        qreal radius = diameter * 0.15;
        QPointF center = circleRect.center();

        if(!isPaused)
        {
            //叉号
            painter.setPen(QPen(QColor(68, 68, 68), 1.0, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(center.x() - radius, center.y() - radius), QPointF(center.x() + radius, center.y() + radius));
            painter.drawLine(QPointF(center.x() - radius, center.y() + radius), QPointF(center.x() + radius, center.y() - radius));
        }
        else
        {
            if(icon_normal.isNull())
                return;

            int diameter_paused = qRound(diameter * 0.55);
            QSize logicalSize(diameter_paused, diameter_paused);

            QPixmap pix = icon_normal.pixmap(logicalSize);
            if(pix.isNull())
                return;

            int x = qRound(center.x() - logicalSize.width() / 2.0);
            int y = qRound(center.y() - logicalSize.height() / 2.0);
            painter.drawPixmap(x, y, pix);
        }
    }
    else
    {
        const QIcon& icon = this->currentState == State::None ? this->icon_normal : this->icon_finished;

        if(icon.isNull())
            return;

        QSize logicalSize = this->iconSize();

        QPixmap pix = icon.pixmap(logicalSize);
        if(pix.isNull())
            return;

        int x = (this->width() - logicalSize.width()) / 2;
        int y = (this->height() - logicalSize.height()) / 2;
        painter.drawPixmap(x, y, pix);
    }
}

void SaveButton::enterEvent(QEnterEvent *event)
{
    this->isHovered = true;
    update();
    QToolButton::enterEvent(event);
}

void SaveButton::leaveEvent(QEvent *event)
{
    this->isHovered = false;
    update();
    QToolButton::leaveEvent(event);
}

SaveButton::State SaveButton::getState() const
{
    return this->currentState;
}
