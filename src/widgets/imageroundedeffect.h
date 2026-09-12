#ifndef IMAGEROUNDEDEFFECT_H
#define IMAGEROUNDEDEFFECT_H

#include <QObject>
#include <QGraphicsEffect>
#include <QPainter>
#include <QPainterPath>

class ImageRoundedEffect : public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit ImageRoundedEffect(int radius, QObject *parent = nullptr);

signals:

protected:
    void draw(QPainter* painter) override;

private:
    int radius;
};

#endif // IMAGEROUNDEDEFFECT_H
