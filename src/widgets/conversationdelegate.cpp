#include "conversationdelegate.h"

ConversationDelegate::ConversationDelegate(int* loadingAngle, QObject *parent)
    : QStyledItemDelegate{parent}
{
    this->loadingAngle = loadingAngle;
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->fillRect(option.rect, option.palette.base());

    QRect TimeStamp;
    QRect contain;
    QSize avatarSize(40, 40);
    QRect avatarRect;
    QRect textRegionRect;
    QSize statusSize(15, 15);
    QRect statusRect;
    int textTotalHeight = 0;

    getLayout(option, index, TimeStamp, contain, avatarRect, textRegionRect, statusRect, textTotalHeight);

    QVariant vi1 = index.data(IsNeedShowTime);
    bool isTimeStamp = vi1.isValid() ? vi1.toBool() : false;
    if(isTimeStamp)
    {
        painter->setPen(Qt::gray);
        QFont font = option.font;
        font.setPointSize(8);
        painter->setFont(font);
        QString timeStr = formatTimestamp(index.data(Role::TimeStamp).toLongLong());
        painter->drawText(TimeStamp, Qt::AlignCenter, timeStr);
    }

    QPixmap avatar = index.data(AvatarRole).value<QPixmap>();
    qreal dpr = 1.0;
    if(const QWidget* widget = option.widget)
        dpr = widget->devicePixelRatioF();
    if(avatar.isNull())
    {
        avatar = QPixmap(":/default/images/defaultAvatar.png");
        // avatar = QPixmap::fromImage(QImage(":/default/images/defaultAvatar.png").scaled(avatarSize*dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        avatar.setDevicePixelRatio(dpr);
    }
    painter->save();
    QPainterPath avatarPath;
    int avatarRadius = avatarSize.width()/2;
    avatarPath.addRoundedRect(avatarRect, avatarRadius, avatarRadius);
    painter->setClipPath(avatarPath);
    painter->drawPixmap(avatarRect, avatar);
    painter->restore();

    bool isSelf = index.data(IsMyselfRole).toBool();
    QFont font = option.font;

    ContentType type = static_cast<ContentType>(index.data(ContentTypeRole).toInt());
    if(type == ContentType::Image)
    {
        ImageCacheManager::ImageState state = static_cast<ImageCacheManager::ImageState>(index.data(ImageStateRole).toInt());
        QPixmap pix = index.data(ImageRole).value<QPixmap>();
        if(state == ImageCacheManager::ImageState::Success && !pix.isNull())
        {
            int originalWidth = qRound(pix.width()*static_cast<double>(textRegionRect.height()) / pix.height());
            QSize drawSize(qMax(originalWidth, textRegionRect.width()), textRegionRect.height());

            QPixmap scaled = pix.scaled(drawSize * pix.devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(pix.devicePixelRatio());

            int x = textRegionRect.center().x() - scaled.deviceIndependentSize().toSize().width() / 2;
            int y = textRegionRect.center().y() - scaled.deviceIndependentSize().toSize().height() / 2;

            QPainterPath clipPath;
            int radius = 8;
            QRect imageRect(x, y, scaled.deviceIndependentSize().toSize().width(), scaled.deviceIndependentSize().toSize().height());
            clipPath.addRoundedRect(textRegionRect, radius, radius);

            painter->save();
            painter->setClipPath(clipPath);
            painter->drawPixmap(x, y, scaled);
            painter->setPen(QPen(QColor(209, 213, 219), 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(clipPath);
            painter->restore();
        }
        else
        {
            painter->setBrush(QColor(240, 240, 240));
            painter->setPen(QPen(QColor(209, 213, 219), 1));
            painter->drawRoundedRect(textRegionRect, 8, 8);

            painter->setPen(Qt::gray);
            font.setPointSize(15);
            painter->setFont(font);

            if(state == ImageCacheManager::ImageState::Failed)
            {
                //图片加载失败
                QSize loadingSize(60, 60);
                int x = textRegionRect.topLeft().x() + (textRegionRect.width() - loadingSize.width()) / 2;
                int y = textRegionRect.topLeft().y() + (textRegionRect.height() - loadingSize.height()) / 2;
                QRect loadingRect(x, y, loadingSize.width(), loadingSize.height());

                painter->drawPixmap(loadingRect, QPixmap(":/default/images/fresh.png"));
            }
            else if(state == ImageCacheManager::ImageState::NotExist)
            {
                //图片不存在
                QSize warnSize(50, 50);
                int x = textRegionRect.topLeft().x() + (textRegionRect.width() - warnSize.width()) / 2;
                int y = textRegionRect.topLeft().y() + (textRegionRect.height() - warnSize.height()) / 3;
                QRect warnRect(x, y, warnSize.width(), warnSize.height());
                QRect warnTextRect(QPoint(textRegionRect.topLeft().x(), warnRect.bottomLeft().y()), QSize(textRegionRect.width(), 40));
                painter->drawEllipse(warnRect);
                painter->drawText(warnRect, Qt::AlignCenter, "!");
                font.setPointSize(10);
                painter->setFont(font);
                painter->drawText(warnTextRect, Qt::AlignCenter, "图片不存在");
            }
            else if(state == ImageCacheManager::ImageState::Loading)
            {
                //图片正在加载
                painter->setPen(QPen(QColor(209, 213, 219), 3));
                QSize loadingSize(40, 40);
                int x = textRegionRect.topLeft().x() + (textRegionRect.width() - loadingSize.width()) / 2;
                int y = textRegionRect.topLeft().y() + (textRegionRect.height() - loadingSize.height()) / 2;
                QRect loadingRect(x, y, loadingSize.width(), loadingSize.height());

                int startAngle = -(*(this->loadingAngle)) * 16;
                int spanAngle = 240 * 16;
                painter->drawArc(loadingRect, startAngle, spanAngle);
            }
        }
    }
    else
    {
        QPainterPath path;
        path.addRoundedRect(textRegionRect, 8, 8);

        QColor borderColor;
        QColor textColor;
        if(isSelf)
        {
            borderColor = QColor(99, 102, 241);
            textColor = QColor(Qt::white);
        }
        else
        {
            borderColor = QColor(209, 213, 219);
            textColor = QColor(17, 24, 39);
        }
        painter->setBrush(borderColor);
        painter->setPen(Qt::NoPen);
        painter->drawPath(path);

        font.setPointSizeF(10.2);
        painter->setFont(font);
        QString text = index.data(ContentRole).toString();

        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        QTextLayout layout(text, font);
        layout.setTextOption(textOption);

        QRect textRect = textRegionRect.adjusted(15, 10, -15, -10);
        qreal drawY = textRect.top() + (textRect.height() - textTotalHeight) / 2.0;
        qreal drawX = textRect.left();

        layout.beginLayout();
        qreal y = 0;
        while (true)
        {
            QTextLine line = layout.createLine();
            if(!line.isValid())
                break;
            line.setLineWidth(textRect.width());
            line.setPosition(QPointF(drawX, drawY + y));
            y += line.height();
        }
        layout.endLayout();

        painter->setPen(textColor);
        layout.draw(painter, QPointF(0, 0));
    }

    if(isSelf)
    {
        painter->save();
        if(index.data(MessageStatusRole).toInt() == Sending)
        {
            QPen pen(QColor(100, 100, 100), 1.5, Qt::SolidLine, Qt::RoundCap);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);

            QRect arcRect = statusRect.adjusted(2, 2, -2, -2);
            int startAngle = (*(this->loadingAngle)) * 16;
            int spanAngle = 240 * 16;
            painter->drawArc(arcRect, startAngle, spanAngle);
        }
        else if(index.data(MessageStatusRole).toInt() == Failed)
        {
            painter->setBrush(Qt::red);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(statusRect, statusSize.width()/2, statusSize.height()/2);

            painter->setPen(Qt::white);
            font.setPointSize(8);
            font.setBold(true);
            painter->setFont(font);
            painter->drawText(statusRect, Qt::AlignCenter, "!");
        }
        painter->restore();
    }
    painter->restore();
}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Bottom预留30
    int bottomPadding = 0;
    if(index.row() == index.model()->rowCount() - 1)
        bottomPadding = 30;

    //timestamp预留40
    int topTimeStampPadding = 0;
    QVariant vi = index.data(IsNeedShowTime);
    bool isShowTime = vi.isValid() ? vi.toBool() : false;
    if(isShowTime)
        topTimeStampPadding = 40;

    int rectWidth = getViewportWidth(option);

    int totalHeight = 0;
    ContentType type = static_cast<ContentType>(index.data(ContentTypeRole).toInt());
    if(type == ContentType::Image)
    {
        QPixmap pix = index.data(ImageRole).value<QPixmap>();
        QSize imageSize;

        int maxWidth = qMax(20, static_cast<int>((rectWidth - 60) * 0.6));
        int maxHeight = 200;
        if(!pix.isNull())
        {
            imageSize = pix.size();
            imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        }
        else
            imageSize = QSize(160, 120);

        if(imageSize.height() < 40 && imageSize.height() > 0)
        {
            int width = qRound(imageSize.width() * 40.0 / imageSize.height());
            imageSize = QSize(qMin(width, maxWidth), 40);
        }

        totalHeight = imageSize.height() + 16;//16为了居中
    }
    else
    {
        QString message = index.data(ContentRole).toString();
        QFont font = option.font;
        font.setPointSizeF(10.2);

        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

        int textMaxWidth = (rectWidth - 60) * 0.7 - 30;//30是文字与气泡左右两边的间距相加15+15
        if(textMaxWidth < 20)
            textMaxWidth = 20;

        QTextLayout layout(message, font);
        layout.setTextOption(textOption);
        layout.beginLayout();
        qreal height = 0;

        while(true)
        {
            QTextLine line = layout.createLine();
            if(!line.isValid())
                break;
            line.setLineWidth(textMaxWidth);
            line.setPosition(QPointF(0, height));
            height += line.height();
        }
        layout.endLayout();
        int textHeight = qCeil(height);
        int bubbleHeight = qMax(textHeight + 20, 40);//20是文本相对于气泡内部的上下间距
        totalHeight = bubbleHeight + 16;//16是单纯为了居中
    }

    totalHeight += topTimeStampPadding + bottomPadding;
    return QSize(-1, totalHeight);
}

bool ConversationDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));
    if(!view)
        return false;
    if(event->type() == QMouseEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        if(mouse->button() != Qt::LeftButton)
            return false;

        bool isSelf = index.data(IsMyselfRole).toBool();
        if (!isSelf)
            return false;

        QRect TimeStamp;
        QRect contain;
        QRect avatarRect;
        QRect textRegionRect;
        QRect statusRect;
        int textTotalHeight = 0;
        getLayout(option, index, TimeStamp, contain, avatarRect, textRegionRect, statusRect, textTotalHeight);

        if(statusRect.contains(mouse->pos()))
        {
            emit ReSendClicked(index.data(MessageIDRole).toString());
            return true;
        }
        else if(textRegionRect.contains(mouse->pos()))
        {
            if(static_cast<ImageCacheManager::ImageState>(index.data(ImageStateRole).toInt()) == ImageCacheManager::ImageState::Failed)
            {
                emit ReloadImageClicked(index.data(MessageIDRole).toString());
                return true;
            }
            else if(static_cast<ImageCacheManager::ImageState>(index.data(ImageStateRole).toInt()) == ImageCacheManager::ImageState::Success)
            {
                emit previewImageClicked(index.data(ContentRole).toString());
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QString ConversationDelegate::formatTimestamp(int64_t timestamp) const
{
    //插入时间戳
    QDateTime now = QDateTime::currentDateTime();
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QString timeStr;
    //非本年
    if(msgTime.date().year() != now.date().year())
    {
        timeStr = msgTime.toString("yyyy年M月d日 hh:mm");
    }
    else if(msgTime.date() == now.date())
    {
        //今天
        timeStr = msgTime.toString("hh:mm");
    }
    else if(msgTime.date() == now.date().addDays(-1))
    {
        //昨天
        timeStr = QString("昨天 %1").arg(msgTime.toString("hh:mm"));
    }
    else if(msgTime.date() == now.date().addDays(-2))
    {
        //前天
        timeStr = QString("前天 %1").arg(msgTime.toString("hh:mm"));
    }
    else
    {
        int mondayOfTime = now.date().dayOfWeek() - 1;
        QDate thisMonday = now.date().addDays(-mondayOfTime);
        QDate thisSunday = thisMonday.addDays(6);
        if(msgTime.date() >= thisMonday && msgTime.date() <= thisSunday)
        {
            QStringList weekDays = {"","星期一","星期二","星期三","星期四","星期五","星期六","星期日"};
            timeStr = QString("%1 %2").arg(weekDays.at(msgTime.date().dayOfWeek()), msgTime.toString("hh:mm"));
        }
        else
        {
            timeStr = msgTime.toString("M月d日 hh:mm");
        }
    }
    return timeStr;
}

int ConversationDelegate::getViewportWidth(const QStyleOptionViewItem &option) const
{
    const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(option.widget);
    if(view && view->viewport())
        return view->viewport()->width();

    if(option.rect.width() > 0)
        return option.rect.width();

    return 536;
}

void ConversationDelegate::getLayout(const QStyleOptionViewItem &option, const QModelIndex& index, QRect &timestamp,
                                     QRect &contain, QRect &avatarRect, QRect &textRegionRect, QRect& statusRect, int& textTotalHeight) const
{
    QRect total = option.rect;
    QVariant vi1 = index.data(IsNeedShowTime);
    bool isTimeStamp = vi1.isValid() ? vi1.toBool() : false;
    if(isTimeStamp)
        timestamp = QRect(total.topLeft(), QSize(total.width(), 40));

    int topOffset = isTimeStamp ? 40 : 0;
    contain = option.rect.adjusted(0, topOffset, 0, 0);

    int rectWidth = getViewportWidth(option);

    bool isSelf = index.data(IsMyselfRole).toBool();
    QSize avatarSize(40, 40);
    if(isSelf)
    {
        avatarRect = QRect(contain.topRight() + QPoint(-10 - avatarSize.width(), 8), avatarSize);
    }
    else
    {
        avatarRect = QRect(contain.topLeft() + QPoint(10, 8), avatarSize);
    }

    ContentType type = static_cast<ContentType>(index.data(ContentTypeRole).toInt());
    if(type == ContentType::Image)
    {
        QPixmap pix = index.data(ImageRole).value<QPixmap>();
        QSize imageSize;

        int maxWidth = qMax(20, static_cast<int>((rectWidth - 20 - avatarSize.width()) * 0.6));
        int maxHeight = 200;
        if(!pix.isNull())
        {
            imageSize = pix.size();
            imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        }
        else
            imageSize = QSize(160, 120);

        if(imageSize.height() < 40 && imageSize.height() > 0)
        {
            int width = qRound(imageSize.width() * 40.0 / imageSize.height());
            imageSize = QSize(qMin(width, maxWidth), 40);
        }

        if(isSelf)
            textRegionRect = QRect(avatarRect.topLeft() - QPoint(10 + imageSize.width(), 0), imageSize);
        else
            textRegionRect = QRect(avatarRect.topRight() + QPoint(10, 0), imageSize);
    }
    else
    {
        QFont font = option.font;
        font.setPointSizeF(10.2);

        QString text = index.data(ContentRole).toString();

        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        int textMaxWidth = (rectWidth - 20 - avatarSize.width()) * 0.7 - 30;
        if(textMaxWidth < 20)
            textMaxWidth = 20;

        QTextLayout layout(text, font);
        layout.setTextOption(textOption);
        layout.beginLayout();
        qreal textHeight = 0;
        qreal textWidth = 0;
        while(true)
        {
            QTextLine line = layout.createLine();
            if(!line.isValid())
                break;
            line.setLineWidth(textMaxWidth);
            textHeight += line.height();
            textWidth = qMax(textWidth, line.naturalTextWidth());
        }
        layout.endLayout();
        textTotalHeight = qCeil(textHeight);

        int bubbleWidth = qCeil(textWidth) + 30;
        int bubbleHeight = textTotalHeight + 20;
        bubbleHeight = qMax(bubbleHeight, 40);

        if(isSelf)
            textRegionRect = QRect(avatarRect.topLeft() - QPoint(10 + bubbleWidth, 0), QSize(bubbleWidth, bubbleHeight));
        else
            textRegionRect = QRect(avatarRect.topRight() + QPoint(10, 0), QSize(bubbleWidth, bubbleHeight));
    }

    if(isSelf)
    {
        QSize statusSize(15, 15);
        statusRect = QRect(textRegionRect.bottomLeft() - QPoint(10 + statusSize.width(), 10 + statusSize.height()), statusSize);
    }
}
