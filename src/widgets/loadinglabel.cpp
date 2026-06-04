#include "loadinglabel.h"

LoadingLabel::LoadingLabel(QWidget *parent)
    : QWidget{parent}
{}

qreal LoadingLabel::angle() const
{
    return this->m_angle;
}

void LoadingLabel::setAngle(qreal angle)
{
    this->m_angle = angle;
    update();
}

void LoadingLabel::setPixmap(const QPixmap &pixmap)
{
    this->m_pix = pixmap;
    update();
}

void LoadingLabel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    //开启平滑图像变换
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    //中心点调整到图片中心
    p.translate(width() / 2.0,height() / 2.0);

    p.rotate(m_angle);
    //恢复中心点
    p.translate(-width() / 2.0, -height() / 2.0);
    p.drawPixmap(this->rect(), this->m_pix);
}
