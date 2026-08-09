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
    if(avatar.isNull())
        avatar = QPixmap(":/default/images/defaultAvatar.png");
    avatar = setRadius(avatar, avatarSize.height());
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPixmap(avatarRect, avatar);

    bool isSelf = index.data(IsMyselfRole).toBool();

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

    QFont font = option.font;
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


    QString message = index.data(ContentRole).toString();
    QFont font = option.font;
    font.setPointSizeF(10.2);

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    int textMaxWidth = (option.rect.width() - 60) * 0.7 - 30;//536
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

    int textHeight = qCeil(height) + 20;
    int totalHeight = qMax(textHeight, 40) + 16;
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
            timeStr = QString("%1 %2").arg(weekDays.at(msgTime.date().dayOfWeek()), "hh:mm");
        }
        else
        {
            timeStr = msgTime.toString("M月d日 hh:mm");
        }
    }
    return timeStr;
}

QPixmap ConversationDelegate::setRadius(const QPixmap& pixmap, int hei_wid) const
{
    if(pixmap.isNull())
        return {};

    //适配高DPI
    const qreal dpr = pixmap.devicePixelRatioF();
    const int pixmapSize = qRound(hei_wid * dpr);
    QPixmap scaled = pixmap.scaled(pixmapSize, pixmapSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(dpr);

    QPixmap roundedPix(pixmapSize, pixmapSize);
    roundedPix.fill(Qt::transparent);
    roundedPix.setDevicePixelRatio(pixmap.devicePixelRatioF());

    QPainter painter(&roundedPix);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0,0,hei_wid,hei_wid);

    painter.setClipPath(path);
    painter.drawPixmap(0,0,scaled);
    painter.end();
    return roundedPix;
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

    QFont font = option.font;
    font.setPointSizeF(10.2);

    QString text = index.data(ContentRole).toString();

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    int textMaxWidth = (contain.width() - 20 - avatarSize.width()) * 0.7 - 30;
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

    if(isSelf)
    {
        QSize statusSize(15, 15);
        statusRect = QRect(textRegionRect.bottomLeft() - QPoint(10 + statusSize.width(), 10 + statusSize.height()), statusSize);
    }
}
