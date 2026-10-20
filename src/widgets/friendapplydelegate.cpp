#include "friendapplydelegate.h"

FriendApplyDelegate::FriendApplyDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void FriendApplyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    bool isHovered = option.state & QStyle::State_MouseOver;

    int margin = 16;//4
    //确定卡片大小和伪阴影的大小
    QRect cardRect = option.rect.adjusted(margin,margin/2,-margin,0);
    QRect shadowRect = cardRect.adjusted(-1, -1, 1, 1);

    if(isHovered)
    {
        QColor shadowColor(0, 0, 0, 15);
        painter->setPen(Qt::NoPen);
        painter->setBrush(shadowColor);
        painter->drawRoundedRect(shadowRect, 12, 12);
    }

    QColor borderColor = isHovered ? QColor(0, 0, 0, 20) : QColor(0, 0, 0, 13);
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(cardRect, 10, 10);

    //绘制数据项
    QSize avatarSize = QSize(48, 48);
    QPixmap avatar = index.data(AvatarRole).value<QPixmap>();
    if(avatar.isNull())
    {
        avatar = QPixmap(":/default/images/defaultAvatar.png");
        avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    }

    QRect pos_avatarRect = QRect(cardRect.topLeft() + QPoint(16, (cardRect.height() - avatarSize.height())/2), avatarSize);
    painter->save();
    QPainterPath path;
    path.addRoundedRect(pos_avatarRect, avatarSize.width()/2, avatarSize.height()/2);
    painter->setClipPath(path);
    painter->drawPixmap(pos_avatarRect, avatar);
    painter->restore();

    bool isGroup = index.data(IsGroupRequest).toBool();
    QFontMetrics fm = painter->fontMetrics();

    //绘制拒绝按钮
    QFont btnFont = option.font;
    btnFont.setPointSize(9);
    painter->setFont(btnFont);

    QString refuseText = "拒绝";
    int refuseTextWidth = fm.horizontalAdvance(refuseText) + 4;
    int refuseTextHeight = fm.height() + 4;
    QSize button_size(qMax(refuseTextWidth, 60), qMax(refuseTextHeight ,32));

    QRect pos_refuseRect;
    QRect pos_agreeRect;
    getButtonRect(option, pos_refuseRect, pos_agreeRect);

    if(this->refuseIndex == index)
    {
        if(this->refuseState == Hover)
        {
            painter->setPen(QPen(QColor(156, 163, 175), 1));
            painter->setBrush(QColor(243, 244, 246));
        }
        else if(this->refuseState == Pressed)
        {
            painter->setPen(QPen(QColor(107, 114, 128), 1));
            painter->setBrush(QColor(229, 231, 235));
        }
        else
        {
            painter->setPen(QPen(QColor(209, 213, 219), 1));
            painter->setBrush(Qt::NoBrush);
        }
    }
    else
    {
        painter->setPen(QPen(QColor(209, 213, 219), 1));
        painter->setBrush(Qt::NoBrush);
    }

    painter->drawRoundedRect(pos_refuseRect, 8, 8);
    //绘制拒绝按钮的拒绝
    painter->setPen(QPen(QColor(75, 85, 99), 1));
    painter->drawText(pos_refuseRect, Qt::AlignCenter, refuseText);

    //绘制同意按钮
    QString agreeText = "同意";

    if(this->agreeIndex == index)
    {
        if(this->agreeState == Hover)
        {
            painter->setBrush(QColor(79, 70, 229));
        }
        else if(this->agreeState == Pressed)
        {
            painter->setBrush(QColor(67, 56, 202));
        }
        else
        {
            painter->setBrush(QColor(99, 102, 241));
        }
    }
    else
    {
        painter->setBrush(QColor(99, 102, 241));
    }
    painter->setPen(Qt::NoPen);

    painter->drawRoundedRect(pos_agreeRect, 8, 8);
    //绘制同意按钮的同意
    painter->setPen(QPen(Qt::white, 1));
    painter->drawText(pos_agreeRect, Qt::AlignCenter, agreeText);

    //绘制用户名和sid
    QFont nameFont = option.font;
    nameFont.setPointSize(11);
    painter->setFont(nameFont);
    painter->setPen(QColor(17, 24, 39));

    int width_nameText = cardRect.width() - (pos_avatarRect.topRight() + QPoint(15, 5)).x() - (cardRect.width() - pos_agreeRect.left() - 10);
    int height_nameText = fm.height();
    int margin_verMsg = 0;

    QString name = index.data(UsernameRole).toString();
    if(!isGroup)
    {
        QString name_sid;

        name_sid += " (ID:" + index.data(SIDRole).toString() + ")";
        int width_nameSID = fm.horizontalAdvance(name_sid);

        name = fm.elidedText(name, Qt::ElideRight, width_nameText - width_nameSID);
        name += name_sid;

        QRect pos_nameRect = QRect(pos_avatarRect.topRight() + QPoint(15, 5), QSize(width_nameText, height_nameText));
        painter->drawText(pos_nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);
    }
    else
    {
        int source = index.data(Source).toInt();
        QRect rect_name = QRect(pos_avatarRect.topRight() + QPoint(15, 0), QSize(width_nameText, height_nameText));
        QRect rect_sourceTitle = QRect(rect_name.bottomLeft() + QPoint(0, 5), QSize(width_nameText, height_nameText));
        margin_verMsg = 10;

        QString groupName = index.data(GroupName).toString();
        QString sourceTitle;
        if(source == 0)
        {
            QString inviterName = index.data(InviterName).toString();
            QString base = " 邀请您加入群聊";
            int width_base = fm.horizontalAdvance(base);
            int width_available = rect_sourceTitle.width() - width_base;
            sourceTitle = fm.elidedText(inviterName, Qt::ElideMiddle, width_available) + base;
            name = fm.elidedText(name, Qt::ElideRight, rect_name.width());
        }
        else if(source == 1)
        {
            QString applicantName = index.data(UsernameRole).toString();
            QString base = "申请加入群聊 ";
            int width_base = fm.horizontalAdvance(base);
            int width_available = rect_sourceTitle.width() - width_base;

            int width_groupName = fm.horizontalAdvance(groupName);
            if(width_groupName > width_available / 2)
            {
                groupName = fm.elidedText(groupName, Qt::ElideMiddle, width_available / 2);
                width_groupName = fm.horizontalAdvance(groupName);
            }
            width_available -= width_groupName;

            sourceTitle = fm.elidedText(applicantName, Qt::ElideMiddle, width_available) + base + groupName;

            QString sid = "(ID:" + index.data(SIDRole).toString() + ")";
            int width_sid = fm.horizontalAdvance(sid);
            name = fm.elidedText(name, Qt::ElideRight, rect_name.width() - width_sid) + sid;
        }
        else
            sourceTitle = "邀请您加入群聊";

        painter->drawText(rect_name, Qt::AlignLeft | Qt::AlignVCenter, name);

        nameFont.setPointSize(8);
        painter->setFont(nameFont);
        painter->setPen(QColor(107, 114, 128));
        painter->drawText(rect_sourceTitle, Qt::AlignLeft | Qt::AlignVCenter, sourceTitle);
    }

    //绘制验证信息
    nameFont.setPointSize(8);
    painter->setFont(nameFont);
    painter->setPen(QColor(107, 114, 128));
    QString msg = "验证信息：";
    QString verMsg = index.data(VerMsgRole).toString();
    if(verMsg.isEmpty())
        msg += "加个好友认识一下!";
    else
        msg += fm.elidedText(verMsg, Qt::ElideRight, width_nameText);

    QRect pos_msgRect = QRect(pos_avatarRect.topRight() + QPoint(16, 26 + margin_verMsg), QSize(width_nameText, height_nameText));
    painter->drawText(pos_msgRect, Qt::AlignLeft | Qt::AlignVCenter, msg);

    painter->restore();
}

QSize FriendApplyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(100, 90);
}

bool FriendApplyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));
    if(!view)
        return false;
    if(event->type() == QEvent::MouseMove)
    {
        //按钮Hover状态处理
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        QRect agreeRect, refuseRect;
        getButtonRect(option, refuseRect, agreeRect);

        ButtonState newAgreeState = None;
        ButtonState newRefuseState = None;
        if(agreeRect.contains(mouse->pos()))
            newAgreeState = this->isAgreePressed ? Pressed : Hover;
        if(refuseRect.contains(mouse->pos()))
            newRefuseState = this->isRefusePressed ? Pressed : Hover;

        bool needUpdate = false;
        if(this->agreeState != newAgreeState || this->agreeIndex != index)
        {
            this->agreeState = newAgreeState;
            this->agreeIndex = index;
            needUpdate = true;
        }
        if(this->refuseState != newRefuseState || this->refuseIndex != index)
        {
            this->refuseState = newRefuseState;
            this->refuseIndex = index;
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
            QRect agreeRect, refuseRect;
            getButtonRect(option, refuseRect, agreeRect);

            ButtonState newAgreeState = None;
            ButtonState newRefuseState = None;
            if(agreeRect.contains(mouse->pos()))
            {
                newAgreeState = Pressed;
                this->isAgreePressed = true;
            }
            if(refuseRect.contains(mouse->pos()))
            {
                newRefuseState = Pressed;
                this->isRefusePressed = true;
            }

            bool needUpdate = false;
            if(this->agreeState != newAgreeState || this->agreeIndex != index)
            {
                this->agreeState = newAgreeState;
                this->agreeIndex = index;
                needUpdate = true;
            }
            if(this->refuseState != newRefuseState || this->refuseIndex != index)
            {
                this->refuseState = newRefuseState;
                this->refuseIndex = index;
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
        QRect agreeRect, refuseRect;
        getButtonRect(option, refuseRect, agreeRect);

        ButtonState newAgreeState = None;
        ButtonState newRefuseState = None;
        if(agreeRect.contains(mouse->pos()))
        {
            if(this->isAgreePressed)
            {
                if(index.data(IsGroupRequest).toBool())
                    emit agreeGroupClicked(index.data(RequestID).toString(), index);
                else
                    emit agreeFriendClicked(index.data(UIDRole).toString(), index);
            }
            newAgreeState = Hover;
        }
        if(refuseRect.contains(mouse->pos()))
        {
            if(this->isRefusePressed)
            {
                if(index.data(IsGroupRequest).toBool())
                    emit refuseGroupClicked(index.data(RequestID).toString(), index);
                else
                    emit refuseFriendClicked(index.data(UIDRole).toString(), index);
            }
            newRefuseState = Hover;
        }
        this->isAgreePressed = false;
        this->isRefusePressed = false;

        bool needUpdate = false;
        if(this->agreeState != newAgreeState || this->agreeIndex != index)
        {
            this->agreeState = newAgreeState;
            this->agreeIndex = index;
            needUpdate = true;
        }
        if(this->refuseState != newRefuseState || this->refuseIndex != index)
        {
            this->refuseState = newRefuseState;
            this->refuseIndex = index;
            needUpdate = true;
        }

        if(needUpdate)
            view->viewport()->update(option.rect);

        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void FriendApplyDelegate::getButtonRect(const QStyleOptionViewItem& option, QRect& refuseRect, QRect& agreeRect) const
{
    int margin = 16;
    QRect cardRect = option.rect.adjusted(margin,margin/2,-margin,0);

    QFont font;
    font.setPointSize(9);
    QFontMetrics fm(font);

    QString refuseText = "拒绝";
    int refuseTextWidth = fm.horizontalAdvance(refuseText) + 4;
    int refuseTextHeight = fm.height() + 4;
    QSize button_size(qMax(refuseTextWidth, 60), qMax(refuseTextHeight ,32));

    QRect pos_refuseRect = QRect(cardRect.topRight() - QPoint(16 + button_size.width(), -(cardRect.height() - button_size.height())/2), button_size);
    QRect pos_agreeRect = QRect(pos_refuseRect.topLeft() - QPoint(8 + button_size.width(), 0), button_size);

    refuseRect = pos_refuseRect;
    agreeRect = pos_agreeRect;
}
