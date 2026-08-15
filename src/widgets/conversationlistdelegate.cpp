#include "conversationlistdelegate.h"

ConversationListDelegate::ConversationListDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void ConversationListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    bool isHovered = option.state & QStyle::State_MouseOver;
    bool isClicked = index.data(IsSelectedRole).toBool();

    painter->setBrush(Qt::NoBrush);
    if(isHovered && !isClicked)
        painter->setBrush(QColor(243, 244, 246));
    else if(isClicked)
        painter->setBrush(QColor(245, 242, 254));
    painter->drawRect(contain);

    if(isClicked)
    {
        QRect clickedSide(QRect(contain.topLeft(), QSize(4, contain.height())));
        painter->setBrush(QColor(70, 72, 212));
        painter->drawRect(clickedSide);
    }
    QSize avatarSize(contain.height() * 0.6, contain.height() * 0.6);
    QRect avatar(contain.topLeft() + QPoint(10, (contain.height() - avatarSize.height())/2), avatarSize);
    QPixmap avatarIcon = index.data(AvatarRole).value<QPixmap>();
    if(avatarIcon.isNull())
        avatarIcon = setRadius(QPixmap(":/default/images/defaultAvatar.png"), avatarSize.width());
    else
        avatarIcon = setRadius(index.data(AvatarRole).value<QPixmap>(), avatarSize.width());
    painter->drawPixmap(avatar, avatarIcon);

    QSize unReadSize(14, 14);
    QRect unReadRect(avatar.topRight() - QPoint(10, 0), unReadSize);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(247, 76, 48));

    QFont font = option.font;
    font.setPointSize(7);
    painter->setFont(font);

    int unReadCount = index.data(UnReadRole).toInt();
    if(unReadCount > 0)
    {
        painter->drawRoundedRect(unReadRect, unReadSize.width()/2, unReadSize.height()/2);
        painter->setPen(QPen(QColor(Qt::white), 1));
        QString unReadText = QString::number(unReadCount > 99 ? 99 : unReadCount);
        painter->drawText(unReadRect, Qt::AlignCenter, unReadText);
    }

    font.setPointSize(10);
    font.setBold(true);

    painter->setFont(font);

    QString text = index.data(UsernameRole).toString();
    QFontMetrics fm = painter->fontMetrics();

    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 8, 0), QSize(85, textHeight));

    QString drawText = fm.elidedText(text, Qt::ElideRight, username.width());
    painter->setPen(Qt::black);
    painter->drawText(username, Qt::AlignVCenter, drawText);

    QColor lastColor = isClicked ? QColor(85, 85, 85) : QColor(128, 128, 128);

    QString lastMsg = index.data(LastMsgRole).toString();
    font.setPointSize(8);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(lastColor);
    fm = painter->fontMetrics();
    int lastMsgHeight = fm.height();

    QRect lastMsgRect(username.bottomLeft() + QPoint(0, 5), QSize(140, lastMsgHeight));
    QString drawLastMsg = fm.elidedText(lastMsg, Qt::ElideRight, lastMsgRect.width());
    painter->drawText(lastMsgRect, Qt::AlignVCenter, drawLastMsg);

    font.setPointSizeF(7.5);
    painter->setFont(font);
    fm = painter->fontMetrics();
    QRect timestampRect(username.topRight(), QSize(contain.topRight().x() - username.topRight().x() - 5, fm.height()));
    QString drawTime = formatTimestamp(index.data(LastTimestampRole).toLongLong());
    painter->drawText(timestampRect, Qt::AlignVCenter | Qt::AlignRight, drawTime);

    QSize onlineSize(contain.height() * 0.15, contain.height() * 0.15);
    QRect onlineIcon(avatar.bottomRight() - QPoint(9, 9), onlineSize);
    painter->setPen(QColor(240, 240, 240));

    QVariant v = index.data(IsOnlineRole);
    bool isOnline = v.isValid() ? v.toBool() : false;
    painter->setBrush(Qt::gray);
    if(isOnline)
        painter->setBrush(QColor(102, 255, 0));

    painter->drawRoundedRect(onlineIcon, onlineSize.width()/2, onlineSize.height()/2);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(240, 240, 240));
    painter->drawLine(contain.bottomLeft() + QPoint(contain.width()*0.1, 0), contain.bottomRight() - QPoint(contain.width()*0.1, 0));

    painter->restore();
}

QSize ConversationListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(-1, 60);
}

bool ConversationListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        if(option.rect.contains(mouse->pos()))
        {
            emit itemClicked(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QPixmap ConversationListDelegate::setRadius(const QPixmap& pixmap, int hei_wid) const
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

QString ConversationListDelegate::formatTimestamp(int64_t timestamp) const
{
    if(timestamp <= 0)
        return {};

    //插入时间戳
    QDateTime now = QDateTime::currentDateTime();
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QString timeStr;
    //非本年
    if(msgTime.date().year() != now.date().year())
    {
        timeStr = msgTime.toString("yyyy/M/d");
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
            timeStr = QString("%1").arg(weekDays.at(msgTime.date().dayOfWeek()));
        }
        else
        {
            timeStr = msgTime.toString("M/d");
        }
    }
    return timeStr;
}

