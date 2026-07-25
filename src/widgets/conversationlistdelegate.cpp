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
    QSize avatarSize(contain.height() * 0.6, contain.height() * 0.6);
    QRect avatar(contain.topLeft() + QPoint(10, (contain.height() - avatarSize.height())/2), avatarSize);
    QPixmap avatarIcon = index.data(AvatarRole).value<QPixmap>();
    if(avatarIcon.isNull())
        avatarIcon = setRadius(QPixmap(":/default/images/defaultAvatar.png"), avatarSize.width());
    else
        avatarIcon = setRadius(index.data(AvatarRole).value<QPixmap>(), avatarSize.width());
    painter->drawPixmap(avatar, avatarIcon);

    QFont font = option.font;
    font.setPointSize(10);
    font.setBold(true);

    painter->setFont(font);

    QString text = index.data(UsernameRole).toString();
    QFontMetrics fm = painter->fontMetrics();
    int textWidth = fm.horizontalAdvance(text) + 4;
    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 8, 0), QSize(textWidth, textHeight));

    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(username, Qt::AlignVCenter, text);

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

