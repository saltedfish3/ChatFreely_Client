#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QWheelEvent>
#include "imageroundedeffect.h"

class ImageView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);
    void setPixmap(const QPixmap& pix);
    bool isDragging() const;
    void rotateLeft(int angle);
    void rotateRight(int angle);
    void zoomIn();
    void zoomOut();

signals:

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    void applyFitScale();
    void centerItem();
    QPointF getItemPos(const QPointF& hopePos) const;

    QGraphicsScene* scene;
    QGraphicsPixmapItem* item;

    qreal scale = 1.0;
    bool userScale = false;

    QPointF pos_item = QPointF(qQNaN(), qQNaN());
};

#endif // IMAGEVIEW_H
