#include "imagepreviewwidget.h"

ImagePreviewWidget &ImagePreviewWidget::getPreviewWidget()
{
    static ImagePreviewWidget widget;
    return widget;
}

void ImagePreviewWidget::setPixmap(const QPixmap &pix)
{
    this->view->setPixmap(pix);
}

void ImagePreviewWidget::setPixmapUrl(const QString &url)
{
    this->pixmapUrl = url;
    this->updateSaveButton();
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect = QRect(QRect(0, 0, this->width(), this->height()));
    rect.adjust(1, 1, -1, -1);

    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(QPen(QColor(0, 0, 0, 26), 1));
    painter.drawRoundedRect(rect, 8, 8);
}

bool ImagePreviewWidget::eventFilter(QObject *obj, QEvent *ev)
{
    //过滤窗口拖动事件
    if (obj != this && obj != this->view && obj != this->view->viewport())
        return QObject::eventFilter(obj, ev);

    if(ev->type() != QEvent::MouseButtonPress && ev->type() != QEvent::MouseButtonRelease && ev->type() != QEvent::MouseMove)
        return QObject::eventFilter(obj, ev);

    QMouseEvent* qme = static_cast<QMouseEvent*>(ev);
    if(!qme)
        return QObject::eventFilter(obj, ev);

    QPoint posThis;
    if(obj == this)
        posThis = qme->pos();
    else if(obj == this->view)
        posThis = this->view->mapTo(this, qme->pos());
    else if(obj == this->view->viewport())
        posThis = this->view->viewport()->mapTo(this, qme->pos());

    switch(ev->type())
    {
    case QEvent::MouseButtonPress:
    {
        if(qme->button() != Qt::LeftButton || this->view->isDragging())
            break;

        this->edge = edgeAt(posThis);
        if(this->edge != Edge::None)
        {
            this->pos_startScaleGlobal = qme->globalPosition().toPoint();
            this->rect_startGeometry = this->geometry();
            return true;
        }

        if(posThis.y() <= this->widget_titleBar->height())
        {
            this->pos_widget = posThis;
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        this->pos_widget = QPoint();
        this->updateCursor(this->edgeAt(posThis));
        if(obj != this)
        {
            this->view->setCursor(this->cursor());
            this->view->viewport()->setCursor(this->cursor());
        }

        break;
    }
    case QEvent::MouseMove:
    {
        if(this->view->isDragging())
            break;

        this->updateCursor(this->edgeAt(posThis));
        if(obj != this)
        {
            this->view->setCursor(this->cursor());
            this->view->viewport()->setCursor(this->cursor());
        }

        if(this->edge != Edge::None && (qme->buttons() & Qt::LeftButton))
        {
            QPoint offset = qme->globalPosition().toPoint() - this->pos_startScaleGlobal;
            QRect geometry = this->rect_startGeometry;

            switch(this->edge)
            {
            case Edge::Left:
                geometry.setLeft(this->rect_startGeometry.left() + offset.x());
                break;
            case Edge::Right:
                geometry.setRight(this->rect_startGeometry.right() + offset.x());
                break;
            case Edge::Top:
                geometry.setTop(this->rect_startGeometry.top() + offset.y());
                break;
            case Edge::Bottom:
                geometry.setBottom(this->rect_startGeometry.bottom() + offset.y());
                break;
            case Edge::TopLeft:
                geometry.setTopLeft(this->rect_startGeometry.topLeft() + offset);
                break;
            case Edge::TopRight:
                geometry.setTopRight(this->rect_startGeometry.topRight() + offset);
                break;
            case Edge::BottomLeft:
                geometry.setBottomLeft(this->rect_startGeometry.bottomLeft() + offset);
                break;
            case Edge::BottomRight:
                geometry.setBottomRight(this->rect_startGeometry.bottomRight() + offset);
                break;
            default:
                break;
            }
            if(geometry.width() < this->minimumWidth())
            {
                if(this->edge == Edge::Left || this->edge == Edge::TopLeft || this->edge == Edge::BottomLeft)
                    geometry.setLeft(geometry.right() - this->minimumWidth());
                else
                    geometry.setRight(geometry.left() + this->minimumWidth());
            }
            if(geometry.height() < this->minimumHeight())
            {
                if(this->edge == Edge::Top || this->edge == Edge::TopLeft || this->edge == Edge::TopRight)
                    geometry.setTop(geometry.bottom() - this->minimumHeight());
                else
                    geometry.setBottom(geometry.top() + this->minimumHeight());
            }

            this->setGeometry(geometry);
            return true;
        }

        //移动
        if(this->pos_widget.isNull() == false && (qme->buttons() & Qt::LeftButton))
        {
            this->move(qme->globalPosition().toPoint() - this->pos_widget);
            return true;
        }
        else
            this->pos_widget = QPoint();
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(obj, ev);
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(this->widget_titleBar)
    {
        this->widget_titleBar->resize(this->width() - 2, 40);
        this->widget_titleBar->move(1, 1);
        this->view->resize(this->width() - 4, this->height() - this->widget_titleBar->height() - 4);
        this->view->move(2, this->widget_titleBar->pos().y() + this->widget_titleBar->height() + 1);
    }
    update();
}

ImagePreviewWidget::ImagePreviewWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->setMinimumSize(450, 500);
    this->resize(width, height);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    this->widget_titleBar = new TitleBarWidget(this->width() - 2, 40, 8, this);
    this->widget_titleBar->move(1,1);

    this->btn_rotateLeft = new QPushButton(this->widget_titleBar);
    this->btn_rotateLeft->setObjectName("btn_rotateLeft");
    this->btn_rotateLeft->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_rotateLeft->setIcon(QIcon(":/default/images/left90.png"));
    this->btn_rotateLeft->setIconSize(QSize(16, 16));
    this->btn_rotateLeft->move(this->widget_titleBar->leftLimit(), 0);

    connect(this->btn_rotateLeft, &QPushButton::clicked, this, [this](){
        this->view->rotateLeft(90);
    });

    this->btn_rotateRight = new QPushButton(this->widget_titleBar);
    this->btn_rotateRight->setObjectName("btn_rotateRight");
    this->btn_rotateRight->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_rotateRight->setIcon(QIcon(":/default/images/right90.png"));
    this->btn_rotateRight->setIconSize(QSize(16, 16));
    this->btn_rotateRight->move(this->btn_rotateLeft->pos().x() + this->btn_rotateLeft->width(), 0);

    connect(this->btn_rotateRight, &QPushButton::clicked, this, [this](){
        this->view->rotateRight(90);
    });

    this->btn_zoomIn = new QPushButton(this->widget_titleBar);
    this->btn_zoomIn->setObjectName("btn_zoomIn");
    this->btn_zoomIn->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_zoomIn->setIcon(QIcon(":/default/images/zoomIn.png"));
    this->btn_zoomIn->setIconSize(QSize(20, 20));
    this->btn_zoomIn->move(this->btn_rotateRight->pos().x() + this->btn_rotateRight->width(), 0);

    connect(this->btn_zoomIn, &QPushButton::clicked, this, [this](){
        this->view->zoomIn();
    });

    this->btn_zoomOut = new QPushButton(this->widget_titleBar);
    this->btn_zoomOut->setObjectName("btn_zoomOut");
    this->btn_zoomOut->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_zoomOut->setIcon(QIcon(":/default/images/zoomOut.png"));
    this->btn_zoomOut->setIconSize(QSize(20, 20));
    this->btn_zoomOut->move(this->btn_zoomIn->pos().x() + this->btn_zoomIn->width(), 0);

    connect(this->btn_zoomOut, &QPushButton::clicked, this, [this](){
        this->view->zoomOut();
    });

    this->btn_save = new QPushButton(this->widget_titleBar);
    this->btn_save->setObjectName("btn_save");
    this->btn_save->resize(this->widget_titleBar->height(), this->widget_titleBar->height());
    this->btn_save->setIcon(QIcon(":/default/images/download.png"));
    this->btn_save->setIconSize(QSize(20, 20));
    this->btn_save->move(this->btn_zoomOut->pos().x() + this->btn_zoomOut->width(), 0);

    connect(this->btn_save, &QPushButton::clicked, this, [this](){
        if(this->isSaved)
            return;

        if(this->pixmapUrl.isEmpty())
        {
            ToastManager::getToastManager(this).success("保存失败，请稍后重试");
            return;
        }

        if(!ImageCacheManager::getManager().saveTo(this->pixmapUrl, GlobalVariable::getPosOfDownloadFile()))
            return;

        this->updateSaveButton();
        ToastManager::getToastManager(this).success("保存成功");
    });

    this->view = new ImageView(this);
    this->view->setObjectName("view");
    this->view->resize(this->width() - 4, this->height() - this->widget_titleBar->height() - 4);
    this->view->move(2, this->widget_titleBar->pos().y() + this->widget_titleBar->height() + 1);
    this->view->setMouseTracking(true);

    this->installEventFilter(this);
    this->view->installEventFilter(this);
    this->view->viewport()->setMouseTracking(true);
    this->view->viewport()->installEventFilter(this);

    connect(this->widget_titleBar, &TitleBarWidget::closeApp, this, &ImagePreviewWidget::hide);
    connect(this->widget_titleBar, &TitleBarWidget::minimizeApp, this, &ImagePreviewWidget::showMinimized);

    initStyle();

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, [this](){
        setPixmap(QPixmap());
        setPixmapUrl({});
        this->hide();
    });
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, [this](){
        setPixmap(QPixmap());
        setPixmapUrl({});
        this->hide();
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, [this](){
        setPixmap(QPixmap());
        setPixmapUrl({});
        this->hide();
    });
}

ImagePreviewWidget::Edge ImagePreviewWidget::edgeAt(const QPoint &pos)
{
    int limitWidth = 8;
    int x = pos.x();
    int y = pos.y();
    int width = this->width();
    int heigth = this->height();

    bool left = x < limitWidth;
    bool right = x > width - limitWidth;
    bool top = y < limitWidth;
    bool bottom = y > heigth - limitWidth;

    if(top && left)
        return Edge::TopLeft;
    if(top && right)
        return Edge::TopRight;
    if(bottom && left)
        return Edge::BottomLeft;
    if(bottom && right)
        return Edge::BottomRight;
    if(left)
        return Edge::Left;
    if(right)
        return Edge::Right;
    if(top)
        return Edge::Top;
    if(bottom)
        return Edge::Bottom;
    return Edge::None;
}

void ImagePreviewWidget::updateCursor(Edge edge)
{
    switch(edge)
    {
    case Edge::Left:
    case Edge::Right:
        this->setCursor(Qt::SizeHorCursor);
        break;
    case Edge::Top:
    case Edge::Bottom:
        this->setCursor(Qt::SizeVerCursor);
        break;
    case Edge::TopLeft:
    case Edge::BottomRight:
        this->setCursor(Qt::SizeFDiagCursor);
        break;
    case Edge::TopRight:
    case Edge::BottomLeft:
        this->setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        this->unsetCursor();
        break;
    }
}

void ImagePreviewWidget::updateSaveButton()
{
    QString filename = ImageCacheManager::getManager().getFilenameFromUrl(this->pixmapUrl, 1.0);
    if(filename.isEmpty())
    {
        this->btn_save->setIcon(QIcon(":/default/images/download.png"));
        this->isSaved = false;
        return;
    }

    bool saved = GlobalVariable::isDownloadFileExists(filename);
    this->btn_save->setIcon(saved ? QIcon(":/default/images/downloadSuccess.png") : QIcon(":/default/images/download.png"));
    this->isSaved = saved;
}

void ImagePreviewWidget::initStyle()
{
    this->setStyleSheet(R"(
                            #view
                            {
                                background: transparent;
                            }
                            #btn_rotateLeft,#btn_rotateRight,#btn_zoomIn,#btn_zoomOut,#btn_save
                            {
                                background: transparent;
                                border: none;
                            }
                            #btn_rotateLeft:hover,#btn_rotateRight:hover,#btn_zoomIn:hover,#btn_zoomOut:hover,#btn_save:hover
                            {
                                background: rgba(0, 0, 0, 26);
                            }
                            #btn_rotateLeft:hover,#btn_rotateRight:pressed,#btn_zoomIn:pressed,#btn_zoomOut:pressed,#btn_save:pressed
                            {
                                background: rgba(0, 0, 0, 51);
                            }
)");
}
