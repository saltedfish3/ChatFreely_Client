#include "avatarview.h"

AvatarView::AvatarView(qreal scaleFactor, QWidget *parent)
    : QGraphicsView{parent}, scaleFactor(scaleFactor)
{
    this->scene = new QGraphicsScene(this);
    this->item = new QGraphicsPixmapItem();
    this->scene->addItem(this->item);
    this->setScene(this->scene);

    this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    this->setFrameShape(QFrame::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->item->setTransformationMode(Qt::SmoothTransformation);
    this->item->setTransformOriginPoint(this->item->boundingRect().center());

    this->animation = new QVariantAnimation(this);
    this->animation->setDuration(100);
    this->animation->setEasingCurve(QEasingCurve::OutCurve);

    connect(this->animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value){
        this->item->setScale(value.toReal());
        QRectF sceneRect = this->item->sceneBoundingRect();
        QPoint topLeft = this->mapFromScene(sceneRect.topLeft());
        QPoint bottomRight = this->mapFromScene(sceneRect.bottomRight());
        emit valueChanged(QRect(topLeft, bottomRight));
    });

    this->original = 1.00;
    this->hoverScale = 1.00;
}

void AvatarView::setAvatar(const QPixmap &pixmap)
{
    if(pixmap.isNull() || this->size().isEmpty())
        return;

    this->item->setPixmap(pixmap);
    this->item->setTransformOriginPoint(this->item->boundingRect().center());

    QSizeF viewSize = this->size();
    this->scene->setSceneRect(0, 0, viewSize.width(), viewSize.height());

    QSizeF pixSize = pixmap.deviceIndependentSize();
    qreal fitScale = qMin(viewSize.width() / pixSize.width(), viewSize.height() / pixSize.height());

    this->original = fitScale*this->scaleFactor;
    hoverScale = fitScale;

    this->item->setScale(this->original);

    QPointF origin = this->item->transformOriginPoint();
    this->item->setPos(viewSize.width() / 2.0 - origin.x(),
                       viewSize.height() / 2.0 - origin.y());

    this->animation->setStartValue(this->original);

    QRectF sceneRect = this->item->sceneBoundingRect();
    QPoint topLeft = this->mapFromScene(sceneRect.topLeft());
    QPoint bottomRight = this->mapFromScene(sceneRect.bottomRight());
    emit valueChanged(QRect(topLeft, bottomRight));
}

void AvatarView::enterEvent(QEnterEvent *event)
{
    this->animation->stop();
    this->animation->setStartValue(this->item->scale());
    this->animation->setEndValue(this->hoverScale);
    this->animation->start();
    QGraphicsView::enterEvent(event);
}

void AvatarView::leaveEvent(QEvent *event)
{
    this->animation->stop();
    this->animation->setStartValue(this->item->scale());
    this->animation->setEndValue(this->original);
    this->animation->start();
    QGraphicsView::leaveEvent(event);
}

void AvatarView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        event->accept();
}

void AvatarView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit itemClicked();
        event->accept();
    }
}
