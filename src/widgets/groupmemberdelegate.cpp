#include "groupmemberdelegate.h"

GroupMemberDelegate::GroupMemberDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void GroupMemberDelegate::setIsOwner(bool isOwner)
{
    this->isOwner = isOwner;
}

void GroupMemberDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    bool isAddFriendHovered = this->addFriendIndex == index && this->addFriendState == Hover;
    bool isAddFriendPressed = this->addFriendIndex == index && this->addFriendState == Pressed;
    bool isRemoveMemberHovered = this->removeMemberIndex == index && this->removeMemberState == Hover;
    bool isRemoveMemberPressed = this->removeMemberIndex == index && this->removeMemberState == Pressed;

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
    QRect rect_removeMember;
    getButtonsRect(contain, rect_addFriend, rect_removeMember);

    if(!index.data(IsFriendRole).toBool())
    {
        painter->save();

        QColor color_addFriendBackground(Qt::transparent);
        QColor color_addFriendBorder(99, 102, 241);
        QColor color_addFriendText(99, 102, 241);

        if(isAddFriendHovered)
            color_addFriendBackground = QColor(99, 102, 241, 20);

        if(isAddFriendPressed)
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

    if(isOwner)
    {
        painter->save();

        QColor color_removeBackground(Qt::transparent);
        QColor color_removeBorder(220, 38, 38);
        QColor color_removeText(220, 38, 38);

        if(isRemoveMemberHovered)
            color_removeBackground = QColor(220, 38, 38, 20);

        if(isRemoveMemberPressed)
            color_removeBackground = QColor(220, 38, 38, 50);

        painter->setPen(QPen(color_removeBorder, 1));
        painter->setBrush(color_removeBackground);
        painter->drawRoundedRect(rect_removeMember, 4, 4);

        QFont btnFont = option.font;
        btnFont.setPointSize(9);
        btnFont.setBold(false);
        painter->setFont(btnFont);
        painter->setPen(color_removeText);

        painter->drawText(rect_removeMember, Qt::AlignCenter, "踢出");

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
    if(!index.data(IsFriendRole).toBool() || this->isOwner)
    {
        if(event->type() == QEvent::MouseMove)
        {
            //按钮Hover状态处理
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);

            QRect rect_addFriend;
            QRect rect_removeMember;
            getButtonsRect(contain, rect_addFriend, rect_removeMember);

            ButtonState newAddFriendState = None;
            ButtonState newRemoveMemberState = None;

            if(rect_addFriend.contains(mouse->pos()))
                newAddFriendState = this->isAddFriendPressed ? Pressed : Hover;
            else if(rect_removeMember.contains(mouse->pos()))
                newRemoveMemberState = this->isRemoveMemberPressed ? Pressed : Hover;

            bool needUpdate = false;
            if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
            {
                this->addFriendState = newAddFriendState;
                this->addFriendIndex = index;
                needUpdate = true;
            }
            else if(this->removeMemberState != newRemoveMemberState || this->removeMemberIndex != index)
            {
                this->removeMemberState = newRemoveMemberState;
                this->removeMemberIndex = index;
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
                QRect rect_removeMember;
                getButtonsRect(contain, rect_addFriend, rect_removeMember);

                ButtonState newAddFriendState = None;
                ButtonState newRemoveMemberState = None;

                if(rect_addFriend.contains(mouse->pos()))
                {
                    newAddFriendState = Pressed;
                    this->isAddFriendPressed = true;
                }
                else if(rect_removeMember.contains(mouse->pos()))
                {
                    newRemoveMemberState = Pressed;
                    this->isRemoveMemberPressed = true;
                }

                bool needUpdate = false;
                if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
                {
                    this->addFriendState = newAddFriendState;
                    this->addFriendIndex = index;
                    needUpdate = true;
                }
                else if(this->removeMemberState != newRemoveMemberState || this->removeMemberIndex != index)
                {
                    this->removeMemberState = newRemoveMemberState;
                    this->removeMemberIndex = index;
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
            QRect rect_removeMember;
            getButtonsRect(contain, rect_addFriend, rect_removeMember);

            ButtonState newAddFriendState = None;
            ButtonState newRemoveMemberState = None;

            if(rect_addFriend.contains(mouse->pos()))
            {
                if(this->isAddFriendPressed)
                {
                    emit addFriendClicked(index.data(SIDRole).toString(), index);
                }
                newAddFriendState = Hover;
            }
            else if(rect_removeMember.contains(mouse->pos()))
            {
                if(this->isRemoveMemberPressed)
                {
                    emit removeMemberClicked(index.data(UIDRole).toString(), index);
                }
                newRemoveMemberState = Hover;
            }

            this->isRemoveMemberPressed = false;

            bool needUpdate = false;
            if(this->addFriendState != newAddFriendState || this->addFriendIndex != index)
            {
                this->addFriendState = newAddFriendState;
                this->addFriendIndex = index;
                needUpdate = true;
            }
            else if(this->removeMemberState != newRemoveMemberState || this->removeMemberIndex != index)
            {
                this->removeMemberState = newRemoveMemberState;
                this->removeMemberIndex = index;
                needUpdate = true;
            }

            if(needUpdate)
                view->viewport()->update(option.rect);

            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void GroupMemberDelegate::getButtonsRect(const QRect &containRect, QRect &outAddFriendRect, QRect& outRemoveMember) const
{
    if(containRect.isNull() || containRect.isEmpty())
        return;

    int marginRight = 10;
    QSize size_btn(40, 24);
    outRemoveMember = QRect();
    if(this->isOwner)
        outRemoveMember = QRect(containRect.topRight() - QPoint(marginRight + size_btn.width(), -(containRect.height() - size_btn.height()) / 2), size_btn);
    outAddFriendRect = QRect(containRect.topRight() - QPoint(marginRight + size_btn.width() + outRemoveMember.width() + (this->isOwner ? 10 : 0), -(containRect.height() - size_btn.height()) / 2), size_btn);
}