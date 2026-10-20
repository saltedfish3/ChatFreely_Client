#ifndef FRIENDAPPLYDELEGATE_H
#define FRIENDAPPLYDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QEvent>
#include <QMouseEvent>
#include <QAbstractItemView>
#include "../utils/GlobalVariable.h"

class FriendApplyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum DataRole
    {
        SIDRole = Qt::UserRole + 1,
        UIDRole,
        AvatarRole,
        UsernameRole,
        VerMsgRole,
        IsGroupRequest,
        GroupName,
        Source,
        RequestID,
        InviterName
    };

    explicit FriendApplyDelegate(QObject *parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void agreeFriendClicked(const QString& UID, const QModelIndex& index);
    void refuseFriendClicked(const QString& UID, const QModelIndex& index);
    void agreeGroupClicked(const QString& requestID, const QModelIndex& index);
    void refuseGroupClicked(const QString& requestID, const QModelIndex& index);

private:
    enum ButtonState
    {
        None,
        Hover,
        Pressed
    };
    QPersistentModelIndex agreeIndex;
    ButtonState agreeState = None;
    bool isAgreePressed = false;
    QPersistentModelIndex refuseIndex;
    ButtonState refuseState = None;
    bool isRefusePressed = false;

    void getButtonRect(const QStyleOptionViewItem& option, QRect& refuseRect, QRect& agreeRect) const;

};

#endif // FRIENDAPPLYDELEGATE_H
