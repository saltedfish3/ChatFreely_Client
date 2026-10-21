#include "groupmemberdelegate.h"

GroupMemberDelegate::GroupMemberDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void GroupMemberDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    bool isHovered = this->addFriendIndex == index && this->addFriendState == Hover;
    bool isPressed = this->addFriendIndex == index && this->addFriendState == Pressed;

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

    QRect rect_addFriend;
    getButtonsRect(contain, rect_addFriend);

    if(!index.data(IsFriendRole).toBool())
    {
        painter->save();

        QColor color_addFriendBackground(Qt::transparent);
        QColor color_addFriendBorder(99, 102, 241);
        QColor color_addFriendText(99, 102, 241);

        if(isHovered)
            color_addFriendBackground = QColor(99, 102, 241, 20);

        if(isPressed)
            color_addFriendBackground = QColor(99, 102, 241, 50);

        painter->setPen(QPen(color_addFriendBorder, 1));
        painter->setBrush(color_addFriendBackground);
        painter->drawRoundedRect(rect_addFriend, 4, 4);

        QFont btnFont = option.font;
        btnFont.setPointSize(9);
        btnFont.setBold(false);
        painter->setFont(btnFont);
        painter->setPen(color_addFriendText);

        painter->drawText(rect_addFriend, Qt::AlignCenter, "添加");

        painter->restore();
    }

    QFont font = option.font;
    font.setPointSize(10);
    font.setBold(true);

    painter->setFont(font);

    QString text = index.data(UsernameRole).toString();
    QFontMetrics fm = painter->fontMetrics();
    int textHeight = fm.height();
    QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 6, 2), QSize(contain.topRight().x() - avatar.topRight().x() - 6 - (contain.topRight().x() - rect_addFriend.left()), qMax(textHeight, 20)));

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

bool GroupMemberDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));
    if(!view)
        return false;

    QRect contain = option.rect.adjusted(-1, 0, 1, 0);
    if(!index.data(IsFriendRole).toBool())
    {
        if(event->type() == QEvent::MouseMove)
        {
            //按钮Hover状态处理
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);

            QRect rect_addFriend;
            getButtonsRect(contain, rect_addFriend);

            ButtonState newAddFriendState = None;

            if(rect_addFriend.contains(mouse->pos()))
                newAddFriendState = this->isAddFriendPressed ? Pressed : Hover;

            bool needUpdate = false;
            if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
            {
                this->addFriendState = newAddFriendState;
                this->addFriendIndex = index;
                needUpdate = true;
            }

            if(needUpdate)
                view->viewport()->update(option.rect);

            return true;
        }
        else if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
            if(mouse->button() == Qt::LeftButton)
            {
                QRect rect_addFriend;
                getButtonsRect(contain, rect_addFriend);

                ButtonState newAddFriendState = None;

                if(rect_addFriend.contains(mouse->pos()))
                {
                    newAddFriendState = Pressed;
                    this->isAddFriendPressed = true;
                }

                bool needUpdate = false;
                if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
                {
                    this->addFriendState = newAddFriendState;
                    this->addFriendIndex = index;
                    needUpdate = true;
                }

                if(needUpdate)
                    view->viewport()->update(option.rect);

                return true;
            }
            return false;
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);

            QRect rect_addFriend;
            getButtonsRect(contain, rect_addFriend);

            ButtonState newAddFriendState = None;

            if(rect_addFriend.contains(mouse->pos()))
            {
                if(this->isAddFriendPressed)
                {
                    emit addFriendClicked(index.data(SIDRole).toString(), index);
                }
                newAddFriendState = Hover;
            }

            this->isAddFriendPressed = false;

            bool needUpdate = false;
            if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
            {
                this->addFriendState = newAddFriendState;
                this->addFriendIndex = index;
                needUpdate = true;
            }

            if(needUpdate)
                view->viewport()->update(option.rect);

            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void GroupMemberDelegate::getButtonsRect(const QRect &containRect, QRect &outAddFriendRect) const
{
    if(containRect.isNull() || containRect.isEmpty())
        return;

    int marginRight = 10;
    QSize size_addFriend(40, 24);
    outAddFriendRect = QRect(containRect.topRight() - QPoint(marginRight + size_addFriend.width(), -(containRect.height() - size_addFriend.height()) / 2), size_addFriend);
}