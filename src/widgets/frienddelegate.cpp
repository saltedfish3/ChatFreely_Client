#include "frienddelegate.h"

FriendDelegate::FriendDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void FriendDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    QVariant vGroup = index.data(GroupRole);
    bool isGroup = vGroup.isValid() ? vGroup.toBool() : false;
    if(isGroup)
    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QColor(224, 224, 224));
        painter->drawLine(contain.bottomLeft() - QPoint(0, 0), contain.bottomRight() - QPoint(0, 0));

        QFont font = option.font;
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);

        painter->setPen(QColor(107, 114, 128));
        QRect groupText(contain.bottomLeft() - QPoint(-15, 30), QSize(contain.width(), 30));
        painter->drawText(groupText, Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

        painter->restore();
        return;
    }

    bool isHovered = option.state & QStyle::State_MouseOver;
    bool isClicked = option.state & QStyle::State_Selected;
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
    QSize avatarSize(contain.height() * 0.7, contain.height() * 0.7);
    QRect avatar(contain.topLeft() + QPoint(15, (contain.height() - avatarSize.height())/2), avatarSize);
    QPixmap avatarIcon = index.data(AvatarRole).value<QPixmap>();
    if(avatarIcon.isNull())
        avatarIcon = setRadius(QPixmap(":/default/images/defaultAvatar.png"), avatarSize.width());
    else
        avatarIcon = setRadius(index.data(AvatarRole).value<QPixmap>(), avatarSize.width());
    painter->drawPixmap(avatar, avatarIcon);

    QString text = index.data(UsernameRole).toString();
    QFontMetrics fm = painter->fontMetrics();
    int textWidth = fm.horizontalAdvance(text) + 4;
    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 8, 3), QSize(textWidth, textHeight));

    QFont font = option.font;
    font.setPointSize(9);
    font.setBold(true);

    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(username, Qt::AlignVCenter, text);

    QSize onlineSize(contain.height() * 0.13, contain.height() * 0.13);
    QRect onlineIcon(avatar.topLeft() + QPoint(10 + avatarSize.width(), (contain.height() - onlineSize.height())/2), onlineSize);
    painter->setPen(Qt::NoPen);
    QString isOnlineText = "离线";

    QVariant v = index.data(IsOnlineRole);
    bool isOnline = v.isValid() ? v.toBool() : false;
    painter->setBrush(Qt::gray);
    if(isOnline)
    {
        painter->setBrush(Qt::green);
        isOnlineText = "在线";
    }
    painter->drawRoundedRect(onlineIcon, onlineSize.width()/2, onlineSize.height()/2);

    QRect onlineState(onlineIcon.topLeft() - QPoint(0, textHeight/2) + QPoint(onlineSize.width() + 5, onlineSize.height()/2),
                      QSize(30 ,textHeight));
    font.setPointSize(8);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(onlineState, Qt::AlignVCenter, isOnlineText);


    painter->restore();
}

QSize FriendDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant i = index.data(GroupRole);
    bool isGroup = i.isValid() ? i.toBool() : false;
    if(isGroup)
        return QSize(-1, 55*0.7);
    return QSize(-1, 55);
}

bool FriendDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QVariant v = index.data(GroupRole);
    bool isGroup = v.isValid() ? v.toBool() : false;
    if(isGroup)
        return true;
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

QPixmap FriendDelegate::setRadius(const QPixmap& pixmap, int hei_wid) const
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
