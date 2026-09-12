#include "imageroundedeffect.h"

ImageRoundedEffect::ImageRoundedEffect(int radius, QObject *parent)
    : QGraphicsEffect{parent}, radius(radius)
{}

void ImageRoundedEffect::draw(QPainter *painter)
{
    QPoint offset;
    QPixmap src = sourcePixmap(Qt::LogicalCoordinates, &offset);
    if(src.isNull())
        return;

    QSizeF logicalSize = src.deviceIndependentSize();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF rect(offset, logicalSize);

    QPainterPath path;
    path.addRoundedRect(rect, this->radius, this->radius);

    QPainterPath path_top;
    path_top.addRect(QRectF(rect.left(), rect.top(), rect.width(), this->radius));
    path = path.united(path_top);

    painter->setClipPath(path, Qt::IntersectClip);
    painter->drawPixmap(offset, src);
}
