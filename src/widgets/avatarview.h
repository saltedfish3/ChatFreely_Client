#ifndef AVATARVIEW_H
#define AVATARVIEW_H

#include <QGraphicsView>
#include <QPixmap>
#include <QVariantAnimation>
#include <QGraphicsPixmapItem>
#include <QEvent>
#include <QMouseEvent>

class AvatarView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit AvatarView(qreal scaleFactor = 0.95, QWidget *parent = nullptr);
    void setAvatar(const QPixmap& pixmap);

signals:
    void itemClicked();
    void valueChanged(const QRect& rect);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* item;
    QVariantAnimation* animation;

    qreal original;
    qreal hoverScale;
    qreal scaleFactor;
};

#endif // AVATARVIEW_H
