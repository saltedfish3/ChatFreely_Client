#ifndef GROUPDELEGATE_H
#define GROUPDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include "../utils/GlobalVariable.h"

class GroupDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        ConversationIDRole,
        GroupIDRole,
        OwnerUIDRole,
        MemberCountRole,
        AvatarRole,
        GroupRole
    };

    explicit GroupDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:

signals:
    void itemClicked(const QModelIndex& index);

};

#endif // GROUPDELEGATE_H
