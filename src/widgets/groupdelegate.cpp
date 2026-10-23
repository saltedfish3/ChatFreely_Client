#include "groupdelegate.h"

GroupDelegate::GroupDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void GroupDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    QSize avatarSize(38, 38);
    QRect avatar(contain.topLeft() + QPoint(15, (contain.height() - avatarSize.height())/2), avatarSize);
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

    QString text = index.data(NameRole).toString();
    QFontMetrics fm = painter->fontMetrics();
    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 8, 3), QSize(contain.topRight().x() - avatar.topRight().x() - 15, textHeight));

    QString memberCount = "(" + QString::number(index.data(MemberCountRole).toLongLong()) + ")";
    int width_memberCount = fm.horizontalAdvance(memberCount);

    QString showText = fm.elidedText(text, Qt::ElideRight, username.width() - width_memberCount - 10);
    showText += memberCount;

    QFont font = option.font;
    font.setPointSize(9);
    font.setBold(true);

    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(username, Qt::AlignVCenter, showText);

    painter->restore();
}

QSize GroupDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant i = index.data(GroupRole);
    bool isGroup = i.isValid() ? i.toBool() : false;
    if(isGroup)
        return QSize(-1, 55*0.7);
    return QSize(-1, 55);
}

bool GroupDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
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