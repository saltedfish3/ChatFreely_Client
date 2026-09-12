#include "imageview.h"

ImageView::ImageView(QWidget *parent)
    : QGraphicsView{parent}
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

    setGraphicsEffect(new ImageRoundedEffect(8, this));

    this->viewport()->installEventFilter(this);
}

void ImageView::setPixmap(const QPixmap &pix)
{
    this->scale = 1.0;
    this->userScale = false;
    this->item->setScale(1.0);
    this->item->setPos(0,0);
    this->item->setRotation(0);
    this->item->setTransformOriginPoint(0, 0);
    this->item->setPixmap(pix);

    if(pix.isNull())
    {
        this->setSceneRect(QRectF());
        return;
    }

    this->item->setTransformOriginPoint(this->item->boundingRect().center());

    if(this->isVisible())
    {
        applyFitScale();
        centerItem();
    }

    this->viewport()->update();
}

bool ImageView::isDragging() const
{
    return !qIsNaN(this->pos_item.x());
}

void ImageView::rotateLeft(int angle)
{
    QPixmap pix = this->item->pixmap();
    if(pix.isNull())
        return;

    this->item->setTransformOriginPoint(this->item->boundingRect().center());

    this->item->setRotation(this->item->rotation() - angle);
    applyFitScale();
    centerItem();
    this->viewport()->update();
}

void ImageView::rotateRight(int angle)
{
    QPixmap pix = this->item->pixmap();
    if(pix.isNull())
        return;

    this->item->setTransformOriginPoint(this->item->boundingRect().center());

    this->item->setRotation(this->item->rotation() + angle);
    applyFitScale();
    centerItem();
    this->viewport()->update();
}

void ImageView::zoomIn()
{
    QPixmap pix = this->item->pixmap();
    if(pix.isNull())
        return;

    const qreal step = 1.15;
    qreal newScale = qBound(0.1, this->scale * step, 10.0);
    this->scale = newScale;
    this->item->setScale(this->scale);
    this->item->update();
}

void ImageView::zoomOut()
{
    QPixmap pix = this->item->pixmap();
    if(pix.isNull())
        return;

    const qreal step = 1.15;
    qreal newScale = qBound(0.1, this->scale * (1.0 / step), 10.0);
    this->scale = newScale;
    this->item->setScale(this->scale);
    this->item->update();
}

void ImageView::showEvent(QShowEvent *event)
{
    if(this->item->pixmap().isNull() || this->userScale)
        return;

    applyFitScale();
    centerItem();
    this->viewport()->update();
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    if(this->item->pixmap().isNull() || this->userScale)
        return;

    applyFitScale();
    centerItem();
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    if(this->item->pixmap().isNull())
    {
        QGraphicsView::wheelEvent(event);
        return;
    }

    const qreal step = 1.15;
    qreal factor = event->angleDelta().y() > 0 ? step : 1.0 / step;

    qreal newScale = qBound(0.1, this->scale*factor, 10.0);

    if(qFuzzyCompare(newScale, this->scale))
    {
        event->accept();
        return;
    }

    this->userScale = true;
    QPointF mouseScene = mapToScene(event->position().toPoint());
    QPointF mouseItem = this->item->mapFromScene(mouseScene);

    this->scale = newScale;
    this->item->setScale(this->scale);

    QPointF newMouseScene = this->item->mapToScene(mouseItem);
    QPointF mouseOffset = newMouseScene - mouseScene;
    QPointF hodePos = this->item->pos() - mouseOffset;

    this->item->setPos(getItemPos(hodePos));
    event->accept();
}

void ImageView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !this->item->pixmap().isNull())
    {
        this->userScale = false;
        applyFitScale();
        centerItem();
        this->viewport()->update();
        event->accept();
        return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

bool ImageView::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == this->viewport())
    {
        QMouseEvent* qme = static_cast<QMouseEvent*>(ev);
        if(!qme)
            return QObject::eventFilter(obj, ev);

        if(ev->type() == QEvent::MouseButtonPress)
        {
            if(qme->button() != Qt::LeftButton || this->item->pixmap().isNull())
                return QObject::eventFilter(obj, ev);

            QPointF scenePos = this->mapToScene(qme->pos());
            QPointF itemPos = this->item->mapFromScene(scenePos);
            if(!this->item->contains(itemPos))
                return QObject::eventFilter(obj, ev);

            this->pos_item = scenePos - this->item->pos();
            return true;
        }
        else if(ev->type() == QEvent::MouseButtonRelease)
        {
            this->pos_item = QPointF(qQNaN(), qQNaN());
        }
        else if(ev->type() == QEvent::MouseMove && !qIsNaN(this->pos_item.x()))
        {
            if(!this->viewport()->rect().contains(qme->pos()))
            {
                this->pos_item = QPointF(qQNaN(), qQNaN());
                return true;
            }

            QPointF hopePos = this->mapToScene(qme->pos()) - this->pos_item;
            this->item->setPos(getItemPos(hopePos));
            return true;
        }
    }
    return QObject::eventFilter(obj, ev);
}

void ImageView::applyFitScale()
{
    QPixmap pix = this->item->pixmap();
    if(pix.isNull())
        return;

    QSizeF logicalSize = pix.deviceIndependentSize();

    QSize viewSize = this->viewport()->size();
    if(viewSize.isEmpty() || logicalSize.isEmpty())
        return;

    int angle = (static_cast<int>(this->item->rotation()) % 360 + 360) % 360;

    qreal imgHeight = angle == 90 || angle == 270 ? logicalSize.width() : logicalSize.height();
    qreal imgWidth = angle == 90 || angle == 270 ? logicalSize.height() : logicalSize.width();

    qreal scaleWidth = viewSize.width() / imgWidth;
    qreal scaleHeight = viewSize.height() / imgHeight;

    this->scale = qMin(1.0, qMin(scaleWidth, scaleHeight));

    this->item->setScale(this->scale);
}

void ImageView::centerItem()
{
    QPointF boundCenter = this->item->sceneBoundingRect().center();
    this->item->setPos(this->item->pos() - boundCenter);

    QRectF itemRect = this->item->sceneBoundingRect();

    QSize vs = this->viewport()->size();
    qreal halfWidth = qMax(itemRect.width() / 2, vs.width() / 2.0);
    qreal halfHeight = qMax(itemRect.height() / 2, vs.height() / 2.0);

    this->setSceneRect(-halfWidth, -halfHeight, halfWidth*2, halfHeight*2);

    this->centerOn(0, 0);
}

QPointF ImageView::getItemPos(const QPointF &hopePos) const
{
    QPointF imgPos = this->item->pos();
    QPointF offset = hopePos - imgPos;

    QRectF imgRect = this->item->sceneBoundingRect();
    QRectF viewRect = this->mapToScene(this->viewport()->rect()).boundingRect();

    if(imgRect.width() <= viewRect.width())
        offset.setX(viewRect.center().x() - imgRect.center().x());
    else
    {
        qreal newLeft = imgRect.left() + offset.x();
        qreal newRight = imgRect.right() + offset.x();
        if(newLeft > viewRect.left())
            offset.setX(viewRect.left() - imgRect.left());
        else if(newRight < viewRect.right())
            offset.setX(viewRect.right() - imgRect.right());
    }

    if(imgRect.height() <= viewRect.height())
        offset.setY(viewRect.center().y() - imgRect.center().y());
    else
    {
        qreal newTop = imgRect.top() + offset.y();
        qreal newBottom = imgRect.bottom() + offset.y();
        if(newTop > viewRect.top())
            offset.setY(viewRect.top() - imgRect.top());
        else if(newBottom < viewRect.bottom())
            offset.setY(viewRect.bottom() - imgRect.bottom());
    }

    return imgPos + offset;
}
