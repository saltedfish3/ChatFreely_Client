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
        if(!pix.isNull())
        {
            int originalWidth = qRound(pix.width()*static_cast<double>(textRegionRect.height()) / pix.height());
            QSize drawSize(qMax(originalWidth, textRegionRect.width()), textRegionRect.height());

            QPixmap scaled = pix.scaled(drawSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(dpr);

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
    else if(type == ContentType::Video)
    {
        QPixmap thumbnail = index.data(VideoThumbnail).value<QPixmap>();
        if(!thumbnail.isNull())
        {
            int originalWidth = qRound(thumbnail.width()*static_cast<double>(textRegionRect.height()) / thumbnail.height());
            QSize drawSize(qMax(originalWidth, textRegionRect.width()), textRegionRect.height());

            QPixmap scaled = thumbnail.scaled(drawSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(dpr);

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
        }

        int uploadPercent = index.data(MediaUploadProgress).toInt();
        bool isUploading = uploadPercent >= 0;
        Status status = static_cast<Status>(index.data(MessageStatusRole).toInt());

        QSize centerSize(60, 60);
        QPointF centerPoint(textRegionRect.x() + (textRegionRect.width()  - centerSize.width())  / 2.0, textRegionRect.y() + (textRegionRect.height() - centerSize.height()) / 2.0);
        QRectF centerRect(centerPoint, centerSize);

        if(isUploading)
        {
            QRectF arcRect = centerRect.adjusted(4, 4, -4, -4);
            painter->save();
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(QColor(255, 255, 255, 60), 3, Qt::SolidLine, Qt::RoundCap));
            painter->drawArc(arcRect, 0, 360 * 16);

            if(uploadPercent > 0)
            {
                painter->setPen(QPen(QColor(91, 155, 213), 3, Qt::SolidLine, Qt::RoundCap));
                painter->drawArc(arcRect, 90 * 16, -uploadPercent * 360 * 16 /100);
            }

            qreal radius = arcRect.width() * 0.15;
            QPointF center = arcRect.center();
            painter->setPen(QPen(QColor(255, 255, 255, 220), 2, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(QPointF(center.x() - radius, center.y() - radius), QPointF(center.x() + radius, center.y() + radius));
            painter->drawLine(QPointF(center.x() - radius, center.y() + radius), QPointF(center.x() + radius, center.y() - radius));

            painter->restore();
        }
        else if(status == Cancelled || status == Failed)
        {
            QSize iconSize(32, 32);
            QPixmap icon(":/default/images/fresh.png");
            if(!icon.isNull())
            {
                icon = icon.scaled(iconSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                icon.setDevicePixelRatio(dpr);

                QSize logicalSize = icon.deviceIndependentSize().toSize();
                int x = centerRect.center().x() - logicalSize.width() / 2;
                int y = centerRect.center().y() - logicalSize.height() / 2;
                painter->drawPixmap(x, y, icon);
            }
        }
        else if(index.data(MediaExpired).toBool())
        {
            painter->save();
            painter->setPen(QColor(160, 160, 160));
            font.setPointSize(10);
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(textRegionRect, Qt::AlignCenter, "视频已过期");
            painter->restore();
        }
        else
        {
            painter->save();
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(QColor(209, 213, 219), 2));

            painter->drawRoundedRect(centerRect, centerSize.width() / 2, centerSize.width() / 2);

            int lineWidth = centerSize.height() / 2;

            QPointF triangleCenter = centerRect.center();

            int padding = qCeil(std::sqrt(lineWidth*lineWidth - (lineWidth / 2) * (lineWidth / 2))) / 2;
            int margin = 4;

            QPointF point1 = triangleCenter - QPointF(padding - margin, lineWidth / 2);
            QPointF point2 = triangleCenter + QPointF(padding + margin, 0);
            QPointF point3 = point1 + QPointF(0, lineWidth);

            painter->drawLine(point1, point2);
            painter->drawLine(point2, point3);
            painter->drawLine(point1, point3);

            QPolygonF triangle;
            triangle << point1 << point2 << point3;
            painter->setBrush(QColor(209, 213, 219));
            painter->setPen(Qt::NoPen);
            painter->drawPolygon(triangle);

            painter->setBrush(Qt::NoBrush);
            if(!thumbnail.isNull())
                painter->setPen(Qt::white);
            else
                painter->setPen(QColor(107, 114, 128));

            qint64 duration = index.data(VideoDuration).toLongLong();
            if(duration > 0)
            {
                font.setPointSize(9);
                painter->setFont(font);
                QRect durationRect(textRegionRect.bottomLeft() - QPoint(-15, 30), QSize(textRegionRect.width(), 30));
                qint64 totalSec = duration / 1000;
                qint64 hours = totalSec / 3600;
                qint64 minutes = (totalSec % 3600) / 60;
                qint64 seconds = totalSec % 60;

                QString durationText;
                if(hours > 0)
                    durationText = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0'))
                                       .arg(minutes, 2, 10, QChar('0'))
                                       .arg(seconds, 2, 10, QChar('0'));
                else
                    durationText = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));

                painter->drawText(durationRect, Qt::AlignVCenter | Qt::AlignLeft, durationText);
            }

            painter->restore();
        }
    }
    else if(type == ContentType::File)
    {
        painter->setBrush(QColor(229, 231, 235));
        painter->setPen(QPen(QColor(209, 213, 219), 1));
        painter->drawRoundedRect(textRegionRect, 8, 8);

        //绘制图标
        const int padding = 5;
        const QSize iconSize(46, 46);
        int textX = padding + 10;
        int rightMargin = padding + 10;
        int textWidth = textRegionRect.width() - textX - rightMargin - iconSize.width();

        QFont font = painter->font();
        font.setPointSize(9);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QColor(55, 65, 81));

        QFontMetrics fm(font);
        QString showName = fm.elidedText(index.data(FileName).toString(), Qt::ElideMiddle, textWidth);
        painter->drawText(QRect((textRegionRect.topLeft() + QPoint(textX, 8)), QSize(textWidth, 20)), Qt::AlignVCenter | Qt::AlignLeft, showName);

        QString sizeText;
        qint64 fileSize = index.data(MediaSize).toLongLong();
        if(fileSize < 1024)
            sizeText = QString::number(fileSize) + " B";
        else if(fileSize < 1024 * 1024)
            sizeText = QString::number(fileSize / 1024.0, 'f', 2) + " KB";
        else if(fileSize < 1024LL * 1024 * 1024)
            sizeText = QString::number(fileSize / 1024.0 / 1024.0, 'f', 2) + " MB";
        else
            sizeText = QString::number(fileSize / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GB";

        font.setBold(false);
        font.setPointSize(8);
        painter->setFont(font);
        painter->setPen(QColor(107, 114, 128));
        painter->drawText(QRectF((textRegionRect.topLeft() + QPoint(textX, textRegionRect.height() - 26)), QSize(textWidth, 18)), Qt::AlignVCenter | Qt::AlignLeft, sizeText);

        QPixmap icon(":/default/images/showFile.png");
        icon = icon.scaled(iconSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        icon.setDevicePixelRatio(dpr);

        QRect iconRect(textRegionRect.topLeft() + QPoint(textX + textWidth, (textRegionRect.height() - iconSize.height()) / 2), iconSize);
        painter->drawPixmap(iconRect, icon);
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
        int uploadProgress = index.data(MediaUploadProgress).toInt();
        bool isVideoUploading = (type == ContentType::Video && uploadProgress >= 0);
        bool isVideoFailed = type == ContentType::Video && (index.data(MessageStatusRole).toInt() == Cancelled || index.data(MessageStatusRole).toInt() == Failed);

        if(!isVideoUploading && !isVideoFailed)
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
            imageSize = pix.deviceIndependentSize().toSize();
            imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        }
        else
        {
            int width = index.data(MediaWidth).toInt();
            int height = index.data(MediaHeight).toInt();
            if(width <= 0 || height <= 0)
                imageSize = QSize(160, 120);
            else
            {
                imageSize = QSize(width, height);
                imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
            }
        }

        if(imageSize.height() < 40 && imageSize.height() > 0)
        {
            int width = qRound(imageSize.width() * 40.0 / imageSize.height());
            imageSize = QSize(qMin(width, maxWidth), 40);
        }

        totalHeight = imageSize.height() + 16;//16为了居中
    }
    else if(type == ContentType::Video)
    {
        int maxWidth = qMax(20, static_cast<int>((rectWidth - 60) * 0.6));
        int maxHeight = 200;

        QSize thumbnailSize(index.data(MediaWidth).toInt(), index.data(MediaHeight).toInt());
        if(thumbnailSize.width() <= 0 || thumbnailSize.height() <= 0)
            thumbnailSize = QSize(160, 120);

        thumbnailSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);

        if(thumbnailSize.height() < 40 && thumbnailSize.height() > 0)
        {
            int width = qRound(thumbnailSize.width() * 40.0 / thumbnailSize.height());
            thumbnailSize = QSize(qMin(width, maxWidth), 40);
        }

        totalHeight = thumbnailSize.height() + 16;
    }
    else if(type == ContentType::File)
    {
        int maxWidth = qMax(20, static_cast<int>((rectWidth - 60) * 0.5));
        int maxHeight = 60;

        QSize fileSize(maxWidth, maxHeight);

        totalHeight = fileSize.height() + 16;
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

        QRect TimeStamp;
        QRect contain;
        QRect avatarRect;
        QRect textRegionRect;
        QRect statusRect;
        int textTotalHeight = 0;
        getLayout(option, index, TimeStamp, contain, avatarRect, textRegionRect, statusRect, textTotalHeight);

        Status status = static_cast<Status>(index.data(MessageStatusRole).toInt());
        ContentType type = static_cast<ContentType>(index.data(ContentTypeRole).toInt());
        if(isSelf && type == ContentType::Video)
        {
            QSize centerSize(60, 60);
            QPointF centerPoint(textRegionRect.x() + (textRegionRect.width()  - centerSize.width())  / 2.0, textRegionRect.y() + (textRegionRect.height() - centerSize.height()) / 2.0);
            QRectF centerRect(centerPoint, centerSize);
            int uploadPercent = index.data(MediaUploadProgress).toInt();
            bool isUploading = uploadPercent >= 0;

            if(isUploading && centerRect.contains(mouse->pos()))
            {
                emit CancelUploadClicked(index.data(MessageIDRole).toString());
                return true;
            }
            if((status == Cancelled || status == Failed) && centerRect.contains(mouse->pos()))
            {
                emit ReSendClicked(index.data(MessageIDRole).toString());
                return true;
            }
        }
        if(type == ContentType::Video && textRegionRect.contains(mouse->pos()))
        {
            if(index.data(MediaExpired).toBool())
                return true;
        }

        if(statusRect.contains(mouse->pos()) && isSelf && status == Failed && type != ContentType::Video)
        {
            emit ReSendClicked(index.data(MessageIDRole).toString());
            return true;
        }
        else if(textRegionRect.contains(mouse->pos()) && status == Success)
        {
            if(type == ContentType::Image)
            {
                QPixmap pix = index.data(ImageRole).value<QPixmap>();
                if(!pix.isNull())
                {
                    emit previewImageClicked(index.data(MediaUrl).toString());
                    return true;
                }
                if(static_cast<ImageCacheManager::ImageState>(index.data(ImageStateRole).toInt()) == ImageCacheManager::ImageState::Failed)
                {
                    emit ReloadImageClicked(index.data(MessageIDRole).toString());
                    return true;
                }
                // else if(static_cast<ImageCacheManager::ImageState>(index.data(ImageStateRole).toInt()) == ImageCacheManager::ImageState::Success)
                // {
                //     emit previewImageClicked(index.data(MediaUrl).toString());
                //     return true;
                // }
            }
            else if(type == ContentType::Video && status == Success)
            {
                QString url = index.data(MediaUrl).toString();
                if(url.isEmpty() || !url.startsWith("http"))
                    return true;

                emit previewVideoClicked(index.data(MediaUrl).toString(), index.data(MediaSize).toLongLong());
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
            imageSize = pix.deviceIndependentSize().toSize();
            imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        }
        else
        {
            int width = index.data(MediaWidth).toInt();
            int height = index.data(MediaHeight).toInt();
            if(width <= 0 || height <= 0)
                imageSize = QSize(160, 120);
            else
            {
                imageSize = QSize(width, height);
                imageSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
            }
        }

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
    else if(type == ContentType::Video)
    {
        // QPixmap thumbnail = index.data(VideoThumbnail).value<QPixmap>();
        int maxWidth = qMax(20, static_cast<int>((rectWidth - 20 - avatarSize.width()) * 0.6));
        int maxHeight = 200;

        QSize thumbnailSize(index.data(MediaWidth).toInt(), index.data(MediaHeight).toInt());
        if(thumbnailSize.width() <= 0 || thumbnailSize.height() <= 0)
            thumbnailSize = QSize(160, 120);

        thumbnailSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);

        if(thumbnailSize.height() < 40 && thumbnailSize.height() > 0)
        {
            int width = qRound(thumbnailSize.width() * 40.0 / thumbnailSize.height());
            thumbnailSize = QSize(qMin(width, maxWidth), 40);
        }

        if(isSelf)
            textRegionRect = QRect(avatarRect.topLeft() - QPoint(10 + thumbnailSize.width(), 0), thumbnailSize);
        else
            textRegionRect = QRect(avatarRect.topRight() + QPoint(10, 0), thumbnailSize);
    }
    else if(type == ContentType::File)
    {
        int maxWidth = qMax(20, static_cast<int>((rectWidth - 20 - avatarSize.width()) * 0.5));
        int maxHeight = 60;

        QSize fileSize(maxWidth, maxHeight);

        if(isSelf)
            textRegionRect = QRect(avatarRect.topLeft() - QPoint(10 + fileSize.width(), 0), fileSize);
        else
            textRegionRect = QRect(avatarRect.topRight() + QPoint(10, 0), fileSize);
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
