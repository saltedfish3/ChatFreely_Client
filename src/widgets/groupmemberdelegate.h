#ifndef GROUPMEMBERDELEGATE_H
#define GROUPMEMBERDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include <QAbstractItemView>
#include "../utils/GlobalVariable.h"

class GroupMemberDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        UsernameRole = Qt::UserRole + 1,
        SIDRole,
        AvatarRole,
        UIDRole,
        IsFriendRole
    };

    explicit GroupMemberDelegate(QObject *parent = nullptr);
    void setIsOwner(bool isOwner);

signals:
    void addFriendClicked(const QString& SID, const QModelIndex& index);
    void removeMemberClicked(const QString& UID, const QModelIndex& index);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    enum ButtonState
    {
        None,
        Hover,
        Pressed
    };

    void getButtonsRect(const QRect& containRect, QRect& outAddFriendRect, QRect& outRemoveMember) const;

    QPersistentModelIndex addFriendIndex;
    ButtonState addFriendState = None;
    bool isAddFriendPressed = false;

    QPersistentModelIndex removeMemberIndex;
    ButtonState removeMemberState = None;
    bool isRemoveMemberPressed = false;

    bool isOwner = false;
};

#endif // GROUPMEMBERDELEGATE_H
