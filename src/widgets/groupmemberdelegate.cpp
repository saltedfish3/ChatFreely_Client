#include "groupmemberdelegate.h"

GroupMemberDelegate::GroupMemberDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void GroupMemberDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    // bool isHovered = option.state & QStyle::State_MouseOver;
    // bool isClicked = option.state & QStyle::State_Selected;
    // painter->setBrush(Qt::NoBrush);
    // if(isHovered && !isClicked)
    //     painter->setBrush(QColor(243, 244, 246));
    // else if(isClicked)
    //     painter->setBrush(QColor(245, 242, 254));
    // painter->drawRect(contain);

    // if(isClicked)
    // {
    //     QRect clickedSide(QRect(contain.topLeft(), QSize(4, contain.height())));
    //     painter->setBrush(QColor(70, 72, 212));
    //     painter->drawRect(clickedSide);
    // }

    QSize avatarSize(38, 38);
    QRect avatar(contain.topLeft() + QPoint(10, (contain.height() - avatarSize.height())/2), avatarSize);
    QPixmap avatarIcon = index.data(AvatarRole).value<QPixmap>();

    if(avatarIcon.isNull())
    {
        avatarIcon = QPixmap(":/default/images/defaultAvatar.png");
        avatarIcon.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    }

    painter->save();
    QPainterPath path;
    path.addRoundedRect(avatar, avatarSize.width()/2, avatarSize.height()/2);
    painter->setClipPath(path);
    painter->drawPixmap(avatar, avatarIcon);
    painter->restore();

    QFont font = option.font;
    font.setPointSize(10);
    font.setBold(true);

    painter->setFont(font);

    QString text = index.data(UsernameRole).toString();
    QFontMetrics fm = painter->fontMetrics();
    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 6, 2), QSize(contain.topRight().x() - avatar.topRight().x() - 6, qMax(textHeight, 20)));

    QString showText = fm.elidedText(text, Qt::ElideRight, username.width());

    painter->setPen(QColor(17, 24, 39, 255));
    painter->setBrush(Qt::NoBrush);
    painter->drawText(username, Qt::AlignVCenter, showText);

    QString sidText = "ID:" + index.data(SIDRole).toString();
    QRect sid(username.bottomLeft(), QSize(username.width(), qMax(textHeight, 15)));

    font.setPointSize(8);
    font.setBold(false);
    painter->setFont(font);

    painter->setPen(QColor(107, 114, 128, 255));
    painter->setBrush(Qt::NoBrush);
    painter->drawText(sid, Qt::AlignVCenter, sidText);

    painter->restore();
}

QSize GroupMemberDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(-1, 60);
}