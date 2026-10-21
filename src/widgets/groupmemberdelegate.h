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

signals:
    void addFriendClicked(const QString& SID, const QModelIndex& index);

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

    void getButtonsRect(const QRect& containRect, QRect& outAddFriendRect) const;

    QPersistentModelIndex addFriendIndex;
    ButtonState addFriendState = None;
    bool isAddFriendPressed = false;
};

#endif // GROUPMEMBERDELEGATE_H
